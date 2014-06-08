#ifndef __WORKTHREAD_HH__
#define __WORKTHREAD_HH__

#include <pthread.h>

#define MAX_PARAMS_SIZE

class WorkThread {

private:

  void Thread1();

private:
  struct timeval timestamp;
  HandlerList *handlerList;
  FrameList *frameList;
  uint32_t params[MAX_PARAMS_SIZE];
  pthread_t tid;
}

#endif
