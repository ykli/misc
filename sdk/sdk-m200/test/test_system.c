/*
 * XXX
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author:
 */

#include <CUnit/CUnit.h>
#include <CUnit/Automated.h>
#include <CUnit/Basic.h>
#include <CUnit/Console.h>

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include "../src/libimp/include/system/system.h"
#include "../src/libimp/include/emulator/imp_emu_fakedev.h"

#define TAG "SystemTest"

static void test1_bind_test()
{
	int ret;

	IMP_LOG_DBG(TAG, "IMP_System_Init\n");
	ret = IMP_System_Init();
	if (ret < 0) {
		IMP_LOG_DBG(TAG, "IMP_System_Init() failed:%d\n", ret);
		CU_FAIL();
		return;
	}

	IMP_LOG_DBG(TAG, "Create groups\n");
	IMP_EmuFakedev_CreateGroup(0, 0, NULL);
	IMP_EmuFakedev_CreateGroup(1, 0, NULL);
	IMP_EmuFakedev_CreateGroup(1, 1, NULL);
	IMP_EmuFakedev_CreateGroup(2, 0, NULL);
	IMP_EmuFakedev_CreateGroup(3, 0, NULL);
	IMP_EmuFakedev_CreateGroup(4, 0, NULL);

	IMP_LOG_DBG(TAG, "Define DGCs...\n");
	IMPChannel dev0_chn0 = { DEV_ID_EMU_FAKE(0), 0, 0 };
	IMPChannel dev0_chn1 = { DEV_ID_EMU_FAKE(0), 0, 1 };
	IMPChannel dev1_grp0 = { DEV_ID_EMU_FAKE(1), 0, 0 };
	IMPChannel dev1_grp1 = { DEV_ID_EMU_FAKE(1), 1, 0 };
	IMPChannel dev2_grp0 = { DEV_ID_EMU_FAKE(2), 0, 0 };
	IMPChannel dev3_grp0 = { DEV_ID_EMU_FAKE(3), 0, 0 };
	IMPChannel dev4_grp0 = { DEV_ID_EMU_FAKE(4), 0, 0 };

	IMP_LOG_DBG(TAG, "Bind...\n");
#define BIND(A, B)														\
	ret = IMP_System_Bind(&A, &B);										\
	if (ret < 0) {														\
		IMP_LOG_ERR(TAG, "Bind src(%d,%d,%d) dst(%d,%d,%d) error\n",	\
					A.devID, A.grpID, A.chnID,							\
					B.devID, B.grpID, B.chnID);							\
		goto out;														\
	}
	BIND(dev0_chn0, dev1_grp0);
	BIND(dev0_chn1, dev1_grp1);
	BIND(dev1_grp0, dev2_grp0);
	BIND(dev1_grp1, dev3_grp0);
	BIND(dev2_grp0, dev4_grp0);
#undef BIND

	IMP_LOG_DBG(TAG, "Dump...\n");
	ret = system_bind_dump();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Dump error\n");
		CU_FAIL();
		return;
	}

#define UNBIND(A, B)													\
	ret = IMP_System_UnBind(&A, &B);										\
	if (ret < 0) {														\
		IMP_LOG_ERR(TAG, "UnBind src(%d,%d,%d) dst(%d,%d,%d) error\n",	\
					A.devID, A.grpID, A.chnID,							\
					B.devID, B.grpID, B.chnID);							\
		goto out;														\
	}
	UNBIND(dev0_chn0, dev1_grp0);
	UNBIND(dev0_chn1, dev1_grp1);
	UNBIND(dev1_grp0, dev2_grp0);
	UNBIND(dev1_grp1, dev3_grp0);
	UNBIND(dev2_grp0, dev4_grp0);
#undef UNBIND

	IMP_LOG_DBG(TAG, "Dump...\n");
	ret = system_bind_dump();
	if (ret == 0) {
		IMP_LOG_ERR(TAG, "Dump should be fail because no source now\n");
		CU_FAIL();
		return;
	}

out:
	IMP_LOG_DBG(TAG, "IMP_System_Exit\n");
	ret = IMP_System_Exit();
	if (ret < 0) {
		IMP_LOG_DBG(TAG, "IMP_System_Exit() failed:%d\n", ret);
		CU_FAIL();
		return;
	}

	CU_PASS();
}

CU_TestInfo SystemTest[] = {
	{"Bind test", test1_bind_test},
	CU_TEST_INFO_NULL,
};
