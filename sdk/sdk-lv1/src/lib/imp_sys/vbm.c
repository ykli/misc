/*
 * Ingenic IMP VBM, Video Buffer Manager.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#include <imp_sys.h>

#define TAG "VBM"

static VBM *vbm_instance = NULL;

VBM *VBMGetInstance(void)
{
  return vbm_instance;
}

VBMCluster *frame_to_cluster(FrameInfo *frame)
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
		return -1;
	}

	if (interface == NULL) {
		return -1;
	}

	vbm = VBMGetInstance();
	if (vbm != NULL)
	  return -2;

	int nr_bufs = config->nr_pools + config->nr_cluster;
	struct_size = sizeof(VBM) + nr_bufs * sizeof(FrameInfo);
	vbm = valloc(struct_size);
	if (vbm) {
		return -1;
	}
	memset(vbm, 0, struct_size);

	memcpy(&vbm->config, config, sizeof(VBMConfig));
	memcpy(&vbm->interface, interface, sizeof(VBMInterface));

	int cluster_size = 0, i;
	for (i = 0; i < config->nr_pools; i++) {
	  cluster_size += config->pool_buffer_size[i];
	}

	int vb_size_total = cluster_size * config->nr_cluster;
	vbm->vb_base = (uint32_t)valloc(vb_size_total);
	if (vbm->vb_base == 0) {
	  goto vb_base_alloc_err;
	}

	for (i = 0; i < config->nr_cluster; i++) { /* i=v4l2 buf.index */
	  uint32_t cluster_base = vbm->vb_base + cluster_size;
	  VBMCluster *cluster = &vbm->clusters[i];
	  cluster->index = i;
	  cluster->ref_cnt = 0;
	  cluster->vaddr = cluster_base;
	  cluster->paddr = 0; /* Use pmem instead of DMMU */
	  cluster->nr_buffers = config->nr_pools;
	  pthread_mutex_init(&cluster->mutex, NULL);

	  int j;
	  uint32_t tmp_vaddr = cluster->vaddr;
	  uint32_t tmp_paddr = cluster->paddr;
	  for (j = 0; j < cluster->nr_buffers; j++) { /* j=num of channel */
	    FrameInfo *frame = &vbm->frames[i * config->nr_pools + j];
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

  free(vbm->vb_base);
  free(vbm);

  vbm_instance = NULL;

  return 0;
}

int VBMGetFrames(FrameInfo **frames_p, int *nr_frames_p)
{
  VBM *vbm = VBMGetInstance();

  if (vbm == NULL)
    return -1;

  VBMInterface *inf = &vbm->Interface;
  VBMCluster *cluster = inf->GetCluster(inf->pri);

  cluster->ref_cnt = cluster->nr_buffers;
  *frames_p = cluster->frames[0];
  *nr_frames_p = cluster->nr_buffers;

  return 0;
}

int VBMReleaseFrame(FrameInfo *frame)
{
  VBM *vbm = VBMGetInstance();

  if (vbm == NULL)
    return -1;

  VBMInterface *inf = &vbm->Interface;
  VBMCluster *cluster = frame_to_cluster(frame);
  int do_release = 0;

  pthread_mutex_lock(&cluster->mutex);
  cluster->ref_cnt--; 
  if (cluster->ref_cnt == 0)
    do_release = 1;
  pthread_mutex_unlock(&cluster->mutex);

  if (do_release)
    inf->ReleaseCluster(inf->pri);

  return 0;
}

int VBMFlushFrames(void)
{
  VBM *vbm = VBMGetInstance();

  if (vbm == NULL)
    return -1;

  VBMInterface *inf = &vbm->Interface; 
  inf->Flush(inf->pri);

  return 0;
}

int VBMFillFrames(void)
{
  VBM *vbm = VBMGetInstance();

  if (vbm == NULL)
    return -1;

  VBMInterface *inf = &vbm->Interface;
  inf->Fill(inf->pri);

  return 0;
}
