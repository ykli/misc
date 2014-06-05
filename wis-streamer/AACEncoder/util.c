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
 * $Id: util.c,v 1.6 2006/07/10 20:30:38 kyang Exp $
 */


#include "fixed.h"
#include "util.h"

/* Returns the sample rate index */
int32_t GetSRIndex(uint32_t sampleRate)
{
    if (92017 <= sampleRate) return 0;
    if (75132 <= sampleRate) return 1;
    if (55426 <= sampleRate) return 2;
    if (46009 <= sampleRate) return 3;
    if (37566 <= sampleRate) return 4;
    if (27713 <= sampleRate) return 5;
    if (23004 <= sampleRate) return 6;
    if (18783 <= sampleRate) return 7;
    if (13856 <= sampleRate) return 8;
    if (11502 <= sampleRate) return 9;
    if (9391 <= sampleRate) return 10;

    return 11;
}

/* Returns the maximum bitrate per channel for that sampling frequency */
uint32_t MaxBitrate(ulong_t sampleRate)
{
    /*
     *  Maximum of 6144 bit for a channel
     */
    return (int32_t)REAL2INT(6144 * (REAL_ICONST(sampleRate)>>10) + REAL_CONST(0.5));
}

/* Returns the minimum bitrate per channel for that sampling frequency */
uint32_t MinBitrate()
{
    return 8000;
}


/* Max prediction band for backward predictionas function of fs index */
const int32_t MaxPredSfb[] = { 33, 33, 38, 40, 40, 40, 41, 41, 37, 37, 37, 34, 0 };

int32_t GetMaxPredSfb(int32_t samplingRateIdx)
{
    return MaxPredSfb[samplingRateIdx];
}

/* Returns the maximum bit reservoir size */
uint32_t MaxBitresSize(ulong_t bitRate, ulong_t sampleRate)
{
    return 6144 - REAL2INT(REAL_ICONST(bitRate)/(sampleRate<<10));
}
