#ifndef __STREAMSINK_HH__
#define __STREAMSINK_HH__

#include "Handler.hh"

class StreamSink: public Handler {
public:
  void process(frame_t* frame_list, int nFrames, uint32_t *params, frame_t& stream);

protected:
  StreamSink(const char *name);
  ~StreamSink();

  virtual void doProcess(frame_t* frame_list, int nFrames, uint32_t *params, frame_t& stream) = 0;
};

#endif
