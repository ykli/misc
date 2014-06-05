/*
 * ===================================================================
 *  TS 26.104
 *  REL-5 V5.4.0 2004-03
 *  REL-6 V6.1.0 2004-03
 *  3GPP AMR Floating-point Speech Codec
 * ===================================================================
 *
 */

/*
 * interf_enc.c
 *
 *
 * Project:
 *    AMR Floating-Point Codec
 *
 * Contains:
 *    This module contains all the functions needed encoding 160
 *    16-bit speech samples to AMR encoder parameters.
 *
 */

/*
 * include files
 */
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "sp_enc.h"
#include "interf_rom.h"

/* Subjective importance of the speech encoded bits */
static Word16 order_MR475[] =
{
   0, 0x80,
   0, 0x40,
   0, 0x20,
   0, 0x10,
   0, 0x8,
   0, 0x4,
   0, 0x2,
   0, 0x1,
   1, 0x80,
   1, 0x40,
   1, 0x20,
   1, 0x10,
   1, 0x8,
   1, 0x4,
   1, 0x2,
   1, 0x1,
   3, 0x80,
   3, 0x40,
   3, 0x20,
   3, 0x10,
   3, 0x8,
   3, 0x4,
   7, 0x8,
   7, 0x4,
   10, 0x8,
   10, 0x4,
   14, 0x8,
   14, 0x4,
   6, 0x1,
   6, 0x2,
   6, 0x4,
   6, 0x8,
   13, 0x1,
   13, 0x2,
   13, 0x4,
   13, 0x8,
   2, 0x20,
   2, 0x10,
   2, 0x4,
   2, 0x1,
   13, 0x10,
   13, 0x20,
   13, 0x40,
   13, 0x80,
   3, 0x2,
   3, 0x1,
   6, 0x10,
   6, 0x20,
   6, 0x40,
   6, 0x80,
   5, 0x2,
   5, 0x1,
   2, 0x40,
   2, 0x8,
   2, 0x2,
   7, 0x2,
   7, 0x1,
   9, 0x2,
   9, 0x1,
   10, 0x2,
   10, 0x1,
   12, 0x2,
   12, 0x1,
   14, 0x2,
   14, 0x1,
   16, 0x2,
   16, 0x1,
   4, 0x20,
   4, 0x10,
   4, 0x4,
   4, 0x2,
   8, 0x20,
   8, 0x10,
   8, 0x4,
   8, 0x2,
   11, 0x20,
   11, 0x10,
   11, 0x4,
   11, 0x2,
   15, 0x20,
   15, 0x10,
   15, 0x4,
   15, 0x2,
   4, 0x8,
   8, 0x8,
   11, 0x8,
   15, 0x8,
   4, 0x1,
   8, 0x1,
   11, 0x1,
   15, 0x1,
   4, 0x40,
   8, 0x40,
   11, 0x40,
   15, 0x40
};
static Word16 order_MR515[] =
{
   0, 0x1,
   0, 0x2,
   0, 0x4,
   0, 0x8,
   0, 0x10,
   0, 0x20,
   0, 0x40,
   0, 0x80,
   1, 0x1,
   1, 0x2,
   1, 0x4,
   1, 0x8,
   1, 0x10,
   1, 0x20,
   1, 0x40,
   1, 0x80,
   3, 0x80,
   3, 0x40,
   3, 0x20,
   3, 0x10,
   3, 0x8,
   7, 0x8,
   11, 0x8,
   15, 0x8,
   6, 0x1,
   6, 0x2,
   6, 0x4,
   10, 0x1,
   10, 0x2,
   10, 0x4,
   14, 0x1,
   14, 0x2,
   14, 0x4,
   18, 0x1,
   18, 0x2,
   18, 0x4,
   6, 0x8,
   10, 0x8,
   14, 0x8,
   18, 0x8,
   3, 0x4,
   7, 0x4,
   11, 0x4,
   15, 0x4,
   2, 0x10,
   6, 0x10,
   10, 0x10,
   14, 0x10,
   18, 0x10,
   3, 0x2,
   7, 0x2,
   11, 0x2,
   2, 0x20,
   2, 0x4,
   2, 0x1,
   6, 0x20,
   10, 0x20,
   14, 0x20,
   18, 0x20,
   2, 0x2,
   3, 0x1,
   7, 0x1,
   11, 0x1,
   15, 0x2,
   2, 0x8,
   2, 0x40,
   15, 0x1,
   5, 0x1,
   5, 0x2,
   9, 0x1,
   9, 0x2,
   13, 0x1,
   4, 0x4,
   8, 0x4,
   12, 0x4,
   16, 0x4,
   13, 0x2,
   17, 0x1,
   17, 0x2,
   4, 0x2,
   8, 0x2,
   12, 0x2,
   16, 0x2,
   4, 0x20,
   8, 0x20,
   4, 0x10,
   8, 0x10,
   12, 0x20,
   12, 0x10,
   16, 0x20,
   16, 0x10,
   4, 0x40,
   8, 0x40,
   12, 0x40,
   16, 0x40,
   4, 0x1,
   8, 0x1,
   12, 0x1,
   16, 0x1,
   4, 0x8,
   8, 0x8,
   12, 0x8,
   16, 0x8
};
static Word16 order_MR59[] =
{
   0, 0x80,
   0, 0x40,
   0, 0x8,
   0, 0x4,
   0, 0x10,
   0, 0x2,
   0, 0x1,
   0, 0x20,
   1, 0x8,
   1, 0x2,
   1, 0x100,
   1, 0x80,
   1, 0x20,
   1, 0x10,
   1, 0x4,
   1, 0x40,
   1, 0x1,
   3, 0x20,
   11, 0x20,
   3, 0x10,
   11, 0x10,
   3, 0x40,
   11, 0x40,
   3, 0x80,
   11, 0x80,
   3, 0x8,
   11, 0x8,
   7, 0x8,
   15, 0x8,
   6, 0x1,
   10, 0x1,
   14, 0x1,
   18, 0x1,
   3, 0x4,
   11, 0x4,
   7, 0x4,
   15, 0x4,
   6, 0x2,
   10, 0x2,
   14, 0x2,
   18, 0x2,
   7, 0x2,
   15, 0x2,
   3, 0x2,
   11, 0x2,
   3, 0x1,
   11, 0x1,
   6, 0x4,
   10, 0x4,
   14, 0x4,
   18, 0x4,
   6, 0x8,
   10, 0x8,
   14, 0x8,
   18, 0x8,
   6, 0x10,
   10, 0x10,
   14, 0x10,
   18, 0x10,
   2, 0x40,
   2, 0x10,
   2, 0x4,
   2, 0x8,
   2, 0x80,
   2, 0x100,
   2, 0x20,
   2, 0x2,
   17, 0x1,
   5, 0x2,
   13, 0x2,
   17, 0x2,
   9, 0x2,
   9, 0x1,
   5, 0x1,
   13, 0x1,
   2, 0x1,
   6, 0x20,
   10, 0x20,
   14, 0x20,
   18, 0x20,
   7, 0x1,
   15, 0x1,
   4, 0x4,
   8, 0x4,
   12, 0x4,
   16, 0x4,
   4, 0x8,
   8, 0x8,
   12, 0x8,
   16, 0x8,
   4, 0x40,
   8, 0x40,
   12, 0x40,
   16, 0x40,
   4, 0x80,
   8, 0x80,
   12, 0x80,
   16, 0x80,
   4, 0x100,
   8, 0x100,
   12, 0x100,
   16, 0x100,
   4, 0x1,
   8, 0x1,
   12, 0x1,
   16, 0x1,
   4, 0x2,
   8, 0x2,
   12, 0x2,
   16, 0x2,
   4, 0x10,
   8, 0x10,
   12, 0x10,
   16, 0x10,
   4, 0x20,
   8, 0x20,
   12, 0x20,
   16, 0x20
};
static Word16 order_MR67[] =
{
   0, 0x80,
   0, 0x40,
   0, 0x8,
   0, 0x10,
   0, 0x4,
   0, 0x2,
   1, 0x8,
   0, 0x1,
   0, 0x20,
   1, 0x100,
   1, 0x80,
   1, 0x20,
   1, 0x2,
   1, 0x10,
   1, 0x4,
   1, 0x40,
   3, 0x20,
   11, 0x20,
   3, 0x10,
   11, 0x10,
   3, 0x40,
   11, 0x40,
   3, 0x80,
   11, 0x80,
   3, 0x8,
   11, 0x8,
   1, 0x1,
   7, 0x8,
   15, 0x8,
   7, 0x4,
   15, 0x4,
   3, 0x4,
   11, 0x4,
   7, 0x2,
   15, 0x2,
   6, 0x40,
   10, 0x40,
   14, 0x40,
   18, 0x40,
   3, 0x2,
   11, 0x2,
   6, 0x8,
   10, 0x8,
   14, 0x8,
   18, 0x8,
   6, 0x4,
   10, 0x4,
   14, 0x4,
   18, 0x4,
   7, 0x1,
   15, 0x1,
   3, 0x1,
   11, 0x1,
   2, 0x40,
   2, 0x4,
   6, 0x2,
   10, 0x2,
   14, 0x2,
   18, 0x2,
   2, 0x10,
   2, 0x8,
   2, 0x80,
   2, 0x100,
   2, 0x20,
   2, 0x2,
   2, 0x1,
   6, 0x10,
   10, 0x10,
   14, 0x10,
   18, 0x10,
   5, 0x1,
   9, 0x1,
   13, 0x1,
   17, 0x1,
   6, 0x1,
   10, 0x1,
   14, 0x1,
   18, 0x1,
   5, 0x2,
   9, 0x2,
   13, 0x2,
   17, 0x2,
   18, 0x20,
   14, 0x20,
   10, 0x20,
   6, 0x20,
   5, 0x4,
   9, 0x4,
   13, 0x4,
   17, 0x4,
   4, 0x4,
   8, 0x4,
   12, 0x4,
   16, 0x4,
   4, 0x20,
   8, 0x20,
   12, 0x20,
   16, 0x20,
   4, 0x40,
   8, 0x40,
   12, 0x40,
   16, 0x40,
   4, 0x200,
   8, 0x200,
   12, 0x200,
   16, 0x200,
   4, 0x400,
   8, 0x400,
   12, 0x400,
   16, 0x400,
   4, 0x1,
   8, 0x1,
   12, 0x1,
   16, 0x1,
   4, 0x2,
   8, 0x2,
   12, 0x2,
   16, 0x2,
   4, 0x8,
   8, 0x8,
   12, 0x8,
   16, 0x8,
   4, 0x10,
   8, 0x10,
   12, 0x10,
   16, 0x10,
   4, 0x80,
   8, 0x80,
   12, 0x80,
   16, 0x80,
   4, 0x100,
   8, 0x100,
   12, 0x100,
   16, 0x100
};
static Word16 order_MR74[] =
{
   0, 0x80,
   0, 0x40,
   0, 0x20,
   0, 0x10,
   0, 0x8,
   0, 0x4,
   0, 0x2,
   0, 0x1,
   1, 0x100,
   1, 0x80,
   1, 0x40,
   1, 0x20,
   1, 0x10,
   1, 0x8,
   1, 0x4,
   1, 0x2,
   1, 0x1,
   3, 0x80,
   11, 0x80,
   3, 0x40,
   11, 0x40,
   3, 0x20,
   11, 0x20,
   3, 0x10,
   11, 0x10,
   3, 0x8,
   11, 0x8,
   6, 0x40,
   10, 0x40,
   14, 0x40,
   18, 0x40,
   6, 0x20,
   10, 0x20,
   14, 0x20,
   18, 0x20,
   6, 0x8,
   10, 0x8,
   14, 0x8,
   18, 0x8,
   6, 0x4,
   10, 0x4,
   14, 0x4,
   18, 0x4,
   7, 0x10,
   15, 0x10,
   7, 0x8,
   15, 0x8,
   2, 0x10,
   2, 0x8,
   2, 0x4,
   2, 0x100,
   2, 0x80,
   2, 0x40,
   3, 0x4,
   7, 0x4,
   11, 0x4,
   15, 0x4,
   6, 0x2,
   10, 0x2,
   14, 0x2,
   18, 0x2,
   2, 0x20,
   2, 0x2,
   2, 0x1,
   5, 0x1,
   9, 0x1,
   13, 0x1,
   17, 0x1,
   6, 0x1,
   10, 0x1,
   14, 0x1,
   18, 0x1,
   5, 0x2,
   9, 0x2,
   13, 0x2,
   17, 0x2,
   5, 0x4,
   9, 0x4,
   6, 0x10,
   10, 0x10,
   14, 0x10,
   18, 0x10,
   13, 0x4,
   17, 0x4,
   5, 0x8,
   9, 0x8,
   13, 0x8,
   17, 0x8,
   3, 0x2,
   3, 0x1,
   7, 0x2,
   7, 0x1,
   11, 0x2,
   11, 0x1,
   15, 0x2,
   15, 0x1,
   4, 0x20,
   4, 0x10,
   4, 0x8,
   4, 0x4,
   4, 0x2,
   4, 0x1,
   8, 0x20,
   8, 0x10,
   8, 0x8,
   8, 0x4,
   8, 0x2,
   8, 0x1,
   12, 0x20,
   12, 0x10,
   12, 0x8,
   12, 0x4,
   12, 0x2,
   12, 0x1,
   16, 0x20,
   16, 0x10,
   16, 0x8,
   16, 0x4,
   16, 0x2,
   16, 0x1,
   4, 0x1000,
   8, 0x1000,
   12, 0x1000,
   16, 0x1000,
   4, 0x800,
   8, 0x800,
   12, 0x800,
   16, 0x800,
   4, 0x400,
   8, 0x400,
   12, 0x400,
   16, 0x400,
   4, 0x200,
   8, 0x200,
   12, 0x200,
   16, 0x200,
   4, 0x100,
   8, 0x100,
   12, 0x100,
   16, 0x100,
   4, 0x80,
   8, 0x80,
   12, 0x80,
   16, 0x80,
   4, 0x40,
   8, 0x40,
   12, 0x40,
   16, 0x40
};
static Word16 order_MR795[] =
{
   0, 0x1,
   0, 0x2,
   0, 0x4,
   0, 0x8,
   0, 0x10,
   0, 0x20,
   0, 0x40,
   1, 0x8,
   1, 0x2,
   1, 0x100,
   1, 0x80,
   1, 0x20,
   1, 0x10,
   1, 0x4,
   1, 0x40,
   1, 0x1,
   2, 0x40,
   2, 0x10,
   2, 0x4,
   2, 0x8,
   2, 0x80,
   2, 0x100,
   2, 0x20,
   7, 0x10,
   12, 0x10,
   17, 0x10,
   22, 0x10,
   7, 0x8,
   12, 0x8,
   17, 0x8,
   22, 0x8,
   7, 0x4,
   12, 0x4,
   17, 0x4,
   22, 0x4,
   6, 0x8,
   11, 0x8,
   16, 0x8,
   21, 0x8,
   6, 0x4,
   11, 0x4,
   16, 0x4,
   21, 0x4,
   3, 0x80,
   13, 0x80,
   3, 0x40,
   13, 0x40,
   3, 0x20,
   13, 0x20,
   3, 0x10,
   13, 0x10,
   3, 0x8,
   13, 0x8,
   8, 0x20,
   18, 0x20,
   8, 0x10,
   18, 0x10,
   8, 0x8,
   18, 0x8,
   7, 0x2,
   12, 0x2,
   17, 0x2,
   22, 0x2,
   3, 0x4,
   13, 0x4,
   8, 0x4,
   18, 0x4,
   0, 0x80,
   0, 0x100,
   2, 0x2,
   2, 0x1,
   3, 0x2,
   13, 0x2,
   3, 0x1,
   13, 0x1,
   8, 0x2,
   18, 0x2,
   8, 0x1,
   18, 0x1,
   6, 0x2,
   11, 0x2,
   16, 0x2,
   21, 0x2,
   7, 0x1,
   12, 0x1,
   17, 0x1,
   22, 0x1,
   6, 0x1,
   11, 0x1,
   16, 0x1,
   21, 0x1,
   15, 0x1,
   15, 0x2,
   15, 0x4,
   4, 0x2,
   9, 0x2,
   14, 0x2,
   19, 0x2,
   4, 0x10,
   9, 0x10,
   14, 0x10,
   19, 0x10,
   4, 0x80,
   9, 0x80,
   14, 0x80,
   19, 0x80,
   4, 0x800,
   9, 0x800,
   14, 0x800,
   19, 0x800,
   15, 0x8,
   20, 0x1,
   20, 0x2,
   20, 0x4,
   20, 0x8,
   10, 0x1,
   10, 0x2,
   10, 0x4,
   10, 0x8,
   5, 0x1,
   5, 0x2,
   5, 0x4,
   5, 0x8,
   4, 0x1,
   4, 0x4,
   4, 0x8,
   4, 0x20,
   4, 0x100,
   4, 0x1000,
   9, 0x1,
   9, 0x4,
   9, 0x8,
   9, 0x20,
   9, 0x100,
   9, 0x1000,
   14, 0x1,
   14, 0x4,
   14, 0x8,
   14, 0x20,
   14, 0x100,
   14, 0x1000,
   19, 0x1,
   19, 0x4,
   19, 0x8,
   19, 0x20,
   19, 0x100,
   19, 0x1000,
   4, 0x40,
   9, 0x40,
   14, 0x40,
   19, 0x40,
   4, 0x400,
   9, 0x400,
   14, 0x400,
   19, 0x400,
   4, 0x200,
   9, 0x200,
   14, 0x200,
   19, 0x200,
   0, 0x1,
   0, 0x2,
   0, 0x4,
   0, 0x8,
   0, 0x10,
   0, 0x20,
   0, 0x40,
   1, 0x8,
   1, 0x2,
   1, 0x100,
   1, 0x80,
   1, 0x20,
   1, 0x10,
   1, 0x4,
   1, 0x40,
   1, 0x1,
   2, 0x40,
   2, 0x10,
   2, 0x4,
   2, 0x8,
   2, 0x80,
   2, 0x100,
   2, 0x20,
   7, 0x10,
   12, 0x10,
   17, 0x10,
   22, 0x10,
   7, 0x8,
   12, 0x8,
   17, 0x8,
   22, 0x8,
   7, 0x4,
   12, 0x4,
   17, 0x4,
   22, 0x4,
   6, 0x8,
   11, 0x8,
   16, 0x8,
   21, 0x8,
   6, 0x4,
   11, 0x4,
   16, 0x4,
   21, 0x4,
   3, 0x80,
   13, 0x80,
   3, 0x40,
   13, 0x40,
   3, 0x20,
   13, 0x20,
   3, 0x10,
   13, 0x10,
   3, 0x8,
   13, 0x8,
   8, 0x20,
   18, 0x20,
   8, 0x10,
   18, 0x10,
   8, 0x8,
   18, 0x8,
   7, 0x2,
   12, 0x2,
   17, 0x2,
   22, 0x2,
   3, 0x4,
   13, 0x4,
   8, 0x4,
   18, 0x4,
   0, 0x80,
   0, 0x100,
   2, 0x2,
   2, 0x1,
   3, 0x2,
   13, 0x2,
   3, 0x1,
   13, 0x1,
   8, 0x2,
   18, 0x2,
   8, 0x1,
   18, 0x1,
   6, 0x2,
   11, 0x2,
   16, 0x2,
   21, 0x2,
   7, 0x1,
   12, 0x1,
   17, 0x1,
   22, 0x1,
   6, 0x1,
   11, 0x1,
   16, 0x1,
   21, 0x1,
   15, 0x1,
   15, 0x2,
   15, 0x4,
   4, 0x2,
   9, 0x2,
   14, 0x2,
   19, 0x2,
   4, 0x10,
   9, 0x10,
   14, 0x10,
   19, 0x10,
   4, 0x80,
   9, 0x80,
   14, 0x80,
   19, 0x80,
   4, 0x800,
   9, 0x800,
   14, 0x800,
   19, 0x800,
   15, 0x8,
   20, 0x1,
   20, 0x2,
   20, 0x4,
   20, 0x8,
   10, 0x1,
   10, 0x2,
   10, 0x4,
   10, 0x8,
   5, 0x1,
   5, 0x2,
   5, 0x4,
   5, 0x8,
   4, 0x1,
   4, 0x4,
   4, 0x8,
   4, 0x20,
   4, 0x100,
   4, 0x1000,
   9, 0x1,
   9, 0x4,
   9, 0x8,
   9, 0x20,
   9, 0x100,
   9, 0x1000,
   14, 0x1,
   14, 0x4,
   14, 0x8,
   14, 0x20,
   14, 0x100,
   14, 0x1000,
   19, 0x1,
   19, 0x4,
   19, 0x8,
   19, 0x20,
   19, 0x100,
   19, 0x1000,
   4, 0x40,
   9, 0x40,
   14, 0x40,
   19, 0x40,
   4, 0x400,
   9, 0x400,
   14, 0x400,
   19, 0x400,
   4, 0x200,
   9, 0x200,
   14, 0x200,
   19, 0x200
};
static Word16 order_MR102[] =
{
   0, 0x1,
   0, 0x2,
   0, 0x4,
   0, 0x8,
   0, 0x10,
   0, 0x20,
   0, 0x40,
   0, 0x80,
   1, 0x1,
   1, 0x2,
   1, 0x4,
   1, 0x8,
   1, 0x10,
   1, 0x20,
   1, 0x40,
   1, 0x80,
   1, 0x100,
   3, 0x80,
   3, 0x40,
   3, 0x20,
   3, 0x10,
   3, 0x8,
   3, 0x4,
   21, 0x80,
   21, 0x40,
   21, 0x20,
   21, 0x10,
   21, 0x8,
   21, 0x4,
   12, 0x10,
   12, 0x8,
   30, 0x10,
   30, 0x8,
   11, 0x40,
   11, 0x8,
   11, 0x4,
   20, 0x40,
   20, 0x8,
   20, 0x4,
   29, 0x40,
   29, 0x8,
   29, 0x4,
   38, 0x40,
   38, 0x8,
   38, 0x4,
   3, 0x2,
   3, 0x1,
   21, 0x2,
   21, 0x1,
   12, 0x4,
   12, 0x2,
   30, 0x4,
   30, 0x2,
   11, 0x20,
   20, 0x20,
   29, 0x20,
   38, 0x20,
   2, 0x40,
   2, 0x4,
   2, 0x10,
   2, 0x8,
   2, 0x80,
   2, 0x100,
   2, 0x20,
   2, 0x2,
   2, 0x1,
   7, 0x1,
   6, 0x1,
   5, 0x1,
   4, 0x1,
   16, 0x1,
   15, 0x1,
   14, 0x1,
   13, 0x1,
   25, 0x1,
   24, 0x1,
   23, 0x1,
   22, 0x1,
   34, 0x1,
   33, 0x1,
   32, 0x1,
   31, 0x1,
   11, 0x2,
   11, 0x10,
   11, 0x1,
   20, 0x2,
   20, 0x10,
   20, 0x1,
   29, 0x2,
   29, 0x10,
   29, 0x1,
   38, 0x2,
   38, 0x10,
   38, 0x1,
   12, 0x1,
   30, 0x1,
   17, 0x200,
   17, 0x100,
   18, 0x100,
   18, 0x200,
   18, 0x80,
   17, 0x80,
   18, 0x20,
   17, 0x20,
   17, 0x40,
   18, 0x40,
   19, 0x40,
   19, 0x20,
   18, 0x10,
   19, 0x8,
   17, 0x10,
   19, 0x10,
   17, 0x8,
   18, 0x8,
   26, 0x200,
   26, 0x100,
   27, 0x100,
   27, 0x200,
   27, 0x80,
   26, 0x80,
   27, 0x20,
   26, 0x20,
   26, 0x40,
   27, 0x40,
   28, 0x40,
   28, 0x20,
   27, 0x10,
   28, 0x8,
   26, 0x10,
   28, 0x10,
   26, 0x8,
   27, 0x8,
   35, 0x200,
   35, 0x100,
   36, 0x100,
   36, 0x200,
   36, 0x80,
   35, 0x80,
   36, 0x20,
   35, 0x20,
   35, 0x40,
   36, 0x40,
   37, 0x40,
   37, 0x20,
   36, 0x10,
   37, 0x8,
   35, 0x10,
   37, 0x10,
   35, 0x8,
   36, 0x8,
   8, 0x200,
   8, 0x100,
   9, 0x100,
   9, 0x200,
   9, 0x80,
   8, 0x80,
   9, 0x20,
   8, 0x20,
   8, 0x40,
   9, 0x40,
   10, 0x40,
   10, 0x20,
   9, 0x10,
   10, 0x8,
   8, 0x10,
   10, 0x10,
   8, 0x8,
   9, 0x8,
   37, 0x4,
   35, 0x1,
   36, 0x1,
   37, 0x1,
   35, 0x4,
   37, 0x2,
   35, 0x2,
   36, 0x4,
   36, 0x2,
   28, 0x4,
   26, 0x1,
   27, 0x1,
   28, 0x1,
   26, 0x4,
   28, 0x2,
   26, 0x2,
   27, 0x4,
   27, 0x2,
   19, 0x4,
   17, 0x1,
   18, 0x1,
   19, 0x1,
   17, 0x4,
   19, 0x2,
   17, 0x2,
   18, 0x4,
   18, 0x2,
   10, 0x4,
   8, 0x1,
   9, 0x1,
   10, 0x1,
   8, 0x4,
   10, 0x2,
   8, 0x2,
   9, 0x4,
   9, 0x2
};
static Word16 order_MR122[] =
{
   0, 0x40,
   0, 0x20,
   0, 0x10,
   0, 0x8,
   0, 0x4,
   0, 0x2,
   0, 0x1,
   1, 0x80,
   1, 0x40,
   1, 0x20,
   1, 0x10,
   1, 0x8,
   1, 0x4,
   1, 0x2,
   1, 0x1,
   2, 0x1,
   2, 0x100,
   2, 0x80,
   2, 0x40,
   2, 0x20,
   2, 0x10,
   2, 0x8,
   2, 0x4,
   2, 0x2,
   3, 0x80,
   3, 0x40,
   3, 0x20,
   3, 0x10,
   3, 0x8,
   5, 0x100,
   31, 0x100,
   5, 0x80,
   31, 0x80,
   5, 0x40,
   31, 0x40,
   5, 0x20,
   31, 0x20,
   5, 0x10,
   31, 0x10,
   5, 0x8,
   31, 0x8,
   5, 0x4,
   31, 0x4,
   5, 0x2,
   31, 0x2,
   5, 0x1,
   31, 0x1,
   6, 0x8,
   19, 0x8,
   32, 0x8,
   45, 0x8,
   6, 0x4,
   19, 0x4,
   32, 0x4,
   45, 0x4,
   6, 0x2,
   19, 0x2,
   32, 0x2,
   45, 0x2,
   17, 0x10,
   30, 0x10,
   43, 0x10,
   56, 0x10,
   17, 0x8,
   30, 0x8,
   43, 0x8,
   56, 0x8,
   17, 0x4,
   30, 0x4,
   43, 0x4,
   56, 0x4,
   18, 0x20,
   44, 0x20,
   18, 0x10,
   44, 0x10,
   18, 0x8,
   44, 0x8,
   18, 0x4,
   44, 0x4,
   18, 0x2,
   44, 0x2,
   3, 0x4,
   3, 0x2,
   3, 0x1,
   4, 0x20,
   4, 0x10,
   4, 0x8,
   4, 0x4,
   6, 0x1,
   19, 0x1,
   32, 0x1,
   45, 0x1,
   17, 0x2,
   30, 0x2,
   43, 0x2,
   56, 0x2,
   7, 0x8,
   20, 0x8,
   33, 0x8,
   46, 0x8,
   8, 0x8,
   21, 0x8,
   34, 0x8,
   47, 0x8,
   17, 0x1,
   30, 0x1,
   43, 0x1,
   56, 0x1,
   9, 0x8,
   22, 0x8,
   35, 0x8,
   48, 0x8,
   10, 0x8,
   23, 0x8,
   36, 0x8,
   49, 0x8,
   11, 0x8,
   24, 0x8,
   37, 0x8,
   50, 0x8,
   4, 0x2,
   4, 0x1,
   7, 0x1,
   7, 0x2,
   7, 0x4,
   8, 0x1,
   8, 0x2,
   8, 0x4,
   9, 0x1,
   9, 0x2,
   9, 0x4,
   10, 0x1,
   10, 0x2,
   10, 0x4,
   11, 0x1,
   11, 0x2,
   11, 0x4,
   20, 0x1,
   20, 0x2,
   20, 0x4,
   21, 0x1,
   21, 0x2,
   21, 0x4,
   22, 0x1,
   22, 0x2,
   22, 0x4,
   23, 0x1,
   23, 0x2,
   23, 0x4,
   24, 0x1,
   24, 0x2,
   24, 0x4,
   33, 0x1,
   33, 0x2,
   33, 0x4,
   34, 0x1,
   34, 0x2,
   34, 0x4,
   35, 0x1,
   35, 0x2,
   35, 0x4,
   36, 0x1,
   36, 0x2,
   36, 0x4,
   37, 0x1,
   37, 0x2,
   37, 0x4,
   46, 0x1,
   46, 0x2,
   46, 0x4,
   47, 0x1,
   47, 0x2,
   47, 0x4,
   48, 0x1,
   48, 0x2,
   48, 0x4,
   49, 0x1,
   49, 0x2,
   49, 0x4,
   50, 0x1,
   50, 0x2,
   50, 0x4,
   12, 0x1,
   12, 0x2,
   12, 0x4,
   13, 0x1,
   13, 0x2,
   13, 0x4,
   14, 0x1,
   14, 0x2,
   14, 0x4,
   15, 0x1,
   15, 0x2,
   15, 0x4,
   16, 0x1,
   16, 0x2,
   16, 0x4,
   25, 0x1,
   25, 0x2,
   25, 0x4,
   26, 0x1,
   26, 0x2,
   26, 0x4,
   27, 0x1,
   27, 0x2,
   27, 0x4,
   28, 0x1,
   28, 0x2,
   28, 0x4,
   29, 0x1,
   29, 0x2,
   29, 0x4,
   38, 0x1,
   38, 0x2,
   38, 0x4,
   39, 0x1,
   39, 0x2,
   39, 0x4,
   40, 0x1,
   40, 0x2,
   40, 0x4,
   41, 0x1,
   41, 0x2,
   41, 0x4,
   42, 0x1,
   42, 0x2,
   42, 0x4,
   51, 0x1,
   51, 0x2,
   51, 0x4,
   52, 0x1,
   52, 0x2,
   52, 0x4,
   53, 0x1,
   53, 0x2,
   53, 0x4,
   54, 0x1,
   54, 0x2,
   54, 0x4,
   55, 0x1,
   55, 0x2,
   55, 0x4,
   18, 0x1,
   44, 0x1
};
static Word16 order_MRDTX[] =
{
   0, 0x4,
   0, 0x2,
   0, 0x1,
   1, 0x80,
   1, 0x40,
   1, 0x20,
   1, 0x10,
   1, 0x8,
   1, 0x4,
   1, 0x2,
   1, 0x1,
   2, 0x100,
   2, 0x80,
   2, 0x40,
   2, 0x20,
   2, 0x10,
   2, 0x8,
   2, 0x4,
   2, 0x2,
   2, 0x1,
   3, 0x100,
   3, 0x80,
   3, 0x40,
   3, 0x20,
   3, 0x10,
   3, 0x8,
   3, 0x4,
   3, 0x2,
   3, 0x1,
   4, 0x20,
   4, 0x10,
   4, 0x8,
   4, 0x4,
   4, 0x2,
   4, 0x1
};

/*
 * Declare structure types
 */
/* Declaration transmitted frame types */
enum TXFrameType { TX_SPEECH_GOOD = 0,
                   TX_SID_FIRST,
                   TX_SID_UPDATE,
                   TX_NO_DATA,
                   TX_SPEECH_DEGRADED,
                   TX_SPEECH_BAD,
                   TX_SID_BAD,
                   TX_ONSET,
                   TX_N_FRAMETYPES     /* number of frame types */
};

/* Declaration of interface structure */
typedef struct
{
   Word16 sid_update_counter;   /* Number of frames since last SID */
   Word16 sid_handover_debt;   /* Number of extra SID_UPD frames to schedule */
   Word32 dtx;
   enum TXFrameType prev_ft;   /* Type of the previous frame */
   void *encoderState;   /* Points encoder state structure */
} enc_interface_State;


#ifdef ETSI
/*
 * Prm2Bits
 *
 *
 * Parameters:
 *    value             I: value to be converted to binary
 *    no_of_bits        I: number of bits associated with value
 *    bitstream         O: address where bits are written
 *
 * Function:
 *    Convert integer to binary and write the bits to the array.
 *    The most significant bits are written first.
 * Returns:
 *    void
 */
static void Int2Bin( Word16 value, Word16 no_of_bits, Word16 *bitstream )
{
   Word32 i, bit;
   Word16 *pt_bitstream;

   pt_bitstream = &bitstream[no_of_bits];

   for ( i = 0; i < no_of_bits; i++ ) {
      bit = value & 0x0001;

      if ( bit == 0 ) {
         * --pt_bitstream = 0;
      }
      else {
         * --pt_bitstream = 1;
      }
      value = ( Word16 )( value >> 1 );
   }
}


/*
 * Prm2Bits
 *
 *
 * Parameters:
 *    mode              I: AMR mode
 *    prm               I: analysis parameters
 *    bits              O: serial bits
 *
 * Function:
 *    converts the encoder parameter vector into a vector of serial bits.
 * Returns:
 *    void
 */
static void Prm2Bits( enum Mode mode, Word16 prm[], Word16 bits[] )
{
   Word32 i;

   switch ( mode ) {
      case MR122:
         for ( i = 0; i < PRMNO_MR122; i++ ) {
            Int2Bin( prm[i], bitno_MR122[i], bits );
            bits += bitno_MR122[i];
         }
         break;

      case MR102:
         for ( i = 0; i < PRMNO_MR102; i++ ) {
            Int2Bin( prm[i], bitno_MR102[i], bits );
            bits += bitno_MR102[i];
         }
         break;

      case MR795:
         for ( i = 0; i < PRMNO_MR795; i++ ) {
            Int2Bin( prm[i], bitno_MR795[i], bits );
            bits += bitno_MR795[i];
         }
         break;

      case MR74:
         for ( i = 0; i < PRMNO_MR74; i++ ) {
            Int2Bin( prm[i], bitno_MR74[i], bits );
            bits += bitno_MR74[i];
         }
         break;

      case MR67:
         for ( i = 0; i < PRMNO_MR67; i++ ) {
            Int2Bin( prm[i], bitno_MR67[i], bits );
            bits += bitno_MR67[i];
         }
         break;

      case MR59:
         for ( i = 0; i < PRMNO_MR59; i++ ) {
            Int2Bin( prm[i], bitno_MR59[i], bits );
            bits += bitno_MR59[i];
         }
         break;

      case MR515:
         for ( i = 0; i < PRMNO_MR515; i++ ) {
            Int2Bin( prm[i], bitno_MR515[i], bits );
            bits += bitno_MR515[i];
         }
         break;

      case MR475:
         for ( i = 0; i < PRMNO_MR475; i++ ) {
            Int2Bin( prm[i], bitno_MR475[i], bits );
            bits += bitno_MR475[i];
         }
         break;

      case MRDTX:
         for ( i = 0; i < PRMNO_MRDTX; i++ ) {
            Int2Bin( prm[i], bitno_MRDTX[i], bits );
            bits += bitno_MRDTX[i];
         }
         break;
   }
   return;
}

#else

#ifndef IF2

/*
 * EncoderMMS
 *
 *
 * Parameters:
 *    mode                 I: AMR mode
 *    param                I: Encoder output parameters
 *    stream               O: packed speech frame
 *    frame_type           I: frame type (DTX)
 *    speech_mode          I: speech mode (DTX)
 *
 * Function:
 *    Pack encoder output parameters to octet structure according
 *    importance table and AMR file storage format according to
 *    RFC 3267.
 * Returns:
 *    number of octets
 */
int EncoderIncludeHeaderByte = 1;
static int EncoderMMS( enum Mode mode, Word16 *param, UWord8 *stream, enum
      TXFrameType frame_type, enum Mode speech_mode )
{
   Word32 j = 0, k;
   Word16 *mask;
   int resultFrameSize = block_size[mode] - !EncoderIncludeHeaderByte;

   memset(stream, 0, resultFrameSize);

   if (EncoderIncludeHeaderByte) {
     *stream = toc_byte[mode];
     stream++;
   }

   if ( mode == 15 ) {
      return 1;
   }
   else if ( mode == MRDTX ) {
      mask = order_MRDTX;

      for ( j = 1; j < 36; j++ ) {
         if ( param[ * mask] & *( mask + 1 ) )
            *stream += 0x01;
         mask += 2;

         if ( j % 8 )
            *stream <<= 1;
         else
            stream++;
      }

      /* add SID type information */
      if ( frame_type == TX_SID_UPDATE )
         *stream += 0x01;
      *stream <<= 3;

      /* speech mode indication */
      *stream += ( unsigned char )(speech_mode & 0x0007);

	  *stream <<= 1;

      /* don't shift at the end of the function */
      return 6;
   }
   else if ( mode == MR475 ) {
      mask = order_MR475;

      for ( j = 1; j < 96; j++ ) {
         if ( param[ * mask] & *( mask + 1 ) )
            *stream += 0x01;
         mask += 2;

         if ( j % 8 )
            *stream <<= 1;
         else
            stream++;
      }
   }
   else if ( mode == MR515 ) {
      mask = order_MR515;

      for ( j = 1; j < 104; j++ ) {
         if ( param[ * mask] & *( mask + 1 ) )
            *stream += 0x01;
         mask += 2;

         if ( j % 8 )
            *stream <<= 1;
         else
            stream++;
      }
   }
   else if ( mode == MR59 ) {
      mask = order_MR59;

      for ( j = 1; j < 119; j++ ) {
         if ( param[ * mask] & *( mask + 1 ) )
            *stream += 0x01;
         mask += 2;

         if ( j % 8 )
            *stream <<= 1;
         else
            stream++;
      }
   }
   else if ( mode == MR67 ) {
      mask = order_MR67;

      for ( j = 1; j < 135; j++ ) {
         if ( param[ * mask] & *( mask + 1 ) )
            *stream += 0x01;
         mask += 2;

         if ( j % 8 )
            *stream <<= 1;
         else
            stream++;
      }
   }
   else if ( mode == MR74 ) {
      mask = order_MR74;

      for ( j = 1; j < 149; j++ ) {
         if ( param[ * mask] & *( mask + 1 ) )
            *stream += 0x01;
         mask += 2;

         if ( j % 8 )
            *stream <<= 1;
         else
            stream++;
      }
   }
   else if ( mode == MR795 ) {
      mask = order_MR795;

      for ( j = 1; j < 160; j++ ) {
         if ( param[ * mask] & *( mask + 1 ) )
            *stream += 0x01;
         mask += 2;

         if ( j % 8 )
            *stream <<= 1;
         else
            stream++;
      }
   }
   else if ( mode == MR102 ) {
      mask = order_MR102;

      for ( j = 1; j < 205; j++ ) {
         if ( param[ * mask] & *( mask + 1 ) )
            *stream += 0x01;
         mask += 2;

         if ( j % 8 )
            *stream <<= 1;
         else
            stream++;
      }
   }
   else if ( mode == MR122 ) {
      mask = order_MR122;

      for ( j = 1; j < 245; j++ ) {
         if ( param[ * mask] & *( mask + 1 ) )
            *stream += 0x01;
         mask += 2;

         if ( j % 8 )
            *stream <<= 1;
         else
            stream++;
      }
   }

   /* shift remaining bits */
   if ( (k = (j % 8)) )
       *stream <<= ( 8 - k );
   
   return resultFrameSize;
}

#else

/*
 * Encoder3GPP
 *
 *
 * Parameters:
 *    mode                 I: AMR mode
 *    param                I: Encoder output parameters
 *    stream               O: packed speech frame
 *    frame_type           I: frame type (DTX)
 *    speech_mode          I: speech mode (DTX)
 *
 * Function:
 *    Pack encoder output parameters to octet structure according
 *    importance table.
 * Returns:
 *    number of octets
 */
static int Encoder3GPP( enum Mode mode, Word16 *param, UWord8 *stream, enum
      TXFrameType frame_type, enum Mode speech_mode )
{
   Word32 j = 0;
   Word16 *mask;

   memset(stream, 0, block_size[mode]);

   if ( mode == 15 ) {
      *stream = 0xF;
      return 1;
   }
   else if ( mode == MRDTX ) {
      mask = order_MRDTX;
      *stream = 0x40;

      for ( j = 5; j < 40; j++ ) {
         if ( param[ * mask] & *( mask + 1 ) )
            *stream += 0x80;
         mask += 2;

         if ( j % 8 )
            *stream >>= 1;
         else
            stream++;
      }

      /* add SID type information */
      if ( frame_type == TX_SID_UPDATE )
         *stream += 0x80;
      stream++;

      /* speech mode indication */
      *stream = ( unsigned char )speech_mode;

      /* don't shift at the end of the function */
      return 6;
   }
   else if ( mode == MR475 ) {
      mask = order_MR475;
      *stream = 0;

      for ( j = 5; j < 100; j++ ) {
         if ( param[ * mask] & *( mask + 1 ) )
            *stream += 0x80;
         mask += 2;

         if ( j % 8 )
            *stream >>= 1;
         else
            stream++;
      }
   }
   else if ( mode == MR515 ) {
      mask = order_MR515;
      *stream = 0x8;

      for ( j = 5; j < 108; j++ ) {
         if ( param[ * mask] & *( mask + 1 ) )
            *stream += 0x80;
         mask += 2;

         if ( j % 8 )
            *stream >>= 1;
         else
            stream++;
      }
   }
   else if ( mode == MR59 ) {
      mask = order_MR59;
      *stream = 0x10;

      for ( j = 5; j < 123; j++ ) {
         if ( param[ * mask] & *( mask + 1 ) )
            *stream += 0x80;
         mask += 2;

         if ( j % 8 )
            *stream >>= 1;
         else
            stream++;
      }
   }
   else if ( mode == MR67 ) {
      mask = order_MR67;
      *stream = 0x18;

      for ( j = 5; j < 139; j++ ) {
         if ( param[ * mask] & *( mask + 1 ) )
            *stream += 0x80;
         mask += 2;

         if ( j % 8 )
            *stream >>= 1;
         else
            stream++;
      }
   }
   else if ( mode == MR74 ) {
      mask = order_MR74;
      *stream = 0x20;

      for ( j = 5; j < 153; j++ ) {
         if ( param[ * mask] & *( mask + 1 ) )
            *stream += 0x80;
         mask += 2;

         if ( j % 8 )
            *stream >>= 1;
         else
            stream++;
      }
   }
   else if ( mode == MR795 ) {
      mask = order_MR795;
      *stream = 0x28;

      for ( j = 5; j < 164; j++ ) {
         if ( param[ * mask] & *( mask + 1 ) )
            *stream += 0x80;
         mask += 2;

         if ( j % 8 )
            *stream >>= 1;
         else
            stream++;
      }
   }
   else if ( mode == MR102 ) {
      mask = order_MR102;
      *stream = 0x30;

      for ( j = 5; j < 209; j++ ) {
         if ( param[ * mask] & *( mask + 1 ) )
            *stream += 0x80;
         mask += 2;

         if ( j % 8 )
            *stream >>= 1;
         else
            stream++;
      }
   }
   else if ( mode == MR122 ) {
      mask = order_MR122;
      *stream = 0x38;

      for ( j = 5; j < 249; j++ ) {
         if ( param[ * mask] & *( mask + 1 ) )
            *stream += 0x80;
         mask += 2;

         if ( j % 8 )
            *stream >>= 1;
         else
            stream++;
      }
   }

   /* shift remaining bits */
   *stream >>= ( 8 - j % 8 );
   return( (int)block_size[mode] );
}
#endif
#endif

/*
 * Sid_Sync_reset
 *
 *
 * Parameters:
 *    st                O: state structure
 *
 * Function:
 *    Initializes state memory
 *
 * Returns:
 *    void
 */
static void Sid_Sync_reset( enc_interface_State *st )
{
   st->sid_update_counter = 3;
   st->sid_handover_debt = 0;
   st->prev_ft = TX_SPEECH_GOOD;
}


/*
 * Encoder_Interface_Encode
 *
 *
 * Parameters:
 *    st                I: pointer to state structure
 *    mode              I: Speech Mode
 *    speech            I: Input speech
 *    serial            O: Output octet structure 3GPP or
 *                         ETSI serial stream
 *    force_speech      I: Force speech in DTX
 *
 * Function:
 *    Encoding and packing one frame of speech
 *
 * Returns:
 *    number of octets
 */
int Encoder_Interface_Encode( void *st, enum Mode mode, Word16 *speech,

#ifndef ETSI
      UWord8 *serial,

#else
      Word16 *serial,
#endif

      int force_speech )
{
   Word16 prm[PRMNO_MR122];   /* speech parameters, max size */
   const Word16 *homing;   /* pointer to homing frame */
   Word16 homing_size;   /* frame size for homing frame */


   enc_interface_State * s;
   enum TXFrameType txFrameType;   /* frame type */

   int i, noHoming = 0;


   /*
    * used encoder mode,
    * if used_mode == -1, force VAD on
    */
   enum Mode used_mode = -force_speech;

   memset(prm,0,sizeof(Word16)*PRMNO_MR122);

   s = ( enc_interface_State * )st;

    /*
     * Checks if all samples of the input frame matches the encoder
     * homing frame pattern, which is 0x0008 for all samples.
     */
   for ( i = 0; i < 160; i++ ) {
      noHoming = speech[i] ^ 0x0008;

      if ( noHoming )
         break;
   }

   if (noHoming){
      Speech_Encode_Frame( s->encoderState, mode, speech, prm, &used_mode );
   }
   else {
      switch ( mode ) {
         case MR122:
            homing = dhf_MR122;
            homing_size = 18;
            break;

         case MR102:
            homing = dhf_MR102;
            homing_size = 12;
            break;

         case MR795:
            homing = dhf_MR795;
            homing_size = 8;
            break;

         case MR74:
            homing = dhf_MR74;
            homing_size = 7;
            break;

         case MR67:
            homing = dhf_MR67;
            homing_size = 7;
            break;

         case MR59:
            homing = dhf_MR59;
            homing_size = 7;
            break;

         case MR515:
            homing = dhf_MR515;
            homing_size = 7;
            break;

         case MR475:
            homing = dhf_MR475;
            homing_size = 7;
            break;

         default:
            homing = NULL;
            homing_size = 0;
            break;
      }
      for( i = 0; i < homing_size; i++){
         prm[i] = homing[i];
      }
      /* rest of the parameters are zero */
      memset(&prm[homing_size], 0, (PRMNO_MR122 - homing_size)*sizeof(Word16));
      used_mode = mode;
   }
   if ( used_mode == MRDTX ) {
      s->sid_update_counter--;

      if ( s->prev_ft == TX_SPEECH_GOOD ) {
         txFrameType = TX_SID_FIRST;
         s->sid_update_counter = 3;
      }
      else {
         /* TX_SID_UPDATE or TX_NO_DATA */
         if ( ( s->sid_handover_debt > 0 ) && ( s->sid_update_counter > 2 ) ) {
              /*
               * ensure extra updates are properly delayed after
               * a possible SID_FIRST
               */
            txFrameType = TX_SID_UPDATE;
            s->sid_handover_debt--;
         }
         else {
            if ( s->sid_update_counter == 0 ) {
               txFrameType = TX_SID_UPDATE;
               s->sid_update_counter = 8;
            }
            else {
               txFrameType = TX_NO_DATA;
               used_mode = 15;
            }
         }
      }
   }
   else {
      s->sid_update_counter = 8;
      txFrameType = TX_SPEECH_GOOD;
   }
   s->prev_ft = txFrameType;

   if ( noHoming == 0 ) {
      Speech_Encode_Frame_reset( s->encoderState, s->dtx );
      Sid_Sync_reset( s );
   }

#ifndef ETSI
#ifdef IF2
   return Encoder3GPP( used_mode, prm, serial, txFrameType, mode );

#else
   return EncoderMMS( used_mode, prm, serial, txFrameType, mode );

#endif
#else

   Prm2Bits( used_mode, prm, &serial[1] );
   serial[0] = ( Word16 )txFrameType;
   serial[245] = ( Word16 )mode;
   return 500;
#endif

}


/*
 * Encoder_Interface_init
 *
 *
 * Parameters:
 *    dtx               I: DTX flag
 *
 * Function:
 *    Allocates state memory and initializes state memory
 *
 * Returns:
 *    pointer to encoder interface structure
 */
void * Encoder_Interface_init( int dtx )
{
   enc_interface_State * s;

   /* allocate memory */
   if ( ( s = ( enc_interface_State * ) malloc( sizeof( enc_interface_State ) ) ) ==
         NULL ) {
      fprintf( stderr, "Encoder_Interface_init: "
            "can not malloc state structure\n" );
      return NULL;
   }
   s->encoderState = Speech_Encode_Frame_init( dtx );
   Sid_Sync_reset( s );
   s->dtx = dtx;
   return s;
}


/*
 * DecoderInterfaceExit
 *
 *
 * Parameters:
 *    state             I: state structure
 *
 * Function:
 *    The memory used for state memory is freed
 *
 * Returns:
 *    Void
 */
void Encoder_Interface_exit( void *state )
{
   enc_interface_State * s;
   s = ( enc_interface_State * )state;

   /* free memory */
   Speech_Encode_Frame_exit( &s->encoderState );
   free( s );
   state = NULL;
}
