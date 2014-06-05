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
// Set up and start multicast streaming
// Implementation

#include "MulticastStreaming.hh"
#include "Options.hh"
#include "AudioRTPCommon.hh"
#include "WISJPEGStreamSource.hh"
#include "MPEG2TransportStreamAccumulator.hh"

// Objects used for multicast streaming:
static Groupsock* rtpGroupsockAudio = NULL;
static Groupsock* rtcpGroupsockAudio = NULL;
static Groupsock* rtpGroupsockVideo = NULL;
static Groupsock* rtcpGroupsockVideo = NULL;
static FramedSource* sourceAudio = NULL;
static RTPSink* sinkAudio = NULL;
static RTCPInstance* rtcpAudio = NULL;
static FramedSource* sourceVideo = NULL;
static RTPSink* sinkVideo = NULL;
static RTCPInstance* rtcpVideo = NULL;

void setupMulticastStreaming(WISInput& inputDevice, ServerMediaSession* sms) {
  UsageEnvironment& env = sms->envir();
  struct in_addr dest; dest.s_addr = multicastAddress;
  const unsigned char ttl = 255;

  // For RTCP:
  const unsigned maxCNAMElen = 100;
  unsigned char CNAME[maxCNAMElen + 1];
  gethostname((char *) CNAME, maxCNAMElen);
  CNAME[maxCNAMElen] = '\0';      // just in case

  /******************audio ***********************/
  if (audioFormat != AFMT_NONE) {
    // Create the audio source:
    sourceAudio = createAudioSource(env, inputDevice.audioSource());

    if (packageFormat != PFMT_TRANSPORT_STREAM) { // there's a separate RTP stream for audio
      // Create 'groupsocks' for RTP and RTCP:
      const Port rtpPortAudio(audioRTPPortNum);
      const Port rtcpPortAudio(audioRTPPortNum+1);
      
      rtpGroupsockAudio = new Groupsock(env, dest, rtpPortAudio, ttl);
      rtcpGroupsockAudio = new Groupsock(env, dest, rtcpPortAudio, ttl);
      
      if (streamingMode == STREAMING_MULTICAST_SSM) {
	rtpGroupsockAudio->multicastSendOnly();
	rtcpGroupsockAudio->multicastSendOnly();
      }
      
      sinkAudio = createAudioRTPSink(env, rtpGroupsockAudio);

      // Create (and start) a 'RTCP instance' for this RTP sink:
      unsigned totalSessionBandwidthAudio = (audioOutputBitrate+500)/1000; // in kbps; for RTCP b/w share
      rtcpAudio = RTCPInstance::createNew(env, rtcpGroupsockAudio,
					  totalSessionBandwidthAudio, CNAME,
					  sinkAudio, NULL /* we're a server */,
					  streamingMode == STREAMING_MULTICAST_SSM);
          // Note: This starts RTCP running automatically

      sms->addSubsession(PassiveServerMediaSubsession::createNew(*sinkAudio, rtcpAudio));

      // Start streaming:
      sinkAudio->startPlaying(*sourceAudio, NULL, NULL);
    }
  }

  /******************video ***********************/
  if (videoFormat != VFMT_NONE) {
    // Create the video source:
    if (packageFormat == PFMT_TRANSPORT_STREAM) {
      MPEG2TransportStreamFromESSource* tsSource
	= MPEG2TransportStreamFromESSource::createNew(env);
      tsSource->addNewVideoSource(inputDevice.videoSource(), 2);
      if (sourceAudio != NULL) tsSource->addNewAudioSource(sourceAudio, 2);
      // Gather the Transport packets into network packet-sized chunks:
      sourceVideo = MPEG2TransportStreamAccumulator::createNew(env, tsSource);
      sourceAudio = NULL;
    } else {
      switch (videoFormat) {
      case VFMT_NONE: // not used
	break;
      case VFMT_MJPEG: {
	sourceVideo = WISJPEGStreamSource::createNew(inputDevice.videoSource());
	break;
      }
      case VFMT_MPEG1:
      case VFMT_MPEG2: {
	sourceVideo = MPEG1or2VideoStreamDiscreteFramer::createNew(env, inputDevice.videoSource());
	break;
      }
      case VFMT_MPEG4: {
	sourceVideo = MPEG4VideoStreamDiscreteFramer::createNew(env, inputDevice.videoSource());
	break;
      }
      }
    }

    // Create 'groupsocks' for RTP and RTCP:
    const Port rtpPortVideo(videoRTPPortNum);
    const Port rtcpPortVideo(videoRTPPortNum+1);

    rtpGroupsockVideo = new Groupsock(env, dest, rtpPortVideo, ttl);
    rtcpGroupsockVideo = new Groupsock(env, dest, rtcpPortVideo, ttl);

    if (streamingMode == STREAMING_MULTICAST_SSM) {
      rtpGroupsockVideo->multicastSendOnly();
      rtcpGroupsockVideo->multicastSendOnly();
    }

    // Create an appropriate 'Video RTP' sink from the RTP 'groupsock':
    unsigned char payloadFormatCode = 97; // if dynamic
    setVideoRTPSinkBufferSize();
    if (packageFormat == PFMT_TRANSPORT_STREAM) {
      sinkVideo = SimpleRTPSink::createNew(env, rtpGroupsockVideo,
					   33, 90000, "video", "mp2t",
					   1, True, False/*no 'M' bit*/);
    } else {
      switch (videoFormat) {
      case VFMT_NONE: // not used
	break;
      case VFMT_MJPEG: {
	sinkVideo = JPEGVideoRTPSink::createNew(env, rtpGroupsockVideo);
	break;
      }
      case VFMT_MPEG1:
      case VFMT_MPEG2: {
	sinkVideo = MPEG1or2VideoRTPSink::createNew(env, rtpGroupsockVideo);
	break;
      }
      case VFMT_MPEG4: {
	sinkVideo = MPEG4ESVideoRTPSink::createNew(env, rtpGroupsockVideo, payloadFormatCode);
	break;
      }
      }
    }

    // Create (and start) a 'RTCP instance' for this RTP sink:
    unsigned totalSessionBandwidthVideo = (videoBitrate+500)/1000; // in kbps; for RTCP b/w share
    rtcpVideo = RTCPInstance::createNew(env, rtcpGroupsockVideo,
					totalSessionBandwidthVideo, CNAME,
					sinkVideo, NULL /* we're a server */ ,
					streamingMode == STREAMING_MULTICAST_SSM);
        // Note: This starts RTCP running automatically

    sms->addSubsession(PassiveServerMediaSubsession::createNew(*sinkVideo, rtcpVideo));

    // Start streaming:
    sinkVideo->startPlaying(*sourceVideo, NULL, NULL);
  }
}

void reclaimMulticastStreaming() {
  Medium::close(rtcpAudio);
  Medium::close(sinkAudio);
  Medium::close(sourceAudio);
  delete rtpGroupsockAudio;
  delete rtcpGroupsockAudio;

  Medium::close(rtcpVideo);
  Medium::close(sinkVideo);
  Medium::close(sourceVideo);
  delete rtpGroupsockVideo;
  delete rtcpGroupsockVideo;
}
