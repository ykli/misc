/*
 * Video Buffer Manager header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#ifndef __VBM_H__
#define __VBM_H__

#include <stdint.h>
#include <pthread.h>

typedef struct VBM VBM;
typedef struct VBMConfig VBMConfig;
typedef struct VBMCluster VBMCluster;
typedef struct VBMInterface VBMInterface;

struct VBM {
	VBMConfig config;
	VBMInterface interface;

	uint32_t vb_base;
	VBMCluster clusters[NR_MAX_CLUSTERS];
	FrameInfo frames[0];
};

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

struct VBMInterface {
	void *pri;
	int (*GetCluster)(void *pri);
	int (*ReleaseCluster)(void *pri);
	int (*Flush)(void *pri);
	int (*Fill)(void *pri);	
};

VBM *VBMGetInstance(void);
VBMCluster *frame_to_cluster(FrameInfo *frame);
int VBMInit(VBMConfig *config, VBMInterface *interface);
int VBMExit(void);
int VBMGetFrames(FrameInfo **frames_p, int *nr_frames_p);
int VBMReleaseFrame(FrameInfo *frame);
int VBMFlushFrames(void);
int VBMFillFrames(void);

#endif /* __VBM_H__ */
