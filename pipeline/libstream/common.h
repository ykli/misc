#ifndef __COMMON_H__
#define __COMMON_H__

#include <sys/syscall.h>
#include <sys/time.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define DEBUG_THREAD

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

#ifdef DEBUG_THREAD
#define DBG(fmt, ...)	printf("[%ld]" fmt, gettid(), ##__VA_ARGS__)
#else
#define DBG		printf
#endif
#define LOCATION()	DBG("%s:%d\n", __func__, __LINE__)

typedef struct frame {
  int index;
  void *addr;
  int size;
  struct timeval timestamp;
} frame_t;

static inline void getTimeStamp(struct timeval& tv_date)
{
	gettimeofday( &tv_date, NULL );
}

#endif
