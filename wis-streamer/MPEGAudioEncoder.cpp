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
// "LIVE555 Streaming Media" interface to a MPEG (1 or 2) audio encoder.
// Implementation

#include "MPEGAudioEncoder.hh"
extern "C" {
#include "avcodec.h"
#include "mpegaudio.h"
}

MPEGAudioEncoder* MPEGAudioEncoder::createNew(UsageEnvironment& env,
					      FramedSource* inputSource,
					      unsigned numChannels,
					      unsigned samplingRate,
					      unsigned outputKbps) {
  return new MPEGAudioEncoder(env, inputSource,
			      numChannels, samplingRate, outputKbps);
}

#define MILLION 1000000

MPEGAudioEncoder
::MPEGAudioEncoder(UsageEnvironment& env, FramedSource* inputSource,
		   unsigned numChannels, unsigned samplingRate,
		   unsigned outputKbps)
  : FramedFilter(env, inputSource) {
  AVCodecContext* ctx;
  fCodecContext = ctx = new AVCodecContext;
  ctx->bit_rate = outputKbps*1000;
  ctx->bit_rate_tolerance = ctx->flags = ctx->sub_id = ctx->me_method = 0;
  ctx->extradata = NULL;
  ctx->extradata_size = 0;
  ctx->sample_rate = samplingRate;
  ctx->channels = numChannels;
  ctx->sample_fmt = 0;
  ctx->priv_data = new unsigned char[mp2_encoder.priv_data_size];

  mp2_encoder.init(ctx);

  fFrameDurationInMicroseconds = samplingRate == 0 ? 0
    : ((MPA_FRAME_SIZE*2*MILLION)/samplingRate + 1)/2; // rounds to nearest int
  fMicrosecondsPerByte
    = (1.0*MILLION)/(samplingRate*numChannels*sizeof (unsigned short));
  fInputSampleBufferBytesDesired
    = MPA_FRAME_SIZE*numChannels*sizeof (unsigned short);
      // the number of bytes necessary in order to encode a frame
  fInputSampleBufferSize = 10*fInputSampleBufferBytesDesired;
  fInputSampleBuffer = new unsigned char[fInputSampleBufferSize];
  fInputSampleBufferBytesFull = 0;

  fLastInputDataPresentationTime.tv_sec = 0;
  fLastInputDataPresentationTime.tv_usec = 0;
}

MPEGAudioEncoder::~MPEGAudioEncoder() {
  AVCodecContext* ctx = (AVCodecContext*)fCodecContext;
  delete[] (unsigned char*)(ctx->priv_data);
  delete ctx;

  delete[] fInputSampleBuffer;
}

void MPEGAudioEncoder::doGetNextFrame() {
  // If we have enough samples in order to encode a frame, do this now:
  if (fInputSampleBufferBytesFull >= fInputSampleBufferBytesDesired) {
    if (fMaxSize < MPA_MAX_CODED_FRAME_SIZE) {
      // Our sink hasn't given us enough space for a frame.  We can't encode.
      fFrameSize = 0;
      fNumTruncatedBytes = MPA_MAX_CODED_FRAME_SIZE;
    } else {
      AVCodecContext* ctx = (AVCodecContext*)fCodecContext;
      fFrameSize = mp2_encoder.encode(ctx, fTo, fMaxSize, fInputSampleBuffer);
      fNumTruncatedBytes = 0;

      // Shift any remaining samples down the the start of the buffer:
      fInputSampleBufferBytesFull -= fInputSampleBufferBytesDesired;
      memmove(fInputSampleBuffer,
	      &fInputSampleBuffer[fInputSampleBufferBytesDesired],
	      fInputSampleBufferBytesFull);
    }

    // Complete delivery to the client:
    fPresentationTime = fLastInputDataPresentationTime;
    //fDurationInMicroseconds = fFrameDurationInMicroseconds;
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

void MPEGAudioEncoder
::afterGettingFrame(void* clientData, unsigned frameSize,
                    unsigned numTruncatedBytes,
                    struct timeval presentationTime,
                    unsigned durationInMicroseconds) {
  MPEGAudioEncoder* source = (MPEGAudioEncoder*)clientData;
  source->afterGettingFrame1(frameSize, numTruncatedBytes,
                             presentationTime, durationInMicroseconds);
}

void MPEGAudioEncoder
::afterGettingFrame1(unsigned frameSize, unsigned numTruncatedBytes,
                     struct timeval presentationTime,
                     unsigned durationInMicroseconds) {
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
