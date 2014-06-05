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
*  $Id: fixed.h,v 1.1 2006/06/14 02:15:34 ross Exp $
*
****************************************************************************/

#ifndef __FIXED_H__
#define __FIXED_H__

#ifdef WIN32
#include <stdlib.h>
#else
#include <stdint.h>
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* debug flags for data dump */
//#define DUMP_FILTERBANK     1
//#define DUMP_SAMPLEBUFFER   1
//#define DUMP_XI
//#define DUMP_XMIN   1
//#define DUMP_P_O_MDCT    1
//#define DUMP_MAXFIXSTEP 1
//#define DUMP_BITSTREAM  1
//#define DUMP_XR_POW 1
//#define DUMP_FFT    1

/* define FIXED_DEBUG to output debug information via fixed_debug, which will trace CPU time */
//#define FIXED_DEBUG  1

/* define FIXED_POINT to use fixed point operation instead float-point operation */
#define FIXED_POINT  1

/* define FIXED_USE_ASM to make use of ASM to speedup, still some problem with ASM now */
#ifndef WIN32
//#define FIXED_USE_ASM    1
#endif

/* define FIXED_RUNTIME_TABLE to generate lookup table at runtime when initializing */
//#define FIXED_RUNTIME_TABLE  1

/* define FIXED_STATIC_MEMORY to use static memory instead dynamic memory */
//#define FIXED_STATIC_MEMORY 1

/* define FIXED_LAGRANGE to generate fixed-point operation result via Lagrange interpolation,
    which will improve the precision, but consume more MIPS because of extra multiplication */
#define FIXED_LAGRANGE   1

/* debug flags for fixed-point operations */
//#define FIXED_DEBUG_LOG  1
//#define FIXED_DEBUG_POW  1
//#define FIXED_DEBUG_SQRT 1
//#define FIXED_DEBUG_SIN  1
//#define FIXED_DEBUG_POW34  1


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
/*typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short int uint16_t;
typedef unsigned char uint8_t;
typedef long long int64_t;
typedef int int32_t;
typedef short int16_t;
typedef char  int8_t; */ 
typedef float float32_t; 
#endif

typedef long long_t;
typedef unsigned long ulong_t;
typedef short short_t;
typedef unsigned short ushort_t;
typedef char char_t;
typedef unsigned char uchar_t;

#ifndef FIXED_POINT
#include <math.h>
#endif

#define INLINE __inline
#define ALIGN

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifdef FIXED_POINT
typedef int64_t real_t;
typedef int64_t frac_t;
typedef int64_t coef_t;
typedef int32_t real_32_t;
typedef int32_t frac_32_t;
typedef int64_t eng_t;
typedef int64_t pow_t;
typedef int32_t fftfloat;

/* real */
#define REAL_BITS 16 // MAXIMUM BITS FOR FIXED POINT SBR
#define REAL_PRECISION (1 << REAL_BITS)
#define DIV_R(A, B) ((real_t)(((real_t)(A) << REAL_BITS)/(B)))
//#define MUL_R(A,B) (real_t)(((int64_t)(A)*(int64_t)(B)+(1 << (REAL_BITS-1))) >> REAL_BITS)
#define MUL_R(A,B) (real_t)(((int64_t)(A)*(B)) >> REAL_BITS)
//extern real_t MUL_R(real_t x, real_t y);

#define MAX_REAL ((real_t)(((int64_t)1<<(63-REAL_BITS))-1))
#define MAX_REAL32 ((real_32_t)0x7FFFFFFF)
#define MIN_REAL ((real_t)(((int64_t)1<<(63-REAL_BITS)))
#define MAX_INT64 (((int64_t)1<<62)-1)

#define REAL_CONST(A) (((A) >= 0) ? ((real_t)((A)*(REAL_PRECISION)+0.5)) : ((real_t)((A)*(REAL_PRECISION)-0.5)))
//#define REAL_ICONST(A) (((A) >= 0) ? (((real_t)(A))<<REAL_BITS):(-((-(real_t)(A))<<REAL_BITS)))
#define REAL_ICONST(A) (((real_t)(A))<<REAL_BITS)
//#define REAL_NCONST(A) (REAL_ICONST(A) | (REAL_ICONST(1)-1))
#define REAL2FLOAT(A) ((A)/((float)(REAL_PRECISION)))
#define REAL2INT(A) ((A)>>(REAL_BITS))
#define REAL2INT32(A) ((int32_t)((A)>>(REAL_BITS)))
#define REAL32(A) ((real_32_t)((A)&0xFFFFFFFF))
//#define REAL32(A) (A)

/* coef */
#define COEF_BITS 8
#define COEF_PRECISION (1 << COEF_BITS)
#define COEF_CONST(A) (((A) >= 0) ? ((real_t)((A)*(COEF_PRECISION)+0.5)) : ((real_t)((A)*(COEF_PRECISION)-0.5)))
//#define COEF_CONST(A) (((coef_t)(A))<<COEF_BITS)
#define COEF_ICONST(A)  ((frac_t)(A)<<FRAC_BITS)
#define COEF2FLOAT(A) (((float)(A))/(COEF_PRECISION))
#define DIV_C(A, B) ((coef_t)(((coef_t)(A) << COEF_BITS)/(B)))
//#define MUL_C(A,B) (real_t)(((int64_t)(A)*(int64_t)(B)+(1 << (COEF_BITS-1))) >> COEF_BITS)
#define MUL_C(A,B) (coef_t)(((int64_t)(A)*(B)) >> COEF_BITS)
#define REAL2COEF_BIT   (REAL_BITS-COEF_BITS) // should > 0

/* fraction */
/* FRAC is the fractional only part of the fixed point number [0.0..1.0) */
#define FRAC_SIZE 32 /* frac is a 32 bit integer */
#define FRAC_BITS 30
#define FRAC_PRECISION ((uint32_t)(1 << FRAC_BITS))
#define FRAC_MAX 0x7FFFFFFF
#define DIV_F(A, B) ((frac_t)(((frac_t)(A) << FRAC_BITS)/(B)))
//#define MUL_F(A,B) (real_t)(((int64_t)(A)*(int64_t)(B)+(1 << (FRAC_BITS-1))) >> FRAC_BITS)
//#define MUL_F(A,B) (frac_t)(((int64_t)((A)>>5)*((B)>>5)) >> (FRAC_BITS-10))
#define MUL_F(A,B) (frac_t)(((int64_t)(A)*(B)) >> FRAC_BITS)
#define MUL_F_EXT(A,B,sa,sb) (frac_t)(((int64_t)((A)>>(sa))*((B)>>(sb))) >> (FRAC_BITS-(sa)-(sb)))
//#define MUL_F(A,B) MUL_F_EXT(A,B,5,5)

//#define FRAC_CONST(A) (((A) == 1.00) ? ((frac_t)FRAC_MAX) : (((A) >= 0) ? ((frac_t)((A)*(FRAC_PRECISION)+0.5)) : ((frac_t)((A)*(FRAC_PRECISION)-0.5))))
#define FRAC_CONST(A) ((((A) >= 0) ? ((frac_t)((A)*(FRAC_PRECISION)+0.5)) : ((frac_t)((A)*(FRAC_PRECISION)-0.5))))
//#define FRAC_CONST(A) (((A) >= 0) ? ((frac_t)((A)*(FRAC_PRECISION)+0.5)) : ((frac_t)((A)*(FRAC_PRECISION)-0.5)))
#define FRAC2REAL_BIT   (FRAC_BITS-REAL_BITS) // should > 0
#define FRAC_ICONST(A)  ((frac_t)(A)<<FRAC_BITS)
#define FRAC2FLOAT(A) (((float)(A))/(FRAC_PRECISION))

#define SAMPLE_BITS    14
#define FIXED_SAMPLES    (1<<SAMPLE_BITS)
#define REAL2SAMPLE_BIT (REAL_BITS-SAMPLE_BITS) // should > 0

/* pow34(x) */
#define POW34_SAMPLE_BIT      14
#define FIXED_SAMPLES_POW34    (1<<POW34_SAMPLE_BIT)
#define POW34_SCALE_BIT       14
#define FIXED_POW34_MAXSAMPLE  (1<<POW34_SCALE_BIT)
#define FIXED_POW34_REALCONST1 ((1<<REAL_BITS)>>1) //REAL_CONST(0.5)
#define FIXED_POW34_REALCONST2 (REAL_BITS+POW34_SCALE_BIT-SAMPLE_BITS)
#define FIXED_POW34_INDEX(x)   ((x+FIXED_POW34_REALCONST1)>>FIXED_POW34_REALCONST2)

/* fixed-point operations */
extern INLINE real_t fixed_sqrt(real_t value);
extern INLINE real_t fixed_log(real_t x);
extern INLINE real_t fixed_log10(real_t x);
extern INLINE real_t fixed_pow(real_t x, real_t y);
extern INLINE real_t fixed_pow34(real_t x);
extern INLINE frac_t fixed_cos(real_t x);
extern INLINE frac_t fixed_sin(real_t x);
extern INLINE real_t fixed_acos(real_t x);
extern INLINE real_t fixed_floor(real_t x);
extern INLINE real_t fixed_frexp(int32_t x, int32_t *exp);
extern INLINE frac_t fixed_cos_pi_div_4k(real_t x);

#undef FLT_MAX
#define FLT_MAX MAX_REAL

#else

typedef double real_t;
typedef double real_32_t;
typedef double frac_t;
typedef double frac_32_t;
typedef double coef_t;
typedef double float_t;
typedef double eng_t;
typedef double pow_t;
typedef double fftfloat;

#define REAL_CONST(A) ((real_t)(A))
#define REAL_ICONST(A) ((real_t)(A))
#define REAL2FLOAT(A)  (A)
#define REAL2INT(A) ((int32_t)(A))
#define DIV_R(A,B) ((A)/(B))
#define MUL_R(A,B) ((A)*(B))

#define COEF_CONST(A) (A)
#define MUL_C(A,B) ((A)*(B))
#define DIV_C(A, B) ((A)/(B))

#define FRAC_CONST(A) (A)
#define FRAC_ICONST(A)  (A)
#define DIV_F(A,B) ((A)/(B))
#define MUL_F(A,B) ((A)*(B))

#define fixed_sqrt(x) sqrt(x)
#define fixed_log(x) log(x)
#define fixed_log10(x) log10(x)
#define fixed_pow(x,y) pow((x),(y))
#define fixed_pow34(x) pow(x,0.75)
#define fixed_cos(x) cos(x)
#define fixed_sin(x) sin(x)
#define fixed_acos(x) acos(x)
#define fixed_floor(x) floor(x)
#define fixed_frexp(x,y) frexp(x,y)
#endif
#define fixed_fabs(x) abs(x)

/* fft */
#define FIXED_FFT_LOGM   9
#define FIXED_FFT_SINCOS_SIZE     512
extern fftfloat fixed_fft_costbl[1<<(FIXED_FFT_LOGM-1)];
extern fftfloat fixed_fft_negsintbl[1<<(FIXED_FFT_LOGM-1)];
extern real_32_t fixed_fft_cosx[FIXED_FFT_SINCOS_SIZE];
extern real_32_t fixed_fft_sinx[FIXED_FFT_SINCOS_SIZE];

/* definination for PI */
#define FIXED_M_PI      REAL_CONST(3.14159265358979323846)
#define FIXED_M_TWOPI   (FIXED_M_PI<<1)
#define FIXED_M_PI_F    (3.14159265358979323846)
#define FIXED_TWOPI_F   (FIXED_M_PI_F*2)

extern void fixed_fixed_init();
extern void fixed_fixed_exit();

#ifdef FIXED_DEBUG
extern void fixed_debug(const char *fmt, ...);
#endif

#ifdef FIXED_USE_ASM
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
#endif

#define FIXED_INT64(hi,lo) ((int64_t)(((int64_t)(hi)<<32)|(lo)))
#define FIXED_INT64_R(hi,lo) (((hi&0xffff)<<16)|((lo>>16)&0xffff))

#ifdef  __cplusplus
}
#endif
#endif
