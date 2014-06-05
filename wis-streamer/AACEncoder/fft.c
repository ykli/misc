/*
 * FAAC - Freeware Advanced Audio Coder
 * $Id: fft.c,v 1.7 2006/06/07 01:50:10 kyang Exp $
 * Copyright (C) 2002 Krzysztof Nikiel
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
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "fixed.h"
#include "fft.h"
#include "util.h"

#ifdef FAAC_STATIC_MEMORY
static ushort_t faac_recodertbl[1<<FAAC_FFT_LOGM];
#endif

void fft_initialize( FFT_Tables *fft_tables )
{
    register int32_t i;
    int32_t size = (1<<FAAC_FFT_LOGM);

#ifdef FAAC_STATIC_MEMORY
    fft_tables->reordertbl = (ushort_t *)faac_recodertbl;
#else
    fft_tables->reordertbl = (ushort_t *)AllocMemory((1<<FAAC_FFT_LOGM)*sizeof(ushort_t));
#endif

    for (i = 0; i < size; i++) {
        register int32_t b0;
        int32_t reversed = 0;
        int32_t tmp = i;
        for (b0 = 0; b0 < FAAC_FFT_LOGM; b0++) {
            reversed = (reversed << 1) | (tmp & 1);
            tmp >>= 1;
        }
        fft_tables->reordertbl[i] = (ushort_t)reversed;
    }

    fft_tables->costbl = faac_fft_costbl;
    fft_tables->negsintbl = faac_fft_negsintbl;
}

void fft_terminate( FFT_Tables *fft_tables )
{
    fft_tables->costbl      = NULL;
    fft_tables->negsintbl   = NULL;
    fft_tables->reordertbl  = NULL;

#ifndef FAAC_STATIC_MEMORY
    if ( fft_tables->reordertbl )
        FreeMemory(fft_tables->reordertbl);
#endif
}

static void reorder( FFT_Tables *fft_tables, coef_t *x, int32_t logm)
{
    int32_t j;
    register int32_t i;
    static int32_t size = (1<<FAAC_FFT_LOGM);
    coef_t tmp;

    for (i = 0; i < size; i++) {
        j = fft_tables->reordertbl[i];
        
        if (j > i) {
            tmp = x[i];
            x[i] = x[j];
            x[j] = tmp;
        }
    }
}

INLINE static void fft_proc(coef_t *xr, 
                            coef_t *xi,
                            fftfloat *refac, 
                            fftfloat *imfac, 
                            int32_t size)	
{
    register int32_t step, shift, pos;
    register int32_t x1;
    register int32_t x2;
    int32_t exp, estep;
    int64_t v2r, v2i;
//    int64_t v2i1;
#ifdef FAAC_USE_ASM
    int32_t hi,lo;
#endif
    
    estep = size;
    for (step = 1; step < size; step = (step<<1)) {
        x2 = 0;
        estep >>= 1;
        for (pos = 0; pos < size; pos += (step<<1)) {
            x1 = x2;
            x2 += step;
            exp = 0;
            for (shift = 0; shift < step; shift++) {
#ifdef FAAC_USE_ASM                   
                FIXED_MUL(hi,lo,xr[x2],refac[exp]);
                FIXED_MSUB(hi,lo,xi[x2],imfac[exp]);
                v2r = FIXED_INT64_R(hi,lo);
                FIXED_MUL(hi,lo,xr[x2],imfac[exp]);
                FIXED_MADD(hi,lo,xi[x2],refac[exp]);
                v2i = FIXED_INT64_R(hi,lo);
#else
                v2r = MUL_R(xr[x2],refac[exp]) - MUL_R(xi[x2],imfac[exp]);
                v2i = MUL_R(xr[x2],imfac[exp]) + MUL_R(xi[x2],refac[exp]);
#endif                
#ifdef DUMP_FFT
                printf("step = %d, pos = %d, shift = %d, v2r = %.8f, v2i = %.8f, xr = %.8f, xi = %.8f, refac = %.8f, imfac = %.8f\n",
                        step,pos,shift,
                        COEF2FLOAT(v2r),COEF2FLOAT(v2i),
                        COEF2FLOAT(xr[x2]),COEF2FLOAT(xi[x2]),
                        REAL2FLOAT(refac[exp]),REAL2FLOAT(imfac[exp]));
#endif
                xr[x2] = xr[x1] - v2r;
                xr[x1] += v2r;
                xi[x2] = xi[x1] - v2i;
                xi[x1] += v2i;
                exp += estep;
                x1++;
                x2++;
            }
        }
    }
}


void fft( FFT_Tables *fft_tables, coef_t *xr, coef_t *xi, int32_t logm)
{
    reorder( fft_tables, xr, logm);
    reorder( fft_tables, xi, logm);
    fft_proc( xr, xi, fft_tables->costbl, fft_tables->negsintbl, 1 << logm );
}

/*
$Log: fft.c,v $
Revision 1.7  2006/06/07 01:50:10  kyang
Fix bug2562: Increase AAC quality

Revision 1.6  2006/02/23 18:44:10  kyang
release FAAC code

Revision 1.5  2006/02/22 00:49:15  kyang
Improve AAC quality by increase fixed-point function precision

Revision 1.4  2006/02/20 22:53:29  kyang
Decrease FAAC startup time

Revision 1.3  2006/02/20 02:40:43  kyang
Improve aac quality

Revision 1.2  2006/02/15 01:20:13  kyang
Realtime FAAC for Cypher

Revision 1.11  2004/04/02 14:56:17  danchr
fix name clash w/ libavcodec: fft_init -> fft_initialize
bump version number to 1.24 beta

Revision 1.10  2003/11/16 05:02:51  stux
moved global tables from fft.c into hEncoder FFT_Tables. Add fft_init and fft_terminate, flowed through all necessary changes. This should remove at least one instance of a memory leak, and fix some thread-safety problems. Version update to 1.23.3

Revision 1.9  2003/09/07 16:48:01  knik
reduced arrays size

Revision 1.8  2002/11/23 17:32:54  knik
rfft: made xi a local variable

Revision 1.7  2002/08/21 16:52:25  knik
new simplier and faster fft routine and correct real fft
new real fft is just a complex fft wrapper so it is slower than optimal but
by surprise it seems to be at least as fast as the old buggy function

*/
