/*
 * Ingenic IMP SDK test framesource file
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Tiger <bohu.liang@ingenic.com>
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#include <CUnit/CUnit.h>
#include <CUnit/Automated.h>
#include <CUnit/Basic.h>
#include <CUnit/Console.h>

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>

#include "../src/libimp/include/system/system.h"
#include "../src/libimp/include/emulator/imp_emu_fakedev.h"

#define TAG "FramesourceTest"

#define FRAME_RATE_IN		30
//#define FRAME_RATE_IN		25

#define FRAME_FORMAT_1		PIX_FMT_NV12
//#define FRAME_FORMAT_1		PIX_FMT_YUYV422
#define FRAME_FORMAT_2		PIX_FMT_YUYV422

#define FRAME_W_1		1280
#define FRAME_H_1		720

#define FRAME_W_2		640
#define FRAME_H_2		480

#define FRAME_RATE_OUT_1	30
#define FRAME_RATE_OUT_2	30

struct frame_rate_s {
	int time;
	int frame_count;
	int frame_count_error;
};

struct frame_rate_s frame_rate_s_0;
struct frame_rate_s frame_rate_s_1;

char *get_filename(char *path, char *format)
{
	char *buf;
	struct timeval tv;

	gettimeofday(&tv, NULL);

	buf = malloc(64);
	if(buf == NULL) {
		printf("malloc() error !\n");
		return NULL;
	}

	memset(buf, 0, 64);

	snprintf(buf, 64, "%s/%d-%d.%s",			\
				path,				\
				(int)(tv.tv_sec % 100),		\
				(int)(tv.tv_usec/1000),		\
				format);
	return buf;
}

void call_back_frame_rate0(IMPFrameInfo *p)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	if(frame_rate_s_0.time == tv.tv_sec) {
		frame_rate_s_0.frame_count++;
	} else {
		if(frame_rate_s_0.frame_count != FRAME_RATE_OUT_1)
			frame_rate_s_0.frame_count_error++;
		frame_rate_s_0.time = tv.tv_sec;
		frame_rate_s_0.frame_count = 1;
	}

	return ;
}
void call_back_frame_rate1(IMPFrameInfo *p)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	if(frame_rate_s_1.time == tv.tv_sec) {
		frame_rate_s_1.frame_count++;
	} else {
		if(frame_rate_s_1.frame_count != FRAME_RATE_OUT_2)
			frame_rate_s_1.frame_count_error++;
		frame_rate_s_1.time = tv.tv_sec;
		frame_rate_s_1.frame_count = 1;
	}

	return ;
}

void call_back_frame_resolution0(IMPFrameInfo *p)
{
	int fd;
	int ret;
	char *path;

	path = get_filename("0", (FRAME_FORMAT_1 == PIX_FMT_NV12) ? "nv12" : "yuv422");

	fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if(fd < 0)
		CU_FAIL("open() error !");

	ret = write(fd, (void *)p->virAddr, p->size);
	if(ret != p->size)
		CU_FAIL("write() error !");

	close(fd);

	return ;
}
void call_back_frame_resolution1(IMPFrameInfo *p)
{
	int fd;
	int ret;
	char *path;

	path = get_filename("1", (FRAME_FORMAT_2 == PIX_FMT_NV12) ? "nv12" : "yuv422");

	fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if(fd < 0)
		CU_FAIL("open() error !");

	ret = write(fd, (void *)p->virAddr, p->size);
	if(ret != p->size)
		CU_FAIL("write() error !");

	close(fd);

	return ;
}
void call_back_framesource1(IMPFrameInfo *p2)
{
	IMP_LOG_DBG(TAG, "%s\n", __func__);
}

static void framesource_test(void *p1, void *p2)
{
	IMP_LOG_DBG(TAG, "IMP_Framesource_Init\n");

	int ret;
	ret = IMP_System_Init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_System_Init() failed:%d\n", ret);
		CU_FAIL();
		return;
	}

	IMP_LOG_DBG(TAG, "Create EmuFake groups\n");
	IMP_EmuFakedev_CreateGroup(0, 0, p1);
	IMP_EmuFakedev_CreateGroup(1, 0, p2);

	IMP_LOG_DBG(TAG, "Define DGCs...\n");
	IMPChannel framesource0 = { DEV_ID_FS, 0, 0 };
	IMPChannel framesource1 = { DEV_ID_FS, 0, 1 };
	IMPChannel dev0_chn0    = { DEV_ID_EMU_FAKE(0), 0, 0 };
	IMPChannel dev1_chn0    = { DEV_ID_EMU_FAKE(1), 0, 0 };

	/* Configure Dev attr */
	IMPFSDevAttr imp_dev_attr;
	imp_dev_attr.inFrmRate = FRAME_RATE_IN;
	imp_dev_attr.nrVBs = 4;
	ret = IMP_FrameSource_SetDevAttr(&imp_dev_attr);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_EmuFrameSource_SetDevAttr() error: %d\n", ret);
		return ;
	}

	/* Check Dev attr */
	IMPFSDevAttr imp_dev_attr_check;
	ret =IMP_FrameSource_GetDevAttr(&imp_dev_attr_check);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_EmuFrameSource_GetDevAttr() error: %d\n", ret);
		return ;
	}

	if(imp_dev_attr_check.inFrmRate != imp_dev_attr.inFrmRate)
		CU_FAIL("IMP_FrameSource_GetDevAttr");

	IMP_LOG_DBG(TAG, "Bind...\n");
#define BIND(A, B)								\
	ret = IMP_System_Bind(&A, &B);						\
	if (ret < 0) {								\
		IMP_LOG_ERR(TAG, "Bind src(%d,%d,%d) dst(%d,%d,%d) error\n",	\
					A.devID, A.grpID, A.chnID,		\
					B.devID, B.grpID, B.chnID);		\
		goto out;							\
	}

	BIND(framesource0, dev0_chn0);
	BIND(framesource1, dev1_chn0);
#undef BIND

	IMP_LOG_DBG(TAG, "Dump...\n");
	ret = system_bind_dump();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Dump error\n");
		CU_FAIL();
		return;
	}

	/* Configure channel 0 attr */
	IMPFSChnAttr imp_chn_attr;
	imp_chn_attr.picWidth = FRAME_W_1;
	imp_chn_attr.picHeight = FRAME_H_1;
	imp_chn_attr.pixFmt = FRAME_FORMAT_1;
	imp_chn_attr.outFrmRate = FRAME_RATE_OUT_1;
	ret = IMP_FrameSource_SetChnAttr(0, &imp_chn_attr);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_EmuFrameSource_SetChnAttr(0) error: %d\n", ret);
		return ;
	}

	/* Check channel 0 attr */
	IMPFSChnAttr imp_chn_attr_check;
	ret = IMP_FrameSource_GetChnAttr(0, &imp_chn_attr_check);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_EmuFrameSource_GetChnAttr(1) error: %d\n", ret);
		return ;
	}
	IMP_LOG_DBG(TAG, "FS Chn0 ChnAttr.picWidth=%d\n", imp_chn_attr_check.picWidth);
	IMP_LOG_DBG(TAG, "FS Chn0 ChnAttr.picHeight=%d\n", imp_chn_attr_check.picHeight);
	IMP_LOG_DBG(TAG, "FS Chn0 ChnAttr.pixFmt=%d\n", imp_chn_attr_check.pixFmt);
	IMP_LOG_DBG(TAG, "FS Chn0 ChnAttr.outFrmRate=%d\n", imp_chn_attr_check.outFrmRate);

	/* Configure channel 1 attr */
	imp_chn_attr.picWidth = FRAME_W_2;
	imp_chn_attr.picHeight = FRAME_H_2;
	imp_chn_attr.pixFmt = FRAME_FORMAT_2;
	imp_chn_attr.outFrmRate = FRAME_RATE_OUT_2;
	ret = IMP_FrameSource_SetChnAttr(1, &imp_chn_attr);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_EmuFrameSource_SetChnAttr(0) error: %d\n", ret);
		return ;
	}

	/* Check channel 0 attr */
	ret = IMP_FrameSource_GetChnAttr(1, &imp_chn_attr_check);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_EmuFrameSource_GetChnAttr(1) error: %d\n", ret);
		return ;
	}
	IMP_LOG_DBG(TAG, "FS Chn1 ChnAttr.picWidth=%d\n", imp_chn_attr_check.picWidth);
	IMP_LOG_DBG(TAG, "FS Chn1 ChnAttr.picHeight=%d\n", imp_chn_attr_check.picHeight);
	IMP_LOG_DBG(TAG, "FS Chn1 ChnAttr.pixFmt=%d\n", imp_chn_attr_check.pixFmt);
	IMP_LOG_DBG(TAG, "FS Chn1 ChnAttr.outFrmRate=%d\n", imp_chn_attr_check.outFrmRate);

	/* Enable channels */
	ret = IMP_FrameSource_EnableChn(0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_EmuFrameSource_EnableChn(0) error: %d\n", ret);
		return ;
	}

	ret = IMP_FrameSource_EnableChn(1);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_EmuFrameSource_EnableChn(1) error: %d\n", ret);
		return ;
	}

	/* Enable Device */
	ret = IMP_FrameSource_EnableDev();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_EmuFrameSource_EnableDev error: %d\n", ret);
		return ;
	}

	IMP_LOG_DBG(TAG, "%s(): OK.\n", __func__);

	IMP_FrameSource_StreamOn();

	struct timeval tv = {6, 0};
	select(0, NULL, NULL, NULL, &tv);

	IMP_FrameSource_StreamOff();

	IMP_FrameSource_DisableDev();

	IMP_FrameSource_DisableChn(1);
	IMP_FrameSource_DisableChn(0);

	IMP_LOG_DBG(TAG, "Unbind...\n");
#define UNBIND(A, B)								\
	ret = IMP_System_UnBind(&A, &B);					\
	if (ret < 0) {								\
		IMP_LOG_ERR(TAG, "UnBind src(%d,%d,%d) dst(%d,%d,%d) error\n",	\
					A.devID, A.grpID, A.chnID,		\
					B.devID, B.grpID, B.chnID);		\
		goto out;							\
	}

	UNBIND(framesource0, dev0_chn0);
	UNBIND(framesource1, dev1_chn0);
#undef UNBIND

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

void framesource_test_frame_rate()
{
	frame_rate_s_0.time = 0;
	frame_rate_s_0.frame_count = 0;
	frame_rate_s_0.frame_count_error = 0;

	frame_rate_s_1.time = 0;
	frame_rate_s_1.frame_count = 0;
	frame_rate_s_1.frame_count_error = 0;

	framesource_test(call_back_frame_rate0, call_back_frame_rate1);

	printf("1 frame rate error : %d\n", frame_rate_s_0.frame_count_error);
	printf("2 frame rate error : %d\n", frame_rate_s_1.frame_count_error);

	sleep(1);

	if((frame_rate_s_0.frame_count_error > 2) || (frame_rate_s_1.frame_count_error > 2))
		CU_FAIL("frame rate error");

	return ;
}

void framesource_test_frame_resolution()
{
	framesource_test(call_back_frame_resolution0, call_back_frame_resolution1);

	return ;
}

CU_TestInfo FrameSourceTest[] = {
	{"framesource test 1 - frame rate", framesource_test_frame_rate},
	{"framesource test 2 - frame resolution", framesource_test_frame_resolution},
	CU_TEST_INFO_NULL,
};
