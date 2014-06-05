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
// "LIVE555 Streaming Media" interface to an AMR audio encoder.
// Implementation

#include "AMRAudioEncoder.hh"
extern "C" {
#include "AMREncoder/interf_enc.h"
#include "AMREncoder/interf_rom.h"
}

AMRAudioEncoder* AMRAudioEncoder
::createNew(UsageEnvironment& env, FramedSource* inputPCMSource, unsigned numChannels) {
  return new AMRAudioEncoder(env, inputPCMSource, numChannels);
}

#define MILLION 1000000
#define AMR_SAMPLES_PER_FRAME 160
#define AMR_SAMPLES_PER_SECOND 8000
#define AMR_MICROSECONDS_PER_FRAME ((MILLION*AMR_SAMPLES_PER_FRAME)/AMR_SAMPLES_PER_SECOND)
#define AMR_MAX_CODED_FRAME_SIZE 320 /*?????*/

AMRAudioEncoder
::AMRAudioEncoder(UsageEnvironment& env, FramedSource* inputPCMSource, unsigned numChannels)
  : AMRAudioSource(env, False/*isWideband*/, numChannels),
    fInputPCMSource(inputPCMSource) {
  extern int EncoderIncludeHeaderByte;
  EncoderIncludeHeaderByte = 0;
  fEncoderState = Encoder_Interface_init(0/*no DTX*/);

  fMicrosecondsPerByte
    = (1.0*MILLION)/(AMR_SAMPLES_PER_SECOND*numChannels*sizeof (unsigned short));
  fInputSampleBufferBytesDesired
    = AMR_SAMPLES_PER_FRAME*numChannels*sizeof (unsigned short);
      // the number of samples necessary in order to encode a frame
  fInputSampleBufferSize = 10*fInputSampleBufferBytesDesired;
  fInputSampleBuffer = new unsigned char[fInputSampleBufferSize];
  fInputSampleBufferBytesFull = 0;

  fLastInputDataPresentationTime.tv_sec = 0;
  fLastInputDataPresentationTime.tv_usec = 0;
}

AMRAudioEncoder::~AMRAudioEncoder() {
  Encoder_Interface_exit(fEncoderState);
  delete[] fInputSampleBuffer;
}

void AMRAudioEncoder::doGetNextFrame() {
  // If we have enough samples in order to encode a frame, do this now:
  if (fInputSampleBufferBytesFull >= fInputSampleBufferBytesDesired) {
    if (fMaxSize < AMR_MAX_CODED_FRAME_SIZE) {
      // Our sink hasn't given us enough space for a frame.  We can't encode.
      fFrameSize = 0;
      fNumTruncatedBytes = AMR_MAX_CODED_FRAME_SIZE;
    } else {
      enum Mode ourAMRMode = MR122; // the only mode that we support
      fFrameSize = Encoder_Interface_Encode(fEncoderState, ourAMRMode,
					    (short*)fInputSampleBuffer, fTo,
					    0/*disable DTX*/);
      // Note the 1-byte AMR frame header (which wasn't included in the encoded data):
      fLastFrameHeader = toc_byte[ourAMRMode];

      fNumTruncatedBytes = 0;

      // Shift any remaining samples down the the start of the buffer:
      fInputSampleBufferBytesFull -= fInputSampleBufferBytesDesired;
      memmove(fInputSampleBuffer,
	      &fInputSampleBuffer[fInputSampleBufferBytesDesired],
	      fInputSampleBufferBytesFull);
    }

    // Complete delivery to the client:
    fPresentationTime = fLastInputDataPresentationTime;
    //fDurationInMicroseconds = AMR_MICROSECONDS_PER_FRAME;
    fDurationInMicroseconds = 0; // because audio capture is bursty, check for it ASAP
    afterGetting(this);
  } else {
    // Read more samples from our source, then try again:
    unsigned maxBytesToRead
      = fInputSampleBufferSize - fInputSampleBufferBytesFull;
    fInputPCMSource
      ->getNextFrame(&fInputSampleBuffer[fInputSampleBufferBytesFull],
		     maxBytesToRead,
		     afterGettingFrame, this,
		     FramedSource::handleClosure, this);
  }
}

void AMRAudioEncoder::doStopGettingFrames() {
  fInputPCMSource->stopGettingFrames();
}

void AMRAudioEncoder
::afterGettingFrame(void* clientData, unsigned frameSize,
                    unsigned numTruncatedBytes,
                    struct timeval presentationTime,
                    unsigned durationInMicroseconds) {
  AMRAudioEncoder* source = (AMRAudioEncoder*)clientData;
  source->afterGettingFrame1(frameSize, numTruncatedBytes,
                             presentationTime, durationInMicroseconds);
}

void AMRAudioEncoder
::afterGettingFrame1(unsigned frameSize, unsigned numTruncatedBytes,
                     struct timeval presentationTime, unsigned durationInMicroseconds) {
  // Adjust presentationTime to allow for the data that's still in our buffer:
  int uSecondsAdjustment
    = (int)(fInputSampleBufferBytesFull*fMicrosecondsPerByte);
  presentationTime.tv_sec -= uSecondsAdjustment/MILLION;
  uSecondsAdjustment %= MILLION;
  if (presentationTime.tv_usec < uSecondsAdjustment) {
    --presentationTime.tv_sec;
    presentationTime.tv_usec += MILLION;
  }
  presentationTime.tv_usec -= uSecondsAdjustment;

  // Don't allow the presentation time to decrease:
  if (presentationTime.tv_sec > fLastInputDataPresentationTime.tv_sec ||
      (presentationTime.tv_sec == fLastInputDataPresentationTime.tv_sec &&
       presentationTime.tv_usec > fLastInputDataPresentationTime.tv_usec)) {
    fLastInputDataPresentationTime = presentationTime;
  }
  fInputSampleBufferBytesFull += frameSize;  

  // Try again to encode and deliver data to the sink:
  doGetNextFrame();
}
