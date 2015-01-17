/*
 * Ingenic IMP group.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#include <imp_log.h>
#include <imp_sys.h>
#include <system/module.h>
#include <system/constraints.h>
#include <system/group.h>

#define TAG "Group"

static Module *g_modules[NR_MAX_DEVICES][NR_MAX_GROUPS_IN_DEV];

inline Module *get_module(DeviceID dev_id, int grp_id)
{
	return g_modules[dev_id][grp_id];
}

void clear_all_modules(void)
{
	int i, j;
	for (i = 0; i < NR_MAX_DEVICES; i++) {
		for (j = 0; j < NR_MAX_GROUPS_IN_DEV; j++)
			g_modules[i][j] = NULL;
	}
}

static int is_last_node(Group *group)
{
}

static int group_update(void *m, void *data)
{
	Group *group = (Group *)m;
	FrameInfo *frame = (FrameInfo *)data;

	group->OnGroupDataUpdate(group, frame);

	if (is_last_node(group))
		VbmReleaseFrame(frame);

	return 0;
}

Group *create_group(DeviceID dev_id, int grp_id,
					const char *mod_name,
					int (*update_cb)(Group *, FrameInfo *))
{
	Group *group;

	AllocModuleHelper(group, mod_name, Group);
	SetUpdateCallback(group->module, group_update);

	group->OnGroupDataUpdate = update_cb;
	group->group_index = grp_id;

	g_modules[dev_id][grp_id] = group->module;

	return group;
}

int destroy_group(Group *group, DeviceID dev_id)
{
	g_modules[dev_id][group->group_index] = NULL;
	FreeModule(group->module);

	return 0;
}
