/*
 * Ingenic IMP Device.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#include <stdlib.h>
#include <string.h>

#include <imp_log.h>
#include <imp_sys.h>
#include <system/constraints.h>
#include <system/module.h>
#include <system/group.h>
#include <system/device.h>

#define TAG "Device"

Device *alloc_device(char *name, int extras)
{
	Device *device;
	int i;

	device = malloc(sizeof(struct Device) + extras);
	if (device == NULL) {
		IMP_LOG_ERR(TAG, "malloc module error\n");
		return device;
	}
	memset(device, 0, sizeof(struct Device) + extras);

	i = strlen(name);
	if (i > MAX_NAME_LEN) {
		IMP_LOG_ERR(TAG, "The length of name %d is longer that %d\n", i, MAX_NAME_LEN);
		goto free;
	}
	strcpy(device->name, name);

	return device;
free:
	free(device);
	return NULL;
}

void free_device(Device *device)
{
	int i;

	if (device == NULL) {
		IMP_LOG_ERR(TAG, "device is NULL\n");
		return;
	}

	for (i = 0; i < device->nr_groups; i++) {
		Group *grp = device->groups[i];
		if (grp != NULL) {
			destroy_group(grp, device->dev_id);
			device->groups[i] = NULL;
		}
	}

	free(device);
}
