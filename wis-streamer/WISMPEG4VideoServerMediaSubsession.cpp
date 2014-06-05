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
// A "ServerMediaSubsession" subclass for on-demand unicast streaming
// of MPEG-4 video from a WIS GO7007 capture device.
// Implementation

#include "WISMPEG4VideoServerMediaSubsession.hh"
#include <MPEG4ESVideoRTPSink.hh>
#include <MPEG4VideoStreamDiscreteFramer.hh>

WISMPEG4VideoServerMediaSubsession* WISMPEG4VideoServerMediaSubsession
::createNew(UsageEnvironment& env, WISInput& wisInput, unsigned estimatedBitrate) {
  return new WISMPEG4VideoServerMediaSubsession(env, wisInput, estimatedBitrate);
}

WISMPEG4VideoServerMediaSubsession
::WISMPEG4VideoServerMediaSubsession(UsageEnvironment& env, WISInput& wisInput,
				     unsigned estimatedBitrate)
  : WISServerMediaSubsession(env, wisInput, estimatedBitrate) {
}

WISMPEG4VideoServerMediaSubsession::~WISMPEG4VideoServerMediaSubsession() {
}

static void afterPlayingDummy(void* clientData) {
  WISMPEG4VideoServerMediaSubsession* subsess
    = (WISMPEG4VideoServerMediaSubsession*)clientData;
  // Signal the event loop that we're done:
  subsess->setDoneFlag();
}

static void checkForAuxSDPLine(void* clientData) {
  WISMPEG4VideoServerMediaSubsession* subsess
    = (WISMPEG4VideoServerMediaSubsession*)clientData;
  subsess->checkForAuxSDPLine1();
}

void WISMPEG4VideoServerMediaSubsession::checkForAuxSDPLine1() {
  if (fDummyRTPSink->auxSDPLine() != NULL) {
    // Signal the event loop that we're done:
    setDoneFlag();
  } else {
    // try again after a brief delay:
    int uSecsToDelay = 100000; // 100 ms
    nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
			      (TaskFunc*)checkForAuxSDPLine, this);
  }
}

char const* WISMPEG4VideoServerMediaSubsession
::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource) {
  // Note: For MPEG-4 video buffer, the 'config' information isn't known
  // until we start reading the Buffer.  This means that "rtpSink"s
  // "auxSDPLine()" will be NULL initially, and we need to start reading
  // data from our buffer until this changes.
  fDummyRTPSink = rtpSink;
    
  // Start reading the buffer:
  fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);
    
  // Check whether the sink's 'auxSDPLine()' is ready:
  checkForAuxSDPLine(this);
    
  fDoneFlag = 0;
  envir().taskScheduler().doEventLoop(&fDoneFlag);
    
  char const* auxSDPLine = fDummyRTPSink->auxSDPLine();
  return auxSDPLine;
  //return NULL;
}

FramedSource* WISMPEG4VideoServerMediaSubsession
::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) {
  estBitrate = fEstimatedKbps;

  // Create a framer for the Video Elementary Stream:
  return MPEG4VideoStreamDiscreteFramer::createNew(envir(), fWISInput.videoSource());
}

RTPSink* WISMPEG4VideoServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
		   unsigned char rtpPayloadTypeIfDynamic,
		   FramedSource* /*inputSource*/) {
  setVideoRTPSinkBufferSize();
  return MPEG4ESVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}
