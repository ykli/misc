/*
 * Cache operations for the cache instruction.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * (C) Copyright 1996, 1997 by Ralf Baechle
 */
#ifndef	__ASM_MIPS_CACHE_H__
#define	__ASM_MIPS_CACHE_H__

#include <cachecfg.h>

#define cache_unroll(base,op)	        	\
	__asm__ __volatile__("	         	\
		.set noreorder;		        \
		.set mips3;		        \
		cache %1, (%0);	                \
		.set mips0;			\
		.set reorder"			\
		:				\
		: "r" (base),			\
		  "i" (op));

void flush_dcache_range(unsigned int start,unsigned int end);

/* cpu pipeline/write buffer flush */
static inline void jz_sync(void)
{
	__asm__ volatile ("sync");
}

#endif	/* __ASM_MIPS_CACHEOPS_H */
