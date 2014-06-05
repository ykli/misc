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
 * $Id: channels.h,v 1.3 2006/02/20 22:53:29 kyang Exp $
 */

#ifndef CHANNEL_H
#define CHANNEL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "coder.h"

typedef struct {
    int32_t is_present;
    int32_t ms_used[MAX_SCFAC_BANDS];
} MSInfo;

typedef struct {
    int32_t tag;
    int32_t present;
    int32_t ch_is_left;
    int32_t paired_ch;
    int32_t common_window;
    int32_t cpe;
    int32_t sce;
    int32_t lfe;
    MSInfo msInfo;
} ChannelInfo;

void GetChannelInfo(ChannelInfo *channelInfo, int32_t numChannels, int32_t useLfe);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CHANNEL_H */
