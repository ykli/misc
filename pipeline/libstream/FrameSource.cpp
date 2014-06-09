#include "FrameSource.hh"

FrameSource* FrameSource::createNew(void)
{

}

void FrameSource::doProcess(frame_t& frame, uint32_t params)
{
	getFrame(frame, params);
}
