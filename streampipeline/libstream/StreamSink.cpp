#include "StreamSink.hh"

StreamSink::StreamSink(const char *name)
  : Handler(name)
{

}

StreamSink::~StreamSink()
{

}

void StreamSink::process(frame_t* frame_list, int nFrames, uint32_t *params, frame_t& stream)
{
	lock();
	preStamp();
	doProcess(frame_list, nFrames, params, stream);
	afterStamp();
	unlock();
}
