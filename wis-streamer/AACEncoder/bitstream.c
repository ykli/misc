/**********************************************************************

This software module was originally developed by
and edited by Texas Instruments in the course of
development of the MPEG-2 NBC/MPEG-4 Audio standard
ISO/IEC 13818-7, 14496-1,2 and 3. This software module is an
implementation of a part of one or more MPEG-2 NBC/MPEG-4 Audio tools
as specified by the MPEG-2 NBC/MPEG-4 Audio standard. ISO/IEC gives
users of the MPEG-2 NBC/MPEG-4 Audio standards free license to this
software module or modifications thereof for use in hardware or
software products claiming conformance to the MPEG-2 NBC/ MPEG-4 Audio
standards. Those intending to use this software module in hardware or
software products are advised that this use may infringe existing
patents. The original developer of this software module and his/her
company, the subsequent editors and their companies, and ISO/IEC have
no liability for use of this software module or modifications thereof
in an implementation. Copyright is not released for non MPEG-2
NBC/MPEG-4 Audio conforming products. The original developer retains
full right to use the code for his/her own purpose, assign or donate
the code to a third party and to inhibit third party from using the
code for non MPEG-2 NBC/MPEG-4 Audio conforming products. This
copyright notice must be included in all copies or derivative works.

Copyright (c) 1997.
**********************************************************************/
/*
 * $Id: bitstream.c,v 1.6 2006/07/13 00:49:48 kyang Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#include <byteswap.h>
#endif

#include "fixed.h"
#include "coder.h"
#include "channels.h"
#include "huffman.h"
#include "bitstream.h"
#include "util.h"

#ifdef FAAC_STATIC_MEMORY
static BitStream faac_bitStream;
#endif

static int32_t CountBitstream(faacEncHandle hEncoder,
                              CoderInfo *coderInfo,
                              ChannelInfo *channelInfo,
                              BitStream *bitStream,
                              int32_t numChannels);
static int32_t WriteADTSHeader(faacEncHandle hEncoder,
                               BitStream *bitStream,
                               int32_t writeFlag);
static int32_t WriteCPE(CoderInfo *coderInfoL,
                        CoderInfo *coderInfoR,
                        ChannelInfo *channelInfo,
                        BitStream* bitStream,
                        int32_t objectType,
                        int32_t writeFlag);
static int32_t WriteSCE(CoderInfo *coderInfo,
                        ChannelInfo *channelInfo,
                        BitStream *bitStream,
                        int32_t objectType,
                        int32_t writeFlag);
static int32_t WriteLFE(CoderInfo *coderInfo,
                        ChannelInfo *channelInfo,
                        BitStream *bitStream,
                        int32_t objectType,
                        int32_t writeFlag);
static int32_t WriteICSInfo(CoderInfo *coderInfo,
                            BitStream *bitStream,
                            int32_t objectType,
                            int32_t common_window,
                            int32_t writeFlag);
static int32_t WriteICS(CoderInfo *coderInfo,
                        BitStream *bitStream,
                        int32_t commonWindow,
                        int32_t objectType,
                        int32_t writeFlag);
#if 0
static int32_t WriteLTPPredictorData(CoderInfo *coderInfo,
                                     BitStream *bitStream,
                                     int32_t writeFlag);
#endif
static int32_t WritePredictorData(CoderInfo *coderInfo,
                                  BitStream *bitStream,
                                  int32_t writeFlag);
static int32_t WritePulseData(CoderInfo *coderInfo,
                              BitStream *bitStream,
                              int32_t writeFlag);
static int32_t WriteTNSData(CoderInfo *coderInfo,
                            BitStream *bitStream,
                            int32_t writeFlag);
static int32_t WriteGainControlData(CoderInfo *coderInfo,
                                    BitStream *bitStream,
                                    int32_t writeFlag);
static int32_t WriteSpectralData(CoderInfo *coderInfo,
                                 BitStream *bitStream,
                                 int32_t writeFlag);
static int32_t WriteAACFillBits(BitStream* bitStream,
                                int32_t numBits,
                                int32_t writeFlag);
static int32_t FindGroupingBits(CoderInfo *coderInfo);
#if 0
static long_t BufferNumBit(BitStream *bitStream);
static int32_t ByteAlign(BitStream* bitStream,
                         int32_t writeFlag);
#endif
#ifdef DRM
static int32_t PutBitHcr(BitStream *bitStream,
                         ulong_t curpos,
                         ulong_t data,
                         int32_t numBit);
static int32_t rewind_word(int32_t W, int32_t len);
static int32_t WriteReorderedSpectralData(CoderInfo *coderInfo,
                                          BitStream *bitStream,
                                          int32_t writeFlag);
static void calc_CRC(BitStream *bitStream, int32_t len);
#endif

static void PutBits_init(BitStream *bitStream, uchar_t *buffer, int buffer_size);
static int32_t flush_put_bits(BitStream *bitStream,int32_t writeFlag);

#ifdef WORDS_BIGENDIAN
#define be2me_16(x) (x)
#define be2me_32(x) (x)
#define be2me_64(x) (x)
#define le2me_16(x) bswap_16(x)
#define le2me_32(x) bswap_32(x)
#define le2me_64(x) bswap_64(x)
#else
#define be2me_16(x) bswap_16(x)
#define be2me_32(x) bswap_32(x)
#define be2me_64(x) bswap_64(x)
#define le2me_16(x) (x)
#define le2me_32(x) (x)
#define le2me_64(x) (x)
#endif

static int32_t WriteFAACStr(BitStream *bitStream, char *version, int32_t write)
{
    register int32_t i;
    char str[200];
    int32_t len, padbits, count;
    int32_t bitcnt;

    sprintf(str, "libfaac %s", version);

    len = strlen(str) + 1;
    padbits = 1;//(bitStream->bit_left&7);
    count = len + 3;

    bitcnt = LEN_SE_ID + 4 + ((count < 15) ? 0 : 8) + count * 8;
    if (!write)
        return bitcnt;

    PutBit(bitStream, ID_FIL, LEN_SE_ID);
    if (count < 15) {
        PutBit(bitStream, count, 4);
    }
    else {
        PutBit(bitStream, 15, 4);
        PutBit(bitStream, count - 14, 8);
    }

    PutBit(bitStream, 0, padbits);
    PutBit(bitStream, 0, 8);
    PutBit(bitStream, 0, 8); // just in case
    for (i = 0; i < len; i++)
        PutBit(bitStream, str[i], 8);

    PutBit(bitStream, 0, 8 - padbits);

    return bitcnt;
}

int32_t WriteBitstream(faacEncHandle hEncoder,
                       CoderInfo *coderInfo,
                       ChannelInfo *channelInfo,
                       BitStream *bitStream,
                       int32_t numChannel)
{
    int32_t channel;
    int32_t bits = 0;
    int32_t bitsLeftAfterFill, numFillBits;

    CountBitstream(hEncoder, coderInfo, channelInfo, bitStream, numChannel);

    if(hEncoder->config.outputFormat == 1) {
        bits += WriteADTSHeader(hEncoder, bitStream, 1);
    } 
    else {
        bits = 0; // compiler c will remove it, but anyone will see that current size of bitstream is 0
    }

    if (hEncoder->frameNum == 4)
        bits += WriteFAACStr(bitStream, hEncoder->config.name, 1);

    for (channel = 0; channel < numChannel; channel++) {
        if (channelInfo[channel].present) {
            /* Write out a single_channel_element */
            if (!channelInfo[channel].cpe) {

                if (channelInfo[channel].lfe) {
                    /* Write out lfe */
                    bits += WriteLFE(&coderInfo[channel],
                        &channelInfo[channel],
                        bitStream,
                        hEncoder->config.aacObjectType,
                        1);
                } else {
                    /* Write out sce */
                    bits += WriteSCE(&coderInfo[channel],
                        &channelInfo[channel],
                        bitStream,
                        hEncoder->config.aacObjectType,
                        1);
                }
            } else {
                if (channelInfo[channel].ch_is_left) {
                    /* Write out cpe */
                    bits += WriteCPE(&coderInfo[channel],
                        &coderInfo[channelInfo[channel].paired_ch],
                        &channelInfo[channel],
                        bitStream,
                        hEncoder->config.aacObjectType,
                        1);
                }
            }
        }
    }

    /* Compute how many fill bits are needed to avoid overflowing bit reservoir */
    /* Save room for ID_END terminator */
    if (bits < (8 - LEN_SE_ID) ) {
        numFillBits = 8 - LEN_SE_ID - bits;
    } 
    else {
        numFillBits = 0;
    }

    /* Write AAC fill_elements, smallest fill element is 7 bits. */
    /* Function may leave up to 6 bits left after fill, so tell it to fill a few extra */
    numFillBits += 6;
    bitsLeftAfterFill = WriteAACFillBits(bitStream, numFillBits, 1);
    bits += (numFillBits - bitsLeftAfterFill);

    /* Write ID_END terminator */
    bits += LEN_SE_ID;
    PutBit(bitStream, ID_END, LEN_SE_ID);

    flush_put_bits(bitStream, 1);
    hEncoder->usedBytes = (bitStream->buf_ptr-(uint8_t *)bitStream->bit_buf);

#if 0
    tmp1 = bitStream->currentBit;
//    tmp2 = bitStream->numBit;
    bitStream->currentBit = bitStream->numBit = 30;
    PutBit(bitStream, hEncoder->usedBytes, 13);
    bitStream->currentBit = tmp1;
//    bitStream->numBit = tmp2;
#else
    // Write head here
#endif

    return bits;
}

static int32_t CountBitstream(faacEncHandle hEncoder,
                              CoderInfo *coderInfo,
                              ChannelInfo *channelInfo,
                              BitStream *bitStream,
                              int32_t numChannel)
{
    register int32_t channel;
    int32_t bits = 0;
    int32_t bitsLeftAfterFill, numFillBits;

    if(hEncoder->config.outputFormat == 1){
        bits += WriteADTSHeader(hEncoder, bitStream, 0);
    } else {
        bits = 0; // compiler will remove it, but anyone will see that current size of bitstream is 0
    }

    if (hEncoder->frameNum == 4)
      bits += WriteFAACStr(bitStream, hEncoder->config.name, 0);

    for (channel = 0; channel < numChannel; channel++) {
        if (channelInfo[channel].present) {
            /* Write out a single_channel_element */
            if (!channelInfo[channel].cpe) {

                if (channelInfo[channel].lfe) {
                    /* Write out lfe */
                    bits += WriteLFE(&coderInfo[channel],
                        &channelInfo[channel],
                        bitStream,
                        hEncoder->config.aacObjectType,
                        0);
                } else {
                    /* Write out sce */
                    bits += WriteSCE(&coderInfo[channel],
                        &channelInfo[channel],
                        bitStream,
                        hEncoder->config.aacObjectType,
                        0);
                }

            } else {
                if (channelInfo[channel].ch_is_left) {
                    /* Write out cpe */
                    bits += WriteCPE(&coderInfo[channel],
                        &coderInfo[channelInfo[channel].paired_ch],
                        &channelInfo[channel],
                        bitStream,
                        hEncoder->config.aacObjectType,
                        0);
                }
            }
        }
    }

    /* Compute how many fill bits are needed to avoid overflowing bit reservoir */
    /* Save room for ID_END terminator */
    if (bits < (8 - LEN_SE_ID) ) {
        numFillBits = 8 - LEN_SE_ID - bits;
    } else {
        numFillBits = 0;
    }

    /* Write AAC fill_elements, smallest fill element is 7 bits. */
    /* Function may leave up to 6 bits left after fill, so tell it to fill a few extra */
    numFillBits += 6;
    bitsLeftAfterFill = WriteAACFillBits(bitStream, numFillBits, 0);
    bits += (numFillBits - bitsLeftAfterFill);

    /* Write ID_END terminator */
    bits += LEN_SE_ID;

    bits += flush_put_bits(bitStream,0);
    hEncoder->usedBytes = bit2byte(bits);

    return bits;
}

static int32_t WriteADTSHeader(faacEncHandle hEncoder,
                               BitStream *bitStream,
                               int32_t writeFlag)
{
    int32_t bits = 56;

    if (writeFlag) {
        /* Fixed ADTS header */
        PutBit(bitStream, 0xFFFF, 12); /* 12 bit Syncword */
        PutBit(bitStream, hEncoder->config.mpegVersion, 1); /* ID == 0 for MPEG4 AAC, 1 for MPEG2 AAC */
        PutBit(bitStream, 0, 2); /* layer == 0 */
        PutBit(bitStream, 1, 1); /* protection absent */
        PutBit(bitStream, hEncoder->config.aacObjectType - 1, 2); /* profile */
        PutBit(bitStream, hEncoder->sampleRateIdx, 4); /* sampling rate */
        PutBit(bitStream, 0, 1); /* private bit */
        PutBit(bitStream, hEncoder->numChannels, 3); /* ch. config (must be > 0) */
                                                     /* simply using numChannels only works for
                                                        6 channels or less, else a channel
                                                        configuration should be written */
        PutBit(bitStream, 0, 1); /* original/copy */
        PutBit(bitStream, 0, 1); /* home */

#if 0 // Removed in corrigendum 14496-3:2002
        if (hEncoder->config.mpegVersion == 0)
            PutBit(bitStream, 0, 2); /* emphasis */
#endif

        /* Variable ADTS header */
        PutBit(bitStream, 0, 1); /* copyr. id. bit */
        PutBit(bitStream, 0, 1); /* copyr. id. start */
        PutBit(bitStream, hEncoder->usedBytes, 13);
        PutBit(bitStream, 0x7FF, 11); /* buffer fullness (0x7FF for VBR) */
        PutBit(bitStream, 0, 2); /* raw data blocks (0+1=1) */

    }

    /*
     * MPEG2 says byte_aligment() here, but ADTS always is multiple of 8 bits
     * MPEG4 has no byte_alignment() here
     */
    /*
    if (hEncoder->config.mpegVersion == 1)
        bits += ByteAlign(bitStream, writeFlag);
    */

#if 0 // Removed in corrigendum 14496-3:2002
    if (hEncoder->config.mpegVersion == 0)
        bits += 2; /* emphasis */
#endif

    return bits;
}

static int32_t WriteCPE(CoderInfo *coderInfoL,
                        CoderInfo *coderInfoR,
                        ChannelInfo *channelInfo,
                        BitStream* bitStream,
                        int32_t objectType,
                        int32_t writeFlag)
{
    int32_t bits = 0;

    if (writeFlag) {
        /* write ID_CPE, single_element_channel() identifier */
        PutBit(bitStream, ID_CPE, LEN_SE_ID);

        /* write the element_identifier_tag */
        PutBit(bitStream, channelInfo->tag, LEN_TAG);

        /* common_window? */
        PutBit(bitStream, channelInfo->common_window, LEN_COM_WIN);
    }

    bits += LEN_SE_ID;
    bits += LEN_TAG;
    bits += LEN_COM_WIN;

    /* if common_window, write ics_info */
    if (channelInfo->common_window) {
        int32_t numWindows, maxSfb;

        bits += WriteICSInfo(coderInfoL, bitStream, objectType, channelInfo->common_window, writeFlag);
        numWindows = coderInfoL->num_window_groups;
        maxSfb = coderInfoL->max_sfb;

        if (writeFlag) {
            PutBit(bitStream, channelInfo->msInfo.is_present, LEN_MASK_PRES);
            if (channelInfo->msInfo.is_present == 1) {
                int32_t g;
                int32_t b;
                for (g=0;g<numWindows;g++) {
                    for (b=0;b<maxSfb;b++) {
                        PutBit(bitStream, channelInfo->msInfo.ms_used[g*maxSfb+b], LEN_MASK);
                    }
                }
            }
        }
        bits += LEN_MASK_PRES;
        if (channelInfo->msInfo.is_present == 1)
            bits += (numWindows*maxSfb*LEN_MASK);
    }

    /* Write individual_channel_stream elements */
    bits += WriteICS(coderInfoL, bitStream, channelInfo->common_window, objectType, writeFlag);
    bits += WriteICS(coderInfoR, bitStream, channelInfo->common_window, objectType, writeFlag);

    return bits;
}

static int32_t WriteSCE(CoderInfo *coderInfo,
                        ChannelInfo *channelInfo,
                        BitStream *bitStream,
                        int32_t objectType,
                        int32_t writeFlag)
{
    int32_t bits = 0;

#ifndef DRM
    if (writeFlag) {
        /* write Single Element Channel (SCE) identifier */
        PutBit(bitStream, ID_SCE, LEN_SE_ID);

        /* write the element identifier tag */
        PutBit(bitStream, channelInfo->tag, LEN_TAG);
    }

    bits += LEN_SE_ID;
    bits += LEN_TAG;
#endif

    /* Write an Individual Channel Stream element */
    bits += WriteICS(coderInfo, bitStream, 0, objectType, writeFlag);

    return bits;
}

static int32_t WriteLFE(CoderInfo *coderInfo,
                        ChannelInfo *channelInfo,
                        BitStream *bitStream,
                        int32_t objectType,
                        int32_t writeFlag)
{
    int32_t bits = 0;

    if (writeFlag) {
        /* write ID_LFE, lfe_element_channel() identifier */
        PutBit(bitStream, ID_LFE, LEN_SE_ID);

        /* write the element_identifier_tag */
        PutBit(bitStream, channelInfo->tag, LEN_TAG);
    }

    bits += LEN_SE_ID;
    bits += LEN_TAG;

    /* Write an individual_channel_stream element */
    bits += WriteICS(coderInfo, bitStream, 0, objectType, writeFlag);

    return bits;
}

static int32_t WriteICSInfo(CoderInfo *coderInfo,
                            BitStream *bitStream,
                            int32_t objectType,
                            int32_t common_window,
                            int32_t writeFlag)
{
    int32_t grouping_bits;
    int32_t bits = 0;

    if (writeFlag) {
        /* write out ics_info() information */
        PutBit(bitStream, 0, LEN_ICS_RESERV);  /* reserved Bit*/

        /* Write out window sequence */
        PutBit(bitStream, coderInfo->block_type, LEN_WIN_SEQ);  /* block type */

        /* Write out window shape */
        PutBit(bitStream, coderInfo->window_shape, LEN_WIN_SH);  /* window shape */
    }

    bits += LEN_ICS_RESERV;
    bits += LEN_WIN_SEQ;
    bits += LEN_WIN_SH;

    /* For short windows, write out max_sfb and scale_factor_grouping */
    if (coderInfo->block_type == ONLY_SHORT_WINDOW){
        if (writeFlag) {
            PutBit(bitStream, coderInfo->max_sfb, LEN_MAX_SFBS);
            grouping_bits = FindGroupingBits(coderInfo);
            PutBit(bitStream, grouping_bits, MAX_SHORT_WINDOWS - 1);  /* the grouping bits */
        }
        bits += LEN_MAX_SFBS;
        bits += MAX_SHORT_WINDOWS - 1;
    } else { /* Otherwise, write out max_sfb and predictor data */
        if (writeFlag) {
            PutBit(bitStream, coderInfo->max_sfb, LEN_MAX_SFBL);
        }
        bits += LEN_MAX_SFBL;
#ifdef DRM
    }
    if (writeFlag) {
        PutBit(bitStream,coderInfo->tnsInfo.tnsDataPresent,LEN_TNS_PRES);
    }
    bits += LEN_TNS_PRES;
#endif
#if 0
        if (objectType == LTP)
        {
            bits++;
            if(writeFlag)
                PutBit(bitStream, coderInfo->ltpInfo.global_pred_flag, 1); /* Prediction Global used */

            bits += WriteLTPPredictorData(coderInfo, bitStream, writeFlag);
            if (common_window)
                bits += WriteLTPPredictorData(coderInfo, bitStream, writeFlag);
        } else {
#endif
            bits++;
            if (writeFlag)
                PutBit(bitStream, coderInfo->pred_global_flag, LEN_PRED_PRES);  /* predictor_data_present */

            bits += WritePredictorData(coderInfo, bitStream, writeFlag);
#if 0
        }
#endif
#ifndef DRM
    }
#endif

    return bits;
}

static int32_t WriteICS(CoderInfo *coderInfo,
                        BitStream *bitStream,
                        int32_t commonWindow,
                        int32_t objectType,
                        int32_t writeFlag)
{
    /* this function writes out an individual_channel_stream to the bitstream and */
    /* returns the number of bits written to the bitstream */
    int32_t bits = 0;

#ifndef DRM
    /* Write the 8-bit global_gain */
    if (writeFlag)
        PutBit(bitStream, coderInfo->global_gain, LEN_GLOB_GAIN);
    bits += LEN_GLOB_GAIN;
#endif

    /* Write ics information */
    if (!commonWindow) {
        bits += WriteICSInfo(coderInfo, bitStream, objectType, commonWindow, writeFlag);
    }

#ifdef DRM
    /* Write the 8-bit global_gain */
    if (writeFlag)
        PutBit(bitStream, coderInfo->global_gain, LEN_GLOB_GAIN);
    bits += LEN_GLOB_GAIN;
#endif

    bits += SortBookNumbers(coderInfo, bitStream, writeFlag);
    bits += WriteScalefactors(coderInfo, bitStream, writeFlag);
#ifdef DRM
    if (writeFlag) {
        /* length_of_reordered_spectral_data */
        PutBit(bitStream, coderInfo->iLenReordSpData, LEN_HCR_REORDSD);

        /* length_of_longest_codeword */
        PutBit(bitStream, coderInfo->iLenLongestCW, LEN_HCR_LONGCW);
    }
    bits += LEN_HCR_REORDSD + LEN_HCR_LONGCW;
#else
    bits += WritePulseData(coderInfo, bitStream, writeFlag);
#endif
    bits += WriteTNSData(coderInfo, bitStream, writeFlag);
#ifndef DRM
    bits += WriteGainControlData(coderInfo, bitStream, writeFlag);
#endif

#ifdef DRM
    /* DRM CRC calculation */
    if (writeFlag)
        calc_CRC(bitStream, bits);

    bits += WriteReorderedSpectralData(coderInfo, bitStream, writeFlag);
#else
    bits += WriteSpectralData(coderInfo, bitStream, writeFlag);
#endif

    /* Return number of bits */
    return bits;
}

#if 0
static int32_t WriteLTPPredictorData(CoderInfo *coderInfo, BitStream *bitStream, int32_t writeFlag)
{
    int32_t i, last_band;
    int32_t bits;
    LtpInfo *ltpInfo = &coderInfo->ltpInfo;

    bits = 0;

    if (ltpInfo->global_pred_flag) {
        if(writeFlag)
            PutBit(bitStream, 1, 1); /* LTP used */
        bits++;

        switch(coderInfo->block_type)
        {
        case ONLY_LONG_WINDOW:
        case LONG_SHORT_WINDOW:
        case SHORT_LONG_WINDOW:
            bits += LEN_LTP_LAG;
            bits += LEN_LTP_COEF;
            if(writeFlag)
            {
                PutBit(bitStream, ltpInfo->delay[0], LEN_LTP_LAG);
                PutBit(bitStream, ltpInfo->weight_idx,  LEN_LTP_COEF);
            }

            last_band = ((coderInfo->nr_of_sfb < MAX_LT_PRED_LONG_SFB) ?
                coderInfo->nr_of_sfb : MAX_LT_PRED_LONG_SFB);
//            last_band = coderInfo->nr_of_sfb;

            bits += last_band;
            if(writeFlag)
                for (i = 0; i < last_band; i++)
                    PutBit(bitStream, ltpInfo->sfb_prediction_used[i], LEN_LTP_LONG_USED);
            break;

        default:
            break;
        }
    }

    return (bits);
}
#endif

static int32_t WritePredictorData(CoderInfo *coderInfo,
                                  BitStream *bitStream,
                                  int32_t writeFlag)
{
    int32_t bits = 0;

    /* Write global predictor data present */
    short_t predictorDataPresent = (short_t)coderInfo->pred_global_flag;
    int32_t numBands = min(coderInfo->max_pred_sfb, coderInfo->nr_of_sfb);

    if (writeFlag) {
        if (predictorDataPresent) {
            int32_t b;
            if (coderInfo->reset_group_number == -1) {
                PutBit(bitStream, 0, LEN_PRED_RST); /* No prediction reset */
            } else {
                PutBit(bitStream, 1, LEN_PRED_RST);
                PutBit(bitStream, (ulong_t)coderInfo->reset_group_number,
                    LEN_PRED_RSTGRP);
            }

            for (b=0;b<numBands;b++) {
                PutBit(bitStream, coderInfo->pred_sfb_flag[b], LEN_PRED_ENAB);
            }
        }
    }
    bits += (predictorDataPresent) ?
        (LEN_PRED_RST +
        ((coderInfo->reset_group_number)!=-1)*LEN_PRED_RSTGRP +
        numBands*LEN_PRED_ENAB) : 0;

    return bits;
}

static int32_t WritePulseData(CoderInfo *coderInfo,
                              BitStream *bitStream,
                              int32_t writeFlag)
{
    int32_t bits = 0;

    if (writeFlag) {
        PutBit(bitStream, 0, LEN_PULSE_PRES);  /* no pulse_data_present */
    }

    bits += LEN_PULSE_PRES;

    return bits;
}

static int32_t WriteTNSData(CoderInfo *coderInfo,
                            BitStream *bitStream,
                            int32_t writeFlag)
{
    int32_t bits = 0;
#if 0
    int32_t numWindows;
    int32_t len_tns_nfilt;
    int32_t len_tns_length;
    int32_t len_tns_order;
    int32_t filtNumber;
    int32_t resInBits;
    int32_t bitsToTransmit;
    ulong_t unsignedIndex;
    int32_t w;
#endif

    TnsInfo* tnsInfoPtr = &coderInfo->tnsInfo;

#ifndef DRM
    if (writeFlag) {
        PutBit(bitStream,tnsInfoPtr->tnsDataPresent,LEN_TNS_PRES);
    }
    bits += LEN_TNS_PRES;
#endif

    /* If TNS is not present, bail */
    if (!tnsInfoPtr->tnsDataPresent) {
        return bits;
    }

    return 0;

#if 0
    /* Set window-dependent TNS parameters */
    if (coderInfo->block_type == ONLY_SHORT_WINDOW) {
        numWindows = MAX_SHORT_WINDOWS;
        len_tns_nfilt = LEN_TNS_NFILTS;
        len_tns_length = LEN_TNS_LENGTHS;
        len_tns_order = LEN_TNS_ORDERS;
    }
    else {
        numWindows = 1;
        len_tns_nfilt = LEN_TNS_NFILTL;
        len_tns_length = LEN_TNS_LENGTHL;
        len_tns_order = LEN_TNS_ORDERL;
    }

    /* Write TNS data */
    bits += (numWindows * len_tns_nfilt);
    for (w=0;w<numWindows;w++) {
        TnsWindowData* windowDataPtr = &tnsInfoPtr->windowData[w];
        int32_t numFilters = windowDataPtr->numFilters;
        if (writeFlag) {
            PutBit(bitStream,numFilters,len_tns_nfilt); /* n_filt[] = 0 */
        }
        if (numFilters) {
            bits += LEN_TNS_COEFF_RES;
            resInBits = windowDataPtr->coefResolution;
            if (writeFlag) {
                PutBit(bitStream,resInBits-DEF_TNS_RES_OFFSET,LEN_TNS_COEFF_RES);
            }
            bits += numFilters * (len_tns_length+len_tns_order);
            for (filtNumber=0;filtNumber<numFilters;filtNumber++) {
                TnsFilterData* tnsFilterPtr=&windowDataPtr->tnsFilter[filtNumber];
                int32_t order = tnsFilterPtr->order;
                if (writeFlag) {
                    PutBit(bitStream,tnsFilterPtr->length,len_tns_length);
                    PutBit(bitStream,order,len_tns_order);
                }
                if (order) {
                    bits += (LEN_TNS_DIRECTION + LEN_TNS_COMPRESS);
                    if (writeFlag) {
                        PutBit(bitStream,tnsFilterPtr->direction,LEN_TNS_DIRECTION);
                        PutBit(bitStream,tnsFilterPtr->coefCompress,LEN_TNS_COMPRESS);
                    }
                    bitsToTransmit = resInBits - tnsFilterPtr->coefCompress;
                    bits += order * bitsToTransmit;
                    if (writeFlag) {
                        int32_t i;
                        for (i=1;i<=order;i++) {
                            unsignedIndex = (ulong_t) (tnsFilterPtr->index[i])&(~(~0<<bitsToTransmit));
                            PutBit(bitStream,unsignedIndex,bitsToTransmit);
                        }
                    }
                }
            }
        }
    }
    return bits;
#endif
}

static int32_t WriteGainControlData(CoderInfo *coderInfo,
                                    BitStream *bitStream,
                                    int32_t writeFlag)
{
    int32_t bits = 0;

    if (writeFlag) {
        PutBit(bitStream, 0, LEN_GAIN_PRES);
    }

    bits += LEN_GAIN_PRES;

    return bits;
}

static int32_t WriteSpectralData(CoderInfo *coderInfo,
                                 BitStream *bitStream,
                                 int32_t writeFlag)
{
    int32_t i, bits = 0;

    /* set up local pointers to data and len */
    /* data array contains data to be written */
    /* len array contains lengths of data words */
    int* data = coderInfo->data;
    int* len  = coderInfo->len;

    if (writeFlag) {
        for(i = 0; i < coderInfo->spectral_count; i++) {
            if (len[i] > 0) {  /* only send out non-zero codebook data */
                PutBit(bitStream, data[i], len[i]); /* write data */
                bits += len[i];
            }
        }
    } else {
        for(i = 0; i < coderInfo->spectral_count; i++) {
            bits += len[i];
        }
    }

    return bits;
}

static int32_t WriteAACFillBits(BitStream* bitStream,
                                int32_t numBits,
                                int32_t writeFlag)
{
    int32_t numberOfBitsLeft = numBits;

    /* Need at least (LEN_SE_ID + LEN_F_CNT) bits for a fill_element */
    int32_t minNumberOfBits = LEN_SE_ID + LEN_F_CNT;

    while (numberOfBitsLeft >= minNumberOfBits)
    {
        int32_t numberOfBytes;
        int32_t maxCount;

        if (writeFlag) {
            PutBit(bitStream, ID_FIL, LEN_SE_ID);   /* Write fill_element ID */
        }
        numberOfBitsLeft -= minNumberOfBits;    /* Subtract for ID,count */

        numberOfBytes = (int)(numberOfBitsLeft/LEN_BYTE);
        maxCount = (1<<LEN_F_CNT) - 1;  /* Max count without escaping */

        /* if we have less than maxCount bytes, write them now */
        if (numberOfBytes < maxCount) {
            int32_t i;
            if (writeFlag) {
                PutBit(bitStream, numberOfBytes, LEN_F_CNT);
                for (i = 0; i < numberOfBytes; i++) {
                    PutBit(bitStream, 0, LEN_BYTE);
                }
            }
            /* otherwise, we need to write an escape count */
        }
        else {
            int32_t maxEscapeCount, maxNumberOfBytes, escCount;
            int32_t i;
            if (writeFlag) {
                PutBit(bitStream, maxCount, LEN_F_CNT);
            }
            maxEscapeCount = (1<<LEN_BYTE) - 1;  /* Max escape count */
            maxNumberOfBytes = maxCount + maxEscapeCount;
            numberOfBytes = (numberOfBytes > maxNumberOfBytes ) ? (maxNumberOfBytes) : (numberOfBytes);
            escCount = numberOfBytes - maxCount;
            if (writeFlag) {
                PutBit(bitStream, escCount, LEN_BYTE);
                for (i = 0; i < numberOfBytes-1; i++) {
                    PutBit(bitStream, 0, LEN_BYTE);
                }
            }
        }
        numberOfBitsLeft -= LEN_BYTE*numberOfBytes;
    }

    return numberOfBitsLeft;
}

static int32_t FindGroupingBits(CoderInfo *coderInfo)
{
    /* This function inputs the grouping information and outputs the seven bit
    'grouping_bits' field that the AAC decoder expects.  */

    int32_t grouping_bits = 0;
    int32_t tmp[8];
    int32_t i, j;
    int32_t index = 0;

    for (i = 0; i < coderInfo->num_window_groups; i++) {
        for (j = 0; j < coderInfo->window_group_length[i]; j++) {
            tmp[index++] = i;
        }
    }

    for (i = 1; i < 8; i++) {
        grouping_bits = grouping_bits << 1;
        if(tmp[i] == tmp[i-1]) {
            grouping_bits++;
        }
    }

    return grouping_bits;
}

/* size in bytes! */
BitStream *OpenBitStream(int32_t size, uchar_t *buffer)
{
    BitStream *bitStream;

#ifdef FAAC_STATIC_MEMORY
    bitStream = &faac_bitStream;
#else
    bitStream = (BitStream *)AllocMemory(sizeof(BitStream));
#endif

    PutBits_init(bitStream, buffer, size);
    
    return bitStream;
}

int32_t CloseBitStream(BitStream *bitStream)
{
    int32_t bytes;
#ifdef DUMP_BITSTREAM
    int32_t i;
#endif
    bytes = bitStream->buf_ptr - bitStream->buf;
#ifdef DUMP_BITSTREAM
    printf("CloseBitStream, bytes = %d\n",bytes);
    for (i=0;i<bytes;i++) {
        printf("%02x",bitStream->buf[i]);
        if (!((i+1)%16))
            printf("\n");
        else
            printf("-");
    }
    printf("\n");
#endif

#ifndef FAAC_STATIC_MEMORY
    FreeMemory(bitStream);
#endif

    return bytes;
}

#if 0
static long_t BufferNumBit(BitStream *bitStream)
{
    return 0; // may have bug here
}
#endif

static void PutBits_init(BitStream *bitStream, uchar_t *buffer, int buffer_size)
{
    bitStream->buf = buffer;
    bitStream->buf_end = bitStream->buf + buffer_size;
    bitStream->buf_ptr = bitStream->buf;
    bitStream->bit_left=32;
    bitStream->bit_buf=0;
}

void PutBit(BitStream *bitStream, uint32_t value, int32_t n)
{
    unsigned int bit_buf;
    register int bit_left;
   
    bit_buf = bitStream->bit_buf;
    bit_left = bitStream->bit_left;

    /* XXX: optimize */
    if (n < bit_left) {
        bit_buf = (bit_buf<<n) | value;
        bit_left-=n;
    } else {
        bit_buf<<=bit_left;
        bit_buf |= value >> (n - bit_left);
        *(uint32_t *)bitStream->buf_ptr = be2me_32(bit_buf);
        bitStream->buf_ptr+=4;
        bit_left+=32 - n;
        bit_buf = value;
    }

    bitStream->bit_buf = bit_buf;
    bitStream->bit_left = bit_left;

}

/* pad the end of the output stream with zeros */
static int32_t flush_put_bits(BitStream *bitStream,int32_t writeFlag)
{
    int32_t padbits;

    padbits = 32-bitStream->bit_left;

    if ( writeFlag ) {
        bitStream->bit_buf<<= bitStream->bit_left;
        while (bitStream->bit_left < 32) {
            /* XXX: should test end of buffer */
            *bitStream->buf_ptr++=(uint8_t)(bitStream->bit_buf>>24);
            bitStream->bit_buf<<=8;
            bitStream->bit_left+=8;
        }
        bitStream->bit_left=32;
        bitStream->bit_buf=0;
    }
    
    return padbits;
}

#if 0
static int32_t ByteAlign(BitStream *bitStream, int32_t writeFlag)
{
    register int32_t len, i,j;

    len = BufferNumBit(bitStream);

    j = (8 - (len%8))%8;

    if ((len % 8) == 0) j = 0;
    if (writeFlag) {
        for( i=0; i<j; i++ ) {
            PutBit(bitStream, 0, 1);
        }
    }
    return j;
}
#endif

