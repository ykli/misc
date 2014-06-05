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
// Common routines for setting up audio RTP streams
// Implementation

#include "AudioRTPCommon.hh"
#include "Options.hh"
#include "WISInput.hh"
#include "MPEGAudioEncoder.hh"
#include "AMRAudioEncoder.hh"
#include "AACAudioEncoder.hh"

FramedSource* createAudioSource(UsageEnvironment& env, FramedSource* pcmSource) {
  FramedSource* audioSource;

  // Add in any filter necessary to transform the data prior to streaming:
  if (audioFormat == AFMT_PCM_ULAW) { // stream u-law
    // Add a filter that converts from raw 16-bit PCM audio
    // to 8-bit u-law audio:
    audioSource = uLawFromPCMAudioSource::createNew(env, pcmSource);
  } else if (audioFormat == AFMT_PCM_RAW16) { // stream raw PCM
    if (PCM_AUDIO_IS_LITTLE_ENDIAN) {
      // The 16-bit samples are in little-endian order.  Add a filter
      // that converts them to network (i.e., big-endian) order:
      audioSource = EndianSwap16::createNew(env, pcmSource);
    } else {
      // The 16-bit samples are already in big-endian order:
      audioSource = pcmSource;
    }
  } else if (audioFormat == AFMT_MPEG2) { // stream MPEG-2 audio
    // Create a software filter that will encode the PCM audio source to MPEG:
    audioSource = MPEGAudioEncoder
      ::createNew(env, pcmSource,
		  audioNumChannels, audioSamplingFrequency, audioOutputBitrate/1000);
  } else if (audioFormat == AFMT_AMR) { // stream AMR audio
    // Create a software filter that will encode the PCM audio source to AMR:
    audioSource = AMRAudioEncoder::createNew(env, pcmSource, audioNumChannels);
  } else { // AFMT_AAC: stream AAC audio
    // Create a software filter that will encode the PCM audio source to AAC:
    audioSource = AACAudioEncoder
      ::createNew(env, pcmSource,
		  audioNumChannels, audioSamplingFrequency, audioOutputBitrate/1000);
  }

  return audioSource;
}

RTPSink* createAudioRTPSink(UsageEnvironment& env, Groupsock* rtpGroupsockAudio,
			    unsigned char rtpPayloadTypeIfDynamic) {
  setAudioRTPSinkBufferSize();

  RTPSink* audioSink;
  unsigned char payloadFormatCode = rtpPayloadTypeIfDynamic; // if dynamic
  
  if (audioFormat == AFMT_MPEG2) { // stream MPEG audio
    // Create a 'MPEG (1 or 2) audio RTP sink from the RTP 'groupsock':
    audioSink = MPEG1or2AudioRTPSink::createNew(env, rtpGroupsockAudio);
  } else if (audioFormat == AFMT_AMR) { // stream AMR audio
    audioSink = AMRAudioRTPSink::createNew(env, rtpGroupsockAudio,
					   payloadFormatCode, False, audioNumChannels);
  } else if (audioFormat == AFMT_AAC) { // stream AAC audio
    char const* encoderConfigStr = audioNumChannels == 2 ? "1210": "1208";
    audioSink = MPEG4GenericRTPSink::createNew(env, rtpGroupsockAudio,
					       payloadFormatCode,
					       audioSamplingFrequency,
					       "audio", "AAC-hbr",
					       encoderConfigStr, audioNumChannels);
  } else { // stream (raw or u-law) PCM
    // Create a 'Simple RTP' sink from the RTP 'groupsock' (to stream raw or u-law PCM):
    char* mimeType;
    audioOutputBitrate = audioSamplingFrequency*16/*bits-per-sample*/*audioNumChannels;
    if (audioFormat == AFMT_PCM_ULAW) { // stream u-law
      mimeType = "PCMU";
      if (audioSamplingFrequency == 8000 && audioNumChannels == 1) {
	payloadFormatCode = 0; // a static RTP payload type
      }
      audioOutputBitrate /= 2;
    } else { // stream raw PCM
      mimeType = "L16";
      if (audioSamplingFrequency == 44100 && audioNumChannels == 2) {
	payloadFormatCode = 10; // a static RTP payload type
      } else if (audioSamplingFrequency == 44100 && audioNumChannels == 1) {
	payloadFormatCode = 11; // a static RTP payload type
      }
    }
    
    audioSink = SimpleRTPSink::createNew(env, rtpGroupsockAudio, payloadFormatCode,
					 audioSamplingFrequency, "audio",
					 mimeType, audioNumChannels);
  }

  return audioSink;
}
