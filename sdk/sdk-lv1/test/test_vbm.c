/*
 * Module(StreamPipeline test).
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#include <unistd.h>

#include <CUnit/CUnit.h>
#include <CUnit/Automated.h>
#include <CUnit/Basic.h>
#include <CUnit/Console.h>

#include "../src/lib/include/system/vbm.h"

#define TAG "VBMTest"

typedef fake_fs {
  
} FakeFS;
FakeFS g_fs;

static int get_cluster(void *data)
{
  return 0;
}

static int release_cluster(void *data)
{
  return 0;
}

static int flush(void *data)
{
  return 0;
}

static int fill(void *data)
{
  return 0;
}

static void VBM_test_1()
{
  int i, ret;

  VBMConfig vbm_config;
  vbm_config.nr_pools = 2;
  vbm_config.nr_cluster = 4;
  vbm_config.pool_buffer_size[0] = 1280 * 720 * 3 / 2;
  vbm_config.pool_buffer_size[1] = 640 * 360 * 3 / 2;

  VBMInterface vbm_interface;
  vbm_interface.pri = &g_fs;
  vbm_interface.GetCluster = get_cluster;
  vbm_interface.ReleaseCluster = release_cluster;
  vbm_interface.Flush = flush;
  vbm_interface.Fill = fill;

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

static void VBM_test_2()
{
  int i, ret;

  VBMConfig vbm_config;
  vbm_config.nr_pools = 2;
  vbm_config.nr_cluster = 4;
  vbm_config.pool_buffer_size[0] = 1280 * 720 * 3 / 2;
  vbm_config.pool_buffer_size[1] = 640 * 360 * 3 / 2;

  VBMInterface vbm_interface;
  vbm_interface.pri = &g_fs;
  vbm_interface.GetCluster = get_cluster;
  vbm_interface.ReleaseCluster = release_cluster;
  vbm_interface.Flush = flush;
  vbm_interface.Fill = fill;

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
	{"VBM function flow test", VBM_test_2},
	CU_TEST_INFO_NULL,
};
