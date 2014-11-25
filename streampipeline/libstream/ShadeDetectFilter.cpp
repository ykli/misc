#include "ShadeDetectFilter.hh"
#include <stdio.h>

ShadeDetectFilter* ShadeDetectFilter::createNew(int w, int h, char *ret_store_path)
{
	return new ShadeDetectFilter(w > 640 ? 640 : w, h > 360 ? 360 : h, ret_store_path);
}

ShadeDetectFilter::ShadeDetectFilter(int w, int h, char *ret_store_path)
	: Filter("ShadeDetectFilter"), WriteResult(ret_store_path)
{
	shadedetect = ShadeDetect::createNew(w, h);
}

ShadeDetectFilter::~ShadeDetectFilter()
{
}

void ShadeDetectFilter::doProcess(frame_t* frame_list, int nFrames, uint32_t *params)
{
	LOCATION("\n");
	shade_detect_result_t result;
	char buf[10];

	//volatile frame_t* frame1 = getFrameByType(frame_list, nFrames, PRIMARY);			/* Primary stream */
	volatile frame_t* frame2 = getFrameByType(frame_list, nFrames, FUNCTIONAL);		/* For analysis */
	shadedetect->process((unsigned char *)(frame2->addr), params, &result);
	//printf("result.ret = %d\n", result.ret);
	sprintf(buf, "%d", result.ret);
	WriteFd(buf);
}
