#ifndef __PRIMARYINPUT_HH__
#define __PRIMARYINPUT_HH__

#include "StreamSource.hh"
#include <T05MultiFrameSource.hh>

class T05MultiInput: public FrameSource {
public:
  static T05MultiInput* createNew(int w, int h, uint32_t fmt);
  void doGetFrame(frame_t* frame_list, int& nFrames, uint32_t *params);
  void doPutFrame(frame_t* frame_list, int nFrames);
  T05MultiInput();
  ~T05MultiInput();

private:
  static bool initialize();

private:
  static MultiFrameSource* mMultiFrameSource;
  static SingleFrameSource* mPrimarySource;
  static SingleFrameSource* mFunctionSource;
  static bool fHaveInitialized;
  static int videoWidth;
  static int videoHeight;
  static uint32_t videoFormat;
};

#endif
