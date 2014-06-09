#ifndef __FRAMESOURCE_H__
#define __FRAMESOURCE_H__

#include "common.h"
//#include "Handler.hh"

//class FrameSource: public Handler {
class FrameSource {
public:
  static FrameSource* createNew(void);
  void doProcess(frame_t& frame, uint32_t params);
  virtual void getFrame(frame_t& frame, uint32_t params);

private:
  FrameSource();
  ~FrameSource();

};

#endif
