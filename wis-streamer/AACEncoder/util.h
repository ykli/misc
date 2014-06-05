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
 * $Id: util.h,v 1.3 2006/02/20 22:53:31 kyang Exp $
 */

#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdlib.h>
#include <memory.h>

/* Memory functions */
#define AllocMemory(size) malloc(size)
#define FreeMemory(block) free(block)
#define SetMemory(block, value, size) memset(block, value, size)

int32_t GetSRIndex(uint32_t sampleRate);
int32_t GetMaxPredSfb(int32_t samplingRateIdx);
uint32_t MaxBitrate(ulong_t sampleRate);
uint32_t MinBitrate();
uint32_t MaxBitresSize(ulong_t bitRate, ulong_t sampleRate);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UTIL_H */
