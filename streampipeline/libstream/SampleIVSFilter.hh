#ifndef __SAMPLEIVSFILTER_HH__
#define __SAMPLEIVSFILTER_HH__

#include "Filter.hh"

class SampleIVSFilter: public Filter {
public:
  static SampleIVSFilter* createNew(int w, int h);
  void doProcess(frame_t* frame_list, int nFrames, uint32_t *params);

  SampleIVSFilter(int w, int h);
  ~SampleIVSFilter();

private:
  int width;
  int height;
};

#endif
