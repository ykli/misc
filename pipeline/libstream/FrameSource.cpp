#include "FrameSource.hh"

FrameSource::FrameSource()
  : Handler()
{

}

FrameSource::~FrameSource()
{

}

void FrameSource::getFrame(frame_t& frame, uint32_t *params)
{
	lock();
	doGetFrame(frame, params);
	unlock();
}

void FrameSource::putFrame(frame_t& frame)
{
	lock();
	doPutFrame(frame);
	unlock();
}
