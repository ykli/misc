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
// C++ header

#ifndef _WIS_INPUT_HH
#define _WIS_INPUT_HH

#include <MediaSink.hh>
#include "FormatConvertor.hh"

class WISInput: public Medium {
public:
  static WISInput* createNew(UsageEnvironment& env);

  FramedSource* videoSource();
  FramedSource* audioSource();

private:
  WISInput(UsageEnvironment& env); // called only by createNew()
  virtual ~WISInput();

  static Boolean initialize(UsageEnvironment& env);
  static Boolean openFiles(UsageEnvironment& env);
  static Boolean initALSA(UsageEnvironment& env);
  static Boolean initV4L(UsageEnvironment& env);
  static void listVideoInputDevices(UsageEnvironment& env);

private:
  friend class WISVideoOpenFileSource;
  friend class WISAudioOpenFileSource;
  static Boolean fHaveInitialized;
  static int fOurVideoFileNo;
  static FramedSource* fOurVideoSource;
  static int fOurAudioFileNo;
  static FramedSource* fOurAudioSource;
  static FormatConvertor* formatConvertor;
};

// Functions to set the optimal buffer size for RTP sink objects.
// These should be called before each RTPSink is created.
#define AUDIO_MAX_FRAME_SIZE 20480
#define VIDEO_MAX_FRAME_SIZE 250000
inline void setAudioRTPSinkBufferSize() { OutPacketBuffer::maxSize = AUDIO_MAX_FRAME_SIZE; }
inline void setVideoRTPSinkBufferSize() { OutPacketBuffer::maxSize = VIDEO_MAX_FRAME_SIZE; }

#endif
