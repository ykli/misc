
#ifndef __HANDLER_HH__
#define __HANDLER_HH__

#include "common.h"

class Handler {
protected:
  Handler();
  ~Handler();

  void lock() { pthread_mutex_lock(&mutex); }
  void unlock() { pthread_mutex_unlock(&mutex); }

  int step;
  char name[];
  pthread_mutex_t mutex;
};

#endif
