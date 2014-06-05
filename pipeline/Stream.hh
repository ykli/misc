#ifndef __STREAM_H__
#define __STREAM_H__

#define MAX_STREAM_NODE 5

typedef struct frame {
  int index;
  void *addr;
  struct timeval timestamp;
} frame_t;

struct stream_node {
  frame_t frame;
  struct stream_node *next;
  struct stream_node *pre;
};

typedef struct stream_node* stream_node_t;

class StreamQueue {
private:
  Mutex lock;
  int nNodes;
  stream_node_t curNode;

public:
  StreamQueue* createNew();

protected:
  StreamQueue(int nNodes);
  virtual ~StreamQueue();

  void queue(stream_node_t *stream_node);
  void dequeue(stream_node_t *stream_node);
};


typedef struct pic_attr {
  int width;
  int height;
  uint32_t fmt;
} pic_attr_t;

typedef struct video_attr {
  int fps;
} video_attr_t;

class Stream {
public:
  Stream* createNew(int nThread, int nFrames, int w, int h, uint32_t fmt, int fps);

protected:
  Stream(int nThread, int nFrames, pic_attr_t *pic, video_attr_t *video);
  virtual ~Stream();

  void getStream(frame_t *frame);
  void putStream(frame_t *frame);

  void streamInit(frame_t *frame);
  void streamOn(frame_t *frame);
  void streamOff(frame_t *frame);

  void addHandler();
  void delHandler();

private:
  int nWorkThread;
  int nFrames;
  pic_attr_t pic;
  video_attr_t video;
  StreamQueue* queue;
  HandlerList* handlerList;
};

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
