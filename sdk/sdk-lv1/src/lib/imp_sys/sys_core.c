/*
 * Ingenic IMP System Control module implement.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#include <stdio.h>
#include <imp_utils.h>
#include <imp_log.h>
#include <imp_sys.h>
#include <system/module.h>
#include <system/constraints.h>
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
struct system_function sys_funcs[] = {
	SYSTEM_FUNC("FrameSource", EmuFrameSourceInit, EmuFrameSourceExit),
	SYSTEM_FUNC("Encoder", EmuEncoderInit, EmuEncoderExit),
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

int system_bind(IMPChannel *pstSrcChn, IMPChannel *pstDestChn)
{
	Module *module_src = get_module(pstSrcChn->devID, pstSrcChn->grpID);
	Module *module_dst = get_module(pstDestChn->devID, pstDestChn->grpID);

	if (module_src == NULL) {
		IMP_LOG_ERR(TAG, "%s() error: invalid src channel\n", __func__);
		return -1;
	}

	if (module_dst == NULL) {
		IMP_LOG_ERR(TAG, "%s() error: invalid dst channel\n", __func__);
		return -1;
	}

	IMP_LOG_DBG(TAG, "%s(): bind %s(dst) to %s(src)\n", __func__, module_dst->name, module_src->name);

	Group *group = (Group *)module_pri(module_src);
	if (pstSrcChn->chnID >= group->nr_channels) {
		IMP_LOG_ERR(TAG, "%s() error: invalid dst channel\n", __func__);
		return -1;
	}

	void **data = (void **)(&group->for_channel_data[pstSrcChn->chnID]);

	BindObserverToSubject(module_src, module_dst, data);

	return 0;
}

int system_unbind(IMPChannel *pstSrcChn, IMPChannel *pstDestChn)
{
	Module *module_src = get_module(pstSrcChn->devID, pstSrcChn->grpID);
	Module *module_dst = get_module(pstDestChn->devID, pstDestChn->grpID);

	if (module_src == NULL) {
		IMP_LOG_ERR(TAG, "%s() error: invalid src channel\n", __func__);
		return -1;
	}

	if (module_dst == NULL) {
		IMP_LOG_ERR(TAG, "%s() error: invalid dst channel\n", __func__);
		return -1;
	}

	Group *group = (Group *)module_pri(module_src);
	if (pstSrcChn->chnID >= group->nr_channels) {
		IMP_LOG_ERR(TAG, "%s() error: invalid dst channel\n", __func__);
		return -1;
	}

	IMP_LOG_DBG(TAG, "%s(): unbind %s(dst) from %s(src)\n", __func__, module_dst->name, module_src->name);

	UnBindObserverFromSubject(module_src, module_dst);

	return 0;
}
