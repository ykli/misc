/*****************************************************************************
*
*  Copyright WIS Technologies (c) (2005)
*  All Rights Reserved
*
*****************************************************************************
*
*  FILE:
*    fixed.c
*
*  DESCRIPTION:
*  This file provides fixed point math to faac.
*
* AUTHOR:
*    Ken Yang
*
*  $Id: fixed.c,v 1.10 2006/12/05 07:06:47 kyang Exp $
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#ifdef WIN32
#include <winsock.h>
#endif
#include "fixed.h"

/* debug flags for fixed-point operations */
//#define FAAC_DEBUG_LOG  1
//#define FAAC_DEBUG_POW  1
//#define FAAC_DEBUG_SQRT 1
//#define FAAC_DEBUG_SIN  1
//#define FAAC_DEBUG_POW34  1

#ifdef FAAC_DEBUG
void faac_debug(const char *fmt, ...)
{
    ulong_t  time_since_last;
    struct timeval curr;
    static struct timeval last;
    char buf[200];
    va_list ap;

#ifndef WIN32    
    gettimeofday(&curr,NULL);
#endif
    time_since_last = (curr.tv_sec  - last.tv_sec)*1000000 +
                                         (curr.tv_usec - last.tv_usec);
    memcpy(&last,&curr,sizeof(struct timeval));

    va_start(ap, fmt);
    sprintf(buf,fmt,ap);
    va_end(ap);
    
    printf("[%08d][%08d]: %s", curr.tv_sec, time_since_last, buf);
}
#endif

INLINE real_t faac_sqrt(real_t x)
{
    real_t y;
    int32_t n=0;
    static real_32_t realconst1 = REAL_CONST(0.1);
    static real_32_t realconst2 = REAL_ICONST(1);
#ifdef FAAC_LAGRANGE
    int32_t i;
#endif

#ifdef FAAC_DEBUG_SQRT
    printf("faac_sqrt: x = %.8f, sqrt(x) = %.8f",REAL2FLOAT(x),sqrt(REAL2FLOAT(x)));
#endif

    if ( x>0 ) {    
        while ( x>=realconst2) {
            x = x>>2;
            n++;
        }
        while ( x<realconst1) {
            x = x<<2;
            n--;
        }

#ifdef FAAC_LAGRANGE
        i = x>>(REAL_BITS-SAMPLE_BITS);
        y = MUL_R(faac_table_sqrt[i],REAL_ICONST(i+1)-(x<<SAMPLE_BITS)) +
            MUL_R(faac_table_sqrt[i+1],(x<<SAMPLE_BITS)-REAL_ICONST(i)); 
#else
        y = faac_table_sqrt[x>>REAL2SAMPLE_BIT];
#endif

        if ( n > 0 ) {
             y = y<<n;
        }
        else {
            n = -n;
            y = y>>n;
        }
    }    
    else
        y = 0;

#ifdef FAAC_DEBUG_SQRT
    printf(", faac_sqrt(x) = %.8f\n",REAL2FLOAT(y));
#endif
    return y;
}

INLINE real_32_t faac_log(real_t x)
{
    real_32_t y;
    int32_t n=0;
    static real_32_t realconst1 = REAL_CONST(0.1);
    static real_32_t realconst2 = REAL_ICONST(1) ;
#ifdef FAAC_LAGRANGE
    int32_t i;
#endif
    
#ifdef FAAC_DEBUG_LOG
    printf("faac_log: x = %.8f, log(x) = %.8f",REAL2FLOAT(x1),log(REAL2FLOAT(x1)));
#endif

    if ( x > 0 ) {
        while ( x>=realconst2 ) {
            x = x>>1;
            n++;
        }
        while ( x<realconst1) {
            x = x<<1;
            n--;
        }
#ifdef FAAC_LAGRANGE
        i = x>>(REAL_BITS-SAMPLE_BITS);
        y = MUL_R(faac_table_log[i],REAL_ICONST(i+1)-(x<<SAMPLE_BITS)) +
            MUL_R(faac_table_log[i+1],(x<<SAMPLE_BITS)-REAL_ICONST(i)) + 
            ((n>=0)?faac_table_log2[n]:-faac_table_log2[-n]);
#else
        y = faac_table_log[x>>REAL2SAMPLE_BIT] +
            ((n>=0)?faac_table_log2[n]:-faac_table_log2[-n]);
#endif
    }
    else
        y = 0;
    
#ifdef FAAC_DEBUG_LOG
    printf(", faac_log(x) = %.8f\n",REAL2FLOAT(y));
#endif

    return y;
}

INLINE real_32_t faac_pow(real_t x, real_32_t y)
{
    real_32_t z;
    real_32_t fracx;
    int32_t zi;
    real_32_t zf;
    int32_t neg=0;
    real_32_t y1;
    static real_32_t log2 = REAL_CONST(0.6931471806);
    static real_32_t realconst1 = REAL_ICONST(1) ;
#ifdef FAAC_LAGRANGE
    int32_t i;
#endif

    if ( x!= 0 ) {
        y1 = DIV_R(faac_log(x),log2); // y1 = log2(x)
        fracx = MUL_R(y,y1);  // fracx = y*log2(x)
      
#ifdef FAAC_DEBUG_POW
        printf("faac_pow: x = %.8f, y = %.8f, pow(x,y) = %.8f",
                REAL2FLOAT(x),REAL2FLOAT(y),pow(REAL2FLOAT(x),REAL2FLOAT(y)));
#endif

        if ( fracx < 0 ) {
            fracx = -fracx;
            neg = 1;
        }
        zi = (int32_t)REAL2INT(fracx);
        zf = fracx - REAL_ICONST(zi);
#ifdef FAAC_LAGRANGE
        i = zf>>(REAL_BITS-SAMPLE_BITS);
        z = MUL_R(faac_table_pow[i],REAL_ICONST(i+1)-(zf<<SAMPLE_BITS)) +
              MUL_R(faac_table_pow[i+1],(zf<<SAMPLE_BITS)-REAL_ICONST(i));
#else
        z = faac_table_pow[zf>>REAL2SAMPLE_BIT];
#endif
        z = (z<<zi);

        if ( neg ) {
            if ( z )
                z = DIV_R(realconst1,z);
            else
                z = MAX_REAL32;
        }
    }
    else
        z = 0;

#ifdef FAAC_DEBUG_POW
    printf(", faac_pow(x,y) = %.8f\n",REAL2FLOAT(z));
#endif
    return z;
}

INLINE coef_t faac_pow34(coef_t x)
{
    register coef_t y;
    register int32_t n=0;
    static coef_t realconst1 = COEF_ICONST(FAAC_POW34_MAXSAMPLE)>>4;
    static coef_t realconst2 = COEF_ICONST(FAAC_POW34_MAXSAMPLE);

#ifdef FAAC_DEBUG_POW34
    printf("faac_pow34: x = %.8f, pow34(x) = %.8f",
                COEF2FLOAT(x),pow(fabs(COEF2FLOAT(x)),0.75));
#endif

    if ( x != 0 ) {
        if ( x < 0 )
            x = -x;

        while (x<realconst1) {
            x = x<<4;
            n-=3;
        }
    
        while (x>realconst2) {
            x = x>>4;
            n+=3;
        }
    
        if ( n > 0 )
            y = (faac_table_pow34[FAAC_POW34_INDEX(x)])<<n;
        else {
            n = -n;
            y = (faac_table_pow34[FAAC_POW34_INDEX(x)])>>n;
        }
    }
    else
        y = 0;
    
#ifdef FAAC_DEBUG_POW34
    printf(", faac_pow34(x) = %.8f\n",COEF2FLOAT(y));
#endif

    return y;
}

void faac_fixed_init()
{
#ifdef FAAC_RUNTIME_TABLE
    register int32_t i;
    real_32_t ln2 = REAL_CONST(0.69314718);
    double step;
    double cfreq, sfreq;
    double c, s, cold;
    int N = 2048;
    int size = (1<<FAAC_FFT_LOGM);

    faac_table_log[0] = 0;
    for (i=1;i<=FAAC_SAMPLES;i++) {
        faac_table_log[i]=REAL_CONST(log((double)i/FAAC_SAMPLES));
    }

    for (i=0;i<=FAAC_SAMPLES;i++) {
        faac_table_sqrt[i]=REAL_CONST(sqrt((double)i/FAAC_SAMPLES));
    }

    for (i=0;i<=FAAC_SAMPLES;i++) {
        faac_table_pow[i]=REAL_CONST(pow(2,(double)i/FAAC_SAMPLES));
    }

    step = ((double)FAAC_POW34_MAXSAMPLE)/FAAC_SAMPLES_POW34;
    for (i=0;i<=FAAC_SAMPLES_POW34;i+=step) {
        faac_table_pow34[i]=COEF_CONST(pow((double)i,0.75));
    }

    for (i=1;i <= 64;i++) {
        faac_table_log2[i] = i*ln2;
    }

    for (i=0;i<(1<<(FAAC_FFT_LOGM-1));i++) {
        faac_fft_costbl[i] = REAL_CONST(cos(2.0 * FAAC_M_PI_F * ((double) i) / (double) size));
        faac_fft_negsintbl[i] = REAL_CONST(-sin(2.0 * FAAC_M_PI_F * ((double) i) / (double) size));
    }

    /* prepare for recurrence relation in pre-twiddle */
    cfreq = cos(FAAC_TWOPI_F/N);
    sfreq = sin(FAAC_TWOPI_F/N);
    c = cos(FAAC_TWOPI_F/(N<<3));
    s = sin(FAAC_TWOPI_F/(N<<3));

    for ( i=0; i<(N>>2); i++) {
        faac_fft_cosx[i] = REAL_CONST(c);
        faac_fft_sinx[i] = REAL_CONST(s);

        /* use recurrence to prepare cosine and sine for next value of i */
        cold = c;
        c = c*cfreq - s*sfreq;
        s = s*cfreq + cold*sfreq;
    }

#endif
}
