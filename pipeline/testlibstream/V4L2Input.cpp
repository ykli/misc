#include "V4L2Input.hh"

V4L2Input* V4L2Input::createNew()
{
	return new V4L2Input();
}

V4L2Input::V4L2Input()
	: FrameSource()
{

}

void V4L2Input::doGetFrame(frame_t& frame, uint32_t *params)
{
	printf("V4L2Input::%s:%d\n", __func__, __LINE__);
}

void V4L2Input::doPutFrame(frame_t& frame)
{
	printf("V4L2Input::%s:%d\n", __func__, __LINE__);
}
