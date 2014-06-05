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
 * $Id: frame.c,v 1.13 2006/12/06 01:49:31 kyang Exp $
 */

/*
 * CHANGES:
 *  2001/01/17: menno: Added frequency cut off filter.
 *  2001/02/28: menno: Added Temporal Noise Shaping.
 *  2001/03/05: menno: Added Long Term Prediction.
 *  2001/05/01: menno: Added backward prediction.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "fixed.h"
#include "frame.h"
#include "coder.h"
#include "channels.h"
#include "bitstream.h"
#include "filtbank.h"
#include "aacquant.h"
#include "util.h"
#include "huffman.h"
#include "version.h"
#ifndef WIN32
#include <sys/resource.h>
#endif

#ifdef FAAC_STATIC_MEMORY
static faacEncStruct faac_hEncoder;
#endif

#ifdef DUMP_FILTERBANK
static dump_filterbank(faacEncHandle hEncoder)
{
    int32_t i;
    uint32_t channel;

    printf("dumping dump_filterbank start--------------------------\n");
    for ( channel=0; channel<hEncoder->numChannels; channel++)
        for( i=0; i<FRAME_LEN*2; i++ )
            printf("hEncoder->freqBuff[%d] = %.8f\n",i,COEF2FLOAT(hEncoder->freqBuff[channel][i]));
    printf("dumping dump_filterbank end--------------------------\n");
    
}
#endif

#ifdef DUMP_SAMPLEBUFFER
static dump_samplebuffer(faacEncHandle hEncoder)
{
    int32_t i;
    uint32_t channel;

    printf("dumping dump_samplebuffer start--------------------------\n");
    for ( channel=0; channel<hEncoder->numChannels; channel++)
        for( i=0; i<FRAME_LEN; i++ )
            printf("hEncoder->sampleBuff[%d] = %.8f\n",i,(double)(hEncoder->sampleBuff[channel][i<<1]));
    printf("dumping dump_samplebuffer end--------------------------\n");
    
}
#endif

#ifdef DUMP_SAMPLE
void dump_sample(real_t *sample, int nsamples)
{
    int i;

    printf("dumping dump_sample start--------------------------\n");
    printf("sample size = %d\n",nsamples);
    for (i = 0; i < nsamples; i++) {
        printf("sample[%d] = %.8f\n",i,REAL2FLOAT(sample[i]));
    }
    printf("dumping dump_sample end--------------------------\n");
}

void dump_sample_int(int32_t *sample, int nsamples)
{
    int i;

    printf("dumping dump_sample start--------------------------\n");
    printf("sample size = %d\n",nsamples);
    for (i = 0; i < nsamples; i++) {
        printf("sample[%d] = %d\n",i,sample[i]);
    }
    printf("dumping dump_sample end--------------------------\n");
}

void dump_float_sample(double *sample, int nsamples)
{
    int i;

    printf("dumping dump_float_sample start--------------------------\n");
    printf("sample size = %d\n",nsamples);
    for (i = 0; i < nsamples; i++) {
        printf("sample[%d] = %.8f\n",i,sample[i]);
    }
    printf("dumping dump_float_sample end--------------------------\n");
}
#endif

#ifdef FAAC_PROFILE
static unsigned long faac_gettimeused()
{
    struct rusage usage;
    unsigned long cur_timeused,timeused;
    static unsigned long last_timeused = 0;
    
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        cur_timeused = usage.ru_utime.tv_sec * 1000000 +
                   usage.ru_utime.tv_usec;
    }
    else
        cur_timeused = 0;

    timeused = cur_timeused-last_timeused;
    last_timeused = cur_timeused;

    return timeused;
}
#endif

#if FAAC_RELEASE
static char *libfaacName = FAAC_VERSION;
#else
static char *libfaacName = FAAC_VERSION "+ (" __DATE__ ") UNSTABLE";
#endif
static char *libCopyright =
  "FAAC - Freeware Advanced Audio Coder (http://www.audiocoding.com/)\n"
  " Copyright (C) 1999,2000,2001  Menno Bakker\n"
  " Copyright (C) 2002,2003  Krzysztof Nikiel\n"
  "This software is based on the ISO MPEG-4 reference source code.\n";

static SR_INFO srInfo[12+1];

// base bandwidth for q=100
static const int32_t bwbase = 16000;
// bandwidth multiplier (for quantiser)
static const int32_t bwmult = 120;
// max bandwidth/samplerate ratio
static const real_32_t bwfac = REAL_CONST(0.45);

int32_t FAACAPI faacEncGetVersion( char **faac_id_string,
                                  char **faac_copyright_string)
{
    if (faac_id_string)
        *faac_id_string = libfaacName;
    
    if (faac_copyright_string)
        *faac_copyright_string = libCopyright;
    
    return FAAC_CFG_VERSION;
}

int32_t FAACAPI faacEncGetDecoderSpecificInfo(faacEncHandle hEncoder,uchar_t** ppBuffer,ulong_t* pSizeOfDecoderSpecificInfo)
{
    if ((hEncoder == NULL) || (ppBuffer == NULL) || (pSizeOfDecoderSpecificInfo == NULL)) {
        return -1;
    }

    if (hEncoder->config.mpegVersion == MPEG2) {
        return -2; /* not supported */
    }

    *pSizeOfDecoderSpecificInfo = 2;
    *ppBuffer = malloc(2);

    if(*ppBuffer != NULL) {
	(*ppBuffer)[0] = (hEncoder->config.aacObjectType << 3) |
			    (hEncoder->sampleRateIdx >> 1);
	(*ppBuffer)[1] = ((hEncoder->sampleRateIdx & 0x1) << 7) |
			    (hEncoder->numChannels << 3);
        return 0;
    } 
    else {
        return -3;
    }
}

faacEncConfigurationPtr FAACAPI faacEncGetCurrentConfiguration(faacEncHandle hEncoder)
{
    faacEncConfigurationPtr config = &(hEncoder->config);

    return config;
}

int32_t FAACAPI faacEncSetConfiguration(faacEncHandle hEncoder,
                                        faacEncConfigurationPtr config)
{
    int32_t i;

    hEncoder->config.allowMidside = config->allowMidside;
    hEncoder->config.useLfe = config->useLfe;
    hEncoder->config.useTns = config->useTns;
    hEncoder->config.aacObjectType = config->aacObjectType;
    hEncoder->config.mpegVersion = config->mpegVersion;
    hEncoder->config.outputFormat = config->outputFormat;
    hEncoder->config.inputFormat = config->inputFormat;
    hEncoder->config.shortctl = config->shortctl;

    assert((hEncoder->config.outputFormat == 0) || (hEncoder->config.outputFormat == 1));

    switch( hEncoder->config.inputFormat ) {
        case FAAC_INPUT_16BIT:
        //case FAAC_INPUT_24BIT:
        case FAAC_INPUT_32BIT:
        case FAAC_INPUT_FLOAT:
            break;

        default:
            return 0;
            break;
    }

    /* No SSR supported for now */
    if (hEncoder->config.aacObjectType == SSR)
        return 0;

    /* LTP only with MPEG4 */
    if ((hEncoder->config.aacObjectType == LTP) && (hEncoder->config.mpegVersion != MPEG4))
        return 0;

    /* Check for correct bitrate */
    if (config->bitRate > MaxBitrate(hEncoder->sampleRate))
        return 0;

    if (config->bitRate && !config->bandWidth) {	
        static struct {
            int32_t rate; // per channel at 44100 sampling frequency
            int32_t cutoff;
        }	
        rates[] = {
            {29500, 5000},
            {37500, 7000},
            {47000, 10000},
            {64000, 16000},
            {76000, 20000},
            {0, 0}
        };
        
        int32_t f0, f1;
        int32_t r0, r1;

        real_t tmpbitRate = REAL_ICONST(config->bitRate)/hEncoder->sampleRate*44100;

        config->quantqual = 100;
        
        f0 = f1 = rates[0].cutoff;
        r0 = r1 = rates[0].rate;
        
        for (i = 0; rates[i].rate; i++) {
            f0 = f1;
            f1 = rates[i].cutoff;
            r0 = r1;
            r1 = rates[i].rate;
            if (REAL_ICONST(rates[i].rate) >= tmpbitRate)
                break;
        }

        if (tmpbitRate > REAL_ICONST(r1))
            tmpbitRate = REAL_ICONST(r1);

        if (tmpbitRate < REAL_ICONST(r0))
            tmpbitRate = REAL_ICONST(r0);

        if (f1 > f0)
            config->bandWidth = REAL2INT(f1 * faac_pow(tmpbitRate/r1,
                                                DIV_R( faac_log(DIV_R(REAL_ICONST(f1),
                                                                REAL_ICONST(f0))),
                                                       faac_log(DIV_R(REAL_ICONST(r1),
                                                                REAL_ICONST(r0)))))); 
        else
            config->bandWidth = f1;

        config->bandWidth =
            config->bandWidth * hEncoder->sampleRate / 44100;
        config->bitRate = (int32_t)REAL2INT(tmpbitRate*hEncoder->sampleRate/44100);
        if (config->bandWidth > (uint32_t)bwbase)
            config->bandWidth = (uint32_t)bwbase;
    }

    hEncoder->config.bitRate = config->bitRate;

    if (!config->bandWidth)  {
        config->bandWidth = (config->quantqual - 100) * bwmult + bwbase;
    }

    hEncoder->config.bandWidth = config->bandWidth;

    // check bandwidth
    if (hEncoder->config.bandWidth < 100)
        hEncoder->config.bandWidth = 100;
    if (hEncoder->config.bandWidth > (hEncoder->sampleRate / 2))
        hEncoder->config.bandWidth = hEncoder->sampleRate / 2;

    if (config->quantqual > 500)
        config->quantqual = 500;
    if (config->quantqual < 10)
        config->quantqual = 10;

    hEncoder->config.quantqual = config->quantqual;

    /* set quantization quality */
    hEncoder->aacquantCfg.quality = REAL_ICONST(config->quantqual);

    /* load channel_map */
    for( i = 0; i < 64; i++ )
        hEncoder->config.channel_map[i] = config->channel_map[i];

    /* OK */
    return 1;
}

faacEncHandle FAACAPI faacEncOpen(ulong_t sampleRate,
                                  uint32_t numChannels,
                                  ulong_t *inputSamples,
                                  ulong_t *maxOutputBytes)
{
    uint32_t channel;
    faacEncHandle hEncoder;

    faac_fixed_init();

    *inputSamples = FRAME_LEN*numChannels;
    *maxOutputBytes = (6144>>3)*numChannels;

#ifdef FAAC_STATIC_MEMORY
    hEncoder = &faac_hEncoder;
#else
    hEncoder = (faacEncHandle)AllocMemory(sizeof(faacEncStruct));
#endif

    SetMemory(hEncoder, 0, sizeof(faacEncStruct));

    hEncoder->numChannels = numChannels;
    hEncoder->sampleRate = sampleRate;
    hEncoder->sampleRateIdx = GetSRIndex(sampleRate);

    /* Initialize variables to default values */
    hEncoder->frameNum = 0;
    hEncoder->flushFrame = 0;

    /* Default configuration */
    hEncoder->config.version = FAAC_CFG_VERSION;
    hEncoder->config.name = libfaacName;
    hEncoder->config.copyright = libCopyright;
    hEncoder->config.mpegVersion = MPEG4;
    hEncoder->config.aacObjectType = LTP;
    hEncoder->config.allowMidside = 1;
    hEncoder->config.useLfe = 1;
    hEncoder->config.useTns = 0;
    hEncoder->config.bitRate = 0; /* default bitrate / channel */
    hEncoder->config.bandWidth = (int32_t)REAL2INT(bwfac*hEncoder->sampleRate);
    if (hEncoder->config.bandWidth > (uint32_t)bwbase)
        hEncoder->config.bandWidth = (uint32_t)bwbase;
    hEncoder->config.quantqual = 100;
    hEncoder->config.shortctl = SHORTCTL_NORMAL;

    /* default channel map is straight-through */
    for( channel = 0; channel < 64; channel++ )
        hEncoder->config.channel_map[channel] = channel;
	
    /*
        by default we have to be compatible with all previous software
        which assumes that we will generate ADTS
        /AV
    */
    hEncoder->config.outputFormat = 1;

    /*
        be compatible with software which assumes 24bit in 32bit PCM
    */
    hEncoder->config.inputFormat = FAAC_INPUT_32BIT;

    /* find correct sampling rate depending parameters */
    hEncoder->srInfo = &srInfo[hEncoder->sampleRateIdx];

    for (channel = 0; channel < numChannels; channel++) {
        hEncoder->coderInfo[channel].prev_window_shape = SINE_WINDOW;
        hEncoder->coderInfo[channel].window_shape = SINE_WINDOW;
        hEncoder->coderInfo[channel].block_type = ONLY_LONG_WINDOW;
        hEncoder->coderInfo[channel].num_window_groups = 1;
        hEncoder->coderInfo[channel].window_group_length[0] = 1;

        /* FIXME: Use sr_idx here */
        hEncoder->coderInfo[channel].max_pred_sfb = GetMaxPredSfb(hEncoder->sampleRateIdx);

        hEncoder->sampleBuff[channel] = NULL;
    }

    /* Initialize coder functions */
    fft_initialize( &hEncoder->fft_tables );  
    FilterBankInit(hEncoder);
    HuffmanInit(hEncoder->coderInfo, hEncoder->numChannels);

    /* Return handle */
    return hEncoder;
}

int32_t FAACAPI faacEncClose(faacEncHandle hEncoder)
{
    FilterBankEnd(hEncoder);
    HuffmanEnd(hEncoder->coderInfo, hEncoder->numChannels); 
    fft_terminate( &hEncoder->fft_tables );

#ifndef FAAC_STATIC_MEMORY
    if (hEncoder)
        FreeMemory(hEncoder);
#endif

    return 0;
}

int32_t FAACAPI faacEncEncode(faacEncHandle hEncoder,
                          int16_t *inputBuffer,
                          int16_t *overlapBuffer,
                          uint32_t samplesInput,
                          uchar_t *outputBuffer,
                          uint32_t bufferSize
                          )
{
    register uint32_t channel;
    register int32_t i;
    register int32_t sb;
    int32_t frameBytes;
    uint32_t offset;
    BitStream *bitStream; /* bitstream used for writing the frame to */
#ifdef FAAC_PROFILE
    static int encode_timeused = 0;
    static int total_frames = 0;
    static long total_frameBytes = 0;
#endif

    /* local copy's of parameters */
    ChannelInfo *channelInfo = hEncoder->channelInfo;
    CoderInfo *coderInfo = hEncoder->coderInfo;
    uint32_t numChannels = hEncoder->numChannels;
    uint32_t sampleRate = hEncoder->sampleRate;
    uint32_t useLfe = hEncoder->config.useLfe;
    uint32_t bandWidth = hEncoder->config.bandWidth;
    static real_32_t realconst2 = REAL_CONST(0.01);
    static real_32_t realconst3 = REAL_CONST(0.2);
   
    /* Increase frame number */
    hEncoder->frameNum++;

    if (samplesInput == 0)
        hEncoder->flushFrame++;

    /* After 4 flush frames all samples have been encoded,
       return 0 bytes written */
    if (hEncoder->flushFrame > 4)
        return 0;

    /* Determine the channel configuration */
    GetChannelInfo(channelInfo, numChannels, useLfe);

    /* Update current sample buffers */
    for (channel = 0; channel < numChannels; channel++) {

        if (samplesInput == 0) {
            /* start flushing*/
            for (i = 0; i < FRAME_LEN; i++)
                hEncoder->sampleBuff[channel][i]    =  0;
        }
        else {
            /* handle the various input formats and channel remapping */
            if ( hEncoder->config.inputFormat == FAAC_INPUT_16BIT ) {
                hEncoder->sampleBuff[channel] = inputBuffer + hEncoder->config.channel_map[channel];
                hEncoder->overlapBuff[channel] = overlapBuffer + hEncoder->config.channel_map[channel];
            }
            else
                return -1; /* invalid input format */
            
        }
    }

#ifdef DUMP_SAMPLEBUFFER
    dump_samplebuffer(hEncoder);
//    exit(1);
#endif
    
    /* AAC Filterbank, MDCT with overlap and add */
    for (channel = 0; channel < numChannels; channel++) {
        FilterBank(hEncoder,
            &coderInfo[channel],
            hEncoder->sampleBuff[channel],
            hEncoder->freqBuff[channel],
            hEncoder->overlapBuff[channel],
            MOVERLAPPED);
        
        specFilter(hEncoder->freqBuff[channel], sampleRate,
            bandWidth, BLOCK_LEN_LONG);
    }

#ifdef DUMP_FILTERBANK
    dump_filterbank(hEncoder);
    exit(1);
#endif
    
    /* TMP: Build sfb offset table and other stuff */
    for (channel = 0; channel < numChannels; channel++) {
        channelInfo[channel].msInfo.is_present = 0;
        
        coderInfo[channel].max_sfb = hEncoder->srInfo->num_cb_long;
        coderInfo[channel].nr_of_sfb = hEncoder->srInfo->num_cb_long;

        coderInfo[channel].num_window_groups = 1;
        coderInfo[channel].window_group_length[0] = 1;

        offset = 0;
        for (sb = 0; sb < coderInfo[channel].nr_of_sfb; sb++) {
            coderInfo[channel].sfb_offset[sb] = offset;
            offset += hEncoder->srInfo->cb_width_long[sb];
        }
        coderInfo[channel].sfb_offset[coderInfo[channel].nr_of_sfb] = offset;
    }

    for (channel = 0; channel < numChannels; channel++) {
        FilterBankPow(1024, hEncoder->freqBuff[channel], hEncoder->freqBuff2[channel]);
        
        CalcAvgEnrg(&coderInfo[channel], hEncoder->freqBuff[channel], hEncoder->freqBuff2[channel]);
    
        // reduce LFE bandwidth
        if (!channelInfo[channel].cpe && channelInfo[channel].lfe) {
            coderInfo[channel].nr_of_sfb = coderInfo[channel].max_sfb = 3;
        }
    }
  
    /* Quantize and code the signal */
    for (channel = 0; channel < numChannels; channel++) {
        AACQuantize(&coderInfo[channel], 
					&channelInfo[channel], 
                    hEncoder->srInfo->cb_width_long,
                    hEncoder->srInfo->num_cb_long, 
                    hEncoder->freqBuff[channel],
                    hEncoder->freqBuff2[channel],
                    &(hEncoder->aacquantCfg));
    }

    // fix max_sfb in CPE mode
    for (channel = 0; channel < numChannels; channel++) {
        if (channelInfo[channel].present
            && (channelInfo[channel].cpe)
            && (channelInfo[channel].ch_is_left)) {
            CoderInfo *cil, *cir;
            cil = &coderInfo[channel];
            cir = &coderInfo[channelInfo[channel].paired_ch];
            cil->max_sfb = cir->max_sfb = max(cil->max_sfb, cir->max_sfb);
            cil->nr_of_sfb = cir->nr_of_sfb = cil->max_sfb;
        }
    }

    /* Write the AAC bitstream */
    bitStream = OpenBitStream(bufferSize, outputBuffer);

    WriteBitstream(hEncoder, coderInfo, channelInfo, bitStream, numChannels);

    /* Close the bitstream and return the number of bytes written */
    frameBytes = CloseBitStream(bitStream);

    /* Adjust quality to get correct average bitrate */
    if (hEncoder->config.bitRate) {
        real_32_t fix;
        int32_t desbits = numChannels * (hEncoder->config.bitRate << 10)
                          / hEncoder->sampleRate;
        int32_t diff = (frameBytes << 3) - desbits;

        hEncoder->bitDiff += diff;
        fix = hEncoder->bitDiff*realconst2/desbits;
        fix = max(fix, -realconst3);
        fix = min(fix, realconst3);

        if (((diff > 0) && (fix > 0)) || ((diff < 0) && (fix < 0))) {
            hEncoder->aacquantCfg.quality = MUL_R(hEncoder->aacquantCfg.quality,
                                                  REAL_ICONST(1) - fix);
            if (hEncoder->aacquantCfg.quality > REAL_ICONST(300))
                hEncoder->aacquantCfg.quality = REAL_ICONST(300);
            if (hEncoder->aacquantCfg.quality < REAL_ICONST(50))
                hEncoder->aacquantCfg.quality = REAL_ICONST(50);
        }
    }

#ifdef FAAC_PROFILE
    total_frames++;
    total_frameBytes += frameBytes;
    encode_timeused += faac_gettimeused();

    /* MIPS = CPU time/Audio time = (CPU_CYCLE*timeused)/(frames/frames per second)
            = (CPU_CYCLE*timeused*frames per second)/frames
     */
    if ( (total_frames%FAAC_PROFILE_COUNT)==0 ) {
        printf("\nMIPS[%d], encoding time[%7.3f], frames[%d], bandWidth[%d], quantqual[%d], bitRate[%d], real bitrate(kbps)[%.1f]\n",
                (int)(FAAC_CPU_CYCLE*(hEncoder->sampleRate/1024.0)*(encode_timeused/1000000.0)/total_frames),
                encode_timeused/1000000.0,
                total_frames,
                hEncoder->config.bandWidth,
                hEncoder->config.quantqual,
                hEncoder->config.bitRate,
                total_frameBytes*8/1000.0*(hEncoder->sampleRate/1024.0)/total_frames
                );
    }
#endif
    
    return frameBytes;
}


/* Scalefactorband data table for 1024 transform length */
static SR_INFO srInfo[12+1] =
{
    { 96000, 41, 12,
        {
            4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
            8, 8, 8, 8, 8, 12, 12, 12, 12, 12, 16, 16, 24, 28,
            36, 44, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
        },{
            4, 4, 4, 4, 4, 4, 8, 8, 8, 16, 28, 36
        }
    }, { 88200, 41, 12,
        {
            4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
            8, 8, 8, 8, 8, 12, 12, 12, 12, 12, 16, 16, 24, 28,
            36, 44, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
        },{
            4, 4, 4, 4, 4, 4, 8, 8, 8, 16, 28, 36
        }
    }, { 64000, 47, 12,
        {
            4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
            8, 8, 8, 8, 12, 12, 12, 16, 16, 16, 20, 24, 24, 28,
            36, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
            40, 40, 40, 40, 40
        },{
            4, 4, 4, 4, 4, 4, 8, 8, 8, 16, 28, 32
        }
    }, { 48000, 49, 14,
        {
            4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  8,  8,  8,  8,  8,  8,  8,
            12, 12, 12, 12, 16, 16, 20, 20, 24, 24, 28, 28, 32, 32, 32, 32, 32, 32,
            32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 96
        }, {
            4,  4,  4,  4,  4,  8,  8,  8, 12, 12, 12, 16, 16, 16
        }
    }, { 44100, 49, 14,
        {
            4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  8,  8,  8,  8,  8,  8,  8,
            12, 12, 12, 12, 16, 16, 20, 20, 24, 24, 28, 28, 32, 32, 32, 32, 32, 32,
            32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 96
        }, {
            4,  4,  4,  4,  4,  8,  8,  8, 12, 12, 12, 16, 16, 16
        }
    }, { 32000, 51, 14,
        {
            4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  8,  8,  8,  8,
            8,  8,  8,  12, 12, 12, 12, 16, 16, 20, 20, 24, 24, 28,
            28, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
            32, 32, 32, 32, 32, 32, 32, 32, 32
        },{
            4,  4,  4,  4,  4,  8,  8,  8,  12, 12, 12, 16, 16, 16
        }
    }, { 24000, 47, 15,
        {
            4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  8,  8,  8,  8,  8,  8,  8,
            8,  8,  8,  12, 12, 12, 12, 16, 16, 16, 20, 20, 24, 24, 28, 28, 32,
            36, 36, 40, 44, 48, 52, 52, 64, 64, 64, 64, 64
        }, {
            4,  4,  4,  4,  4,  4,  4,  8,  8,  8, 12, 12, 16, 16, 20
        }
    }, { 22050, 47, 15,
        {
            4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  8,  8,  8,  8,  8,  8,  8,
            8,  8,  8,  12, 12, 12, 12, 16, 16, 16, 20, 20, 24, 24, 28, 28, 32,
            36, 36, 40, 44, 48, 52, 52, 64, 64, 64, 64, 64
        }, {
            4,  4,  4,  4,  4,  4,  4,  8,  8,  8, 12, 12, 16, 16, 20
        }
    }, { 16000, 43, 15,
        {
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 12, 12, 12,
            12, 12, 12, 12, 12, 12, 16, 16, 16, 16, 20, 20, 20, 24,
            24, 28, 28, 32, 36, 40, 40, 44, 48, 52, 56, 60, 64, 64, 64
        }, {
            4, 4, 4, 4, 4, 4, 4, 4, 8, 8, 12, 12, 16, 20, 20
        }
    }, { 12000, 43, 15,
        {
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 12, 12, 12,
            12, 12, 12, 12, 12, 12, 16, 16, 16, 16, 20, 20, 20, 24,
            24, 28, 28, 32, 36, 40, 40, 44, 48, 52, 56, 60, 64, 64, 64
        }, {
            4, 4, 4, 4, 4, 4, 4, 4, 8, 8, 12, 12, 16, 20, 20
        }
    }, { 11025, 43, 15,
        {
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 12, 12, 12,
            12, 12, 12, 12, 12, 12, 16, 16, 16, 16, 20, 20, 20, 24,
            24, 28, 28, 32, 36, 40, 40, 44, 48, 52, 56, 60, 64, 64, 64
        }, {
            4, 4, 4, 4, 4, 4, 4, 4, 8, 8, 12, 12, 16, 20, 20
        }
    }, { 8000, 40, 15,
        {
            12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 16,
            16, 16, 16, 16, 16, 16, 20, 20, 20, 20, 24, 24, 24, 28,
            28, 32, 36, 36, 40, 44, 48, 52, 56, 60, 64, 80
        }, {
            4, 4, 4, 4, 4, 4, 4, 8, 8, 8, 8, 12, 16, 20, 20
        }
    },
    { -1 }
};

/*
$Log: frame.c,v $
Revision 1.13  2006/12/06 01:49:31  kyang
Fix error for AAC bit rate

Revision 1.12  2006/12/05 07:06:47  kyang
Fix AAC quality problem when encoding pop MTV audio reported by FAE

Revision 1.11  2006/12/01 08:33:10  kyang
Add bit rate monitor for FAAC

Revision 1.10  2006/07/13 00:49:49  kyang
Improve AAC performance. CBR: from 90MIPS to 70MIPS. VBR: from 100MIPS to 77MIPS

Revision 1.9  2006/07/10 20:30:38  kyang
Remove unnecessary FAAC code

Revision 1.8  2006/06/14 00:02:35  kyang
Fix FAAC bug to decrease AAC output bitrate

Revision 1.7  2006/06/07 01:50:11  kyang
Fix bug2562: Increase AAC quality

Revision 1.6  2006/03/06 01:58:54  nathan
Fix faacEncGetDecoderSpecificInfo()

Revision 1.5  2006/02/23 18:44:11  kyang
release FAAC code

Revision 1.4  2006/02/20 22:53:30  kyang
Decrease FAAC startup time

Revision 1.3  2006/02/20 02:40:44  kyang
Improve aac quality

Revision 1.2  2006/02/15 01:20:14  kyang
Realtime FAAC for Cypher

Revision 1.65  2004/07/18 09:34:24  corrados
New bandwidth settings for DRM, improved quantization quality adaptation (almost constant bit-rate now)

Revision 1.64  2004/07/13 17:56:37  corrados
bug fix with new object type definitions

Revision 1.63  2004/07/08 14:01:25  corrados
New scalefactorband table for 960 transform length, bug fix in HCR

Revision 1.62  2004/07/04 12:10:52  corrados
made faac compliant with Digital Radio Mondiale (DRM) (DRM macro must be set)
implemented HCR tool, VCB11, CRC, scalable bitstream order
note: VCB11 only uses codebook 11! TODO: implement codebooks 16-32
960 transform length is not yet implemented (TODO)! Use 1024 for encoding and 960 for decoding, resulting in a lot of artefacts

Revision 1.61  2004/05/03 11:37:16  danchr
bump version to unstable 1.24+

Revision 1.60  2004/04/13 13:47:33  danchr
clarify release <> unstable status

Revision 1.59  2004/04/02 14:56:17  danchr
fix name clash w/ libavcodec: fft_init -> fft_initialize
bump version number to 1.24 beta

Revision 1.58  2004/03/17 13:34:20  danchr
Automatic, untuned setting of lowpass for VBR.

Revision 1.57  2004/03/15 20:16:42  knik
fixed copyright notice

Revision 1.56  2004/01/23 10:22:26  stux
*** empty log message ***

Revision 1.55  2003/12/17 20:59:55  knik
changed default cutoff to 16k

Revision 1.54  2003/11/24 18:09:12  knik
A safe version of faacEncGetVersion() without string length problem.
Removed Stux from copyright notice. I don't think he contributed something very
substantial to faac and this is not the right place to list all contributors.

Revision 1.53  2003/11/16 05:02:52  stux
moved global tables from fft.c into hEncoder FFT_Tables. Add fft_init and fft_terminate, flowed through all necessary changes. This should remove at least one instance of a memory leak, and fix some thread-safety problems. Version update to 1.23.3

Revision 1.52  2003/11/15 08:13:42  stux
added FaacEncGetVersion(), version 1.23.2, added myself to faacCopyright :-P, does vanity know no bound ;)

Revision 1.51  2003/11/10 17:48:00  knik
Allowed independent bitRate and bandWidth setting.
Small fixes.

Revision 1.50  2003/10/29 10:31:25  stux
Added channel_map to FaacEncHandle, facilitates free generalised channel remapping in the faac core. Default is straight-through, should be *zero* performance hit... and even probably an immeasurable performance gain, updated FAAC_CFG_VERSION to 104 and FAAC_VERSION to 1.22.0

Revision 1.49  2003/10/12 16:43:39  knik
average bitrate control made more stable

Revision 1.48  2003/10/12 14:29:53  knik
more accurate average bitrate control

Revision 1.47  2003/09/24 16:26:54  knik
faacEncStruct: quantizer specific data enclosed in AACQuantCfg structure.
Added config option to enforce block type.

Revision 1.46  2003/09/07 16:48:31  knik
Updated psymodel call. Updated bitrate/cutoff mapping table.

Revision 1.45  2003/08/23 15:02:13  knik
last frame moved back to the library

Revision 1.44  2003/08/15 11:42:08  knik
removed single silent flush frame

Revision 1.43  2003/08/11 09:43:47  menno
thread safety, some tables added to the encoder context

Revision 1.42  2003/08/09 11:39:30  knik
LFE support enabled by default

Revision 1.41  2003/08/08 10:02:09  menno
Small fix

Revision 1.40  2003/08/07 08:17:00  knik
Better LFE support (reduced bandwidth)

Revision 1.39  2003/08/02 11:32:10  stux
added config.inputFormat, and associated defines and code, faac now handles native endian 16bit, 24bit and real_t input. Added faacEncGetDecoderSpecificInfo to the dll exports, needed for MP4. Updated DLL .dsp to compile without error. Updated CFG_VERSION to 102. Version number might need to be updated as the API has technically changed. Did not update libfaac.pdf

Revision 1.38  2003/07/10 19:17:01  knik
24-bit input

Revision 1.37  2003/06/26 19:20:09  knik
Mid/Side support.
Copyright info moved from frontend.
Fixed memory leak.

Revision 1.36  2003/05/12 17:53:16  knik
updated ABR table

Revision 1.35  2003/05/10 09:39:55  knik
added approximate ABR setting
modified default cutoff

Revision 1.34  2003/05/01 09:31:39  knik
removed ISO psyodel
disabled m/s coding
fixed default bandwidth
reduced max_sfb check

Revision 1.33  2003/04/13 08:37:23  knik
version number moved to version.h

Revision 1.32  2003/03/27 17:08:23  knik
added quantizer quality and bandwidth setting

Revision 1.31  2002/10/11 18:00:15  menno
small bugfix

Revision 1.30  2002/10/08 18:53:01  menno
Fixed some memory leakage

Revision 1.29  2002/08/19 16:34:43  knik
added one additional flush frame
fixed sample buffer memory allocation

*/
