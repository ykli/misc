#ifndef __STREAM_H__
#define __STREAM_H__

#include "common.h"
#include "StreamList.hh"
#include "WorkThread.hh"

typedef struct pic_attr {
  int width;
  int height;
  uint32_t fmt;
} pic_attr_t;

typedef struct video_attr {
  int fps;
  int tpf;
} video_attr_t;

struct thread_node {
  WorkThread *workThread;
  struct thread_node *next;
  struct thread_node *pre;
};

typedef struct thread_node thread_node_t;

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
  void tickThread();

protected:
  StreamSource(int nThread, int nFrames, int w, int h, uint32_t fmt, int fps);

  void threadPollInit();

private:
  int nWorkThread;
  int nFrames;
  pic_attr_t pic;
  video_attr_t video;
  StreamList* streamList;
  int streamIsOn;
  thread_node_t* workThreadPoll;
  thread_node_t* curThreadNode;
  pthread_t tid;
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
