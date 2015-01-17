/*
 * IMP Capacity constraints definition header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#ifndef __CONSTRAINTS_H__
#define __CONSTRAINTS_H__

/* Common */
#define MAX_NAME_LEN				16

/* System-DGM */
#define NR_MAX_GROUPS_IN_DEV		4
#define NR_MAX_CHN_IN_GROUP			3

/* System-VBM */
#define NR_MAX_POOLS				2
#define NR_MAX_CLUSTERS				8


/* FrameSource */
#define NR_MAX_FS_GROUPS			1
#define NR_MAX_FS_CHN_IN_GROUP		NR_MAX_POOLS
#define NR_MAX_V4L2_BUFFERS			NR_MAX_CLUSTERS

/* Encoder */
#define NR_MAX_ENC_GROUPS			4
#define NR_MAX_ENC_CHN_IN_GROUP		2

#endif /* __CONSTRAINTS_H__ */
