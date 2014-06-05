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
// Set up and start streaming through a Darwin Streaming Server
// Implementation

#include "DarwinStreaming.hh"
#include "Options.hh"
#include "AudioRTPCommon.hh"
#include "WISJPEGStreamSource.hh"
#include "MPEG2TransportStreamAccumulator.hh"

// Objects used for streaming through Darwin:
static char const* const applicationName = "wis-streamer";
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
static DarwinInjector* injector;

void setupDarwinStreaming(UsageEnvironment& env, WISInput& inputDevice) {
  // Create a 'Darwin injector' object:
  injector = DarwinInjector::createNew(env, applicationName);

  // For RTCP:
  const unsigned maxCNAMElen = 100;
  unsigned char CNAME[maxCNAMElen + 1];
  gethostname((char *) CNAME, maxCNAMElen);
  CNAME[maxCNAMElen] = '\0';      // just in case

  /******************audio***********************/
  if (audioFormat != AFMT_NONE) {
    // Create the audio source:
    sourceAudio = createAudioSource(env, inputDevice.audioSource());

    if (packageFormat != PFMT_TRANSPORT_STREAM) { // there's a separate RTP stream for audio
      // Create 'groupsocks' for RTP and RTCP.
      // (Note: Because we will actually be streaming through a remote Darwin server,
      // via TCP, we just use dummy destination addresses, port numbers, and TTLs here.)
      struct in_addr dummyDestAddress;
      dummyDestAddress.s_addr = 0;
      rtpGroupsockAudio = new Groupsock(env, dummyDestAddress, 0, 0);
      rtcpGroupsockAudio = new Groupsock(env, dummyDestAddress, 0, 0);
      
      // Create a RTP sink for the audio stream:
      sinkAudio = createAudioRTPSink(env, rtpGroupsockAudio);

      // Create (and start) a 'RTCP instance' for this RTP sink:
      unsigned totalSessionBandwidthAudio = (audioOutputBitrate+500)/1000; // in kbps; for RTCP b/w share
      rtcpAudio = RTCPInstance::createNew(env, rtcpGroupsockAudio,
					  totalSessionBandwidthAudio, CNAME,
					  sinkAudio, NULL /* we're a server */);
          // Note: This starts RTCP running automatically

      // Add these to our 'Darwin injector':
      injector->addStream(sinkAudio, rtcpAudio);
    }
  }
  /******************end audio***********************/

  /******************video***********************/
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

    // Create 'groupsocks' for RTP and RTCP.
    // (Note: Because we will actually be streaming through a remote Darwin server,
    // via TCP, we just use dummy destination addresses, port numbers, and TTLs here.)
    struct in_addr dummyDestAddress;
    dummyDestAddress.s_addr = 0;
    rtpGroupsockVideo = new Groupsock(env, dummyDestAddress, 0, 0);
    rtcpGroupsockVideo = new Groupsock(env, dummyDestAddress, 0, 0);

    // Create a RTP sink for the video stream:
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
					sinkVideo, NULL /* we're a server */);
        // Note: This starts RTCP running automatically

    // Add these to our 'Darwin injector':
    injector->addStream(sinkVideo, rtcpVideo);
  }
  /******************end video***********************/

  // Next, specify the destination Darwin Streaming Server:
  char const* remoteStreamName = "test.sdp";//#####@@@@@
  if (!injector->setDestination(remoteDSSNameOrAddress, remoteStreamName,
                                applicationName, "LIVE555 Streaming Media")) {
    env << "Failed to connect to remote Darwin Streaming Server: " << env.getResultMsg() << "\n";
    exit(1);
  }

  env << "Play this stream (from the Darwin Streaming Server) using the URL:\n"
       << "\trtsp://" << remoteDSSNameOrAddress << "/" << remoteStreamName << "\n";

}

//#####@@@@@
#if 0
void reclaimDarwinStreaming() {
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
#endif
