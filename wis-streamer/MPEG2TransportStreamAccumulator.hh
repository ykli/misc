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
// C++ header

#ifndef  _MPEG2_TRANSPORT_STREAM_ACCUMULATOR_HH
#define _MPEG_TRANSPORT_STREAM_ACCUMULATOR_HH

#include "FramedFilter.hh"

class MPEG2TransportStreamAccumulator: public FramedFilter {
public:
  static MPEG2TransportStreamAccumulator* createNew(UsageEnvironment& env,
						    FramedSource* inputSource);

protected:
  MPEG2TransportStreamAccumulator(UsageEnvironment& env, FramedSource* inputSource);
      // called only by createNew()
  virtual ~MPEG2TransportStreamAccumulator();

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
  unsigned fNumBytesGathered;
};

#endif
