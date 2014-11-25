#include "LineDetectFilter.hh"

LineDetectFilter* LineDetectFilter::createNew(int w, int h, char *ret_store_path)
{
	return new LineDetectFilter(w > 640 ? 640 : w, h > 360 ? 360 : h, ret_store_path);
}

LineDetectFilter::LineDetectFilter(int w, int h, char *ret_store_path)
	: Filter("LineDetectFilter"), WriteResult(ret_store_path)
{
	linedetect = LineDetect::createNew(w, h);
}
LineDetectFilter::~LineDetectFilter()
{
}

static void draw_a_line(uint8_t *addr, int x0, int y0, int x1, int y1, float k, int pwidth, int pheight, int color)
{
	int minx = x0 < x1 ? x0 : x1;
	int maxx = x0 > x1 ? x0 : x1;
	int miny = y0 < y1 ? y0 : y1;
	int maxy = y0 > y1 ? y0 : y1;
	int ystart = x0 < x1 ? y0 : y1;
	if (x0 != x1) {
		int fy = 0;
		for (int x = minx; x < maxx; x++) {
			fy = k * (x - minx) + ystart;
			*(addr + fy * pwidth + x) = color;
		}
	} else {
		for (int y = miny; y < maxy; y++) {
			*(addr + y * pwidth + x0) = color;
		}
	}
}

static void draw_detect_line(frame_t* frame, line_detect_param_t *param, int pwidth, int pheight, float scale)
{
	int x0 = param->x0 * scale, y0 = param->y0 * scale, x1 = param->x1 * scale, y1 = param->y1 * scale;
	float k = 0.0;
	if (x1 != x0) {
		k = ((float)(param->y1 - param->y0)) / (param->x1 - param->x0);
	}
	draw_a_line(((uint8_t *)frame->addr), x0, y0, x1, y1, k, pwidth, pheight, 0xff);
	draw_a_line(((uint8_t *)frame->addr), x0, y0 - 1 < 0 ? 0 : y0 - 1, x1, y1 - 1 < 0 ? 0 : y1 - 1, k, pwidth, pheight, 0x0);
	draw_a_line(((uint8_t *)frame->addr), x0, y0 + 1 > pheight ? pheight : y0 + 1, x1, y1 + 1 > pheight ? pheight : y1 + 1, k, pwidth, pheight, 0x0);
}

void LineDetectFilter::doProcess(frame_t* frame_list, int nFrames, uint32_t *params)
{
	LOCATION("\n");
	line_detect_result_t result;
	line_detect_param_t *param = (line_detect_param_t *) params;
	char buf[10];
	int pwidth = 1280, pheight = 640;
	float scale = 2.0;

	param->x0 = 0;
	param->x1 = 640;
	param->y0 = 0;
	param->y1 = 360;


	frame_t* frame2 = getFrameByType(frame_list, nFrames, FUNCTIONAL);		/* For analysis */
	linedetect->process((unsigned char *)(frame2->addr), params, &result);
	sprintf(buf, "%d", result.ret);
	WriteFd(buf);

	frame_t* frame1 = getFrameByType(frame_list, nFrames, PRIMARY);			/* Primary stream */
	draw_detect_line(frame1, param, pwidth, pheight, scale);
}
