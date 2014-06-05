#ifndef _FORMATCONVERTOR_HH
#define _FORMATCONVERTOR_HH

#include <pthread.h>
#include <semaphore.h>

struct convertor_args {
  unsigned char *src;
  int width;
  int height;
  unsigned char *dst;
  int start_mbrow;
  int nr_mbrow;
};

class FormatConvertor {
public:
  FormatConvertor(void);
  ~FormatConvertor();
  
  void yuv422_to_tile420(uint8_t* src, uint8_t* dst, int width, int height);
  void convertor_thread1(void);

public:
  static void tile420_to_grey(uint8_t* src, uint8_t* dest, int width, int height);

private:
  pthread_t tid;
  sem_t sem_start;
  sem_t sem_end;
  int thread_flag;
  struct convertor_args args;
};

#endif
