/*
 * Ingenic IMP System test .
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
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

#define SHOW(NAME, A)													\
	IMP_LOG_DBG(TAG, "%s -> (%d,%d,%d)\n", NAME,						\
				A.devID, A.grpID, A.chnID);

#define BIND(A, B)														\
	ret = IMP_System_Bind(&A, &B);										\
	if (ret < 0) {														\
		IMP_LOG_ERR(TAG, "Bind src(%d,%d,%d) dst(%d,%d,%d) error\n",	\
					A.devID, A.grpID, A.chnID,							\
					B.devID, B.grpID, B.chnID);							\
		goto out;														\
	}

#define UNBIND(A, B)													\
	ret = IMP_System_UnBind(&A, &B);									\
	if (ret < 0) {														\
		IMP_LOG_ERR(TAG, "UnBind src(%d,%d,%d) dst(%d,%d,%d) error\n",	\
					A.devID, A.grpID, A.chnID,							\
					B.devID, B.grpID, B.chnID);							\
		goto out;														\
	}

#define GETSRC(A, B)													\
	ret = IMP_System_GetBindbyDest(&A, &B);								\
	if (ret < 0) {														\
		IMP_LOG_ERR(TAG, "Get (%d,%d,%d)'s src error\n",				\
					A.devID, A.grpID, A.chnID);							\
		goto out;														\
	} else {															\
		IMP_LOG_DBG(TAG, "(%d,%d,%d) src is (%d,%d,%d)\n",				\
					A.devID, A.grpID, A.chnID,							\
					B.devID, B.grpID, B.chnID);							\
	}

#define CHECKSRC(A, B, C)												\
	if ((A.devID == B.devID) && (A.grpID == B.grpID) && (A.chnID == B.chnID)) {	\
		IMP_LOG_DBG(TAG, "(%d,%d,%d)'s src match\n",					\
					C.devID, C.grpID, C.chnID);							\
	} else {															\
		IMP_LOG_ERR(TAG, "(%d,%d,%d)'s src mismatch\n",					\
					C.devID, C.grpID, C.chnID);							\
		goto out;														\
	}

#define ATTACH(A, B)													\
	ret = system_attach(&A, &B);										\
	if (ret < 0) {														\
		IMP_LOG_ERR(TAG, "Attach src(%d,%d,%d) dst(%d,%d,%d) error\n",	\
					A.devID, A.grpID, A.chnID,							\
					B.devID, B.grpID, B.chnID);							\
		goto out;														\
	}

#define DETACH(A, B)													\
	ret = system_detach(&A, &B);										\
	if (ret < 0) {														\
		IMP_LOG_ERR(TAG, "Detach src(%d,%d,%d) dst(%d,%d,%d) error\n",	\
					A.devID, A.grpID, A.chnID,							\
					B.devID, B.grpID, B.chnID);							\
		goto out;														\
	}

static void test1_bind_test()
{
	int ret;

	IMP_LOG_DBG(TAG, "IMP_System_Init\n");
	ret = IMP_System_Init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_System_Init() failed:%d\n", ret);
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

	IMP_LOG_DBG(TAG, "Show DGC number...\n");
	SHOW("FakeDev0-0-0", dev0_chn0);
	SHOW("FakeDev0-0-0", dev0_chn1);
	SHOW("FakeDev1-0-0", dev1_grp0);
	SHOW("FakeDev1-1-0", dev1_grp1);
	SHOW("FakeDev2-0-0", dev2_grp0);
	SHOW("FakeDev3-0-0", dev3_grp0);
	SHOW("FakeDev4-0-0", dev4_grp0);

	IMP_LOG_DBG(TAG, "Bind...\n");
	BIND(dev0_chn0, dev1_grp0);
	BIND(dev0_chn1, dev1_grp1);
	BIND(dev1_grp0, dev2_grp0);
	BIND(dev1_grp1, dev3_grp0);
	BIND(dev2_grp0, dev4_grp0);

	IMP_LOG_DBG(TAG, "Dump...\n");
	ret = system_bind_dump();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Dump error\n");
		CU_FAIL();
		return;
	}

	IMP_LOG_DBG(TAG, "UnBind...\n");
	UNBIND(dev0_chn0, dev1_grp0);
	UNBIND(dev0_chn1, dev1_grp1);
	UNBIND(dev1_grp0, dev2_grp0);
	UNBIND(dev1_grp1, dev3_grp0);
	UNBIND(dev2_grp0, dev4_grp0);

	IMP_LOG_DBG(TAG, "Dump...\n");
	ret = system_bind_dump();
	if (ret == 0) {
		IMP_LOG_ERR(TAG, "Dump should be fail because no source now\n");
		CU_FAIL();
		return;
	} else {
		IMP_LOG_DBG(TAG, "This is right, no Source now after unbind\n");
	}

out:
	IMP_LOG_DBG(TAG, "IMP_System_Exit\n");
	ret = IMP_System_Exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_System_Exit() failed:%d\n", ret);
		CU_FAIL();
		return;
	}

	CU_PASS();
}

static void test2_bind_src_test()
{
	int ret;

	IMP_LOG_DBG(TAG, "IMP_System_Init\n");
	ret = IMP_System_Init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_System_Init() failed:%d\n", ret);
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

	IMP_LOG_DBG(TAG, "Show DGC number...\n");
	SHOW("FakeDev0-0-0", dev0_chn0);
	SHOW("FakeDev0-0-0", dev0_chn1);
	SHOW("FakeDev1-0-0", dev1_grp0);
	SHOW("FakeDev1-1-0", dev1_grp1);
	SHOW("FakeDev2-0-0", dev2_grp0);
	SHOW("FakeDev3-0-0", dev3_grp0);
	SHOW("FakeDev4-0-0", dev4_grp0);

	IMP_LOG_DBG(TAG, "Bind...\n");
	BIND(dev0_chn0, dev1_grp0);
	BIND(dev0_chn1, dev1_grp1);
	BIND(dev1_grp0, dev2_grp0);
	BIND(dev1_grp1, dev3_grp0);
	BIND(dev2_grp0, dev4_grp0);

	IMP_LOG_DBG(TAG, "Dump...\n");
	ret = system_bind_dump();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Dump error\n");
		CU_FAIL();
		return;
	}

	IMP_LOG_DBG(TAG, "Get bind src by dst...\n");
	IMPChannel dev0_chn0_src;
	IMPChannel dev0_chn1_src;
	IMPChannel dev1_grp0_src;
	IMPChannel dev1_grp1_src;
	IMPChannel dev2_grp0_src;
	IMPChannel dev3_grp0_src;
	IMPChannel dev4_grp0_src;

	ret = IMP_System_GetBindbyDest(&dev0_chn0, &dev0_chn0_src);
	if (ret == 0) {
		IMP_LOG_ERR(TAG, "Source(%d,%d,%d) has no src\n",
					dev0_chn0.devID, dev0_chn0.grpID, dev0_chn0.chnID);
		goto out;
	} else {
		IMP_LOG_DBG(TAG, "This is right, (%d,%d,%d) is the source, so it has no src\n",
					dev0_chn0.devID, dev0_chn0.grpID, dev0_chn0.chnID);
	}

	ret = IMP_System_GetBindbyDest(&dev0_chn1, &dev0_chn1_src);
	if (ret == 0) {
		IMP_LOG_ERR(TAG, "Source(%d,%d,%d) has no src\n",
					dev0_chn1.devID, dev0_chn1.grpID, dev0_chn1.chnID);
		goto out;
	} else {
		IMP_LOG_DBG(TAG, "This is right, (%d,%d,%d) is the source, so it has no src\n",
					dev0_chn1.devID, dev0_chn1.grpID, dev0_chn1.chnID);
	}

	GETSRC(dev1_grp0, dev1_grp0_src);
	GETSRC(dev1_grp1, dev1_grp1_src);
	GETSRC(dev2_grp0, dev2_grp0_src);
	GETSRC(dev3_grp0, dev3_grp0_src);
	GETSRC(dev4_grp0, dev4_grp0_src);

	IMP_LOG_DBG(TAG, "Check bind src...\n");
	CHECKSRC(dev1_grp0_src, dev0_chn0, dev1_grp0);
	CHECKSRC(dev1_grp1_src, dev0_chn1, dev1_grp1);
	CHECKSRC(dev2_grp0_src, dev1_grp0, dev2_grp0);
	CHECKSRC(dev3_grp0_src, dev1_grp1, dev3_grp0);
	CHECKSRC(dev4_grp0_src, dev2_grp0, dev4_grp0);

	IMP_LOG_DBG(TAG, "UnBind...\n");
	UNBIND(dev0_chn0, dev1_grp0);
	UNBIND(dev0_chn1, dev1_grp1);
	UNBIND(dev1_grp0, dev2_grp0);
	UNBIND(dev1_grp1, dev3_grp0);
	UNBIND(dev2_grp0, dev4_grp0);

	IMP_LOG_DBG(TAG, "Dump...\n");
	ret = system_bind_dump();
	if (ret == 0) {
		IMP_LOG_ERR(TAG, "Dump should be fail because no source now\n");
		CU_FAIL();
		return;
	} else {
		IMP_LOG_DBG(TAG, "This is right, no Source now after unbind\n");
	}

out:
	IMP_LOG_DBG(TAG, "IMP_System_Exit\n");
	ret = IMP_System_Exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_System_Exit() failed:%d\n", ret);
		CU_FAIL();
		return;
	}

	CU_PASS();
}

static void test3_attach_test()
{
	int ret;

	IMP_LOG_DBG(TAG, "IMP_System_Init\n");
	ret = IMP_System_Init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_System_Init() failed:%d\n", ret);
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

	IMP_LOG_DBG(TAG, "Show DGC number...\n");
	SHOW("FakeDev0-0-0", dev0_chn0);
	SHOW("FakeDev0-0-0", dev0_chn1);
	SHOW("FakeDev1-0-0", dev1_grp0);
	SHOW("FakeDev1-1-0", dev1_grp1);
	SHOW("FakeDev2-0-0", dev2_grp0);
	SHOW("FakeDev3-0-0", dev3_grp0);
	SHOW("FakeDev4-0-0", dev4_grp0);

	IMP_LOG_DBG(TAG, "Bind...\n");
	BIND(dev0_chn0, dev1_grp0);
	BIND(dev0_chn1, dev1_grp1);

	IMP_LOG_DBG(TAG, "Dump...\n");
	ret = system_bind_dump();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Dump error\n");
		CU_FAIL();
		return;
	}

	IMP_LOG_DBG(TAG, "Attach ...\n");
	ATTACH(dev2_grp0, dev1_grp0);
	ATTACH(dev3_grp0, dev1_grp1);
	ATTACH(dev4_grp0, dev2_grp0);

	IMP_LOG_DBG(TAG, "Dump...\n");
	ret = system_bind_dump();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Dump error\n");
		CU_FAIL();
		return;
	}

	IMP_LOG_DBG(TAG, "Detach ...\n");
	DETACH(dev4_grp0, dev2_grp0);
	DETACH(dev2_grp0, dev1_grp0);
	DETACH(dev3_grp0, dev1_grp1);

	IMP_LOG_DBG(TAG, "Dump...\n");
	ret = system_bind_dump();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Dump error\n");
		CU_FAIL();
		return;
	}

	IMP_LOG_DBG(TAG, "UnBind...\n");
	UNBIND(dev0_chn0, dev1_grp0);
	UNBIND(dev0_chn1, dev1_grp1);

	IMP_LOG_DBG(TAG, "Dump...\n");
	ret = system_bind_dump();
	if (ret == 0) {
		IMP_LOG_ERR(TAG, "Dump should be fail because no source now\n");
		CU_FAIL();
		return;
	} else {
		IMP_LOG_DBG(TAG, "This is right, no Source now after unbind\n");
	}

out:
	IMP_LOG_DBG(TAG, "IMP_System_Exit\n");
	ret = IMP_System_Exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_System_Exit() failed:%d\n", ret);
		CU_FAIL();
		return;
	}

	CU_PASS();
}

CU_TestInfo SystemTest[] = {
	{"Bind UnBind test", test1_bind_test},
	{"GetBindbyDest test", test2_bind_src_test},
	{"Attach test", test3_attach_test},
	CU_TEST_INFO_NULL,
};
