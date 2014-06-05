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
//  Collects a stream of incoming MPEG Transport Stream packets into
//  a chunk sufficiently large to send in a single outgoing (RTP or UDP) packet.
// Implementation

#include "MPEG2TransportStreamAccumulator.hh"

MPEG2TransportStreamAccumulator*
MPEG2TransportStreamAccumulator::createNew(UsageEnvironment& env,
					   FramedSource* inputSource) {
  return new MPEG2TransportStreamAccumulator(env, inputSource);
}

MPEG2TransportStreamAccumulator
::MPEG2TransportStreamAccumulator(UsageEnvironment& env, FramedSource* inputSource)
  : FramedFilter(env, inputSource),
    fNumBytesGathered(0) {
}

MPEG2TransportStreamAccumulator::~MPEG2TransportStreamAccumulator() {
}

#define DESIRED_PACKET_SIZE (7*188)

void MPEG2TransportStreamAccumulator::doGetNextFrame() {
  if (fNumBytesGathered >= DESIRED_PACKET_SIZE) {
    // Complete the delivery to the client:
    fFrameSize = fNumBytesGathered;
    fNumBytesGathered = 0;
    afterGetting(this);
  } else {
    // Ask for more data (delivered directly to the client's buffer);
    fInputSource->getNextFrame(fTo, fMaxSize, afterGettingFrame, this,
			       FramedSource::handleClosure, this);
  }
}

void MPEG2TransportStreamAccumulator
::afterGettingFrame(void* clientData, unsigned frameSize,
		    unsigned numTruncatedBytes,
		    struct timeval presentationTime,
		    unsigned durationInMicroseconds) {
  MPEG2TransportStreamAccumulator* accumulator
    = (MPEG2TransportStreamAccumulator*)clientData;
  accumulator->afterGettingFrame1(frameSize, numTruncatedBytes,
				  presentationTime, durationInMicroseconds);
}

void MPEG2TransportStreamAccumulator
::afterGettingFrame1(unsigned frameSize,
		     unsigned numTruncatedBytes,
		     struct timeval presentationTime,
		     unsigned durationInMicroseconds) {
  if (fNumBytesGathered == 0) { // this is the first frame of the new chunk
    fPresentationTime = presentationTime;
    fDurationInMicroseconds = 0;
  }
  fNumBytesGathered += frameSize;
  fTo += frameSize;
  fMaxSize -= frameSize;
  fDurationInMicroseconds += durationInMicroseconds;

  // Try again to complete delivery:
  doGetNextFrame();
}
