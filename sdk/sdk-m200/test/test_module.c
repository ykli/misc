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

#include "../src/libimp/include/system/module.h"

typedef struct AAAModule {
	Module *module;
	int features1;
	int features2;
	void *for_bbb_data;
	void *for_ccc_data;
} AAAModule;

typedef struct BBBModule {
	Module *module;
	int features3;
	void *for_ddd_data;
} BBBModule;

typedef struct CCCModule {
	Module *module;
	int features;
} CCCModule;

typedef struct DDDModule {
	Module *module;
	int features;
} DDDModule;

static int aaa_update(void *m, void *data)
{
	AAAModule *aaa = (AAAModule *)m;
	printf("%s %s data=%p:%d\n", aaa->module->name, __func__, data, data ? *((int *)data) : -1);
//	usleep(400000);
	return 0;
}

static int bbb_update(void *m, void *data)
{
	BBBModule *bbb = (BBBModule *)m;
	printf("%s %s data=%p:%d\n", bbb->module->name, __func__, data, data ? *((int *)data) : -1);
	usleep(400000);
	printf("-%s:%d\n", __func__, __LINE__);
	return 0;
}

static int ccc_update(void *m, void *data)
{
	CCCModule *ccc = (CCCModule *)m;
	printf("%s %s data=%p:%d\n", ccc->module->name, __func__, data, data ? *((int *)data) : -1);
	usleep(400000);
	printf("-%s:%d\n", __func__, __LINE__);
	return 0;
}

static int ddd_update(void *m, void *data)
{
	DDDModule *ddd = (DDDModule *)m;
	printf("%s %s data=%p:%d\n", ddd->module->name, __func__, data, data ? *((int *)data) : -1);
	usleep(400000);
	printf("-%s:%d\n", __func__, __LINE__);
	return 0;
}

static void normal_flow_test()
{
	int data1 = 123;
	int data2 = 234;
	int data3 = 345;
	int data4 = 456;

	AAAModule *aaa;
	AllocModuleHelper(aaa, "AAA", struct AAAModule);
	aaa->for_bbb_data = &data2;
	aaa->for_ccc_data = &data3;
	BBBModule *bbb;
	AllocModuleHelper(bbb, "BBB", struct BBBModule);
	bbb->for_ddd_data = &data4;
	CCCModule *ccc;
	AllocModuleHelper(ccc, "CCC", struct CCCModule);
	DDDModule *ddd;
	AllocModuleHelper(ddd, "DDD", struct DDDModule);

	BindObserverToSubject(aaa->module, bbb->module, &aaa->for_bbb_data);
	BindObserverToSubject(aaa->module, ccc->module, &aaa->for_ccc_data);
	BindObserverToSubject(bbb->module, ddd->module, &bbb->for_ddd_data);

	SetUpdateCallback(aaa->module, aaa_update);
	SetUpdateCallback(bbb->module, bbb_update);
	SetUpdateCallback(ccc->module, ccc_update);
	SetUpdateCallback(ddd->module, ddd_update);

	void *for_aaa_data_p = &data1;
//	void *for_aaa_data_p = NULL;
	int run_cnt = 5;
	while (run_cnt--) {
		aaa->module->Update(aaa->module, (void**)(&for_aaa_data_p));
		usleep(500000);
		data1++;
		data2++;
		data3++;
		data4++;
	}

	CU_FAIL();
}

CU_TestInfo ModuleTest[] = {
	{"Normal flow test", normal_flow_test},
	CU_TEST_INFO_NULL,
};
