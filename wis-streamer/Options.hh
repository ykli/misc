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
// Option variables, modifiable from the command line:
// Header file

#ifndef _OPTIONS_HH
#define _OPTIONS_HH

#include <RTSPServer.hh>
#ifndef _MEDIA_FORMAT_HH
#include "MediaFormat.hh"
#endif

extern portNumBits rtspServerPortNum;
extern char* streamDescription;
extern UserAuthenticationDatabase* authDB;
extern char* remoteDSSNameOrAddress;

extern PackageFormat packageFormat;

extern VideoFormat videoFormat;
extern int videoWidth;
extern int videoHeight;
extern int videoBitrate;
extern int videoQuant;
extern int videoGopsize;
extern int videoBframe;
extern unsigned long long videoType;
extern int videoInputDeviceNumber;
extern int videoFrameRateNumerator;
extern int videoFrameRateDenominator;

extern AudioFormat audioFormat;
extern unsigned audioSamplingFrequency;
extern unsigned audioNumChannels;
extern unsigned audioOutputBitrate; // if we're encoding to MPEG audio

extern int tvFreq;

extern int const useDefaultValue, invalidValue;
extern int videoInputBrightness, videoInputContrast, videoInputSaturation, videoInputHue;

enum  StreamingMode {
  STREAMING_UNICAST,
  STREAMING_UNICAST_THROUGH_DARWIN,
  STREAMING_MULTICAST_ASM,
  STREAMING_MULTICAST_SSM
};

extern StreamingMode streamingMode;
extern netAddressBits multicastAddress;
// The following options are used only if "streamingMode" is not STREAMING_UNICAST*
extern portNumBits videoRTPPortNum;
extern portNumBits audioRTPPortNum;

extern void checkArgs(UsageEnvironment& env, int argc, char** argv);
extern void reclaimArgs();

#define PCM_AUDIO_IS_LITTLE_ENDIAN 1

extern int moveDetectEn;
extern int refreshMax;
extern int showFPS;
extern int ivsDiffThres;
#endif
