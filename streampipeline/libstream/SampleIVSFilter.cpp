#include "SampleIVSFilter.hh"

SampleIVSFilter* SampleIVSFilter::createNew(int w, int h)
{
	return new SampleIVSFilter(w, h);
}

SampleIVSFilter::SampleIVSFilter(int w, int h)
	: Filter("SampleIVSFilter"), width(w), height(h)
{

}

void SampleIVSFilter::doProcess(frame_t* frame_list, int nFrames, uint32_t *params)
{
	LOCATION("\n");

	volatile frame_t* frame1 = getFrameByType(frame_list, nFrames, PRIMARY);			/* Primary stream */
	volatile frame_t* frame2 = getFrameByType(frame_list, nFrames, FUNCTIONAL);		/* For analysis */

	DBG("width=%d, height=%d, nFrames=%d\n", width, height, nFrames);
	if (frame1)
		DBG("frame1:index=%d addr=%p size=%d\n", frame1->index, frame1->addr, frame1->size);

	if (frame2)
	DBG("frame2:index=%d addr=%p size=%d\n", frame2->index, frame2->addr, frame2->size);

	udelay(25000);
	LOCATION("\n");
}
