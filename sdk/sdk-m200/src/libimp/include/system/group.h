/*
 * IMP Group common function header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#ifndef __GROUP_H__
#define __GROUP_H__

#include <imp/imp_constraints.h>
#include <imp/imp_system.h>

#include <system/module.h>

typedef struct Device Device;
typedef struct Group Group;

struct Group {
	Device *device;
	Module *module;
	int group_index;
	int nr_channels;
	IMPFrameInfo *for_channel_data[NR_MAX_CHN_IN_GROUP];
	int update_cb;

	int (*OnGroupDataUpdate)(Group *modulex, IMPFrameInfo *data);
};

static inline void group_tick(Group *group)
{
	static int i;
	static void *tmp = &i;
	Module *module = group->module;

	module->Update(module, &tmp);
}

Module *get_module(IMPDeviceID dev_id, int grp_id);
int get_module_location(Module *module, IMPDeviceID *dev_id, int *grp_id);
void clear_all_modules(void);

Group *create_group(IMPDeviceID mod_id, int dev_id,
					const char *mod_name,
					int (*update_cb)(Group *, IMPFrameInfo *));

int destroy_group(Group *group, IMPDeviceID dev_id);

#endif /* __GROUP_H__ */
