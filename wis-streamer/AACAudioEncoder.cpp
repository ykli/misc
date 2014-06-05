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
// Implementation

#include "AACAudioEncoder.hh"
extern "C" {
#include "AACEncoder/faac.h"
}

AACAudioEncoder* AACAudioEncoder
::createNew(UsageEnvironment& env, FramedSource* inputPCMSource,
	    unsigned numChannels, unsigned samplingRate, unsigned outputKbps) {
  return new AACAudioEncoder(env, inputPCMSource, numChannels, samplingRate, outputKbps);
}

#ifndef MILLION
#define MILLION 1000000
#endif

AACAudioEncoder
::AACAudioEncoder(UsageEnvironment& env, FramedSource* inputPCMSource,
		  unsigned numChannels, unsigned samplingRate, unsigned outputKbps)
  : FramedFilter(env, inputPCMSource) {
  fEncoderState = faacEncOpen(samplingRate, numChannels,
			      &fNumSamplesPerFrame, &fMaxEncodedFrameSize);

  // Note: "fNumSamplesPerFrame" is already multiplied by "numChannels"
  fMicrosecondsPerFrame = (MILLION*fNumSamplesPerFrame)/(numChannels*samplingRate);
  fMicrosecondsPerByte
    = (1.0*MILLION)/(samplingRate*numChannels*sizeof (unsigned short));
  fInputSampleBufferBytesDesired = fNumSamplesPerFrame*sizeof (unsigned short);
  fInputSampleBufferSize = 10*fInputSampleBufferBytesDesired;
  fInputSampleBuffer = new unsigned char[fInputSampleBufferSize];
  fInputSampleBufferBytesFull = 0;

  fLastInputDataPresentationTime.tv_sec = 0;
  fLastInputDataPresentationTime.tv_usec = 0;

  // Set remaining parameters of the encoder:
  faacEncConfiguration* config = faacEncGetCurrentConfiguration(fEncoderState);
  config->mpegVersion = MPEG4;
  config->bitRate = outputKbps/numChannels; // parameter is bit rate per channel
  config->bandWidth = 16000; // as specified in "FAAC.bitrate.README"
  config->quantqual = 200; // as specified in "FAAC.bitrate.README"
  config->outputFormat = 0; // Raw
  config->inputFormat = FAAC_INPUT_16BIT;
  faacEncSetConfiguration(fEncoderState, config);
}

AACAudioEncoder::~AACAudioEncoder() {
  faacEncClose(fEncoderState);
  delete[] fInputSampleBuffer;
}

void AACAudioEncoder::doGetNextFrame() {
  // If we have enough samples in order to encode a frame, do this now:
  if (fInputSampleBufferBytesFull >= fInputSampleBufferBytesDesired*2) {
    // Note: The "*2" is because of the faacEncEncoder() 'overlap' bullshit
    if (fMaxSize < fMaxEncodedFrameSize) {
      // Our sink hasn't given us enough space for a frame.  We can't encode.
      fFrameSize = 0;
      fNumTruncatedBytes = fMaxEncodedFrameSize;
    } else {
      fFrameSize = faacEncEncode(fEncoderState,
				 (int16_t*)(&fInputSampleBuffer[fInputSampleBufferBytesDesired]),
				 (int16_t*)fInputSampleBuffer, // overlapBuffer
				 fNumSamplesPerFrame, fTo, fMaxSize);
      fNumTruncatedBytes = 0;

      // Shift the data that we just encoded, plus any remaining samples,
      // down the the start of the buffer.  (The data that we just encoded will be the overlap
      // buffer for the next encoding.)
      fInputSampleBufferBytesFull -= fInputSampleBufferBytesDesired;
      memmove(fInputSampleBuffer,
	      &fInputSampleBuffer[fInputSampleBufferBytesDesired],
	      fInputSampleBufferBytesFull);
    }

    // Complete delivery to the client:
    fPresentationTime = fLastInputDataPresentationTime;
    //fDurationInMicroseconds = fMicrosecondsPerFrame;
    fDurationInMicroseconds = 0; // because audio capture is bursty, check for it ASAP
    afterGetting(this);
  } else {
    // Read more samples from our source, then try again:
    unsigned maxBytesToRead
      = fInputSampleBufferSize - fInputSampleBufferBytesFull;
    fInputSource
      ->getNextFrame(&fInputSampleBuffer[fInputSampleBufferBytesFull],
		     maxBytesToRead,
		     afterGettingFrame, this,
		     FramedSource::handleClosure, this);
  }
}

void AACAudioEncoder
::afterGettingFrame(void* clientData, unsigned frameSize,
                    unsigned numTruncatedBytes,
                    struct timeval presentationTime,
                    unsigned durationInMicroseconds) {
  AACAudioEncoder* source = (AACAudioEncoder*)clientData;
  source->afterGettingFrame1(frameSize, numTruncatedBytes,
                             presentationTime, durationInMicroseconds);
}

void AACAudioEncoder
::afterGettingFrame1(unsigned frameSize, unsigned numTruncatedBytes,
                     struct timeval presentationTime, unsigned durationInMicroseconds) {
  // Adjust presentationTime to allow for the data that's still in our buffer:
  int uSecondsAdjustment = (int)(fInputSampleBufferBytesFull*fMicrosecondsPerByte);
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
