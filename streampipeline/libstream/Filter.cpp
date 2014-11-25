#include "Filter.hh"

Filter::Filter(const char *name)
  : Handler(name)
{

}

Filter::~Filter()
{

}

void Filter::process(frame_t* frame_list, int nFrames, uint32_t *params)
{
	lock();
	preStamp();
	doProcess(frame_list, nFrames, params);
	afterStamp();
	unlock();
}
