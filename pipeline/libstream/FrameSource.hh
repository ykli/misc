#ifndef __FRAMESOURCE_H__
#define __FRAMESOURCE_H__

#include "common.h"
//#include "Handler.hh"

//class FrameSource: public Handler {
class FrameSource {
public:
  static FrameSource* createNew(void);
  int getFrame(frame_t& frame);

private:
  FrameSource();
  ~FrameSource();

};

#endif
