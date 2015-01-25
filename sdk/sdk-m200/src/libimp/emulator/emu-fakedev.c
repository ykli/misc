/*
 * Ingenic IMP emulation Fake Device.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#include <stdio.h>
#include <unistd.h>

#include <imp/imp_constraints.h>
#include <imp/imp_log.h>
#include <imp/imp_system.h>
#include <imp/imp_utils.h>

#include <system/system.h>
#include <system/module.h>
#include <system/group.h>
#include <system/device.h>

#define TAG "emu-FakeDev"

typedef struct {
	Device *device;
	void (*call_back[NR_MAX_GROUPS_IN_DEV])(IMPFrameInfo *);
	/* Other private data */
} FakeDev;

static FakeDev *gFakeDev[NR_FAKE_DEV] = { NULL };

FakeDev *get_fakedev(int i)
{
	if (i > (NR_FAKE_DEV - 1)) {
		IMP_LOG_ERR(TAG, "Invalid FakeDev num%d\n", i);
		return NULL;
	}

	return gFakeDev[i];
}

static int fakedev_init(FakeDev *fakedev)
{
	return 0;
}

static void fakedev_exit(FakeDev *fakedev)
{
}

static FakeDev *alloc_fakedev(int num)
{
	FakeDev *fakedev;
	char name[12];

	sprintf(name, "FakeDev%d", num);
	Device *dev = alloc_device(name, sizeof(FakeDev));

	if (dev == NULL) {
		IMP_LOG_ERR(TAG, "alloc_device() error\n");
		return NULL;
	}
	dev->nr_groups = NR_MAX_GROUPS_IN_DEV;
	dev->dev_id = DEV_ID_EMU_FAKE(num);

	fakedev = device_pri(dev);
	fakedev->device = dev;

	return fakedev;
}

static void free_fakedev(FakeDev *fakedev)
{
	Device *dev = fakedev->device;
	free_device(dev);
}

static int on_fakedev_group_data_update(Group *group, IMPFrameInfo *frame)
{
	Device *dev = get_device_of_group(group);
	FakeDev *fakedev = (FakeDev *)device_pri(dev);

	IMP_LOG_DBG(TAG, "[%s][%s] update addr=0x%08x, timestamp=%llu\n",
				fakedev->device->name, group->module->name,
				frame->virAddr, frame->timeStamp);

	if (fakedev->call_back[group->group_index])
		fakedev->call_back[group->group_index](frame);

	return 0;
}

int EmuFakedevInit(void)
{
	int ret, i;

	for (i = 0; i < NR_FAKE_DEV; i++) {
		gFakeDev[i] = alloc_fakedev(i);
		if (gFakeDev == NULL)
			return -1;
		ret = fakedev_init(gFakeDev[i]);
		if (ret < 0)
			return ret;
	}

	return ret;
}

int EmuFakedevExit(void)
{
	int i;

	for (i = 0; i < NR_FAKE_DEV; i++) {
		fakedev_exit(gFakeDev[i]);
		free_fakedev(gFakeDev[i]);
		gFakeDev[i] = NULL;
	}

	return 0;
}

int IMP_EmuFakedev_CreateGroup(int fakedev_idx, int group_index,
							   void (*call_back)(IMPFrameInfo *))
{
	FakeDev *fakedev = get_fakedev(fakedev_idx);
	if (fakedev == NULL) {
		IMP_LOG_ERR(TAG, "Invalid FakeDev num%d\n", fakedev_idx);
		return -1;
	}

	Device *dev = fakedev->device;

	if (group_index > NR_MAX_GROUPS_IN_DEV - 1) {
		IMP_LOG_ERR(TAG, "Invalid group num%d\n", group_index);
		return -1;
	}

	char grp_name[MAX_MODULE_NAME_LEN];
	sprintf(grp_name, "%s-%d", dev->name, group_index);

	Group *grp = create_group(DEV_ID_EMU_FAKE(fakedev_idx), group_index, grp_name,
							  on_fakedev_group_data_update);
	grp->device = dev;
	grp->nr_channels = NR_MAX_GROUPS_IN_DEV;
	dev->groups[group_index] = grp;

	fakedev->call_back[group_index] = call_back;

	return 0;
}

int IMP_EmuFakedev_DestroyGroup(int fakedev_idx, int group_index)
{
	FakeDev *fakedev = get_fakedev(fakedev_idx);
	if (fakedev == NULL) {
		IMP_LOG_ERR(TAG, "Invalid FakeDev num%d\n", fakedev_idx);
		return -1;
	}

	Device *dev = fakedev->device;

	if (group_index > NR_MAX_GROUPS_IN_DEV - 1) {
		IMP_LOG_ERR(TAG, "Invalid group num%d\n\n", group_index);
		return -1;
	}

	Group *grp = dev->groups[group_index];

	if (grp == NULL) {
		IMP_LOG_WARN(TAG, "group-%d has not been created\n", group_index);
		return -1;
	}

	destroy_group(grp, DEV_ID_EMU_FAKE(fakedev_idx)); /* TODO return value. */
	dev->groups[group_index] = NULL;

	fakedev->call_back[group_index] = NULL;

	return 0;
}
