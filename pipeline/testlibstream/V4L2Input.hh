#ifndef __V4L2INPUT_HH__
#define __V4L2INPUT_HH__

#include <StreamSource.hh>

class V4L2Input: public FrameSource {
public:
  static V4L2Input* createNew();
  void doGetFrame(frame_t& frame, uint32_t *params);
  void doPutFrame(frame_t& frame);
  V4L2Input();
  ~V4L2Input();

private:

};

#endif
