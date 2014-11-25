#include "HandlerList.hh"

HandlerList* HandlerList::createNew()
{
	return new HandlerList();
}

HandlerList::HandlerList() : numOfFilters(0)
{
	filters = (Filter **)malloc(sizeof(Filter *));
}

HandlerList::~HandlerList()
{
	free(filters);
}

void HandlerList::add(FrameSource *fsource)
{
	frameSource = fsource;
}

void HandlerList::add(StreamSink *ssink)
{
	steamSink = ssink;
}

void HandlerList::add(Filter *filter)
{
	numOfFilters++;
	filters = (Filter **)realloc(filters, numOfFilters * sizeof(Filter *));
	filters[numOfFilters-1] = filter;
}

void HandlerList::del(FrameSource *fsource)
{
	frameSource = NULL;
}

void HandlerList::del(StreamSink *ssink)
{
	steamSink = NULL;
}

void HandlerList::del(Filter *filter)
{
	int i;
	for (i = 0; i < numOfFilters; i++) {
		if (filter == filters[i])
			filters[i] = NULL;
	}
}

Filter* HandlerList::getFilterByStep(int step)
{
	return filters[step];
}

FrameSource* HandlerList::getFrameSource()
{
	return frameSource;
}

StreamSink* HandlerList::getStreamSink()
{
	return steamSink;
}

int HandlerList::getNumOfFilters()
{
	return numOfFilters;
}
