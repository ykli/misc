#ifndef __FRAMESOURCE_HH__
#define __FRAMESOURCE_HH__

#include "common.h"
#include "Handler.hh"

class FrameSource: public Handler {
public:
  void getFrame(frame_t* frame_list, int& nFrames, uint32_t *params);
  void putFrame(frame_t* frame_list, int nFrames);

protected:
  FrameSource(const char *name);
  ~FrameSource();

  virtual void doGetFrame(frame_t* frame_list, int& nFrames, uint32_t *params) = 0;
  virtual void doPutFrame(frame_t* frame_list, int nFrames) = 0;
};

#endif
