#ifndef __H264ENCODER_HH__
#define __H264ENCODER_HH__

#include "StreamSink.hh"
extern "C" {
#include <x264.h>
}

class H264Encoder: public StreamSink {
public:
  static H264Encoder* createNew(int w, int h);
  static H264Encoder* createNew(int w, int h, int fmt);
  void doProcess(frame_t* frame_list, int nFrames, uint32_t *params, frame_t& stream);

  H264Encoder(int w, int h);
  H264Encoder(int w, int h, int fmt);
  ~H264Encoder();

  static void ForceIDRFrame() {force_idr = 1;}
  static int showFPS;
  static int fps;

private:
  void x264_init(int fmt);
  void x264_encode();
  void Print_status(int i_frame, int64_t i_file);
  int set_x264_param(int argc, char **argv, x264_param_t *param);

private:
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
  int videoWidth;
  int videoHeight;
  static int force_idr;
};

#endif
