/*
 * FAAC - Freeware Advanced Audio Coder
 * Copyright (C) 2001 Menno Bakker
 * Copyright (C) 2002, 2003 Krzysztof Nikiel
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
 * $Id: aacquant.c,v 1.9 2006/07/13 00:49:48 kyang Exp $
 */

#include <stdlib.h>
#include <stdio.h>

#include "fixed.h"
#include "frame.h"
#include "aacquant.h"
#include "coder.h"
#include "huffman.h"
#include "util.h"

static void QuantizeBand(const coef_t *xp, int32_t *ix,
                         int32_t offset, int32_t end)
{
    register int32_t j;
    static coef_t realconst1 = COEF_CONST(0.5);

    for (j = offset; j < end; j++) {
        ix[j] = (int32_t)COEF2INT(xp[j]+realconst1);
#ifdef DUMP_XI
        printf("ix[%d] = %d\n",j,ix[j]);
#endif
    }
}

/* 
 * Parameter:
 *  coderInfo(I)
 *  xr(I)
 *  xmin(O)
 *  quality(I)
 */
static void CalcAllowedDist(CoderInfo *coderInfo, 
                            pow_t *xr2, real_32_t *xmin, real_32_t quality)
{
    register int32_t sfb, start, end, l;
    int32_t last = coderInfo->lastx;
    int32_t lastsb = 0;
    int32_t *cb_offset = coderInfo->sfb_offset;
    int32_t num_cb = coderInfo->nr_of_sfb;
    eng_t avgenrg = coderInfo->avgenrg;
    static real_32_t realconst1 = REAL_CONST(0.075);
    static real_32_t realconst2 = REAL_CONST(1.4);
    static real_32_t realconst3 = REAL_CONST(0.4);
    static real_32_t realconst4 = REAL_CONST(147.84); /* 132 * 1.12 */

    for (sfb = 0; sfb < num_cb; sfb++)  {
        if (last > cb_offset[sfb])
            lastsb = sfb;
    }

    for (sfb = 0; sfb < num_cb; sfb++)  {
        real_t thr;
        real_32_t tmp;
        eng_t enrg = 0;

        start = cb_offset[sfb];
        end = cb_offset[sfb + 1];

        if (sfb > lastsb)  {
            xmin[sfb] = 0;
            continue;
        }

        if (coderInfo->block_type != ONLY_SHORT_WINDOW) {
            eng_t enmax = -1;
            int32_t lmax;

            lmax = start;
            for (l = start; l < end; l++) {
                if (enmax < xr2[l]) {
                    enmax = xr2[l];
                    lmax = l;
                }
            }

            start = lmax - 2;
            end = lmax + 3;

            if (start < 0)
                start = 0;
        
            if (end > last)
                end = last;
        }

        for (l = start; l < end; l++) {
            enrg += xr2[l];
        }

        if ( (avgenrg == 0) || (enrg==0) )
            thr = 0;
        else {
            thr = (avgenrg<<REAL_BITS)*(end-start)/enrg;
            thr = faac_pow(thr, REAL_ICONST(sfb)/(lastsb*10)-realconst3);
        }

        tmp = DIV_R(REAL_ICONST(last-start),REAL_ICONST(last));
        tmp = MUL_R(MUL_R(tmp,tmp),tmp) + realconst1;

        thr = MUL_R(realconst2,thr) + tmp;

        xmin[sfb] = DIV_R(DIV_R(realconst4,thr),quality);
#ifdef DUMP_XMIN
        printf("xmin[%d] = %.8f\n",sfb,REAL2FLOAT(xmin[sfb]));
#endif
    }
#ifdef DUMP_XMIN
//    exit(1);
#endif
}

/* 
 * Parameter:
 *  coderInfo(IO)(O:coderInfo->scale_factor)
 *  xr(unused)
 *  xr_pow(IO)
 *  xi(O)
 *  xmin(I)
 */
static int32_t FixNoise(CoderInfo *coderInfo,
                        coef_t *xr_pow,
                        int32_t *xi,
                        real_32_t *xmin)
{
    register int32_t i, sb;
    int32_t start, end;
    static real_32_t log_ifqstep = REAL_CONST(7.6943735514); // 1.0 / log(ifqstep) 
    static real_32_t realconst1 = REAL_CONST(0.6931471806); //log2
    static real_32_t realconst2 = REAL_CONST(0.5);

    for (sb = 0; sb < coderInfo->nr_of_sfb; sb++) {
        eng_t eng = 0;
        frac_t maxfixstep;

        start = coderInfo->sfb_offset[sb];
        end = coderInfo->sfb_offset[sb+1];

        for (  i=start; i< end; i++)
            eng += (int64_t)(COEF2INT(xr_pow[i]))*COEF2INT(xr_pow[i]);

        if ( (eng == 0) || (xmin[sb]==0) )
            maxfixstep = FRAC_ICONST(1);
        else {
            maxfixstep = faac_sqrt(eng/(end-start)*
                                   MUL_R(xmin[sb],faac_sqrt(xmin[sb]))
                                  );
            if ( maxfixstep == 0 )
                maxfixstep = FRAC_ICONST(1);
            else
                maxfixstep = ((real_t)1<<(FRAC_BITS+REAL_BITS))/maxfixstep;
        }
#ifdef DUMP_MAXFIXSTEP
        printf("sb = %d, maxfixstep = %.8f\n",sb, FRAC2FLOAT(maxfixstep));
#endif
        for (i = start; i < end; i++) {
#ifdef DUMP_MAXFIXSTEP
            printf("xr_pow[%d] = %.8f\t",i,COEF2FLOAT(xr_pow[i]));
#endif
            xr_pow[i] = MUL_F(xr_pow[i],maxfixstep);
#ifdef DUMP_MAXFIXSTEP
            printf("xr_pow[%d]*fix = %.8f\n",i,COEF2FLOAT(xr_pow[i]));
#endif
        }
        QuantizeBand(xr_pow, xi, start, end);
        coderInfo->scale_factor[sb] = (int32_t)REAL2INT(MUL_R(faac_log(maxfixstep)-FRAC2REAL_BIT*realconst1,
                                                     log_ifqstep) - realconst2)+1;

#ifdef DUMP_MAXFIXSTEP
      printf("scale_factor = %d\n", coderInfo->scale_factor[sb]);
#endif
    }
    
#ifdef DUMP_MAXFIXSTEP
    exit(1);
#endif
    return 0;
}

void CalcAvgEnrg(CoderInfo *coderInfo, const coef_t *xr, const pow_t *xr2)
{
    int32_t end, l;
    int32_t last = 0;
    eng_t totenrg = 0;

    end = coderInfo->sfb_offset[coderInfo->nr_of_sfb];
    for (l = 0; l < end; l++)  {
        if (xr[l]!=COEF_ICONST(0)) {
            last = l;
            totenrg += xr2[l];
        }
    }
    last++;

    coderInfo->lastx = last;
    coderInfo->avgenrg = totenrg / last;
}

void AACQuantize(CoderInfo *coderInfo,
                    ChannelInfo *channelInfo,
                    int32_t *cb_width,
                    int32_t num_cb,
                    coef_t *xr,
                    pow_t *xr2,
                    AACQuantCfg *aacquantCfg)
{
    register int32_t sb, i;
    coef_t xr_pow[FRAME_LEN];
    real_32_t xmin[MAX_SCFAC_BANDS];
    int32_t xi[FRAME_LEN];
    int32_t *scale_factor;

    /* Use local copy's */
    scale_factor = coderInfo->scale_factor;

    /* Set all scalefactors to 0 */
    coderInfo->global_gain = 0;
    for (sb = 0; sb < coderInfo->nr_of_sfb; sb++)
        scale_factor[sb] = 0;

    /* Compute xr_pow */
    for (i = 0; i < FRAME_LEN; i++) {
        xr_pow[i] = faac_pow34(xr[i]);
#ifdef DUMP_XR_POW
        printf("xr_pow[%d] = %.8f\n",i,COEF2FLOAT(xr_pow[i]));
#endif
    }
#ifdef DUMP_XR_POW
//    exit(1);
#endif
    
    CalcAllowedDist(coderInfo, xr2, xmin, aacquantCfg->quality);
    coderInfo->global_gain = 0;
    
    FixNoise(coderInfo, xr_pow, xi, xmin);
   
    for ( i = 0; i < FRAME_LEN; i++ )  {
        if ( xr[i] < 0 )
            xi[i] = -xi[i];
    }

    BitSearch(coderInfo, xi);

    /* offset the difference of common_scalefac and scalefactors by SF_OFFSET  */
    for (i = 0; i < coderInfo->nr_of_sfb; i++) {
        if ( (coderInfo->book_vector[i]!=INTENSITY_HCB) && 
            (coderInfo->book_vector[i]!=INTENSITY_HCB2) ) {
            scale_factor[i] = coderInfo->global_gain - scale_factor[i] + SF_OFFSET;
        }
    }
    coderInfo->global_gain = scale_factor[0];

    /* place the codewords and their respective lengths in arrays data[] and len[] respectively */
    /* there are 'counter' elements in each array, and these are variable length arrays depending on the input */
    coderInfo->spectral_count = 0;
    sb = 0;
    for(i = 0; i < coderInfo->nr_of_sfb; i++) {
        OutputBits(
            coderInfo,
            coderInfo->book_vector[i],
            xi,
            coderInfo->sfb_offset[i],
            coderInfo->sfb_offset[i+1]-coderInfo->sfb_offset[i]);

        if (coderInfo->book_vector[i])
            sb = i;
    }

    // FIXME: Check those max_sfb/nr_of_sfb. Isn't it the same?
    coderInfo->max_sfb = coderInfo->nr_of_sfb = sb + 1;
}
