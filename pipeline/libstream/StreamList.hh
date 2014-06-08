#ifndef __STREAMLIST_H__
#define __STREAMLIST_H__

#include "common.h"

typedef enum {
  EMPTY,
  FILLED,
  USED,
} node_state_t;

struct stream_node {
  frame_t frame;
  node_state_t state;  
  struct stream_node *next;
  struct stream_node *pre;
};

typedef struct stream_node* stream_node_t;

class StreamList {
public:
  static StreamList* createNew(int nNodes);
  frame_t* getFilledStream();
  void putUsedStream(frame_t*);
  frame_t* getEmptyStream();
  void putFilledStream(frame_t* f);

  void fill(stream_node_t stream_node, void *from, int size);
  void drop(stream_node_t stream_node);

protected:
  StreamList(int nNodes);
  virtual ~StreamList();

private:
  stream_node_t getBlankNode();
  stream_node_t findNodeByFrame(frame_t *frame);
  void updateStreamState(stream_node_t node, node_state_t state);

private:
  pthread_mutex_t lock;
  int nNodes;
  stream_node_t curNode;
};

#endif
