/*
 * Ingenic IMP VBM, Video Buffer Manager.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <imp/imp_constraints.h>
#include <imp/imp_log.h>
#include <imp/imp_system.h>
#include <system/vbm.h>

#define TAG "VBM"

static VBM *vbm_instance = NULL;

VBM *VBMGetInstance(void)
{
	return vbm_instance;
}

VBMCluster *frame_to_cluster(IMPFrameInfo *frame)
{
	VBM *vbm = VBMGetInstance();

	if (vbm == NULL)
		return NULL;

	/* Double check */
	if (frame == vbm->clusters[frame->index].frames[frame->pool_idx])
		return &vbm->clusters[frame->index];

	return NULL;
}

int VBMInit(VBMConfig *config, VBMInterface *interface)
{
	int struct_size;
	VBM *vbm;

	if (config == NULL) {
		IMP_LOG_ERR(TAG, "%s(): config is NULL\n", __func__);
		return -1;
	}

	if (interface == NULL) {
		IMP_LOG_ERR(TAG, "%s(): interface is NULL\n", __func__);
		return -1;
	}

	vbm = VBMGetInstance();
	if (vbm != NULL)
		return -2;

	int nr_frames = config->nr_pools * config->nr_cluster;
	IMP_LOG_DBG(TAG, "%s(): total frames: %d\n", __func__, nr_frames);

	struct_size = sizeof(VBM) + nr_frames * sizeof(IMPFrameInfo);
	IMP_LOG_DBG(TAG, "%s(): VBM struct size: %d\n", __func__, struct_size);

	vbm = valloc(struct_size);
	if (vbm == NULL) {
		IMP_LOG_ERR(TAG, "%s(): valloc vbm error: %s\n",
					__func__, strerror(errno));
		return -1;
	}
	memset(vbm, 0, struct_size);

	vbm->config = *config;
	vbm->interface = *interface;

	int cluster_size = 0, i;
	for (i = 0; i < config->nr_pools; i++) {
		cluster_size += config->pool_buffer_size[i];
	}
	IMP_LOG_DBG(TAG, "%s(): VBM cluster size: %d\n", __func__, cluster_size);

	int vb_size_total = cluster_size * config->nr_cluster;
	IMP_LOG_DBG(TAG, "%s(): Total memory size: %d\n", __func__, vb_size_total);

	vbm->vb_base = (uint32_t)valloc(vb_size_total);
	if (vbm->vb_base == 0) {
		IMP_LOG_ERR(TAG, "%s(): valloc vbm_base error: %s\n",
					__func__, strerror(errno));
		goto vb_base_alloc_err;
	}

	for (i = 0; i < config->nr_cluster; i++) { /* i=v4l2 buf.index */
		uint32_t cluster_base = vbm->vb_base + cluster_size * i;
		VBMCluster *cluster = &vbm->clusters[i];
		cluster->index = i;
		cluster->ref_cnt = 0;
		cluster->vaddr = cluster_base;
		cluster->paddr = 0; /* Use pmem instead of DMMU */
		cluster->length = cluster_size;
		cluster->nr_buffers = config->nr_pools;
		pthread_mutex_init(&cluster->mutex, NULL);

		int j;
		uint32_t tmp_vaddr = cluster->vaddr;
		uint32_t tmp_paddr = cluster->paddr;
		for (j = 0; j < cluster->nr_buffers; j++) { /* j=num of channel */
			IMPFrameInfo *frame = &vbm->frames[i * config->nr_pools + j];
			frame->index = cluster->index;
			frame->pool_idx = j;

			frame->virAddr = tmp_vaddr;
			frame->phyAddr = tmp_paddr;
			frame->size = config->pool_buffer_size[j];
			cluster->frames[j] = frame;
			if (tmp_vaddr)
				tmp_vaddr += frame->size;
			if (tmp_paddr)
				tmp_paddr += frame->size;
		}
	}

	vbm_instance = vbm;
	return 0;
vb_base_alloc_err:
	free(vbm);
	return -1;
}

int VBMExit(void)
{
	VBM *vbm;

	vbm = VBMGetInstance();
	if (vbm == NULL)
		return -2;

	free((void *)vbm->vb_base);
	free(vbm);

	vbm_instance = NULL;

	return 0;
}

int VBMGetFrames(IMPFrameInfo **frames_p, int *nr_frames_p)
{
	VBM *vbm = VBMGetInstance();

	if (vbm == NULL)
		return -1;

	VBMInterface *inf = &vbm->interface;
	int cluster_idx, ret;
	uint64_t time_stamp;

	ret = inf->GetCluster(&cluster_idx, &time_stamp, inf->pri);
	if (ret < 0) {
		return ret;
	}

	VBMCluster *cluster = &vbm->clusters[cluster_idx];
	int i;

	cluster->ref_cnt = cluster->nr_buffers;
	for (i = 0; i < cluster->nr_buffers; i++) {
		frames_p[i] = cluster->frames[i];
		frames_p[i]->timeStamp = time_stamp;
	}

	*nr_frames_p = cluster->nr_buffers;

	return 0;
}

int VBMReleaseFrame(IMPFrameInfo *frame)
{
	VBM *vbm = VBMGetInstance();

	if (vbm == NULL)
		return -1;

	VBMInterface *inf = &vbm->interface;
	VBMCluster *cluster = frame_to_cluster(frame);
	int do_release = 0;

	pthread_mutex_lock(&cluster->mutex);
	cluster->ref_cnt--;
	if (cluster->ref_cnt == 0)
		do_release = 1;
	pthread_mutex_unlock(&cluster->mutex);

	if (do_release)
		inf->ReleaseCluster(cluster, inf->pri);

	return 0;
}

int VBMFlushFrames(void)
{
	VBM *vbm = VBMGetInstance();

	if (vbm == NULL)
		return -1;

	VBMInterface *inf = &vbm->interface;
	int i, clu_id;
	uint64_t timestamp;
	/* Must be checked here! */
	for (i = 0; i < vbm->config.nr_cluster; i++) {

		inf->GetCluster(&clu_id, &timestamp, inf->pri);
	}

	return 0;
}

int VBMFillFrames(void)
{
	VBM *vbm = VBMGetInstance();

	if (vbm == NULL)
		return -1;

	VBMInterface *inf = &vbm->interface;
	int i;
	for (i = 0; i < vbm->config.nr_cluster; i++) {
		VBMCluster *cluster = &vbm->clusters[i];
		inf->ReleaseCluster(cluster, inf->pri);
	}

	return 0;
}
