#ifndef __V4L2INPUT_HH__
#define __V4L2INPUT_HH__

#include "StreamSource.hh"

class V4L2Input: public FrameSource {
public:
  static V4L2Input* createNew(int w, int h, uint32_t fmt);
  void doGetFrame(frame_t* frame_list, int& nFrames, uint32_t *params);
  void doPutFrame(frame_t* frame_list, int nFrames);
  V4L2Input();
  ~V4L2Input();
  static void flushFramesAsync();

private:
  static bool initialize();
  static bool openFiles();
  static void listVideoInputDevices();
  static bool initV4L2();
  static void flushFrames();

private:
  static bool flushFlag;
  static bool fHaveInitialized;
  static int fOurVideoFileNo;
  static int videoWidth;
  static int videoHeight;
  static uint32_t videoFormat;
};

#endif
