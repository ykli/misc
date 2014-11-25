#ifndef __FORMATCONVERTORFILTER_HH__
#define __FORMATCONVERTORFILTER_HH__

#include "Filter.hh"

#define NR_BUFFERS 2

class FormatConvertorFilter: public Filter {
public:
  static FormatConvertorFilter* createNew(int w, int h);
  void doProcess(frame_t* frame_list, int nFrames, uint32_t *params);

  FormatConvertorFilter(int w, int h);
  ~FormatConvertorFilter();

private:
  void *buffer[NR_BUFFERS];
  void *curBuffer;
  int width;
  int height;
};

#endif
