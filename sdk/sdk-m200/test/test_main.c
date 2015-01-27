/*
 * Ingenic IMP SDK test main file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#include <CUnit/CUnit.h>
#include <CUnit/Automated.h>
#include <CUnit/Basic.h>
#include <CUnit/Console.h>
#include <stdio.h>

extern CU_TestInfo LogTest[];
extern CU_TestInfo ModuleTest[];
extern CU_TestInfo VBMTest[];
extern CU_TestInfo EmulatorTest[];
extern CU_TestInfo SystemTest[];
extern CU_TestInfo FrameSourceTest[];
extern CU_TestInfo EncoderTest[];
extern CU_TestInfo SysutilsTest[];
extern CU_TestInfo EntireTest[];
extern CU_TestInfo ErrorTest[];

int InitSuiteDefault()
{
    return 0;
}

int EndSuiteDefault()
{
    return 0;
}

int main()
{
	CU_ErrorCode err;

	err = CU_initialize_registry();
	if(err != CUE_SUCCESS) {
		return CU_get_error();
	}

	CU_SuiteInfo suites[] = {
		/* Basic component test */
		{"Log Test", InitSuiteDefault, EndSuiteDefault, NULL, NULL, LogTest},
		{"Module Test", InitSuiteDefault, EndSuiteDefault, NULL, NULL, ModuleTest},
		{"VBM Test", InitSuiteDefault, EndSuiteDefault, NULL, NULL, VBMTest},

		/* Emulator test */
		{"Emulator Test", InitSuiteDefault, EndSuiteDefault, NULL, NULL, EmulatorTest},

		/* Actual modules test */
		{"System Test", InitSuiteDefault, EndSuiteDefault, NULL, NULL, SystemTest},
		{"FrameSource Test", InitSuiteDefault, EndSuiteDefault, NULL, NULL, FrameSourceTest},
		{"Encoder Test", InitSuiteDefault, EndSuiteDefault, NULL, NULL, EncoderTest},

		/* sysutils test */
		{"Sysutils Test", InitSuiteDefault, EndSuiteDefault, NULL, NULL, SysutilsTest},

		/* Integrated test */
		{"Entire Test", InitSuiteDefault, EndSuiteDefault, NULL, NULL, EntireTest},
		{"Error Test", InitSuiteDefault, EndSuiteDefault, NULL, NULL, ErrorTest},

		CU_SUITE_INFO_NULL,
	};

	err = CU_register_suites(suites);

	if(err != CUE_SUCCESS) {
		CU_cleanup_registry();
		return CU_get_error();
	}

#ifdef RUN_DIRECTLY
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
#else
	CU_console_run_tests();
#endif

	CU_cleanup_registry();

	return 0;
}
