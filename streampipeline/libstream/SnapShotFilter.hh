#ifndef __SNAPSHOTFILTER_HH__
#define __SNAPSHOTFILTER_HH__

#include "Filter.hh"
extern "C" {
#include <ipc.h>
}

class SnapShotFilter: public Filter {
public:
  static SnapShotFilter* createNew(int w, int h, const char* path);
  void doProcess(frame_t* frame_list, int nFrames, uint32_t *params);

  SnapShotFilter(int w, int h, const char* path);
  ~SnapShotFilter();

private:
  int width;
  int height;
  char savePath[PATH_MAX];
  int picCnt;
};

#endif
