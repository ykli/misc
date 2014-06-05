#ifndef __H264VIDEOENCODER_H__
#define __H264VIDEOENCODER_H__

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "FramedFilter.hh"
#include "IVSFramedFilter.hh"
#define UINT64_C(value) __CONCAT(value,ULL)
extern "C" {
#include <x264.h>
#ifdef SW_X264
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#endif
}


class H264VideoEncoder: public FramedFilter {
public:
  static H264VideoEncoder* createNew(UsageEnvironment& env,
				     IVSFramedFilter* inputSource);

protected:
  H264VideoEncoder(UsageEnvironment& env, IVSFramedFilter* inputSource);
      // called only by createNew()
  virtual ~H264VideoEncoder();

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
  void x264_init(UsageEnvironment& env);
  void x264_encode(void);
  void Print_status(int i_frame, int64_t i_file);

protected:
  virtual Boolean isH264VideoStreamFramer() const { return True; }
  virtual unsigned maxFrameSize()  const { return 150000; }

private:
#ifdef SW_X264
  AVPicture Picture;
#endif
  x264_param_t x264Param;
  x264_t *x264EnCoder;
  x264_picture_t xPic;
  x264_picture_t *pPicOut;
  x264_nal_t *nals;
  int nnal;
  int nalIndex;
  int i_pts;
  void *framebuffer;
  bool moveFlag;
  int64_t i_start;
  int64_t i_file;
  FILE* fd;
};

#endif
