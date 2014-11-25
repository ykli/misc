#ifndef __HANDLERLIST_HH__
#define __HANDLERLIST_HH__

#include "FrameSource.hh"
#include "StreamSink.hh"
#include "Filter.hh"

#if 0
struct handler_node {
  Handler *handler;
  struct Handler *next;
  struct Handler *pre;
};

typedef struct handler_node handler_node_t;
#endif
class HandlerList {
public:
  static HandlerList* createNew();
  void add(FrameSource *fsource);
  void add(StreamSink *ssink);
  void add(Filter *filter);
  void del(FrameSource *fsource);
  void del(StreamSink *ssink);
  void del(Filter *filter);
  Filter* getFilterByStep(int step);
  FrameSource* getFrameSource();
  StreamSink* getStreamSink();
  int getNumOfFilters();

protected:
  HandlerList();
  virtual ~HandlerList();

private:
#if 0
  handler_node_t *handlers;
  handler_node_t *curHandler;
  int numOfHandler;
#endif
  FrameSource *frameSource;
  StreamSink *steamSink;
  Filter **filters;
  int numOfFilters;
};

#endif
