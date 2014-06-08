#ifndef __STREAM_H__
#define __STREAM_H__

#include "common.h"
#include "StreamList.hh"

#define MAX_STREAM_NODE 5

typedef struct pic_attr {
  int width;
  int height;
  uint32_t fmt;
} pic_attr_t;

typedef struct video_attr {
  int fps;
} video_attr_t;

class StreamSource {
public:
  static StreamSource* createNew(int nThread, int nFrames, int w, int h, uint32_t fmt, int fps);
  void getStream(void* to, int& len, struct timeval& timestamp);

  void streamInit(int nFrames);
  void streamOn();
  void streamOff();

  void addHandler();
  void delHandler();
  virtual ~StreamSource();

protected:
  StreamSource(int nThread, int nFrames, int w, int h, uint32_t fmt, int fps);

  void tickThread();

private:
  int nWorkThread;
  int nFrames;
  pic_attr_t pic;
  video_attr_t video;
  StreamList* streamList;
  int streamIsOn;
  //  HandlerList* handlerList;
};

#if 0
template<typename TYPE>
class List {
public:
  TYPE getNextAvailableNode();
  TYPE getNextBlankNode();
  void putNode(TYPE);
  void addNode(TYPE);
  void delNode(TYPE);

private:
  TYPE curAvailableNode;
  TYPE curBlankNode;
  int nNodes;
  Mutex lock;
}
#endif
#endif
