#ifndef __COMMON_H__
#define __COMMON_H__

#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <typeinfo>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>

//#define DEBUG
#define DEBUG_THREAD		/* Depend on DEBUG */
//#define FREOPEN
#ifndef FREOPEN
#define COLOR_PRINT
#endif
#include "color.h"

#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE,MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

#define gettid() syscall(__NR_gettid)

extern int64_t b_timestamp;
#define g_timestamp		(getTimer() - b_timestamp)

#ifdef DEBUG
  #ifdef DEBUG_THREAD
    #define DBG(fmt, ...)	fprintf(stdout,  LIGHT_GREEN "[%08lld]" COLOR_NONE LIGHT_CYAN "[%ld]" COLOR_NONE fmt, \
					g_timestamp / 1000, gettid(), ##__VA_ARGS__)
  #else
    #define DBG			fprintf
  #endif
#else
  #define DBG(fmt, ...)
#endif
#define LOCATION(fmt, ...)	DBG(YELLOW "[%s:%s:%d]" COLOR_NONE fmt, __FILE__,  __func__, __LINE__, ##__VA_ARGS__)

#define MAX_STREAM_SIZE (2 * 1024 * 1024)
#define MAX_NAL_NUM 6

typedef enum {
  PRIMARY,
  SECONDARY,
  FUNCTIONAL
} frame_type_t;

typedef struct frame {
  int index;
  void *addr;
  int size;
  int nal_num;
  int nal_size[MAX_NAL_NUM];
  struct timeval timestamp;
  frame_type_t type;
} frame_t;

static inline frame_t* getFrameByType(frame_t* frames, int nFrames, frame_type_t t)
{
	int i;
	frame_t* ret = NULL;

	for (i = 0; i < nFrames; i++)
		if (frames[i].type == t)
			ret = &frames[i];

	return ret;
}

static inline void getTimeStamp(struct timeval& tv_date)
{
	gettimeofday( &tv_date, NULL );
}

static inline int64_t getTimer(void)
{
	struct timeval tv_date;
	gettimeofday( &tv_date, NULL );
	return( (int64_t) tv_date.tv_sec * 1000000 + (int64_t) tv_date.tv_usec );
}

static inline void udelay(int64_t n)
{
	struct timeval t1, t2;
	getTimeStamp(t1);
	t2 = t1;
	while ((t2.tv_usec + 1000000 * t2.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec) <= n) {
		volatile int a;
		a++;
		getTimeStamp(t2);
	}
}

extern int parse_key_value_config(int * const argc, char ***argv, char *filename);
extern void destroy_argc_argv(int argc, char **argv);
extern void dump_argc_argv(int argc, char **argv);

#endif
