#include "V4L2Input.hh"
#include "videodev2.h"

bool V4L2Input::flushFlag = false;
bool V4L2Input::fHaveInitialized = false;
int V4L2Input::fOurVideoFileNo = -1;
int V4L2Input::videoWidth;
int V4L2Input::videoHeight;
uint32_t V4L2Input::videoFormat = V4L2_PIX_FMT_NV12;
static enum v4l2_memory v4l2_buffer_type = V4L2_MEMORY_USERPTR;

V4L2Input* V4L2Input::createNew(int w, int h, uint32_t fmt)
{
	if (!fHaveInitialized) {
		videoWidth = w ? w : 640;
		videoHeight = h ? h : 480;
		videoFormat = fmt ? fmt : V4L2_PIX_FMT_NV12;

		if (!initialize()) return NULL;
		fHaveInitialized = true;
	}

	return new V4L2Input();
}

V4L2Input::V4L2Input()
	: FrameSource("V4L2Input")
{

}

V4L2Input::~V4L2Input()
{
	if(fOurVideoFileNo > 0) {
		unsigned i = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (ioctl(fOurVideoFileNo, VIDIOC_STREAMOFF, &i) < 0) {
			printf("VIDIOC_STREAMOFF");
			return;
		}
		close(fOurVideoFileNo);
	}

	flushFlag = false;
	fHaveInitialized = false;
	fOurVideoFileNo = -1;
}

#define MAX_BUFFERS     4
frame_t buffers[MAX_BUFFERS];
static int capture_start = 0;

bool V4L2Input::initialize() {
  do {
    if (!openFiles()) break;
    if (!initV4L2()) break;

    return true;
  } while (0);

  // An error occurred
  return false;
}

bool V4L2Input::openFiles() {
	do {
		int i = 0;

		// Open it:
		char vDeviceName[PATH_MAX];
		snprintf(vDeviceName, sizeof vDeviceName, "/dev/video%d", i);
		fOurVideoFileNo = open(vDeviceName, O_RDWR);
		if (fOurVideoFileNo < 0) {
			printf("Unable to open %s", vDeviceName);
			break;
		}

		return true;
	} while (0);

	// An error occurred:
	return false;
}

void V4L2Input::listVideoInputDevices() {
	printf("Input devices available:\n");
	for (int i = 0; ; ++i) {
		struct v4l2_input inp;
		memset(&inp, 0, sizeof inp);
		inp.index = i;
		if (ioctl(fOurVideoFileNo, VIDIOC_ENUMINPUT, &inp) < 0) break; // no more
		printf("\tdevice #%d: %s (", i, (char*)(inp.name));
		for (int j = 0; ; ++j) {
			struct v4l2_standard s;
			memset(&s, 0, sizeof s);
			s.index = j;
			if (ioctl(fOurVideoFileNo, VIDIOC_ENUMSTD, &s) < 0) break;
			if (j > 0) printf(", ");
			printf("%s", (char*)(s.name));
		}
		printf(")\n");
	}
}

bool V4L2Input::initV4L2() {
	do {
		// Begin by enumerating the available video input ports, and noting which of these
		// we wish to use:

		listVideoInputDevices();

		int videoInputDeviceNumber = 0;
		printf("(Using video input device #%d\n", videoInputDeviceNumber);

		// Set the video input port:
		struct v4l2_input inp;
		inp.index = videoInputDeviceNumber;
		if (ioctl(fOurVideoFileNo, VIDIOC_ENUMINPUT, &inp) < 0
			|| ioctl(fOurVideoFileNo, VIDIOC_S_INPUT, &videoInputDeviceNumber) < 0) {
			printf("Unable to set video input device %d\n", videoInputDeviceNumber);
			break;
		}

		// Set the encoding format for the streamed output video:
		struct v4l2_format fmt;
		memset(&fmt, 0, sizeof fmt);
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmt.fmt.pix.width = videoWidth;
		fmt.fmt.pix.height = videoHeight;
		fmt.fmt.pix.field = V4L2_FIELD_ANY;
		fmt.fmt.pix.pixelformat = videoFormat;

		if (ioctl(fOurVideoFileNo, VIDIOC_S_FMT, &fmt) < 0) {
			printf("Unable to set the video format (and width,height)");
			break;
		}

		// Request that buffers be allocated for memory mapping:
		struct v4l2_requestbuffers req;
		memset(&req, 0, sizeof req);
		req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		req.memory = v4l2_buffer_type;
		req.count = MAX_BUFFERS;
		if (ioctl(fOurVideoFileNo, VIDIOC_REQBUFS, &req) < 0) {
			printf("VIDIOC_REQBUFS");
			break;
		}
		if (v4l2_buffer_type == V4L2_MEMORY_MMAP) {
			// Map each of the buffers into this process's memory,
			// and queue them for frame capture:
			unsigned j;
			for (j = 0; j < req.count; ++j) {
				struct v4l2_buffer buf;
				memset(&buf, 0, sizeof buf);
				buf.index = j;
				buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				if (ioctl(fOurVideoFileNo, VIDIOC_QUERYBUF, &buf) < 0) {
					printf("VIDIOC_QUERYBUF");
					break;
				}
				buffers[buf.index].addr
					= (unsigned char *)mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED,
											fOurVideoFileNo, buf.m.offset);
				if (buffers[buf.index].addr == MAP_FAILED) {
					printf("mmap() failed");
					break;
				}
				buffers[buf.index].size = buf.length;
				buffers[buf.index].index = buf.index;
			}
			if (j < req.count) break; // an error occurred
		} else if (v4l2_buffer_type == V4L2_MEMORY_USERPTR) {
			for (int i = 0; i < MAX_BUFFERS; i++) {
				buffers[i].size = videoWidth * videoHeight * 2;
				buffers[i].addr = (unsigned char *)valloc(buffers[i].size);
			}
		}
		capture_start = 1;

		return true;
	} while (0);

	// An error occurred:
	return false;
}

void V4L2Input::doGetFrame(frame_t* frame_list, int& nFrames, uint32_t *params)
{
	unsigned i;
	struct v4l2_buffer buf;

	LOCATION("\n");

	if (flushFlag)
		flushFrames();

	if (capture_start) {
		capture_start = 0;
		for (i = 0; i < MAX_BUFFERS; ++i) {
			memset(&buf, 0, sizeof buf);
			buf.index = i;
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = v4l2_buffer_type;
			if (v4l2_buffer_type == V4L2_MEMORY_USERPTR) {
				buf.m.userptr = (long unsigned int)buffers[i].addr;
				buf.length = buffers[i].size;
			}
			if (ioctl(fOurVideoFileNo, VIDIOC_QBUF, &buf) < 0) {
				printf("VIDIOC_QBUF");
				return;
			}
		}

		// Start capturing:
		i = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (ioctl(fOurVideoFileNo, VIDIOC_STREAMON, &i) < 0) {
//			printf("VIDIOC_STREAMON\n");
		}
	}

	memset(&buf, 0, sizeof buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = v4l2_buffer_type;
	if (ioctl(fOurVideoFileNo, VIDIOC_DQBUF, &buf) < 0) {
		printf("VIDIOC_DQBUF error\n");
		return;
	}

	frame_t& frame = frame_list[0];
	frame = buffers[buf.index];
	LOCATION("frame_list=%p buf.index=%d\n", frame_list, buf.index);

	frame.timestamp = buf.timestamp;
	frame.index = buf.index;
	frame.type = PRIMARY;
	nFrames = 1;
}

void V4L2Input::doPutFrame(frame_t* frame_list, int nFrames)
{
	LOCATION("\n");
	struct v4l2_buffer buf;
	frame_t& frame = frame_list[0];

	memset(&buf, 0, sizeof buf);
	buf.index = frame.index;
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = v4l2_buffer_type;
	if (v4l2_buffer_type == V4L2_MEMORY_USERPTR) {
		buf.m.userptr = (long unsigned int)buffers[buf.index].addr;
		buf.length = buffers[buf.index].size;
	}
	if (ioctl(fOurVideoFileNo, VIDIOC_QBUF, &buf) < 0) {
		printf("VIDIOC_QBUF error");
		return;
	}
}

void V4L2Input::flushFrames()
{
	LOCATION("\n");
	struct v4l2_buffer buf;

	if (!capture_start) {
		for (int i = 0; i < MAX_BUFFERS; ++i) {
			memset(&buf, 0, sizeof buf);
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = v4l2_buffer_type;
			if (ioctl(fOurVideoFileNo, VIDIOC_DQBUF, &buf) < 0) {
				printf("VIDIOC_DQBUF error");
				return;
			}
		}
	}
	capture_start = 1;
	flushFlag = false;
}

void V4L2Input::flushFramesAsync()
{
	flushFlag = true;
}
