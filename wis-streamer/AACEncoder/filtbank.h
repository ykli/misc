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
 * $Id: filtbank.h,v 1.5 2006/07/13 00:49:48 kyang Exp $
 */

#ifndef FILTBANK_H
#define FILTBANK_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "frame.h"

#define NFLAT_LS 448


#define MOVERLAPPED     0
#define MNON_OVERLAPPED 1


#define SINE_WINDOW 0
#define KBD_WINDOW  1

void FilterBankInit(faacEncHandle hEncoder);

void FilterBankEnd(faacEncHandle hEncoder);

void FilterBank(faacEncHandle hEncoder,
                CoderInfo *coderInfo,
                int16_t *p_in_data,
                coef_t *p_out_mdct,
                int16_t *p_overlap,
                int32_t overlap_select);

void specFilter(coef_t *freqBuff,
                int32_t sampleRate,
                int32_t lowpassFreq,
                int32_t specLen );

void FilterBankPow(int32_t N, 
                   coef_t* data, 
                   pow_t* data2);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FILTBANK_H */
