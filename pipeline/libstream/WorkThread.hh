#ifndef __WORKTHREAD_HH__
#define __WORKTHREAD_HH__

#include <pthread.h>
#include "common.h"
#include "HandlerList.hh"
#include "FrameList.hh"
#include "StreamList.hh"

#define MAX_PARAMS_SIZE

typedef enum {
  THREAD_SLEEP,
  THREAD_WAKED,
} thread_state_t;

class WorkThread {
public:
  static WorkThread* createNew(HandlerList *h, FrameList *f, StreamList *s);
  void Thread();
  void wakeUpThread();
 
private:
  WorkThread(HandlerList *h, FrameList *f, StreamList *s);
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
