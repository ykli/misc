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
// of MPEG-1 or 2 video from a WIS GO7007 capture device.
// Implementation

#include "WISH264VideoServerMediaSubsession.hh"
#include "H264VideoRTPSink.hh"
#include "H264VideoEncoder.hh"
#include <H264VideoStreamFramer.hh>
#include "Options.hh"

WISH264VideoServerMediaSubsession*
WISH264VideoServerMediaSubsession
::createNew(UsageEnvironment& env, WISInput& wisInput, unsigned estimatedBitrate) {
  return new WISH264VideoServerMediaSubsession(env, wisInput, estimatedBitrate);
}

WISH264VideoServerMediaSubsession
::WISH264VideoServerMediaSubsession(UsageEnvironment& env, WISInput& wisInput,
					unsigned estimatedBitrate)
	: WISServerMediaSubsession(env, wisInput, estimatedBitrate),
	  fAuxSDPLine(NULL), fDoneFlag(0), fDummyRTPSink(NULL) {
}

WISH264VideoServerMediaSubsession
::~WISH264VideoServerMediaSubsession() {
  delete[] fAuxSDPLine;
}

static void afterPlayingDummy(void* clientData) {
  WISH264VideoServerMediaSubsession* subsess = (WISH264VideoServerMediaSubsession*)clientData;
  subsess->afterPlayingDummy1();
}

void WISH264VideoServerMediaSubsession::afterPlayingDummy1() {
  // Unschedule any pending 'checking' task:
  envir().taskScheduler().unscheduleDelayedTask(nextTask());
  // Signal the event loop that we're done:
  setDoneFlag();
}

static void checkForAuxSDPLine(void* clientData) {
  WISH264VideoServerMediaSubsession* subsess = (WISH264VideoServerMediaSubsession*)clientData;
  subsess->checkForAuxSDPLine1();
}

void WISH264VideoServerMediaSubsession::checkForAuxSDPLine1() {
  char const* dasl;

  if (fAuxSDPLine != NULL) {
    // Signal the event loop that we're done:
    setDoneFlag();
  } else if (fDummyRTPSink != NULL && (dasl = fDummyRTPSink->auxSDPLine()) != NULL) {
    fAuxSDPLine = strDup(dasl);
    fDummyRTPSink = NULL;

    // Signal the event loop that we're done:
    setDoneFlag();

  } else if (!fDoneFlag) {
    // try again after a brief delay:
    int uSecsToDelay = 100000; // 100 ms
    nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
			      (TaskFunc*)checkForAuxSDPLine, this);
  }
}

char const* WISH264VideoServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource) {
  if (fAuxSDPLine != NULL) return fAuxSDPLine; // it's already been set up (for a previous client)

  if (fDummyRTPSink == NULL) { // we're not already setting it up for another, concurrent stream
    // Note: For H264 video files, the 'config' information ("profile-level-id" and "sprop-parameter-sets") isn't known
    // until we start reading the file.  This means that "rtpSink"s "auxSDPLine()" will be NULL initially,
    // and we need to start reading data from our file until this changes.
    fDummyRTPSink = rtpSink;

    // Start reading the file:
    fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);

    // Check whether the sink's 'auxSDPLine()' is ready:
    checkForAuxSDPLine(this);
  }
  envir().taskScheduler().doEventLoop(&fDoneFlag);

  return fAuxSDPLine;
}

FramedSource* WISH264VideoServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) {
  H264VideoEncoder *encoder;
  estBitrate = fEstimatedKbps;

  IVSFramedFilter *Filter = IVSFramedFilter::createNew(envir(), fWISInput.videoSource());
  encoder = H264VideoEncoder::createNew(envir(), Filter);

  // Create a framer for the Video Elementary Stream:
  return H264VideoStreamFramer::createNew(envir(), encoder);
}

RTPSink* WISH264VideoServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
		   unsigned char rtpPayloadTypeIfDynamic,
		   FramedSource* /*inputSource*/) {
  setVideoRTPSinkBufferSize();
  return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}
