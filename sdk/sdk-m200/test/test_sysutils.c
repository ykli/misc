/*
 * Ingenic SDK sysutils test.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#include <CUnit/CUnit.h>
#include <CUnit/Automated.h>
#include <CUnit/Basic.h>
#include <CUnit/Console.h>

#include <imp/imp_log.h>
#include <sysutils/su_base.h>

#define TAG "SysutilsTest"

static void test1_reboot_test()
{
//	SU_Base_Reboot();
	CU_PASS();
}

static void test2_poweroff_test()
{
//	SU_Base_Shutdown();
	CU_PASS();
}

static void test3_base_test()
{
	int ret;
	SUDevID devid;

	ret = SU_Base_GetDevID(&devid);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "SU_Base_GetDevID() error\n");
		CU_FAIL();
		return;
	}

	IMP_LOG_INFO(TAG, "Device ID: %s\n", &devid.chr);

	SUTime now_save;
	ret = SU_Base_GetTime(&now_save);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "SU_Base_GetTime() error\n");
		CU_FAIL();
		return;
	}

	IMP_LOG_INFO(TAG, "Now: %d.%d.%d %d:%d:%d\n",
				 now_save.year, now_save.mon, now_save.mday,
				 now_save.hour , now_save.min, now_save.sec);

	SUTime time_to_set, now;
#define TEST_SET_GET_TIME(Y, MON, D, H, MIN, S)			\
	time_to_set.year = Y;								\
	time_to_set.mon = MON;								\
	time_to_set.mday = D;								\
	time_to_set.hour = H;								\
	time_to_set.min = MIN;								\
	time_to_set.sec = S;								\
														\
	ret = SU_Base_SetTime(&time_to_set);				\
	if (ret < 0) {										\
		IMP_LOG_ERR(TAG, "SU_Base_SetTime() error\n");	\
		CU_FAIL();										\
		return;											\
	}													\
														\
	ret = SU_Base_GetTime(&now);						\
	if (ret < 0) {										\
		IMP_LOG_ERR(TAG, "SU_Base_GetTime() error\n");	\
		CU_FAIL();										\
		return;											\
	}													\
	IMP_LOG_DBG(TAG, "After set: %d.%d.%d %d:%d:%d\n",	\
				now.year, now.mon, now.mday,			\
				now.hour , now.min, now.sec);			\
	if ((now.year = time_to_set.year)					\
		&& (now.mon == time_to_set.mon)					\
			&& (now.mday == time_to_set.mday)			\
			&& (now.hour == time_to_set.hour)			\
		&& (now.min == time_to_set.min)					\
		&& (now.sec >= time_to_set.sec)) {				\
		;												\
	} else {											\
		CU_FAIL();										\
		return;											\
	}

	TEST_SET_GET_TIME(2015, 1, 26, 21, 13, 20);
	TEST_SET_GET_TIME(2010, 12, 14, 9, 1, 2);

	ret = SU_Base_SetTime(&now_save);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "SU_Base_SetTime() error\n");
		CU_FAIL();
		return;
	}

	ret = SU_Base_GetTime(&now);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "SU_Base_SetTime() error\n");
		CU_FAIL();
		return;
	}

	IMP_LOG_INFO(TAG, "Restore: %d.%d.%d %d:%d:%d\n",
				 now.year, now.mon, now.mday,
				 now.hour , now.min, now.sec);

	CU_PASS();
}

static void test4_battery_test()
{
	CU_FAIL();
}

CU_TestInfo SysutilsTest[] = {
	{"Poweroff test", test1_reboot_test},
	{"Reboot test", test2_poweroff_test},
	{"Base test", test3_base_test},
	{"Battery test", test4_battery_test},
	CU_TEST_INFO_NULL,
};
