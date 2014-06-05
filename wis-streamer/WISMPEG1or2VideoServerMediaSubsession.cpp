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

#include "WISMPEG1or2VideoServerMediaSubsession.hh"
#include <MPEG1or2VideoStreamDiscreteFramer.hh>
#include <MPEG1or2VideoRTPSink.hh>

WISMPEG1or2VideoServerMediaSubsession*
WISMPEG1or2VideoServerMediaSubsession
::createNew(UsageEnvironment& env, WISInput& wisInput, unsigned estimatedBitrate,
	    Boolean iFramesOnly,
	    double vshPeriod) {
  return new WISMPEG1or2VideoServerMediaSubsession(env, wisInput, estimatedBitrate,
						   iFramesOnly, vshPeriod);
}

WISMPEG1or2VideoServerMediaSubsession
::WISMPEG1or2VideoServerMediaSubsession(UsageEnvironment& env, WISInput& wisInput,
					unsigned estimatedBitrate,
					Boolean iFramesOnly, double vshPeriod)
  : WISServerMediaSubsession(env, wisInput, estimatedBitrate),
    fIFramesOnly(iFramesOnly), fVSHPeriod(vshPeriod) {
}

WISMPEG1or2VideoServerMediaSubsession
::~WISMPEG1or2VideoServerMediaSubsession() {
}

FramedSource* WISMPEG1or2VideoServerMediaSubsession
::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) {
  estBitrate = fEstimatedKbps;

  // Create a framer for the Video Elementary Stream:
  return MPEG1or2VideoStreamDiscreteFramer::createNew(envir(), fWISInput.videoSource());
}

RTPSink* WISMPEG1or2VideoServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
		   unsigned char /*rtpPayloadTypeIfDynamic*/,
		   FramedSource* /*inputSource*/) {
  setVideoRTPSinkBufferSize();
  return MPEG1or2VideoRTPSink::createNew(envir(), rtpGroupsock);
}
