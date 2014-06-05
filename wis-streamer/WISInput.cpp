/*
 * Copyright (C) 2005-2006 WIS Technologies International Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and the associated README documentation file (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
// An interface to the WIS GO7007 capture device.
// Implementation

#include "WISInput.hh"
#include "Options.hh"
#include "Err.hh"
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <linux/soundcard.h>
#ifndef __LINUX_VIDEODEV_H
#include "videodev2.h"
#endif

////////// WISOpenFileSource definition //////////

// A common "FramedSource" subclass, used for reading from an open file:
class WISOpenFileSource: public FramedSource {
protected:
  WISOpenFileSource(UsageEnvironment& env, WISInput& input, int fileNo);
  virtual ~WISOpenFileSource();

  virtual void readFromFile() = 0;

private: // redefined virtual functions:
  virtual void doGetNextFrame();

private:
  static void incomingDataHandler(WISOpenFileSource* source, int mask);
  void incomingDataHandler1();

protected:
  WISInput& fInput;
  int fFileNo;
};


////////// WISVideoOpenFileSource definition //////////

class WISVideoOpenFileSource: public WISOpenFileSource {
public:
  WISVideoOpenFileSource(UsageEnvironment& env, WISInput& input);
  virtual ~WISVideoOpenFileSource();

protected: // redefined virtual functions:
  virtual void readFromFile();
};


////////// WISAudioOpenFileSource definition //////////

class WISAudioOpenFileSource: public WISOpenFileSource {
public:
  WISAudioOpenFileSource(UsageEnvironment& env, WISInput& input);
  virtual ~WISAudioOpenFileSource();

protected: // redefined virtual functions:
  virtual void readFromFile();
};


////////// WISInput implementation //////////

WISInput* WISInput::createNew(UsageEnvironment& env) {
  if (!fHaveInitialized) {
    if (!initialize(env)) return NULL;
    fHaveInitialized = True;
  }

  return new WISInput(env);
}

FramedSource* WISInput::videoSource() {
  if (fOurVideoSource == NULL) {
    fOurVideoSource = new WISVideoOpenFileSource(envir(), *this);
  }
  return fOurVideoSource;
}

FramedSource* WISInput::audioSource() {
  if (fOurAudioSource == NULL) {
    fOurAudioSource = new WISAudioOpenFileSource(envir(), *this);
  }
  return fOurAudioSource;
}

WISInput::WISInput(UsageEnvironment& env)
  : Medium(env) {
	formatConvertor = new FormatConvertor();
}

WISInput::~WISInput() {
	delete formatConvertor;
}

Boolean WISInput::initialize(UsageEnvironment& env) {
  do {
    if (!openFiles(env)) break;
    if (!initV4L(env)) break;

    return True;
  } while (0);

  // An error occurred
  return False;
}

static void printErr(UsageEnvironment& env, char const* str = NULL) {
  if (str != NULL) err(env) << str;
  env << ": " << strerror(env.getErrno()) << "\n";
}

Boolean WISInput::openFiles(UsageEnvironment& env) {
  do {
    int i = 0;
#ifndef IGNORE_DRIVER_CHECK
    // Make sure sysfs is mounted and the driver is loaded:
    // Find a Video4Linux device associated with the go7007 driver:
    char sympath[PATH_MAX], sympath2[PATH_MAX], canonpath[PATH_MAX], gopath[PATH_MAX];
    int const maxFileNum = 20;
    for (i = 0; i < maxFileNum; ++i) {
      snprintf(sympath, sizeof sympath, "/sys/class/video4linux/video%d/driver", i);
      snprintf(sympath2, sizeof sympath2, "/sys/class/video4linux/video%d/device/driver", i);
      if (realpath(sympath, canonpath) != NULL) {
	    if (strcmp(strrchr(canonpath, '/') + 1, "camera") == 0) break;
      } else if (realpath(sympath2, canonpath) != NULL) { // alternative path
        if (strcmp(strrchr(canonpath, '/') + 1, "camera") == 0) break;
      }
    }
    snprintf(sympath, sizeof sympath, "/sys/class/video4linux/video%d/device", i);
    if (i == maxFileNum || realpath(sympath, gopath) == NULL) {
		err(env) << "Driver loaded but no Camera devices found.\n";
      env << "Is the device connected properly?\n";
      break;
    }
#endif

    // Open it:
    char vDeviceName[PATH_MAX];
    snprintf(vDeviceName, sizeof vDeviceName, "/dev/video%d", i);
    fOurVideoFileNo = open(vDeviceName, O_RDWR);
    if (fOurVideoFileNo < 0) {
      err(env) << "Unable to open \"" << vDeviceName << "\""; printErr(env);
      break;
    }

    return True;
  } while (0);

  // An error occurred:
  return False;
}

Boolean WISInput::initALSA(UsageEnvironment& env) {
  do {
    int arg;
    arg = AFMT_S16_LE;
    if (ioctl(fOurAudioFileNo, SNDCTL_DSP_SETFMT, &arg) < 0) {
      printErr(env, "SNDCTL_DSP_SETFMT");
      break;
    }
    arg = audioSamplingFrequency;
    if (ioctl(fOurAudioFileNo, SNDCTL_DSP_SPEED, &arg) < 0) {
      printErr(env, "SNDCTL_DSP_SPEED");
      break;
    }
    arg = audioNumChannels > 1 ? 1 : 0;
    if (ioctl(fOurAudioFileNo, SNDCTL_DSP_STEREO, &arg) < 0) {
      printErr(env, "SNDCTL_DSP_STEREO");
      break;
    }

    return True;
  } while (0);

  // An error occurred:
  return False;
}

#define MAX_BUFFERS     8
static struct {
  unsigned char *addr;
  unsigned int length;
} buffers[MAX_BUFFERS];

static int capture_start = 0;

Boolean WISInput::initV4L(UsageEnvironment& env) {
  do {
    // Begin by enumerating the available video input ports, and noting which of these
    // we wish to use:

    listVideoInputDevices(env);
    env << "(Using video input device #" << videoInputDeviceNumber << ")\n";

    // Set the video input port:
    struct v4l2_input inp;
    inp.index = videoInputDeviceNumber;
    if (ioctl(fOurVideoFileNo, VIDIOC_ENUMINPUT, &inp) < 0
	|| ioctl(fOurVideoFileNo, VIDIOC_S_INPUT, &videoInputDeviceNumber) < 0) {
      err(env) << "Unable to set video input device " << videoInputDeviceNumber; printErr(env);
      break;
    }

    // Set the encoding format for the streamed output video:
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = videoWidth;
    fmt.fmt.pix.height = videoHeight;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;

    switch (videoFormat) {
    case VFMT_H264:
    default:
      fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
      break;
    }
    if (ioctl(fOurVideoFileNo, VIDIOC_S_FMT, &fmt) < 0) {
      printErr(env, "Unable to set the video format (and width,height)");
      break;
    }

    // Request that buffers be allocated for memory mapping:
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof req);
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    req.count = MAX_BUFFERS;
    if (ioctl(fOurVideoFileNo, VIDIOC_REQBUFS, &req) < 0) {
      printErr(env, "VIDIOC_REQBUFS");
      break;
    }
    
    // Map each of the buffers into this process's memory,
    // and queue them for frame capture:
    unsigned j;
    for (j = 0; j < req.count; ++j) {
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof buf);
		buf.index = j;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (ioctl(fOurVideoFileNo, VIDIOC_QUERYBUF, &buf) < 0) {
			printErr(env, "VIDIOC_QUERYBUF");
			break;
		}
		buffers[buf.index].addr
			= (unsigned char *)mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED,
									fOurVideoFileNo, buf.m.offset);
		if (buffers[buf.index].addr == MAP_FAILED) {
			printErr(env, "mmap() failed");
			break;
		}
		buffers[buf.index].length = buf.length;
    }
    if (j < req.count) break; // an error occurred
    
    capture_start = 1;

    return True;
  } while (0);

  // An error occurred:
  return False;
}

void WISInput::listVideoInputDevices(UsageEnvironment& env) {
  env << "Input devices available:\n";
  for (int i = 0; ; ++i) {
    struct v4l2_input inp;
    memset(&inp, 0, sizeof inp);
    inp.index = i;
    if (ioctl(fOurVideoFileNo, VIDIOC_ENUMINPUT, &inp) < 0) break; // no more
    env << "\tdevice #" << i << ": " << (char*)(inp.name) << " (";
    for (int j = 0; ; ++j) {
      struct v4l2_standard s;
      memset(&s, 0, sizeof s);
      s.index = j;
      if (ioctl(fOurVideoFileNo, VIDIOC_ENUMSTD, &s) < 0) break;
      if (j > 0) env << ", ";
      env << (char*)(s.name);
    }
    env << ")\n";
  }
}

Boolean WISInput::fHaveInitialized = False;
int WISInput::fOurVideoFileNo = -1;
FramedSource* WISInput::fOurVideoSource = NULL;
int WISInput::fOurAudioFileNo = -1;
FramedSource* WISInput::fOurAudioSource = NULL;
FormatConvertor* WISInput::formatConvertor = NULL;


////////// WISOpenFileSource implementation //////////

WISOpenFileSource
::WISOpenFileSource(UsageEnvironment& env, WISInput& input, int fileNo)
  : FramedSource(env),
    fInput(input), fFileNo(fileNo) {
}

WISOpenFileSource::~WISOpenFileSource() {
  envir().taskScheduler().turnOffBackgroundReadHandling(fFileNo);
}

void WISOpenFileSource::doGetNextFrame() {
  // Await the next incoming data on our FID:
#define GET_FRAME_SYNC
#ifndef GET_FRAME_SYNC
  envir().taskScheduler().turnOnBackgroundReadHandling(fFileNo,
	       (TaskScheduler::BackgroundHandlerProc*)&incomingDataHandler, this);
#else
  readFromFile();
  afterGetting(this);
#endif
}

void WISOpenFileSource
::incomingDataHandler(WISOpenFileSource* source, int /*mask*/) {
  source->incomingDataHandler1();
}

void WISOpenFileSource::incomingDataHandler1() {
  // Read the data from our file into the client's buffer:
  readFromFile();

  // Stop handling any more input, until we're ready again:
  envir().taskScheduler().turnOffBackgroundReadHandling(fFileNo);

  // Tell our client that we have new data:
  afterGetting(this);
}


////////// WISVideoOpenFileSource implementation //////////

WISVideoOpenFileSource
::WISVideoOpenFileSource(UsageEnvironment& env, WISInput& input)
  : WISOpenFileSource(env, input, input.fOurVideoFileNo) {
}

WISVideoOpenFileSource::~WISVideoOpenFileSource() {
  fInput.fOurVideoSource = NULL;
}

void WISVideoOpenFileSource::readFromFile() {
  // Retrieve a filled video buffer from the kernel:
  unsigned i;  
  struct v4l2_buffer buf;

  if (capture_start) {
    capture_start = 0;
    for (i = 0; i < MAX_BUFFERS; ++i) {
      memset(&buf, 0, sizeof buf);
      buf.index = i;
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      if (ioctl(fFileNo, VIDIOC_QBUF, &buf) < 0) {
        printErr(envir(), "VIDIOC_QBUF");
        return;
      }
    }
    
    // Start capturing:
    i = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fFileNo, VIDIOC_STREAMON, &i) < 0) {
      printErr(envir(), "VIDIOC_STREAMON");
      return;
    }
  }
  
  memset(&buf, 0, sizeof buf);
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  if (ioctl(fFileNo, VIDIOC_DQBUF, &buf) < 0) {
    printErr(envir(), "VIDIOC_DQBUF");
    return;
  }

  // Note the timestamp and size:
  fPresentationTime = buf.timestamp;
  fFrameSize = buf.bytesused;
#if 0
  if (fFrameSize > fMaxSize) {
    fNumTruncatedBytes = fFrameSize - fMaxSize;
    fFrameSize = fMaxSize;
  } else {
    fNumTruncatedBytes = 0;
  }
#endif
  // Copy to the desired place:
#ifdef SW_X264
  memmove(fTo, buffers[buf.index].addr, fFrameSize);
#else
  fInput.formatConvertor->yuv422_to_tile420(buffers[buf.index].addr, fTo, videoWidth, videoHeight);
#endif
  // Send the buffer back to the kernel to be filled in again:
  if (ioctl(fFileNo, VIDIOC_QBUF, &buf) < 0) {
    printErr(envir(), "VIDIOC_QBUF");
    return;
  }
}


////////// WISAudioOpenFileSource implementation //////////

WISAudioOpenFileSource
::WISAudioOpenFileSource(UsageEnvironment& env, WISInput& input)
  : WISOpenFileSource(env, input, input.fOurAudioFileNo) {
}

WISAudioOpenFileSource::~WISAudioOpenFileSource() {
  fInput.fOurAudioSource = NULL;
}

void WISAudioOpenFileSource::readFromFile() {
  // Read available audio data:
  int timeinc;
  int ret = read(fInput.fOurAudioFileNo, fTo, fMaxSize);
  if (ret < 0) ret = 0;
  fFrameSize = (unsigned)ret;
  gettimeofday(&fPresentationTime, NULL);

  /* PR#2665 fix from Robin
   * Assuming audio format = AFMT_S16_LE
   * Get the current time
   * Substract the time increment of the audio oss buffer, which is equal to
   * buffer_size / channel_number / sample_rate / sample_size ==> 400+ millisec
   */
  timeinc = fFrameSize * 1000 / audioNumChannels / (audioSamplingFrequency/1000) / 2;
  while (fPresentationTime.tv_usec < timeinc)
  {
    fPresentationTime.tv_sec -= 1;
    timeinc -= 1000000;
  }
  fPresentationTime.tv_usec -= timeinc;
}

