#include "FrameSource.hh"

FrameSource::FrameSource(const char *name)
  : Handler(name)
{

}

FrameSource::~FrameSource()
{

}

void FrameSource::getFrame(frame_t* frame_list, int& nFrames, uint32_t *params)
{
	lock();
	preStamp();
	doGetFrame(frame_list, nFrames, params);
	afterStamp();
	unlock();
}

void FrameSource::putFrame(frame_t* frame_list, int nFrames)
{
	lock();
	doPutFrame(frame_list, nFrames);
	unlock();
}
