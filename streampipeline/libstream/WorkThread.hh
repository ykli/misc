#ifndef __WORKTHREAD_HH__
#define __WORKTHREAD_HH__

#include <pthread.h>
#include "common.h"
#include "HandlerList.hh"
#include "FrameList.hh"
#include "StreamList.hh"

#define MAX_PARAMS_SIZE 8
#define MAX_FRAMES 3

#define MAX_TIME_RECORD 1000
typedef struct __time_record {
	pid_t pid; //%ld: gettid();
	unsigned short step;
	int64_t start_time; //%lld: getTimer();
	int64_t finish_time; //%lld: getTimer();
} time_record;

typedef enum {
  THREAD_SLEEP,
  THREAD_WAKED,
  THREAD_STOP,
} thread_state_t;

class WorkThread {
public:
  static WorkThread* createNew(HandlerList *h, FrameList *f, StreamList *s);
  void Thread();
  void wakeUpThread();
  int ThreadIsSlept() { return threadState == THREAD_SLEEP; }
 
private:
  WorkThread(HandlerList *h, FrameList *f, StreamList *s);
public:
  ~WorkThread();

private:
  struct timeval timestamp;
  HandlerList *handlerList;
  FrameList *frameList;
  StreamList *streamList;
  uint32_t params[MAX_PARAMS_SIZE];
  pthread_t tid;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  thread_state_t threadState;
};

#endif
