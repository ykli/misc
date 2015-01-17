/*
 * IMP log test file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Andy <xiaobo.yu@ingenic.com>
 */


#include <CUnit/CUnit.h>
#include <CUnit/Automated.h>
#include <CUnit/Basic.h>
#include <CUnit/Console.h>

#include "imp_log.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/wait.h>

#define MS_SLEEP_TIME (100 * (rand()*1.0/RAND_MAX))
#define STRING_NUM(x) (sizeof(x)/sizeof(x[0]))
#define msleep(x) usleep((x)*1000)
#define THREAD_NUM 10
#define PROCESS_NUM 10
#define TMP_FILE_NAME "/tmp/tmp.log"

char *test_str[] = {
	"test string 0",
	"test string 1",
	"test string 2",
	"test string 3",
	"test string 4",
	"test string 5",
	"test string 6",
	"test string 7",
	"test string 8",
	"test string 9",
	"test string 10",
	"test string 11",
	"test string 12",
	"test string 13",
	"test string 14",
	"test string 15",
	"test string 16",
	"test string 17",
	"test string 18",
	"test string 19"
};

pthread_t	id[THREAD_NUM];
pid_t		pid[PROCESS_NUM];
char		tmp_str[1000];

static void thread(void)
{
	int i = 0;
	for (i = 0; i < STRING_NUM(test_str); i++) {
		//msleep(MS_SLEEP_TIME);
		IMP_LOG_ERR("TEST", "[thread:0x%08x][%d]:[%s]\n", pthread_self(), i, test_str[i]);
	}
}


static void process(void)
{
	int i = 0;
	for (i = 0; i < STRING_NUM(test_str); i++) {
		msleep(MS_SLEEP_TIME);
		IMP_LOG_ERR("TEST", "[pid:0x%08x][%d]:[%s]\n", getpid(), i, test_str[i]);
	}
}


static void test_init(void)
{
	int rv = 0;
	rv = system("./logcat -c");
	if (0 != rv) {
		printf("ERROR: exce ./logcat -c error rv = %d\n", rv);
		CU_FAIL();
	}
	IMP_LOG_ERR("LOGTEST", "%d\n", sizeof(id));
	memset(id, 0, sizeof(id));
	memset(pid, 0, sizeof(pid));
}

static void test_start_thread(void)
{

	int i,ret;
	for (i = 0; i <THREAD_NUM; i++) {
		ret=pthread_create(&id[i],NULL,(void *) thread,NULL);

		if(ret!=0) {
			printf ("Create pthread error!\n");
			id[i] = 0;
			CU_FAIL();
			break;
		}
	}
	for (i = 0; i <THREAD_NUM; i++) {
		if (0 != id[i])
			pthread_join(id[i],NULL);
	}

}


static void test_start_process(void)
{
	int i = 0;
	for(i = 0; i < PROCESS_NUM; i++) {
		pid[i] = fork();
        if(pid[i] < 0) {
			printf("ERROR: fork error!\n");
			CU_FAIL();
			break;
		}
        else if (0 == pid[i]) {
			process();
			exit(0);
		}
		else
			;/*printf("INFO: fork pid = %d\n", pid[i]);*/
    }

	for(i = 0; i < PROCESS_NUM; i++) {
		if (0 < pid[i])
			waitpid(pid[i], 0, 0);
	}

}


static void test_check_thread(void)
{
	int rv = 0;
	int i,j;
	sprintf(tmp_str, "./logcat -d >%s", TMP_FILE_NAME);
	rv = system(tmp_str);
	if (0 != rv) {
		printf("ERROR: exce ./logcat -d error rv = %d\n", rv);
		CU_FAIL();
		return;
	}
	for (i = 0; i < THREAD_NUM; i++)
		for (j = 0; j < STRING_NUM(test_str); j++) {
			sprintf(tmp_str, "grep \"\\[thread:0x%08x\\]\\[%d\\]:\\[%s\\]\" %s -rn >/dev/null", (unsigned int)(id[i]), j, test_str[j], TMP_FILE_NAME);
			rv = system(tmp_str);
			if (0 != rv) {
				printf("ERROR: log not found!\n");
				printf("%s\n", tmp_str);
				CU_FAIL();
				return;
			}
		}
}


static void test_check_process(void)
{
	int rv = 0;
	int i,j;
	sprintf(tmp_str, "./logcat -d >%s", TMP_FILE_NAME);
	rv = system(tmp_str);
	if (0 != rv) {
		printf("ERROR: exce ./logcat -d error rv = %d\n", rv);
		CU_FAIL();
		return;
	}
	for (i = 0; i < PROCESS_NUM; i++)
		for (j = 0; j < STRING_NUM(test_str); j++) {
			sprintf(tmp_str, "grep \"\\[pid:0x%08x\\]\\[%d\\]:\\[%s\\]\" %s -rn >/dev/null", pid[i], j, test_str[j], TMP_FILE_NAME);
			rv = system(tmp_str);
			if (0 != rv) {
				printf("ERROR: log not found!\n");
				printf("%s\n", tmp_str);
				CU_FAIL();
				return;
			}
		}
}

static void test_thread(void)
{
	test_init();
	IMP_Log_Set_Option(IMP_LOG_OP_DEFAULT);
	test_start_thread();
	test_check_thread();
}

static void test_process(void)
{
	test_init();
	test_start_process();
	test_check_process();
}
static void test_log_thread()
{
	test_thread();
}

static void test_log_process()
{
	test_process();
}

CU_TestInfo LogTest[] = {
	{"imp log test case thread", test_log_thread},
	{"imp log test case process", test_log_process},
	CU_TEST_INFO_NULL,
};
