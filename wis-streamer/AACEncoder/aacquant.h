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
 * $Id: aacquant.h,v 1.4 2006/07/13 00:49:48 kyang Exp $
 */

#ifndef AACQUANT_H
#define AACQUANT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "coder.h"

#define IXMAX_VAL 8191
#define PRECALC_SIZE (IXMAX_VAL+2)
#define SF_OFFSET 100

#pragma pack(push, 1)
typedef struct
  {
    real_32_t quality;
  } AACQuantCfg;
#pragma pack(pop)

void AACQuantize(CoderInfo *coderInfo,
                    ChannelInfo *channelInfo,
                    int32_t *cb_width,
                    int32_t num_cb,
                    coef_t *xr,
                    pow_t *xr2,
                    AACQuantCfg *aacquantcfg);

void CalcAvgEnrg(CoderInfo *coderInfo,
                 const coef_t *xr,
                 const pow_t *xr2);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AACQUANT_H */
