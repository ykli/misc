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

static void test1_fake_name()
{
	CU_FAIL();
}

static void test2_fake_name()
{
	CU_FAIL();
}

CU_TestInfo FrameSourceTest[] = {
	{"eg: log test case 1", test1_fake_name},
	{"eg: log test case 2", test2_fake_name},
	CU_TEST_INFO_NULL,
};
