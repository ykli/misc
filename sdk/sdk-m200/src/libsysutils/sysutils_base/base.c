/*
 * Ingenic IMP System Control module implement.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#include <unistd.h>
#include <signal.h>
#include <time.h>

#include <sysutils/su_base.h>

static SUDevID g_devID = { .chr = "abcdefABCDEF123456"};

int SU_Base_GetDevID(SUDevID *devID)
{
	*devID = g_devID;

	return 0;
}

int SU_Base_Shutdown(void)
{
	sync();
	kill(1, SIGUSR2);

	return 0;
}

int SU_Base_Reboot(void)
{
	sync();
	kill(1, SIGTERM);

	return 0;
}

static int is_valid_sutime(SUTime *suTime)
{
	if (suTime == NULL)
		return 0;
	/* TODO other case here */
	return 1;
}

int SU_Base_GetTime(SUTime *suTime)
{
	if (!is_valid_sutime(suTime))
		return -1;

	time_t now;
	struct tm *now_tm;

	time(&now);
	now_tm = localtime(&now);
	suTime->sec = now_tm->tm_sec;
	suTime->min = now_tm->tm_min;
	suTime->hour = now_tm->tm_hour;
	suTime->mday = now_tm->tm_mday;
	suTime->mon = now_tm->tm_mon + 1;
	suTime->year = now_tm->tm_year + 1900;

	return 0;
}

int SU_Base_SetTime(SUTime *suTime)
{
	if (!is_valid_sutime(suTime))
		return -1;

	struct tm time_tm;
	time_tm.tm_sec = suTime->sec;
	time_tm.tm_min = suTime->min;
	time_tm.tm_hour = suTime->hour;
	time_tm.tm_mday = suTime->mday;
	time_tm.tm_mon = suTime->mon - 1;
	time_tm.tm_year = suTime->year - 1900;
	time_tm.tm_isdst = -1;

	time_t time_to_set;
	time_to_set = mktime(&time_tm);
	stime(&time_to_set);

	return 0;
}
