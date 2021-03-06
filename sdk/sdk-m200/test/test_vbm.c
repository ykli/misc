/*
 * VBM(Video Buffer Manager) test.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#include <unistd.h>

#include <CUnit/CUnit.h>
#include <CUnit/Automated.h>
#include <CUnit/Basic.h>
#include <CUnit/Console.h>

#include <imp/imp_constraints.h>
#include <imp/imp_log.h>
#include <imp/imp_system.h>
#include "../src/libimp/include/system/vbm.h"

#define TAG "VBMTest"

typedef struct {
	int aa;
} FakeFS;
FakeFS g_fs;

static int get_cluster(int *cluster_idx, uint64_t *time, void *pri)
{
	return 0;
}

static int release_cluster(VBMCluster *cluster, void *pri)
{
	return 0;
}

static void VBM_test_1()
{
	int ret;

	VBMConfig vbm_config;
	vbm_config.nr_pools = 2;
	vbm_config.nr_cluster = 4;
	vbm_config.pool_buffer_size[0] = 1280 * 720 * 3 / 2;
	vbm_config.pool_buffer_size[1] = 640 * 360 * 3 / 2;

	VBMInterface vbm_interface;
	vbm_interface.pri = &g_fs;
	vbm_interface.GetCluster = get_cluster;
	vbm_interface.ReleaseCluster = release_cluster;

	ret = VBMInit(&vbm_config, &vbm_interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "VBMInit() failed, ret=%d", ret);
		CU_FAIL();
		return;
	}

	ret = VBMExit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "VBMExit() failed, ret=%d", ret);
		CU_FAIL();
		return;
	}

	CU_PASS();
}

CU_TestInfo VBMTest[] = {
	{"VBM Init and Exit test", VBM_test_1},
	CU_TEST_INFO_NULL,
};
