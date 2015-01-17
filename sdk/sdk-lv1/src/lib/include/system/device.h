/*
 * IMP Deivce common function header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <imp_sys.h>
#include <system/constraints.h>
#include <system/module.h>

typedef struct Device Device;
typedef struct Group Group;

struct Device {
	char name[MAX_NAME_LEN];
	DeviceID dev_id;
	int nr_groups;
	Group *groups[NR_MAX_GROUPS_IN_DEV];
	unsigned int		private[0];
};

static inline void *device_pri(Device *device)
{
	return (void *)device->private;
}

static inline Device *get_device_of_group(Group *group)
{
	return group->device;
}

Device *alloc_device(char *name, int extras);
void free_device(Device *device);

#endif /* __DEVICE_H__ */
