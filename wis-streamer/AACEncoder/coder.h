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
 * $Id: coder.h,v 1.4 2006/07/13 00:49:48 kyang Exp $
 */

#ifndef CODER_H
#define CODER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Allow encoding of Digital Radio Mondiale (DRM) */
//#define DRM

#define MAX_CHANNELS 2//64

#ifdef DRM
# define FRAME_LEN 1024//960 // not working with 960, fixme
# define BLOCK_LEN_LONG 1024//960
# define BLOCK_LEN_SHORT 128//120
#else
# define FRAME_LEN 1024
# define BLOCK_LEN_LONG 1024
# define BLOCK_LEN_SHORT 128
#endif

#define NSFB_LONG  51
#define NSFB_SHORT 15
#define MAX_SHORT_WINDOWS 8
#define MAX_SCFAC_BANDS ((NSFB_SHORT+1)*MAX_SHORT_WINDOWS)

enum WINDOW_TYPE {
    ONLY_LONG_WINDOW,
    LONG_SHORT_WINDOW,
    ONLY_SHORT_WINDOW,
    SHORT_LONG_WINDOW
};

#define TNS_MAX_ORDER 20
//#define DEF_TNS_GAIN_THRESH REAL_CONST(1.4)
//#define DEF_TNS_COEFF_THRESH REAL_CONST(0.1)
#define DEF_TNS_COEFF_RES 4
#define DEF_TNS_RES_OFFSET 3
#define LEN_TNS_NFILTL 2
#define LEN_TNS_NFILTS 1

#define DELAY 2048
#define LEN_LTP_DATA_PRESENT 1
#define LEN_LTP_LAG 11
#define LEN_LTP_COEF 3
#define LEN_LTP_SHORT_USED 1
#define LEN_LTP_SHORT_LAG_PRESENT 1
#define LEN_LTP_SHORT_LAG 5
#define LTP_LAG_OFFSET 16
#define LEN_LTP_LONG_USED 1
#define MAX_LT_PRED_LONG_SFB 40
#define MAX_LT_PRED_SHORT_SFB 13
#define SHORT_SQ_OFFSET (BLOCK_LEN_LONG-(BLOCK_LEN_SHORT*4+BLOCK_LEN_SHORT/2))
#define CODESIZE 8
#define NOK_LT_BLEN (3 * BLOCK_LEN_LONG)

#define SBMAX_L 49
#define LPC 2

typedef struct {
    int32_t tnsDataPresent;
    int32_t tnsMinBandNumberLong;
    int32_t tnsMinBandNumberShort;
    int32_t tnsMaxBandsLong;
    int32_t tnsMaxBandsShort;
    int32_t tnsMaxOrderLong;
    int32_t tnsMaxOrderShort;
} TnsInfo;

typedef struct {
    int32_t window_shape;
    int32_t prev_window_shape;
    int32_t block_type;
    int32_t desired_block_type;

    int32_t global_gain;
    int32_t scale_factor[MAX_SCFAC_BANDS];

    int32_t num_window_groups;
    int32_t window_group_length[8];
    int32_t max_sfb;
    int32_t nr_of_sfb;
    int32_t sfb_offset[250];
    int32_t lastx;
    eng_t avgenrg;

    int32_t spectral_count;

    /* Huffman codebook selected for each sf band */
    int32_t book_vector[MAX_SCFAC_BANDS];

    /* Data of spectral bitstream elements, for each spectral pair,
       5 elements are required: 1*(esc)+2*(sign)+2*(esc value)=5 */
    int32_t *data;

    /* Lengths of spectral bitstream elements */
    int32_t *len;

    TnsInfo tnsInfo;
    
    int32_t max_pred_sfb;
    int32_t pred_global_flag;
    int32_t pred_sfb_flag[MAX_SCFAC_BANDS];
    int32_t reset_group_number;

} CoderInfo;

typedef struct {
  ulong_t sampling_rate;  /* the following entries are for this sampling rate */
  int32_t num_cb_long;
  int32_t num_cb_short;
  int32_t cb_width_long[NSFB_LONG];
  int32_t cb_width_short[NSFB_SHORT];
} SR_INFO;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CODER_H */
