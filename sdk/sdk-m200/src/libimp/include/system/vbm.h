/*
 * Video Buffer Manager header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#ifndef __VBM_H__
#define __VBM_H__

#include <stdint.h>
#include <errno.h>
#include <pthread.h>

typedef struct VBM VBM;
typedef struct VBMConfig VBMConfig;
typedef struct VBMCluster VBMCluster;
typedef struct VBMInterface VBMInterface;

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
	int length;
	int nr_buffers;
	IMPFrameInfo *frames[NR_MAX_POOLS];
};

struct VBMInterface {
	void *pri;
	int (*GetCluster)(int *cluster_idx, uint64_t *time, void *pri);
	int (*ReleaseCluster)(VBMCluster *cluster, void *pri);
};

struct VBM {
	VBMConfig config;
	VBMInterface interface;

	uint32_t vb_base;
	VBMCluster clusters[NR_MAX_CLUSTERS];
	IMPFrameInfo frames[0];
};

VBM *VBMGetInstance(void);
VBMCluster *frame_to_cluster(IMPFrameInfo *frame);
int VBMInit(VBMConfig *config, VBMInterface *interface);
int VBMExit(void);
int VBMGetFrames(IMPFrameInfo **frames_p, int *nr_frames_p);
int VBMReleaseFrame(IMPFrameInfo *frame);
int VBMFlushFrames(void);
int VBMFillFrames(void);

#endif /* __VBM_H__ */
