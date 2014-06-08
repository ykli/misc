#ifndef __COMMON_H__
#define __COMMON_H__

#include <pthread.h>
#include <stdint.h>

#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE,MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

typedef struct frame {
  int index;
  void *addr;
  int size;
  struct timeval timestamp;
} frame_t;

#endif
