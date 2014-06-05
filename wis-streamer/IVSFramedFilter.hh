#ifndef __IVSFRAMEDFILTER_H__
#define __IVSFRAMEDFILTER_H__

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include "FramedFilter.hh"
#include "FormatConvertor.hh"
#include <ivs.h>

#define NR_FRAMEBUF 2

struct frame_list {
  void *data;
  int inuse;
  struct frame_list *next;
  struct frame_list *pre;
};

struct out_frame {
  void **data;
  struct timeval timestamp;
  bool	*moveFlagAddr;
};

class IVSFramedFilter: public FramedFilter {
public:
  static IVSFramedFilter* createNew(UsageEnvironment& env,
				    FramedSource* inputSource);
  void readFrame1();

protected:
  IVSFramedFilter(UsageEnvironment& env, FramedSource* inputV4L2Source);
      // called only by createNew()
  virtual ~IVSFramedFilter();

private:
  // redefined virtual functions:
  virtual void doGetNextFrame();

private:
  static void afterGettingFrame(void* clientData, unsigned frameSize,
                                unsigned numTruncatedBytes,
                                struct timeval presentationTime,
                                unsigned durationInMicroseconds);
  void afterGettingFrame1(unsigned frameSize,
                          unsigned numTruncatedBytes,
                          struct timeval presentationTime,
                          unsigned durationInMicroseconds);
  void setOutBuffer(void **addr, bool *moveFlagAddr)
  {
	foutFrame.data = addr;
	foutFrame.moveFlagAddr = moveFlagAddr;
  }
  void setDoneFlag() { fDoneFlag = ~0; }
  void clearDoneFlag() { fDoneFlag = 0; }

private:
  friend class H264VideoEncoder;
  struct frame_list framebuffers[NR_FRAMEBUF];
  struct frame_list *curframe;
  uint8_t* detbuf;
  struct out_frame foutFrame;
  char fDoneFlag; // used when setting up "fAuxSDPLine"

  pthread_t tid;
  sem_t semStart;
  int threadFlag;
  int isFirstFrame;
  uint32_t frameCounter;
};

#endif
