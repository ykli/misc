/*
 * FAAC - Freeware Advanced Audio Coder
 * Copyright (C) 2001 Menno Bakker
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: frame.h,v 1.5 2006/07/13 00:49:49 kyang Exp $
 */

#ifndef FRAME_H
#define FRAME_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "coder.h"
#include "channels.h"
#include "aacquant.h"
#include "fft.h"

#ifdef WIN32
  #ifndef FAACAPI
    #define FAACAPI __stdcall
  #endif
#else
  #ifndef FAACAPI
    #define FAACAPI
  #endif
#endif

#pragma pack(push, 1)

#include <faaccfg.h>

typedef struct {
    /* number of channels in AAC file */
    uint32_t numChannels;

    /* samplerate of AAC file */
    ulong_t sampleRate;
    uint32_t sampleRateIdx;

    uint32_t usedBytes;

    /* frame number */
    uint32_t frameNum;
    uint32_t flushFrame;

    /* Scalefactorband data */
    SR_INFO *srInfo;

    /* sample buffers of current next and next next frame*/
    int16_t *sampleBuff[MAX_CHANNELS];

    /* Filterbank buffers */
    coef_t *freqBuff[MAX_CHANNELS];
    pow_t *freqBuff2[MAX_CHANNELS];
    int16_t *overlapBuff[MAX_CHANNELS];

    /* Channel and Coder data for all channels */
    CoderInfo coderInfo[MAX_CHANNELS];
    ChannelInfo channelInfo[MAX_CHANNELS];

    /* Configuration data */
    faacEncConfiguration config;

    /* quantizer specific config */
    AACQuantCfg aacquantCfg;

	/* FFT Tables */
	FFT_Tables	fft_tables;

    /* output bits difference in average bitrate mode */
    int32_t bitDiff;
} faacEncStruct, *faacEncHandle;

int32_t FAACAPI faacEncGetVersion(char **faac_id_string,
			      char **faac_copyright_string);

int32_t FAACAPI faacEncGetDecoderSpecificInfo(faacEncHandle hEncoder,
                                          uchar_t ** ppBuffer,
                                          ulong_t* pSizeOfDecoderSpecificInfo);

faacEncConfigurationPtr FAACAPI faacEncGetCurrentConfiguration(faacEncHandle hEncoder);
int32_t FAACAPI faacEncSetConfiguration (faacEncHandle hEncoder, faacEncConfigurationPtr config);

faacEncHandle FAACAPI faacEncOpen(ulong_t sampleRate,
                                  uint32_t numChannels,
                                  ulong_t *inputSamples,
                                  ulong_t *maxOutputBytes);

int32_t FAACAPI faacEncEncode(faacEncHandle hEncoder,
							  int16_t *inputBuffer,
                              int16_t *overlapBuffer,
							  uint32_t samplesInput,
							  uchar_t *outputBuffer,
							  uint32_t bufferSize);

int32_t FAACAPI faacEncClose(faacEncHandle hEncoder);


#pragma pack(pop)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FRAME_H */
