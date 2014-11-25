
#ifndef __HANDLER_HH__
#define __HANDLER_HH__

extern "C" {
#include "common.h"
}

class Handler {
protected:
  Handler(const char *);
  ~Handler();

  void lock();
  void unlock();
  void preStamp();
  void afterStamp();

  int step;
  char name[32];
  pthread_mutex_t mutex;
  int64_t t1;
  int64_t elapsed;
  int times;
};

#endif
