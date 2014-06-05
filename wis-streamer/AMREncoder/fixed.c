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
*  $Id: fixed.c,v 1.1 2006/06/14 02:15:33 ross Exp $
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


#ifdef FIXED_POINT

/* lookup tables */
extern real_32_t fixed_table_log[FIXED_SAMPLES+1];
extern real_32_t fixed_table_sqrt[FIXED_SAMPLES+1];
extern real_32_t fixed_table_pow[FIXED_SAMPLES+1];
extern real_t fixed_table_pow34[FIXED_SAMPLES_POW34+1];
extern real_32_t fixed_table_log2[64+1];
extern fftfloat fixed_fft_costbl[1<<(FIXED_FFT_LOGM-1)];
extern fftfloat fixed_fft_negsintbl[1<<(FIXED_FFT_LOGM-1)];
extern real_32_t fixed_fft_cosx[FIXED_FFT_SINCOS_SIZE];
extern real_32_t fixed_fft_sinx[FIXED_FFT_SINCOS_SIZE];
extern frac_t fixed_table_cos[FIXED_SAMPLES+1];
extern real_t fixed_table_acos[FIXED_SAMPLES+1];
extern frac_t fixed_table_cos_pi_div_4k[FIXED_SAMPLES+1];

static real_t maxx=0,maxy=0;

#ifdef FIXED_DEBUG
void fixed_debug(const char *fmt, ...)
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

INLINE real_t fixed_sqrt(real_t x)
{
    real_t y;
    int32_t n=0;
    static real_t realconst1 = REAL_CONST(0.1);
    static real_t realconst2 = FIXED_POW34_MAXSAMPLE;
#ifdef FIXED_LAGRANGE
    int32_t i;
#endif

#ifdef FIXED_DEBUG_SQRT
    printf("fixed_sqrt: x = %.8f, sqrt(x) = %.8f",REAL2FLOAT(x),sqrt(REAL2FLOAT(x)));
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

#ifdef FIXED_LAGRANGE
        i = (int32_t)REAL2INT((x<<SAMPLE_BITS));
        y = MUL_R(fixed_table_sqrt[i],REAL_ICONST(i+1)-(x<<SAMPLE_BITS)) +
            MUL_R(fixed_table_sqrt[i+1],(x<<SAMPLE_BITS)-REAL_ICONST(i)); 
#else
        y = fixed_table_sqrt[x>>REAL2SAMPLE_BIT];
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

#ifdef FIXED_DEBUG_SQRT
    printf(", fixed_sqrt(x) = %.8f\n",REAL2FLOAT(y));
#endif
    return y;
}

INLINE real_t fixed_log(real_t x)
{
    real_t y;
    int32_t n=0;
    static real_t realconst1 = REAL_CONST(0.1);
    static real_t realconst2 = REAL_ICONST(1) ;
#ifdef FIXED_LAGRANGE
    int32_t i;
#endif
    
#ifdef FIXED_DEBUG_LOG
    printf("fixed_log: x = %.8f, log(x) = %.8f",REAL2FLOAT(x),log(REAL2FLOAT(x)));
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
#ifdef FIXED_LAGRANGE
         i = (int32_t)REAL2INT((x<<SAMPLE_BITS));
         y = MUL_R(fixed_table_log[i],REAL_ICONST(i+1)-(x<<SAMPLE_BITS)) +
              MUL_R(fixed_table_log[i+1],(x<<SAMPLE_BITS)-REAL_ICONST(i)) + 
              ((n>=0)?fixed_table_log2[n]:-fixed_table_log2[-n]);
#else
         y = fixed_table_log[x>>REAL2SAMPLE_BIT] +
              ((n>=0)?fixed_table_log2[n]:-fixed_table_log2[-n]);
#endif
    }
    else
        y = 0;
    
#ifdef FIXED_DEBUG_LOG
    printf(", fixed_log(x) = %.8f\n",REAL2FLOAT(y));
#endif

    return y;
}

INLINE real_t fixed_log10(real_t x)
{
    static real_t log10 = REAL_CONST(2.3025850930);

    return DIV_R(fixed_log(x),log10);
}

INLINE real_t fixed_pow(real_t x, real_t y)
{
    real_t z;
    real_t fracx;
    int32_t zi;
    real_t zf;
    int32_t neg=0;
    real_t y1;
    static real_32_t log2 = REAL_CONST(0.6931471806);
    static real_t realconst1 = REAL_ICONST(1) ;
#ifdef FIXED_LAGRANGE
    int32_t i;
#endif

    if ( x!= 0 ) {
        y1 = DIV_R(fixed_log(x),log2); // y1 = log2(x)
        fracx = MUL_R(y,y1);  // fracx = y*log2(x)
      
#ifdef FIXED_DEBUG_POW
        printf("fixed_pow: x = %.8f, y = %.8f, pow(x,y) = %.8f",
                REAL2FLOAT(x),REAL2FLOAT(y),pow(REAL2FLOAT(x),REAL2FLOAT(y)));
#endif

        if ( fracx < 0 ) {
            fracx = -fracx;
            neg = 1;
        }
        zi = (int32_t)REAL2INT(fracx);
        zf = fracx - REAL_ICONST(zi);
#ifdef FIXED_LAGRANGE
        i = (int32_t)REAL2INT(zf<<SAMPLE_BITS);
        z = MUL_R(fixed_table_pow[i],REAL_ICONST(i+1)-(zf<<SAMPLE_BITS)) +
              MUL_R(fixed_table_pow[i+1],(zf<<SAMPLE_BITS)-REAL_ICONST(i));
#else
        z = fixed_table_pow[zf>>REAL2SAMPLE_BIT];
#endif
        z = (z<<zi);

        if ( neg ) {
            if ( z )
                z = DIV_R(realconst1,z);
            else
                z = MAX_REAL;
        }
    }
    else
        z = 0;

#ifdef FIXED_DEBUG_POW
    printf(", fixed_pow(x,y) = %.8f\n",REAL2FLOAT(z));
#endif
    return z;
}

INLINE real_t fixed_sin(real_t x)
{
    double y = sin(REAL2FLOAT(x));
#ifdef FIXED_DEBUG_SIN
    printf("fixed_sin: x = %.8f, y = %.8f\n",REAL2FLOAT(x),y);
#endif
    return REAL_CONST(y);
}

INLINE frac_t fixed_cos(real_t x)
{
    register frac_t y;
    int32_t i;
#ifdef FIXED_DEBUG_COS
    printf("fixed_cos: x = %.8f, y = %.8f\n",REAL2FLOAT(x),y);
#endif

    /* Make x in rage [0,8] */
    while (x>REAL_ICONST(8)) {
        x -= (FIXED_M_PI<<1); /* x -= 2*PI, cos(x)=cos(x-2*PI) */
    }
    while (x<REAL_ICONST(0)) {
        x += (FIXED_M_PI<<1); /* x += 2*PI, cos(x)=cos(x+2*PI) */
    }

#ifdef FIXED_LAGRANGE
    i = (int32_t)REAL2INT(x<<(SAMPLE_BITS-3));
    y = MUL_R(fixed_table_cos[i],REAL_ICONST(i+1)-(x<<(SAMPLE_BITS-3))) +
        MUL_R(fixed_table_cos[i+1],(x<<(SAMPLE_BITS-3))-REAL_ICONST(i));
#else
    y = fixed_table_cos[x>>(REAL2SAMPLE_BIT+3)];
#endif

    return y;
}

INLINE real_t fixed_pow34(real_t x)
{
    register real_t y;
    register int32_t n=0;
    static real_t realconst1 = REAL_ICONST(FIXED_POW34_MAXSAMPLE)>>4;
    static real_t realconst2 = REAL_ICONST(FIXED_POW34_MAXSAMPLE);

#ifdef FIXED_DEBUG_POW34
    printf("fixed_pow34: x = %.8f, pow34(x) = %.8f",
                REAL2FLOAT(x),pow(fabs(REAL2FLOAT(x)),0.75));
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
            y = (fixed_table_pow34[FIXED_POW34_INDEX(x)])<<n;
        else {
            n = -n;
            y = (fixed_table_pow34[FIXED_POW34_INDEX(x)])>>n;
        }
    }
    else
        y = 0;
    
#ifdef FIXED_DEBUG_POW34
    printf(", fixed_pow34(x) = %.8f\n",REAL2FLOAT(y));
#endif

    return y;
}

/* -1<=x<=1, 0<=y<=PI */
INLINE real_t fixed_acos(real_t x)
{
    register real_t y;
    int32_t i;

    if ( (x<REAL_ICONST(-1)) || (x>REAL_ICONST(1)) ) {
        printf("fixed_acos overflow: x = %.8f\n",REAL2FLOAT(x));
        return (REAL_ICONST(-1));
    }

    x += REAL_ICONST(1);

    i = x>>(REAL2SAMPLE_BIT+1);
#ifdef FIXED_LAGRANGE
    y = MUL_R(fixed_table_acos[i],REAL_ICONST(i+1)-(x<<(SAMPLE_BITS-1))) +
        MUL_R(fixed_table_acos[i+1],(x<<(SAMPLE_BITS-1))-REAL_ICONST(i));
#else
    y = fixed_table_acos[i];
#endif
    return y;
}

INLINE real_t fixed_floor(real_t x)
{
    return REAL_ICONST(REAL2INT(x));
}

INLINE real_t fixed_frexp(int32_t x, int32_t *exp)
{
    int32_t tmp,frac;

    *exp = 0;

    if ( x == 0 )
        return REAL_ICONST(0);
    
    tmp = x;
    while ( tmp>0 ) {
        tmp = tmp>>1;
        (*exp)++;
    }

    if ( *exp >= REAL_BITS )
        frac = x>>((*exp)-REAL_BITS);
    else
        frac = x<<(REAL_BITS-(*exp));

    return frac;
}

void fixed_fixed_init()
{
#ifdef FIXED_RUNTIME_TABLE
    register int32_t i;
    real_t ln2 = REAL_CONST(0.69314718);
    double step;
    double cfreq, sfreq;
    double c, s, cold;
    int N = 2048;
    int size = (1<<FIXED_FFT_LOGM);

    fixed_table_log[0] = 0;
    for (i=1;i<=FIXED_SAMPLES;i++) {
        fixed_table_log[i]=REAL_CONST(log((double)i/FIXED_SAMPLES));
    }

    for (i=0;i<=FIXED_SAMPLES;i++) {
        fixed_table_sqrt[i]=REAL_CONST(sqrt((double)i/FIXED_SAMPLES));
    }

    for (i=0;i<=FIXED_SAMPLES;i++) {
        fixed_table_pow[i]=REAL_CONST(pow(2,(double)i/FIXED_SAMPLES));
    }

    step = ((double)FIXED_POW34_MAXSAMPLE)/FIXED_SAMPLES_POW34;
    for (i=0;i<=FIXED_SAMPLES_POW34;i+=step) {
        fixed_table_pow34[i]=REAL_CONST(pow((double)i,0.75));
    }

    for (i=1;i <= 64;i++) {
        fixed_table_log2[i] = i*ln2;
    }

    for (i=0;i<(1<<(FIXED_FFT_LOGM-1));i++) {
        fixed_fft_costbl[i] = REAL_CONST(cos(2.0 * FIXED_M_PI_F * ((double) i) / (double) size));
        fixed_fft_negsintbl[i] = REAL_CONST(-sin(2.0 * FIXED_M_PI_F * ((double) i) / (double) size));
    }

    /* prepare for recurrence relation in pre-twiddle */
    cfreq = cos(FIXED_TWOPI_F/N);
    sfreq = sin(FIXED_TWOPI_F/N);
    c = cos(FIXED_TWOPI_F/(N<<3));
    s = sin(FIXED_TWOPI_F/(N<<3));

    for ( i=0; i<(N>>2); i++) {
        fixed_fft_cosx[i] = REAL_CONST(c);
        fixed_fft_sinx[i] = REAL_CONST(s);

        /* use recurrence to prepare cosine and sine for next value of i */
        cold = c;
        c = c*cfreq - s*sfreq;
        s = s*cfreq + cold*sfreq;
    }

    /* Make runtime table for y=cos(x), range: 0<=x<=8 */
    for (i=0;i<=FIXED_SAMPLES;i++) {
        fixed_table_cos[i]=FRAC_CONST(cos((double)8*i/FIXED_SAMPLES));
    }

    /* Make runtime table for y=acos(x) */
    for (i=0;i<=FIXED_SAMPLES;i++) {
        fixed_table_acos[i]=REAL_CONST(acos((double)(i<<1)/FIXED_SAMPLES-1));
    }

    /* Make runtime table for y=cos_pi_div_4000(x) */
    for (i=0;i<=FIXED_SAMPLES;i++) {
        fixed_table_cos_pi_div_4k[i]=FRAC_CONST(cos((double)2*FIXED_M_PI_F*i/FIXED_SAMPLES));
    }
#endif
}

void fixed_fixed_exit()
{
    printf("maxx = %.8f, maxy = %.8f\n",REAL2FLOAT(maxx),REAL2FLOAT(maxy));
}

void print64_t(int64_t x)
{
    uint32_t lo,hi;
        
    lo = (uint32_t)(x&0xffffffff);
    hi = (uint32_t)(x>>32);
    printf("%08x%08x",hi,lo);
}

/* f(x) = cos(x*PI/4000) */
INLINE frac_t fixed_cos_pi_div_4k(real_t x)
{
    register frac_t y;
    int32_t i;
#ifdef FIXED_DEBUG_COS_PI_DIV_4K
    printf("fixed_table_cos_pi_div_4k: x = %.8f, y = %.8f\n",REAL2FLOAT(x),FRAC2FLOAT(y));
#endif

    /* Make x in rage [0,8000] */
    while (x>REAL_ICONST(8000)) {
        x -= 8000; /* x -= 8000, f(x-8000)=f(x) */
    }
    while (x<REAL_ICONST(0)) {
        x += 8000; /* x += 8000, f(x+8000)=f(x) */
    }

    x = (x<<(SAMPLE_BITS-6))/125;
#ifdef FIXED_LAGRANGE
    /* x = x/delta = x/(8000/samples) = x*samples/8000 */

    /* i = [x] */
    i = (int32_t)REAL2INT(x);

    /* y = f(x[i+1])*(x[i+1]-x)/(x[i+1]-x[i]) +
           f(x[i]*(x-x[i])/(x[i+1]-x[i])
         = f(x[i+1])*(x[i+1]-x)/ delta +
           f(x[i]*(x-x[i])/delta
         = f(x[i+1])*((i+1)*delta-x)/delta +
           f(x[i]*(x-i*delta)/delta
         = f(x[i+1])*((i+1)-x/delta) +
           f(x[i]*(x/delta-i)
       delta = 8000/samples = 8000/2^SAMPLE_BITS
    */
    y = MUL_R(fixed_table_cos_pi_div_4k[i],REAL_ICONST(i+1)-x) +
        MUL_R(fixed_table_cos_pi_div_4k[i+1],x-REAL_ICONST(i));
#else
    y = fixed_table_log[REAL2INT(x)];
#endif

    return y;
}

#endif

