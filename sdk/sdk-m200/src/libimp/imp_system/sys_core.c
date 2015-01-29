/*
 * Ingenic IMP System Control module implement.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#include <stdio.h>
#include <string.h>

#include <imp/imp_constraints.h>
#include <imp/imp_utils.h>
#include <imp/imp_log.h>
#include <imp/imp_system.h>

#include <system/module.h>
#include <system/group.h>

#define TAG "System"

extern Module *g_modules[][NR_MAX_GROUPS_IN_DEV];

struct system_function
{
	char name[MAX_NAME_LEN];
	int (*init_func)(void);
	int (*exit_func)(void);
};

#define SYSTEM_FUNC(NAME, INIT_FUNC, EXIT_FUNC)							\
	{ .name = NAME, .init_func = INIT_FUNC, .exit_func = EXIT_FUNC }
#define EXTERN_SYS_FUNC(F1, F2)		extern int F1(void); extern int F2(void)

EXTERN_SYS_FUNC(EmuFrameSourceInit, EmuFrameSourceExit);
EXTERN_SYS_FUNC(EmuEncoderInit, EmuEncoderExit);
EXTERN_SYS_FUNC(EmuFakedevInit, EmuFakedevExit);
EXTERN_SYS_FUNC(EncoderInit, EncoderExit);
struct system_function sys_funcs[] = {
	SYSTEM_FUNC("EmuFrameSource", EmuFrameSourceInit, EmuFrameSourceExit),
	SYSTEM_FUNC("EmuEncoder", EmuEncoderInit, EmuEncoderExit),
	SYSTEM_FUNC("EmuFakeDev", EmuFakedevInit, EmuFakedevExit),
	SYSTEM_FUNC("Encoder", EncoderInit, EncoderExit),
};

int system_init(void)
{
	int i, ret = 0;

	IMP_LOG_DBG(TAG, "%s\n", __func__);

	for (i = 0; i < ARRAY_SIZE(sys_funcs); i++) {
		IMP_LOG_DBG(TAG, "Calling %s\n", sys_funcs[i].name);
		ret = sys_funcs[i].init_func();
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "%s failed\n", sys_funcs[i].name);
			break;
		}
	}

	if (ret < 0) {
		int j;
		for (j = i - 1; j >=0; j--)
			sys_funcs[i].exit_func();
	}

	return ret;
}

int system_exit(void)
{
	int i;

	IMP_LOG_DBG(TAG, "%s\n", __func__);

	for (i = 0; i < ARRAY_SIZE(sys_funcs); i++) {
		IMP_LOG_DBG(TAG, "Calling %s\n", sys_funcs[i].name);
		sys_funcs[i].exit_func();
	}

	return 0;
}

int system_bind(IMPChannel *srcChn, IMPChannel *dstChn)
{
	Module *module_src = get_module(srcChn->devID, srcChn->grpID);
	Module *module_dst = get_module(dstChn->devID, dstChn->grpID);

	if (module_src == NULL) {
		IMP_LOG_ERR(TAG, "%s() error: invalid src channel\n", __func__);
		return -1;
	}

	if (module_dst == NULL) {
		IMP_LOG_ERR(TAG, "%s() error: invalid dst channel\n", __func__);
		return -1;
	}

	IMP_LOG_DBG(TAG, "%s(): bind %s(dst) to %s(src)\n",
				__func__, module_dst->name, module_src->name);

	Group *group = (Group *)module_pri(module_src);
	if (srcChn->chnID >= group->nr_channels) {
		IMP_LOG_ERR(TAG, "%s() error: invalid dst channel\n", __func__);
		return -1;
	}

	void **data = (void **)(&group->for_channel_data[srcChn->chnID]);

	BindObserverToSubject(module_src, module_dst, data);

	return 0;
}

int system_unbind(IMPChannel *srcChn, IMPChannel *dstChn)
{
	Module *module_src = get_module(srcChn->devID, srcChn->grpID);
	Module *module_dst = get_module(dstChn->devID, dstChn->grpID);

	if (module_src == NULL) {
		IMP_LOG_ERR(TAG, "%s() error: invalid src channel\n", __func__);
		return -1;
	}

	if (module_dst == NULL) {
		IMP_LOG_ERR(TAG, "%s() error: invalid dst channel\n", __func__);
		return -1;
	}

	IMP_LOG_DBG(TAG, "%s(): unbind %s(dst) from %s(src)\n",
				__func__, module_dst->name, module_src->name);

	UnBindObserverFromSubject(module_src, module_dst);

	return 0;
}

int system_get_bind_src(IMPChannel *dstChn, IMPChannel *srcChn)
{
	int ret;
	Module *module_dst = get_module(dstChn->devID, dstChn->grpID);
	if (module_dst == NULL) {
		IMP_LOG_ERR(TAG, "%s() error: invalid dst channel\n", __func__);
		return -1;
	}

	Module *module_src = module_subject(module_dst);
	if (module_src == NULL) {
		IMP_LOG_ERR(TAG, "%s() error: dst channel(%s) has not been binded\n",
					__func__, module_dst->name);
		return -1;
	}

	srcChn->chnID = module_ob_id(module_src, module_dst);

	ret = get_module_location(module_src, &srcChn->devID, &srcChn->grpID);

	return ret;
}

int system_attach(IMPChannel *fromChn, IMPChannel *toChn)
{
	int ret;
	IMPChannel srcChn;

	ret = system_get_bind_src(toChn, &srcChn);
	if (ret < 0)
		goto err1;

	ret = system_unbind(&srcChn, toChn);
	if (ret < 0)
		goto err1;

	ret = system_bind(&srcChn, fromChn);
	if (ret < 0)
		goto err2;

	ret = system_bind(fromChn, toChn);
	if (ret < 0)
		goto err3;

	return ret;
err3:
	system_unbind(&srcChn, fromChn);
err2:
	system_bind(&srcChn, toChn);
err1:
	IMP_LOG_ERR(TAG, "%s() error: attach %s error\n", __func__,
				get_module(toChn->devID, toChn->grpID)->name);
	return ret;
}

int system_detach(IMPChannel *fromChn, IMPChannel *toChn)
{
	int ret;
	IMPChannel srcChn;

	ret = system_get_bind_src(fromChn, &srcChn);
	if (ret < 0)
		goto err1;

	ret = system_unbind(&srcChn, fromChn);
	if (ret < 0)
		goto err1;

	ret = system_unbind(fromChn, toChn);
	if (ret < 0)
		goto err2;

	ret = system_bind(&srcChn, toChn);
	if (ret < 0)
		goto err3;

	return ret;
err3:
	system_bind(fromChn, toChn);
err2:
	system_bind(&srcChn, fromChn);
err1:
	IMP_LOG_ERR(TAG, "%s() error: detach %s error\n", __func__,
				get_module(toChn->devID, toChn->grpID)->name);
	return ret;
}

static void dump_ob_modules(Module *sub, int level)
{
	int i;
	char tabs[64];

	memset(tabs, 0, ARRAY_SIZE(tabs));

	for (i = 0; i < level * 4; i++) {
		tabs[i] = ' ';i++;
		tabs[i] = ' ';i++;
		tabs[i] = ' ';i++;
		tabs[i] = ' ';
	}
	tabs[i] = '\0';

	for (i = 0; i < nr_module_observer(sub); i++) {
		Module *ob = module_observer_by_num(sub, i);
		IMP_LOG_INFO(TAG, "%s%s-%d->%s\n", tabs, sub->name, i, ob->name);
		dump_ob_modules(ob, level + 1);
	}
}

int system_bind_dump(void)
{
	int i, j;
	Module *source = NULL;

	/* Find who is the source*/
	for (i = 0; i < NR_MAX_DEVICES; i++) {
		for (j = 0; j < NR_MAX_GROUPS_IN_DEV; j++) {
			Module *module = get_module(i, j);
			if (module) {
				IMP_LOG_INFO(TAG, "enumerate: %s(%d, %d)\n", module->name, i, j);
				if ((nr_module_observer(module) > 0)
					&& (module_subject(module) == NULL)) {
					source = module;
					goto out;
				}
			}
		}
	}

	if (source == NULL) {
		IMP_LOG_ERR(TAG, "%s() error: No source found\n", __func__);
		return -1;
	}
out:
	IMP_LOG_INFO(TAG, "Source:%s(%d, %d)\n", source->name, i, j);
	dump_ob_modules(source, 0);

	return 0;
}
