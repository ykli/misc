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
// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand, that stream a MPEG-2 Transport Stream from an input video
// and audio source.
// Implementation  

#include "WISMPEG2TransportStreamServerMediaSubsession.hh"
#include "Options.hh"
#include "MPEGAudioEncoder.hh"
#include "MPEG2TransportStreamAccumulator.hh"
#include <MPEG2TransportStreamFromESSource.hh>
#include <MPEG1or2VideoStreamDiscreteFramer.hh>
#include <uLawAudioFilter.hh>
#include <MPEG1or2AudioRTPSink.hh>
#include <SimpleRTPSink.hh>

WISMPEG2TransportStreamServerMediaSubsession* WISMPEG2TransportStreamServerMediaSubsession
::createNew(UsageEnvironment& env, WISInput& wisInput) {
  return new WISMPEG2TransportStreamServerMediaSubsession(env, wisInput);
}

WISMPEG2TransportStreamServerMediaSubsession
::WISMPEG2TransportStreamServerMediaSubsession(UsageEnvironment& env, WISInput& wisInput)
  : WISServerMediaSubsession(env, wisInput, audioOutputBitrate) {
}

WISMPEG2TransportStreamServerMediaSubsession
::~WISMPEG2TransportStreamServerMediaSubsession() {
}

FramedSource* WISMPEG2TransportStreamServerMediaSubsession
::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) {
  // Begin by creating the Transport Stream source:
  MPEG2TransportStreamFromESSource* tsSource
    = MPEG2TransportStreamFromESSource::createNew(envir());

  // Then create video and audio sources, as desired, and insert them into the
  // Transport Stream:
  estBitrate = 0;
  tsSource->addNewVideoSource(fWISInput.videoSource(), 2);
  estBitrate += 500; // kbps, estimate
  if (fEstimatedKbps/*audio*/ != 0) {
    FramedSource* pcmSource = fWISInput.audioSource();

    FramedSource* audioSource
      = MPEGAudioEncoder::createNew(envir(), pcmSource, audioNumChannels,
				    audioSamplingFrequency, fEstimatedKbps);
    tsSource->addNewAudioSource(audioSource, 2);
    estBitrate += fEstimatedKbps/*audio*/;
  }

  // Gather the Transport packets into network packet-sized chunks:
  return MPEG2TransportStreamAccumulator::createNew(envir(), tsSource);
}

RTPSink* WISMPEG2TransportStreamServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
		   unsigned char /*rtpPayloadTypeIfDynamic*/,
		   FramedSource* /*inputSource*/) {
  setVideoRTPSinkBufferSize();
  return SimpleRTPSink::createNew(envir(), rtpGroupsock,
				  33, 90000, "video", "mp2t",
				  1, True, False/*no 'M' bit*/);
}
