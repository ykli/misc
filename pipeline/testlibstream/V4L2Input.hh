#ifndef __V4L2INPUT_HH__
#define __V4L2INPUT_HH__

class V4L2Input: public FrameSource {
  void getFrame(frame_t& frame);
}

#endif
