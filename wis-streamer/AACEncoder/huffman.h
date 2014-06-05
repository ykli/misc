/***********

This software module was originally developed by Dolby
Laboratories in the course of development of the MPEG-2 AAC/MPEG-4
Audio standard ISO/IEC13818-7, 14496-1, 2 and 3. This software module is an implementation of a part
of one or more MPEG-2 AAC/MPEG-4 Audio tools as specified by the
MPEG-2 aac/MPEG-4 Audio standard. ISO/IEC  gives users of the
MPEG-2aac/MPEG-4 Audio standards free license to this software module
or modifications thereof for use in hardware or software products
claiming conformance to the MPEG-2 aac/MPEG-4 Audio  standards. Those
intending to use this software module in hardware or software products
are advised that this use may infringe existing patents. The original
developer of this software module, the subsequent
editors and their companies, and ISO/IEC have no liability for use of
this software module or modifications thereof in an
implementation. Copyright is not released for non MPEG-2 aac/MPEG-4
Audio conforming products. The original developer retains full right to
use the code for the developer's own purpose, assign or donate the code to a
third party and to inhibit third party from using the code for non
MPEG-2 aac/MPEG-4 Audio conforming products. This copyright notice
must be included in all copies or derivative works. Copyright 1996.

***********/
/*
 * $Id: huffman.h,v 1.5 2006/06/06 23:07:22 kyang Exp $
 */

#ifndef HUFFMAN_H
#define HUFFMAN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "bitstream.h"
#include "coder.h"

/* Huffman tables */
#define MAXINDEX 289
#define NUMINTAB 2
#define FIRSTINTAB 0
#define LASTINTAB 1

#define INTENSITY_HCB 15
#define INTENSITY_HCB2 14


#define ABS(A) ((A) < 0 ? (-A) : (A))

#if 1
#define MUL3(x) (((x)<<1)+(x))
#define MUL8(x) ((x)<<3)
#define MUL9(x) (((x)<<3)+(x))
#define MUL13(x) (((x)<<3)+((x)<<2)+(x))
#define MUL17(x) (((x)<<4)+(x))
#define MUL27(x) (((x)<<5)-((x)<<2)-(x))
#else
#define MUL3(x) (3*(x))
#define MUL8(x) (8*(x))
#define MUL9(x) (9*(x))
#define MUL13(x) (13*(x))
#define MUL17(x) (17*(x))
#define MUL27(x) (27*(x))
#endif

#include "frame.h"

void HuffmanInit(CoderInfo *coderInfo, uint32_t numChannels);
void HuffmanEnd(CoderInfo *coderInfo, uint32_t numChannels);

int32_t BitSearch(CoderInfo *coderInfo,
              int32_t *quant);

int32_t NoiselessBitCount(CoderInfo *coderInfo,
                      int32_t *quant,
                      int32_t hop,
                      int32_t min_book_choice[112][3]);

//static int32_t CalculateEscSequence(int32_t input, int32_t *len_esc_sequence);

INLINE int32_t CalcBits(CoderInfo *coderInfo,
             int32_t book,
             int32_t *quant,
             int32_t offset,
             int32_t length);

int32_t OutputBits(CoderInfo *coderInfo,
#ifdef DRM
               int32_t *book, /* we need to change book for VCB11 */
#else
               int32_t book,
#endif
               int32_t *quant,
               int32_t offset,
               int32_t length);

int32_t SortBookNumbers(CoderInfo *coderInfo,
                    BitStream *bitStream,
                    int32_t writeFlag);

int32_t WriteScalefactors(CoderInfo *coderInfo,
                      BitStream *bitStream,
                      int32_t writeFlag);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* HUFFMAN_H */
