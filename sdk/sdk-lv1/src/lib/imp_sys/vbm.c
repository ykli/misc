/*
 * Ingenic IMP VBM, Video Buffer Manager.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#include <imp_sys.h>

#define TAG "VBM"

typedef struct {
	void *pri;

	int (*Init)(VBM_CONF *vbm_config);
	int (*Exit)(void);
	int (*GetFrame)(FrameInfo *);
	int (*ReleaseFrame)(FrameInfo *);
	int (*FlushFrames)(void);
	int (*FillFrames)(void);
} VB_INF;		/* Interface */

type2def struct {
	int nr_buffers;
	FrameInfo buffers[0];
} VB_POOL;

typedef {
	uint32_t vb_base;
	int nr_vb_pools;
	VB_POOL *vb_pools[NR_MAX_VB_IN_POOLS];
} VB;

typedef struct {
	int BufCnt;
	int BufSize;
} VB_POOL_CONF;

typedef struct {
	int nr_vb_pools;
	VB_POOL_CONF vb_pools_conf[NR_MAX_VB_IN_POOLS];
} VBM_CONF;

static VB g_vb;
static VB *g_vb_p = NULL;

VB *get_vb_instance(void)
{
	return g_vb_p;
}

VB_INF *get_vb_interface(void)
{
	static VB_INF g_vb_interface;
	return &g_vb_interface;
}

static int vbm_init(VBM_CONF *vbm_config)
{
	int i;
	VB *gvb = get_vb_instance();

	if (vbm_config->nr_vb_pools > NR_MAX_VB_IN_POOLS)
		return -1;

	int buf_size_1 = 0;
	int buf_cnt_s = vbm_config->vb_pools_conf[0].BufCnt;
	for (i = 0; i < vbm_config->nr_vb_pools) {
		int cnt = vbm_config->vb_pools_conf[i].BufCnt;
		if (cnt != buf_cnt_s) {
			IMP_LOG_ERR(TAG ,"Buffer count must be the same\n");
			return -1;
		}
		buf_size_1 += vbm_config->vb_pools_conf[i].BufSize;
	}

	int buf_size_total = buf_size_1 * buf_cnt_s;
	void *vb_mem = valloc(buf_size_total);
	if (vb_mem == NULL) {
		IMP_LOG_ERR(TAG ,"VBM memory alloc failed\n");
		return -1;
	}

	gvb->vb_base = (uint32_t)vb_mem;

	/* Init pools one by one */
	for (i = 0; i < vbm_config->nr_vb_pools) {
		VB_POOL_CONF *pool_conf = &vbm_config->vb_pools_conf[i];
		int buf_size = pool_conf->BufSize;
		gvb->vb_pools[i] = malloc(sizeof(VB_POOL) +
								  pool_conf->BufCnt * sizeof(FrameInfo));

		VB_POOL *vb_pool = gvb->vb_pools[i];
		vb_pool->nr_buffers = pool_conf->BufCnt;

		/* Init buffers in the pool */
		int j;
		for (j = 0; j < vb_pool->nr_buffers; j++) {
			FrameInfo *buffer = &vb_pool->buffers[j];
			buffer->index = j;
			buffer->size = buf_size;
			buffer->phyAddr = 0;
			buffer->virAddr = gvb->vb_base + j * (buf_size_1);
		}
	}
}

static int vbm_exit(VBM_CONF *vbm_config)
{
	VB *gvb = get_vb_instance();
	int i;

	if (gvb != NULL) {
		for (i = 0; i < gvb->nr_vb_pools; i++) {
			if (gvb->vb_pools[i])
				free(gvb->vb_pools[i]);
		}
	}

	free(gvb->vb_base);
	g_vb_p = NULL;
}

FrameInfo *VbmGetFrames(int nr_frames, FrameInfo **frame_p)
{
	VB_INF *vb_inf = get_vb_interface();
	if (vb_inf->GetFrame)
		FrameInfo *frame_e = vb_inf->GetFrame(vb_inf->pri);
	frame_p[0] = frame_e->virAddr;
	frame_p[1] = ;
	/* Get a whole frame */
}

typedef struct
{
	int ref_cnt;
	int nr_frames;

	

	FrameInfo *frames[0];
} FrameCluster;

static FrameCluster *single_frame_to_cluster(FrameInfo *)
{
}

int VbmReleaseFrame(FrameInfo *frame)
{
	FrameCluster *frmc = single_frame_to_cluster(frame);
	int release_cluster = 0;
//lock
	frmc->ref_cnt--;
	if (frmc->ref_cnt == 0) {
		release_cluster = 1;
	}
//unock
	if (release_cluster) {
		VB_INF *vb_inf = get_vb_interface();
		if (vb_inf->ReleaseFrame)
			vb_inf->ReleaseFrame(vb_inf->pri, frmc);
	}
}

int VbmFlushFrames(void)
{
	VB_INF *vb_inf = get_vb_interface();
	if (vb_inf->ReleaseFrame)
		vb_inf->FlushFrames(vb_inf->pri);
}

int VbmFillFrames(void)
{
	VB_INF *vb_inf = get_vb_interface();
	if (vb_inf->FlushFrames)
		vb_inf->FlushFrames(vb_inf->pri);
}

int register_vbm_interface(VB_INF *inf, void *data)
{
	VB_INF *vb_inf = get_vb_interface();

	if (inf->GetFrame)
		vb_inf->GetFrame = inf->GetFrame;

	if (inf->ReleaseFrame)
		vb_inf->ReleaseFrame = inf->ReleaseFrame;

	if (inf->FlushFrames)
		vb_inf->FlushFrames = inf->FlushFrames;

	if (inf->FillFrames)
		vb_inf->FillFrames = inf->FillFrames;

	vb_inf->pri = data;

	return 0;
}

int VbmInit(void)
{
	g_vb_p = &g_vb;
	VB_INF *vb_inf = get_vb_interface();

	vb_inf->Init = vbm_init;
	vb_inf->Exit = vbm_exit;
	vb_inf->GetFrame = NULL;
	vb_inf->ReleaseFrame = NULL;
	vb_inf->FlushFrames = NULL;
	vb_inf->FillFrames = NULL;

	memset(get_vb_instance(), 0 ,sizeof(VB));

	return 0;
}

int VbmExit(void)
{
}





typedef struct VBM VBM;
typedef struct VBMConfig VBMConfig;
typedef struct VBMCluster VBMCluster;
typedef struct VBMInterface VBMInterface;

struct VBM {
	VBMConfig config;
	VBMInterface interface;

	uint32_t vb_base;
	VBMCluster *clusters[NR_MAX_CLUSTERS];
	FrameInfo frames[0];
};
static VBM *vbm_instance = NULL;

struct VBMConfig {
	int nr_pools;
	int nr_cluster;
	int pool_buffer_size[NR_MAX_POOLS];
};

struct VBMCluster {
	int index;
	int ref_cnt;

	pthread_mutex_t mutex;

	uint32_t vaddr;
	uint32_t paddr;
	int nr_buffers;
	FrameInfo *frames[NR_MAX_POOLS];
};
VBMCluster *frame_to_cluster(FrameInfo *);

struct VBMInterface {
	void *pri;
	int (*GetCluster)(void *pri);
	int (*ReleaseCluster)(void *pri);
	int (*Flush)(void *pri);
	int (*Fill)(void *pri);	
};

int VBMCreate(VBMConfig *config, VBMInterface *interface)
{
	int struct_size;
	VBM *vbm;

	if (config == NULL) {
		return -1;
	}

	if (interface == NULL) {
		return -1;
	}

	struct_size = sizeof(VBM) +
		(config->nr_pools + config->nr_cluster) * sizeof(FrameInfo);
	vbm = valloc(struct_size);
	if (vbm) {
		return -1;
	}

	memcpy(&vbm->config, config, sizeof(VBMConfig));
	memcpy(&vbm->interface, interface, sizeof(VBMInterface));





	vbm_instance = vbm;
}

int VBMDestroy(VBM *);
int VBMGetFrames(FrameInfo **, int *);
int VBMReleaseFrames(FrameInfo *);

VBM *VBMGetInstance(void)
{
	return vbm_instance;
}
