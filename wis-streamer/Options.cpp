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
// Implementation

#include "Options.hh"
#include "TV.hh"
#include "Err.hh"
#include <GroupsockHelper.hh>
#include <getopt.h>
#ifndef __LINUX_VIDEODEV_H
#include "videodev2.h"
#endif

// Initialize option variables to default values:

portNumBits rtspServerPortNum = 8554;
char* streamDescription = strDup("RTSP/RTP stream from Ingenic Media");
UserAuthenticationDatabase* authDB = NULL;
char* remoteDSSNameOrAddress = NULL;

PackageFormat packageFormat = PFMT_SEPARATE_STREAMS;

VideoFormat videoFormat = VFMT_H264;
int videoWidth = 640;
int videoHeight = 480;
int videoBitrate = 5000;
int videoQuant = 0;
int videoBframe = 0;
int videoGopsize = 100;
int moveDetectEn = 0;
int refreshMax = 5;
int showFPS = 0;
int ivsDiffThres = -1;

static struct {
	char const* typeName;
	int typeTag;
	unsigned frameRateNumerator;
	unsigned frameRateDenominator;
} allowedVideoTypes[] = {
	{"ntsc", V4L2_STD_NTSC_M, 30000, 1001}, // the first entry (index 0) is the default
	{"pal", V4L2_STD_PAL, 25025, 1001},
	{"secam", V4L2_STD_SECAM, 25025, 1001},
	{"pal-bg", V4L2_STD_PAL_BG, 25025, 1001},
	{"pal-i", V4L2_STD_PAL_I, 25025, 1001},
	{"pal-dk", V4L2_STD_PAL_DK, 25025, 1001},
	{"secam-l", V4L2_STD_SECAM_L, 25025, 1001},
	{"ntsc-j", V4L2_STD_NTSC_M_JP, 30000, 1001},
	{NULL, -1} // to mark the end of the list
};
unsigned long long videoType = allowedVideoTypes[0].typeTag;

int videoInputDeviceNumber = 0;

int videoFrameRateNumerator = 0; // changed later
int videoFrameRateDenominator = 0; // ditto

AudioFormat audioFormat = AFMT_PCM_RAW16;
unsigned audioSamplingFrequency = 48000;
unsigned audioNumChannels = 2;
unsigned audioOutputBitrate = 0; // default: we're not encoding to MPEG audio

int tvFreq = -1; // default value => don't use TV tuner

int const useDefaultValue = 0xFEEDFACE;
int const invalidValue = 0xDEADBEEF;
// the above values are assumed never to be valid for:
int videoInputBrightness = useDefaultValue;
int videoInputContrast = useDefaultValue;
int videoInputSaturation = useDefaultValue;
int videoInputHue = useDefaultValue;

static int strToInt(char const* str) {
	int val;
	if (sscanf(str, "%d", &val) == 1) return val;
	return invalidValue;
}

StreamingMode streamingMode = STREAMING_UNICAST;
netAddressBits multicastAddress = 0;
portNumBits videoRTPPortNum = 6000;
portNumBits audioRTPPortNum = 6002;

void checkArgs(UsageEnvironment& env, int argc, char** argv) {
	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
			// packageFormat:
			{"mpegtransport", 1, 0, 0},

			// videoFormat:
			{"nv", 0, 0, 0},
			{"mjpeg", 0, 0, 0},
			{"mpeg1", 0, 0, 0},
			{"mpeg2", 0, 0, 0},
			{"mpeg4", 0, 0, 0},

			// audioFormat:
			{"na", 0, 0, 0},
			{"pcm", 0, 0, 0},
			{"ulaw", 0, 0, 0},
			{"mpegaudio", 1, 0, 0},
			{"amr", 0, 0, 0},
			{"aac", 1, 0, 0},

			// video input parameters
			{"brightness", 1, 0, 0},
			{"contrast", 1, 0, 0},
			{"saturation", 1, 0, 0},
			{"hue", 1, 0, 0},

			// IVS Options
			{"movedet", 2, 0, 0},
			{"diff_threshold", 1, 0, 0},

			// Debug
			{"showfps", 0, 0, 0},

			{0, 0, 0, 0}
		};

		int c = getopt_long_only(argc, argv, "p:D:u:w:h:r:q:b:g:t:i:R:f:Mc:md:A:v:a:",
								 long_options, &option_index);
		if (c == -1) break;

		switch (c) {
		case 0: {
			// Process 'long' options:
			char const* option = long_options[option_index].name;

			// packageFormat:
			if (strcmp(option, "mpegtransport") == 0) {
				int bitrateArg = strToInt(optarg);
				if (bitrateArg == invalidValue || bitrateArg < 0) {
					err(env) << "Invalid MPEG audio bitrate (kbps) argument: " << optarg << "\n";
					break;
				}
				packageFormat = PFMT_TRANSPORT_STREAM;
				videoFormat = VFMT_MPEG2;
				audioOutputBitrate = (unsigned)(bitrateArg*1000);
				if (audioOutputBitrate == 0) {
					audioFormat = AFMT_NONE;
				} else {
					audioFormat = AFMT_MPEG2;
				}
			}

			// videoFormat:
			if (strcmp(option, "nv") == 0) videoFormat = VFMT_NONE;
			else if (strcmp(option, "mjpeg") == 0) videoFormat = VFMT_MJPEG;
			else if (strcmp(option, "mpeg1") == 0) videoFormat = VFMT_MPEG1;
			else if (strcmp(option, "mpeg2") == 0) videoFormat = VFMT_MPEG2;
			else if (strcmp(option, "mpeg4") == 0) videoFormat = VFMT_MPEG4;

			// audioFormat:
			else if (strcmp(option, "na") == 0) {
				audioFormat = AFMT_NONE;
				audioOutputBitrate = 0;
			} else if (strcmp(option, "pcm") == 0) audioFormat = AFMT_PCM_RAW16;
			else if (strcmp(option, "ulaw") == 0) audioFormat = AFMT_PCM_ULAW;
			else if (strcmp(option, "mpegaudio") == 0) {
				int bitrateArg = strToInt(optarg);
				if (bitrateArg == invalidValue || bitrateArg <= 0) {
					err(env) << "Invalid MPEG audio bitrate (kbps) argument: " << optarg << "\n";
					break;
				}
				audioFormat = AFMT_MPEG2;
				audioOutputBitrate = (unsigned)(bitrateArg*1000);
			} else if (strcmp(option, "amr") == 0) {
				audioFormat = AFMT_AMR;
				audioSamplingFrequency = 8000;
				audioNumChannels = 1; // we currently support mono-only for streaming
			} else if (strcmp(option, "aac") == 0) {
				int bitrateArg = strToInt(optarg);
				if (bitrateArg == invalidValue || bitrateArg <= 0) {
					err(env) << "Invalid AAC audio bitrate (kbps) argument: " << optarg << "\n";
					break;
				}
				audioFormat = AFMT_AAC;
				audioSamplingFrequency = 48000;
				audioOutputBitrate = (unsigned)(bitrateArg*1000);
			}

			// video input parameters
			else if (strcmp(option, "brightness") == 0) videoInputBrightness = strToInt(optarg);
			else if (strcmp(option, "contrast") == 0) videoInputContrast = strToInt(optarg);
			else if (strcmp(option, "saturation") == 0) videoInputSaturation = strToInt(optarg);
			else if (strcmp(option, "hue") == 0) videoInputHue = strToInt(optarg);

			// IVS Options
			else if (strcmp(option, "movedet") == 0) {
				moveDetectEn = 1;
				printf("movedetect = %s\n", optarg);
				if (optarg)
					refreshMax = strToInt(optarg);
			}
			else if (strcmp(option, "diff_threshold") == 0) {
				moveDetectEn = 1;
				ivsDiffThres = strToInt(optarg);
			}

			// Debug
			if (strcmp(option, "showfps") == 0) showFPS = 1;

			break;
		}

			// rtspServerPortNum:
		case 'p': {
			int rtspServerPortNumArg;
			if (sscanf(optarg, "%d", &rtspServerPortNumArg) == 1
				&& rtspServerPortNumArg > 0 && rtspServerPortNumArg < 65536) {
				rtspServerPortNum = rtspServerPortNumArg;
			} else {
				err(env) << "Invalid RTSP port num argument: " << optarg << "\n";
			}
			break;
		}

			// streamDescription:
		case 'D': {
			streamDescription = strDup(optarg);
			break;
		}

			// authDB:
		case 'u': {
			// Parse "optarg" for a colon, indicating username:password 
			char* username = optarg;
			char* password;
			for (password = optarg; *password != '\0'; ++password) {
				if (*password == ':') {
					*password++ = '\0';
					break;
				}
			}
			if (authDB == NULL) authDB = new UserAuthenticationDatabase(streamDescription);
			authDB->addUserRecord(username, password);
			break;
		}

			// videoWidth:
		case 'w': {
			int videoWidthArg;
			if (sscanf(optarg, "%d", &videoWidthArg) == 1
				&& videoWidthArg > 0) {
				videoWidth = videoWidthArg;
			} else {
				err(env) << "Invalid video width argument: " << optarg << "\n";
			}
			break;
		}

			// videoHeight:
		case 'h': {
			int videoHeightArg;
			if (sscanf(optarg, "%d", &videoHeightArg) == 1
				&& videoHeightArg > 0) {
				videoHeight = videoHeightArg;
			} else {
				err(env) << "Invalid video height argument: " << optarg << "\n";
			}
			break;
		}

			// videoBitrate:
		case 'r': {
			int videoBitrateArg;
			if (sscanf(optarg, "%d", &videoBitrateArg) == 1
				&& videoBitrateArg > 0) {
				videoBitrate = videoBitrateArg;
			} else {
				err(env) << "Invalid video bitrate argument: " << optarg << "\n";
			}
			break;
		}

			// videoQuant:
		case 'q': {
			int videoQuantArg;
			if (sscanf(optarg, "%d", &videoQuantArg) == 1
				&& videoQuantArg > 0) {
				videoQuant = videoQuantArg;
			} else {
				err(env) << "Invalid video quant argument: " << optarg << "\n";
			}
			break;
		}

			// videoBframe:
		case 'b': {
			if (strcasecmp(optarg, "ibp") == 0)
				videoBframe = 1;
			else if (strcasecmp(optarg, "ibbpbb") == 0)
				videoBframe = 2;
			break;
		}

			// videoGopsize:
		case 'g': {
			int videoGopArg;
			if (sscanf(optarg, "%d", &videoGopArg) == 1
				&& videoGopArg > 0) {
				videoGopsize = videoGopArg;
			} else {
				err(env) << "Invalid video gop size argument: " << optarg << "\n";
			}
			break;
		}

			// videoType:
		case 't': {
			int i;
			for (i = 0; allowedVideoTypes[i].typeName != NULL; ++i) {
				if (strcasecmp(optarg, allowedVideoTypes[i].typeName) == 0) {
					videoType = allowedVideoTypes[i].typeTag;
					if (videoFrameRateNumerator == 0 && videoFrameRateDenominator == 0) {
						// The video frame rate has not been set on the command line, so
						// set it to a default value now:
						videoFrameRateNumerator = allowedVideoTypes[i].frameRateNumerator;
						videoFrameRateDenominator = allowedVideoTypes[i].frameRateDenominator;
					}
					break;
				}
			}
			if (allowedVideoTypes[i].typeName == NULL) { // no valid type was specified 
				err(env) << "Invalid video type argument: " << optarg << "\n";
				env << "\tValid video types are: ";
				for (i = 0; allowedVideoTypes[i].typeName != NULL; ++i) {
					env << "\"" << allowedVideoTypes[i].typeName << "\""
						<< (allowedVideoTypes[i+1].typeName == NULL ? "" : ", ");
				}
				env << "\n";
			}
			break;
		}

			// videoInputDeviceNumber:
		case 'i': {
			int videoInputDeviceNumberArg;
			if (sscanf(optarg, "%d", &videoInputDeviceNumberArg) == 1
				&& videoInputDeviceNumberArg >= 0) {
				videoInputDeviceNumber = videoInputDeviceNumberArg;
			} else {
				err(env) << "Invalid video input device number argument: " << optarg << "\n";
			}
			break;
		}

			// videoFrameRateNumerator/Denominator
		case 'R': {
			int videoFrameRateNumberArg, videoFrameRateNumeratorArg, videoFrameRateDenominatorArg;
			if (sscanf(optarg, "%d/%d", &videoFrameRateNumeratorArg, &videoFrameRateDenominatorArg) == 2
				&& videoFrameRateNumeratorArg > 0 && videoFrameRateDenominatorArg > 0) {
				videoFrameRateNumerator = videoFrameRateNumeratorArg;
				videoFrameRateDenominator = videoFrameRateDenominatorArg;
			} else if (sscanf(optarg, "%d", &videoFrameRateNumberArg) == 1
					   && videoFrameRateNumberArg > 0) {
				videoFrameRateNumerator = videoFrameRateNumberArg;
				videoFrameRateDenominator = 1;
			} else {
				err(env) << "Invalid video frame rate argument: " << optarg << "\n";
			}
			break;
		}

			// audioSamplingFrequency:
		case 'f': {
			int audioSamplingFrequencyArg;
			if (sscanf(optarg, "%d", &audioSamplingFrequencyArg) == 1
				&& audioSamplingFrequencyArg > 0) {
				if (audioFormat == AFMT_AMR && audioSamplingFrequency != 8000) {
					warn(env) << "Ignoring audio sampling frequency argument: " << optarg << "Hz; AMR audio uses 8000 Hz\n";
				} else {
					audioSamplingFrequency = audioSamplingFrequencyArg;
				}
			} else {
				err(env) << "Invalid audio sampling frequency argument: " << optarg << "\n";
			}
			break;
		}

			// audioNumChannels
		case 'M': {
			audioNumChannels = 1;
			break;
		}

			// streamingMode:
		case 'm': {
			streamingMode = STREAMING_MULTICAST_SSM;
			break;
		}
		case 'd': {
			streamingMode = STREAMING_UNICAST_THROUGH_DARWIN;
			remoteDSSNameOrAddress = strDup(optarg);
			break;
		}

			// multicastAddress:
		case 'A': {
			netAddressBits addr = our_inet_addr(optarg);
			if (IsMulticastAddress(addr)) {
				multicastAddress = addr;
			} else {
				err(env) << "Invalid multicast address: " << optarg << "\n";
			}
			break;
		}

			// videoRTPPortNum:
		case 'v': {
			int videoRTPPortNumArg;
			if (sscanf(optarg, "%d", &videoRTPPortNumArg) == 1
				&& videoRTPPortNumArg > 0 && videoRTPPortNumArg < 65536
				&& (videoRTPPortNumArg&1) == 0) {
				videoRTPPortNum = videoRTPPortNumArg;
			} else {
				err(env) << "Invalid video RTP port num argument: " << optarg;
				if ((videoRTPPortNumArg&1) != 0) env << " (must be even)";
				env << "\n";
			}
			break;
		}

			// audioRTPPortNum:
		case 'a': {
			int audioRTPPortNumArg;
			if (sscanf(optarg, "%d", &audioRTPPortNumArg) == 1
				&& audioRTPPortNumArg > 0 && audioRTPPortNumArg < 65536
				&& (audioRTPPortNumArg&1) == 0) {
				audioRTPPortNum = audioRTPPortNumArg;
			} else {
				err(env) << "Invalid audio RTP port num argument: " << optarg;
				if ((audioRTPPortNumArg&1) != 0) env << " (must be even)";
				env << "\n";
			}
			break;
		}

		default: {
			//printf ("?? getopt returned character '%c' ??\n", c);
		}
		}
	}

	if (optind < argc) {
		warn(env) << "Ignoring non-option command-line arguments: ";
		while (optind < argc) env << argv[optind++] << " ";
		env << "\n";
	}

	if (videoFrameRateNumerator == 0 && videoFrameRateDenominator == 0) {
		// The video frame rate was not set (explicitly or implicitly) on the command line,
		// so set it to the default value now:
		videoFrameRateNumerator = allowedVideoTypes[0].frameRateNumerator;
		videoFrameRateDenominator = allowedVideoTypes[0].frameRateDenominator;
	}

	// If we were asked to stream a transport stream, make sure that the video and audio
	// codecs are suitable for this:
	if (packageFormat == PFMT_TRANSPORT_STREAM &&
		!(videoFormat == VFMT_MPEG2 && (audioFormat == AFMT_MPEG2 || audioFormat == AFMT_NONE))) {
		err(env) << "MPEG Transport Streams must use MPEG-2 video and audio\n";
		exit(1);
	}

	// Determine whether we're streaming multicast, and if so, what flavor:
	if (streamingMode == STREAMING_MULTICAST_SSM) {
		if (multicastAddress == 0) multicastAddress = chooseRandomIPv4SSMAddress(env);
	} else if (multicastAddress != 0) {
		streamingMode = STREAMING_MULTICAST_ASM;
	}
}

void reclaimArgs() {
	delete authDB;
	delete[] streamDescription;
}
