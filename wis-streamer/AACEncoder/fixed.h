/*****************************************************************************
*
*  Copyright WIS Technologies (c) (2005)
*  All Rights Reserved
*
*****************************************************************************
*
*  FILE:
*    fixed.h
*
*  DESCRIPTION:
*  This file provides fixed point math to faac..
*
* AUTHOR:
*    Ken Yang
*
*  $Id: fixed.h,v 1.10 2006/12/05 07:06:47 kyang Exp $
*
****************************************************************************/

#ifdef WIN32
#include <stdlib.h>
#else
#include <stdint.h>
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* debug flags for data dump */
//#define DUMP_SAMPLEBUFFER   1
//#define DUMP_P_O_MDCT    1
//#define DUMP_FFT    1
//#define DUMP_FILTERBANK     1
//#define DUMP_XR_POW 1
//#define DUMP_XMIN   1
//#define DUMP_MAXFIXSTEP 1
//#define DUMP_XI
//#define DUMP_BITSTREAM  1

/* define FAAC_DEBUG to output debug information via faac_debug, which will trace CPU time */
//#define FAAC_DEBUG  1

/* define FAAC_USE_ASM to make use of ASM to speedup, still some problem with ASM now */
#ifdef MIPS_SIM
#define FAAC_USE_ASM    1
#endif

/* define FAAC_RUNTIME_TABLE to generate lookup table at runtime when initializing */
//#define FAAC_RUNTIME_TABLE  1

/* define FAAC_STATIC_MEMORY to use static memory instead dynamic memory */
//#define FAAC_STATIC_MEMORY 1

/* define FAAC_LAGRANGE to generate fixed-point operation result via Lagrange interpolation,
    which will improve the precision, but consume more MIPS because of extra multiplication */
#define FAAC_LAGRANGE   1

/* FAAC profile */
#ifndef WIN32
//#define FAAC_PROFILE    1
#endif

#ifdef FAAC_PROFILE
#define FAAC_CPU_CYCLE  166
#define FAAC_PROFILE_COUNT   240  // Make profile every 5 seconds ( 240=48*5 )
#endif

#ifdef WIN32
typedef unsigned __int64 uint64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int8 uint8_t;
typedef __int64 int64_t;
typedef __int32 int32_t;
typedef __int16 int16_t;
typedef __int8  int8_t;
typedef float float32_t;
#else
typedef float float32_t;
#endif

typedef long long_t;
typedef unsigned long ulong_t;
typedef short short_t;
typedef unsigned short ushort_t;
typedef char char_t;
typedef unsigned char uchar_t;

#define INLINE __inline
#define ALIGN

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

typedef int64_t real_t;
typedef int64_t frac_t;
typedef int32_t coef_t;
typedef int32_t real_32_t;
typedef int64_t eng_t;
typedef int64_t pow_t;
typedef int32_t fftfloat;

/* real */
#define REAL_BITS 16 // MAXIMUM BITS FOR FIXED POINT SBR
#define REAL_PRECISION (1 << REAL_BITS)
#define DIV_R(A, B) ((real_t)(((real_t)(A) << REAL_BITS)/(B)))
#define MUL_R(A,B) (real_t)(((int64_t)(A)*(B)) >> REAL_BITS)

#define MAX_REAL ((real_t)((1<<(REAL_BITS-1))-1))
#define MAX_REAL32 ((real_32_t)0x7FFFFFFF)

#define REAL_CONST(A) (((A) >= 0) ? ((real_t)((A)*(REAL_PRECISION)+0.5)) : ((real_t)((A)*(REAL_PRECISION)-0.5)))
#define REAL_ICONST(A) (((real_t)(A))<<REAL_BITS)
#define REAL2FLOAT(A) (((double)(A))/(REAL_PRECISION))
#define REAL2INT(A) ((A)>>(REAL_BITS))

/* coef */
#define COEF_BITS 7
#define COEF_PRECISION (1 << COEF_BITS)
#define COEF_CONST(A) (((A) >= 0) ? ((real_t)((A)*(COEF_PRECISION)+0.5)) : ((real_t)((A)*(COEF_PRECISION)-0.5)))
#define COEF_ICONST(A)  ((coef_t)(A)<<COEF_BITS)
#define COEF2FLOAT(A) (((double)(A))/(COEF_PRECISION))
#define DIV_C(A, B) ((coef_t)(((coef_t)(A) << COEF_BITS)/(B)))
#define MUL_C(A,B) (real_t)(((int64_t)(A)*(B)) >> COEF_BITS)
#define REAL2COEF_BIT   (REAL_BITS-COEF_BITS) // should > 0
#define COEF2INT(A) ((A)>>(COEF_BITS))

/* fraction */
/* FRAC is the fractional only part of the fixed point number [0.0..1.0) */
#define FRAC_SIZE 32 /* frac is a 32 bit integer */
#define FRAC_BITS 30
#define FRAC_PRECISION ((uint32_t)(1 << FRAC_BITS))
#define FRAC_MAX 0x7FFFFFFF
#define MUL_F(A,B) (real_t)(((int64_t)(A)*(B)) >> FRAC_BITS)

#define FRAC_CONST(A) (((A) == 1.00) ? ((frac_t)FRAC_MAX) : (((A) >= 0) ? ((frac_t)((A)*(FRAC_PRECISION)+0.5)) : ((frac_t)((A)*(FRAC_PRECISION)-0.5))))
#define FRAC2REAL_BIT   (FRAC_BITS-REAL_BITS) // should > 0
#define FRAC2COEF_BIT   (FRAC_BITS-COEF_BITS) // should > 0
#define FRAC_ICONST(A)  ((frac_t)(A)<<FRAC_BITS)
#define FRAC2FLOAT(A) (((double)(A))/(FRAC_PRECISION))

#define SAMPLE_BITS    14
#define FAAC_SAMPLES    (1<<SAMPLE_BITS)
#define REAL2SAMPLE_BIT (REAL_BITS-SAMPLE_BITS) // should > 0

/* pow34(x) */
#define POW34_SAMPLE_BIT      14
#define FAAC_SAMPLES_POW34    (1<<POW34_SAMPLE_BIT)
#define POW34_SCALE_BIT       14
#define FAAC_POW34_MAXSAMPLE  (1<<POW34_SCALE_BIT)
#define FAAC_POW34_COEFCONST1 ((1<<COEF_BITS)>>1) //REAL_CONST(0.5)
#define FAAC_POW34_COEFCONST2 (COEF_BITS+POW34_SCALE_BIT-SAMPLE_BITS)
#define FAAC_POW34_INDEX(x)   ((x+FAAC_POW34_COEFCONST1)>>FAAC_POW34_COEFCONST2)

/* fixed-point operations */
extern INLINE real_t faac_sqrt(real_t x);
extern INLINE real_32_t faac_log(real_t x);
extern INLINE real_32_t faac_pow(real_t x, real_32_t y);
extern INLINE coef_t faac_pow34(coef_t x);

/* lookup tables */
extern real_32_t faac_table_log[FAAC_SAMPLES+1];
extern real_32_t faac_table_sqrt[FAAC_SAMPLES+1];
extern real_32_t faac_table_pow[FAAC_SAMPLES+1];
extern coef_t faac_table_pow34[FAAC_SAMPLES_POW34+1];
extern real_32_t faac_table_log2[64+1];

#define faac_fabs(x) abs(x)

/* fft */
#define FAAC_FFT_LOGM   9
#define FAAC_FFT_SINCOS_SIZE     512
extern fftfloat faac_fft_costbl[1<<(FAAC_FFT_LOGM-1)];
extern fftfloat faac_fft_negsintbl[1<<(FAAC_FFT_LOGM-1)];
extern real_32_t faac_fft_cosx[FAAC_FFT_SINCOS_SIZE];
extern real_32_t faac_fft_sinx[FAAC_FFT_SINCOS_SIZE];

/* definination for PI */
#define FAAC_M_PI      REAL_CONST(3.14159265358979323846)
#define FAAC_M_TWOPI   (FAAC_M_PI<<1)
#define FAAC_M_PI_F    (3.14159265358979323846)
#define FAAC_TWOPI_F   (FAAC_M_PI_F*2)

extern void faac_fixed_init();

#ifdef FAAC_DEBUG
extern void faac_debug(const char *fmt, ...);
#endif

#ifdef FAAC_USE_ASM
#define FIXED_MADD(hi,lo, x, y)  \
asm ("madd	%2,%3"  \
	 :"+l"(lo),"+h"(hi) \
	 :"%r"(x),"r"(y))

#define FIXED_MSUB(hi,lo, x, y)  \
asm ("msub	%2,%3"  \
	 :"+l"(lo),"+h"(hi) \
	 :"%r"(x),"r"(y))

#define FIXED_MUL(hi,lo, x, y )  \
asm ("mult	%2,%3"  \
	 :"=l"(lo),"=h"(hi) \
	 :"%r"(x),"r"(y))

#define FIXED_INT64(hi,lo) ((int64_t)(((int64_t)(hi)<<32)|(lo)))
#define FIXED_INT64_R(hi,lo) (((hi&0xffff)<<16)|((lo>>16)&0xffff))
#define FIXED_INT64_C(hi,lo) (((hi&0xff)<<24)|((lo>>8)&0xffffff))
#endif

#ifdef  __cplusplus
}
#endif
