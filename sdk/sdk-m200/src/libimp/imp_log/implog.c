/*
 * IMP log func file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Andy <xiaobo.yu@ingenic.com>
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

#include <imp/imp_log.h>

#include "logger.h"
#include "logd.h"


#define IMP_LOG_OP_GET_PID(x)	((x)&IMP_LOG_OP_PID)
#define IMP_LOG_OP_GET_USTIME(x) ((x)&IMP_LOG_OP_USTIME)
#define IMP_LOG_OP_GET_MODULE(x) ((x)&IMP_LOG_OP_MODULE)
#define IMP_LOG_OP_GET_FILE(x)   ((x)&IMP_LOG_OP_FILE)
#define IMP_LOG_OP_GET_FUNC(x)   ((x)&IMP_LOG_OP_FUNC)
#define IMP_LOG_OP_GET_LINE(x)   ((x)&IMP_LOG_OP_LINE)


#define IMP_LOG_BUF_SIZE_MAX 1000
typedef struct IMPLogBuf {
	char str[IMP_LOG_BUF_SIZE_MAX];
	int  index;
} IMPLogBuf_t;

static IMPLogBuf_t *buf    = NULL;
static bool  init_flag = false;
static int   log_level = IMP_LOG_LEVEL_DEFAULT;
static char  log_file[100] = "timp.log";
static FILE  *pf = 0;
static int   log_option = 0;

#ifdef HAVE_PTHREADS
static pthread_mutex_t imp_log_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif


#define IMP_LOGGING(le) ((le)>=log_level)

static void _init_imp_log_buf(void)
{
	if (false == init_flag) {
		if (NULL != buf)
			free(buf);
		buf = malloc(sizeof(IMPLogBuf_t));
		if (NULL == buf) {
			printf("ERROR: _init_imp_log_buf malloc err!\n");
		}
		//memset(buf->str, 0, IMP_LOG_BUF_SIZE_MAX);
		buf->index = 0;
		init_flag = true;
	}
}
#if 0
static void _deinit_imp_log_buf(void)
{
	if (NULL != buf)
		free(buf);
	if (NULL != pf)
		fclose(pf);
	init_flag = false;
}
#endif
#if 0
static void _format_str(IMPLogBuf_t *buf, const char *fmt, ...)
{
	char *dst;
	int extra_len;
	int left_len;
	va_list vl;

	dst = buf->str + buf->index;
	left_len = IMP_LOG_BUF_SIZE_MAX - buf->index;
	if (left_len <= 0) {
		printf("ERROR: imp buf too small!\n");
		return;
	}
	va_start(vl, fmt);
	extra_len = vsnprintf(dst, left_len, fmt, vl);
	va_end(vl);
	if (extra_len <= 0) {
		printf("ERROR: vsnprintf!\n");
		return;
	}
	if (extra_len == left_len) {
		printf("ERROR: imp buf too small!\n");
		return;
	}
	buf->index += extra_len;
}
#endif
static void _format_vl(IMPLogBuf_t *buf, const char *fmt, va_list vl)
{
	char *dst;
	int extra_len;
	int left_len;

	dst = buf->str + buf->index;
	left_len = IMP_LOG_BUF_SIZE_MAX - buf->index;
	if (left_len <= 0) {
		printf("ERROR: imp buf too small!\n");
		return;
	}
	extra_len = vsnprintf(dst, left_len, fmt, vl);
	if (extra_len <= 0) {
		printf("ERROR: vsnprintf!\n");
		return;
	}
	if (extra_len == left_len) {
		printf("ERROR: imp buf too small!\n");
		return;
	}
	buf->index += extra_len;
}

static void _format_pid(IMPLogBuf_t *buf)
{
	char *dst;
	int extra_len;
	int left_len;

	dst = buf->str + buf->index;
	left_len = IMP_LOG_BUF_SIZE_MAX - buf->index;
	if (left_len <= 0) {
		printf("ERROR: imp buf too small!\n");
		return;
	}
	extra_len = snprintf(dst, left_len, "[%d]", getpid());
	if (extra_len <= 0) {
		printf("ERROR: vsnprintf!\n");
		return;
	}
	if (extra_len == left_len) {
		printf("ERROR: imp buf too small!\n");
		return;
	}
	buf->index += extra_len;
}

static void _format_ustime(IMPLogBuf_t *buf)
{
	char  *dst;
	int   extra_len;
	int   left_len;
	struct timeval    tv;
	gettimeofday(&tv,NULL);
	dst = buf->str + buf->index;
	left_len = IMP_LOG_BUF_SIZE_MAX - buf->index;
	if (left_len <= 0) {
		printf("ERROR: imp buf too small!\n");
		return;
	}
	extra_len = snprintf(dst, left_len, "[%.6f]", (double)(tv.tv_sec+tv.tv_usec*1.0/1000000));
	if (extra_len <= 0) {
		printf("ERROR: vsnprintf!\n");
		return;
	}
	if (extra_len == left_len) {
		printf("ERROR: imp buf too small!\n");
		return;
	}
	buf->index += extra_len;
}

static void _buf_print(IMPLogBuf_t *buf)
{
	fprintf(stdout,"%s", buf->str);
	buf->index = 0;
}

static void _buf_log(IMPLogBuf_t *buf)
{
	if (NULL == pf) {
		pf = fopen(log_file, "a+");
		if (NULL == pf) {
			printf("ERROR: _buf_log log file open error!\n");
			return;
		}
	}
	fprintf(pf,"%s", buf->str);
	buf->index = 0;
}

int imp_get_log_level(void)
{
	return log_level;
}

void imp_set_log_level(int le)
{
	log_level = le;
}

int IMP_Log_Get_Option(void)
{
	return log_option;
}

void IMP_Log_Set_Option(int op)
{
	log_option = op;
}
void imp_set_log_file(char* file)
{
	int len = 0;
	len = strlen(file);
	if (100 <= len) {
		printf("ERROR: file name too long!\n");
		return;
	}
	strcpy(log_file, file);
}

char* imp_get_log_file(void)
{
	return log_file;
}

void imp_log_format_option(int op)
{
	_init_imp_log_buf();
	if (IMP_LOG_OP_GET_USTIME(op))
		_format_ustime(buf);
	if (IMP_LOG_OP_GET_PID(op))
		_format_pid(buf);
}

void imp_log_format_info_vl(const char *fmt, va_list vl)
{
	_init_imp_log_buf();
	_format_vl(buf, fmt, vl);
}

void imp_log_format_info(const char *fmt, ...)
{
	va_list vl;
	_init_imp_log_buf();
	va_start(vl, fmt);
	_format_vl(buf, fmt, vl);
	va_end(vl);
}

void imp_log_print_buf()
{
	_buf_print(buf);
}

void imp_log_buf()
{
	_buf_log(buf);
}
void imp_log_to_logcat(int le, const char* tag)
{
	__android_log_write(le, tag, buf->str);
	buf->index = 0;
}

void imp_log_fun(int le, int op, int out, const char* tag, const char* file, int line, const char* func, const char* fmt, ...)
{

#ifdef HAVE_PTHREADS
	pthread_mutex_lock(&imp_log_mutex);
#endif

	va_list vl;
	if (IMP_LOGGING(le)) {
		//imp_log_format_option(op);
		if (IMP_LOG_OP_GET_FILE(op))
			imp_log_format_info("[%s]:", file);
		if (IMP_LOG_OP_GET_LINE(op))
			imp_log_format_info("[%d]", line);
		if (IMP_LOG_OP_GET_FUNC(op))
			imp_log_format_info("[%s]", func);
		va_start(vl, fmt);
		imp_log_format_info_vl(fmt, vl);
		va_end(vl);
		if (IMP_LOG_OUT_STDOUT == out)
			imp_log_print_buf();
		else if (IMP_LOG_OUT_LOCAL_FILE == out)
			imp_log_buf();
		else
			imp_log_to_logcat(le, tag);
	}
#ifdef HAVE_PTHREADS
	pthread_mutex_unlock(&imp_log_mutex);
#endif
}


void imp_log_fun_vl(int le, int op, int out, const char* tag, const char* file, int line, const char* func, const char* fmt, va_list vl)
{

#ifdef HAVE_PTHREADS
	 pthread_mutex_lock(&imp_log_mutex);
#endif
	if (IMP_LOGGING(le)) {
		//imp_log_format_option(op);
		if (IMP_LOG_OP_GET_FILE(op))
			imp_log_format_info("[%s]:", file);
		if (IMP_LOG_OP_GET_LINE(op))
			imp_log_format_info("[%d]", line);
		if (IMP_LOG_OP_GET_FUNC(op))
			imp_log_format_info("[%s]", func);
		imp_log_format_info_vl(fmt, vl);
		if (IMP_LOG_OUT_STDOUT == out)
			imp_log_print_buf();
		else if (IMP_LOG_OUT_LOCAL_FILE == out)
			imp_log_buf();
		else
			imp_log_to_logcat(le, tag);
	}

#ifdef HAVE_PTHREADS
	    pthread_mutex_unlock(&imp_log_mutex);
#endif
}

