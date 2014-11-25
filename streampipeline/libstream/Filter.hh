#ifndef __FILTER_HH__
#define __FILTER_HH__

#include "Handler.hh"

class Filter: public Handler {
public:
  void process(frame_t* frame_list, int nFrames, uint32_t *params);

protected:
  Filter(const char *name);
  ~Filter();

  virtual void doProcess(frame_t* frame_list, int nFrames, uint32_t *params) = 0;
};

#endif
