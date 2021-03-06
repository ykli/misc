#ifndef __STREAMLIST_H__
#define __STREAMLIST_H__

#include "common.h"

typedef enum {
  EMPTY = 0,
  USED,
  FILLED,
} node_state_t;

struct stream_node {
  frame_t frame;
  node_state_t state;  
  struct stream_node *next;
  struct stream_node *pre;
};

typedef struct stream_node stream_node_t;

class StreamList {
public:
  static StreamList* createNew(int nNodes);
  frame_t* getFilledStream();
  void putUsedStream(frame_t*);
  frame_t* getEmptyStream();
  void putFilledStream(frame_t* f);
  void flushFilledStream();
  void flushInputFrames();
  void pollingStreamFilled();

protected:
  StreamList(int nNodes);
  virtual ~StreamList();

private:
  stream_node_t* getBlankNode();
  stream_node_t* findNodeByFrame(frame_t *frame);
  void updateStreamState(stream_node_t *node, node_state_t state);
  void lock() { pthread_mutex_lock(&mutex); }
  void unlock() { pthread_mutex_unlock(&mutex); }
  void wakeUpList();

private:
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  int nNodes;
  stream_node_t *streamNodes;
  stream_node_t *curNode;
  stream_node_t *curFilledNode;
  stream_node_t *curPollingNode;
  uint32_t nodesMap;
};

#endif
