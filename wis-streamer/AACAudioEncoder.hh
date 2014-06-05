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
// "LIVE555 Streaming Media" interface to an AAC audio encoder.
// C++ header

#ifndef  _AAC_AUDIO_ENCODER_HH
#define _AAC_AUDIO_ENCODER_HH

#include "FramedFilter.hh"

class AACAudioEncoder: public FramedFilter {
public:
  static AACAudioEncoder* createNew(UsageEnvironment& env,
				    FramedSource* inputPCMSource,
				    unsigned numChannels, unsigned samplingRate,
				    unsigned outputKbps);

protected:
  AACAudioEncoder(UsageEnvironment& env, FramedSource* inputPCMSource,
		  unsigned numChannels, unsigned samplingRate, unsigned outputKbps);
      // called only by createNew()
  virtual ~AACAudioEncoder();

private:
  // redefined virtual functions:
  virtual void doGetNextFrame();

private:
  static void afterGettingFrame(void* clientData, unsigned frameSize,
                                unsigned numTruncatedBytes,
                                struct timeval presentationTime,
                                unsigned durationInMicroseconds);
  void afterGettingFrame1(unsigned frameSize,
                          unsigned numTruncatedBytes,
                          struct timeval presentationTime,
                          unsigned durationInMicroseconds);

private:
  void* fEncoderState;
  unsigned long fNumSamplesPerFrame, fMaxEncodedFrameSize;
  unsigned fMicrosecondsPerFrame;
  double fMicrosecondsPerByte;
  unsigned char* fInputSampleBuffer;
  unsigned fInputSampleBufferSize;
  unsigned fInputSampleBufferBytesDesired, fInputSampleBufferBytesFull;
  struct timeval fLastInputDataPresentationTime;
};

#endif
