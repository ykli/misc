
#ifndef MIPSOP_H
#define MIPSOP_H

/*
 * Macros to access the system control coprocessor
 */

#define __read_32bit_c0_register(source, sel)				\
({ volatile int __res;							\
	if (sel == 0)							\
		__asm__ __volatile__(					\
			"mfc0\t%0, " #source "\n\t"			\
			: "=r" (__res));				\
	else								\
		__asm__ __volatile__(					\
			".set\tmips32\n\t"				\
			"mfc0\t%0, " #source ", " #sel "\n\t"		\
			".set\tmips0\n\t"				\
			: "=r" (__res));				\
	__res;								\
})

#define __write_32bit_c0_register(register, sel, value)			\
do {									\
	if (sel == 0)							\
		__asm__ __volatile__(					\
			"mtc0\t%z0, " #register "\n\t"			\
			: : "Jr" ((unsigned int)(value)));		\
	else								\
		__asm__ __volatile__(					\
			".set\tmips32\n\t"				\
			"mtc0\t%z0, " #register ", " #sel "\n\t"	\
			".set\tmips0"					\
			: : "Jr" ((unsigned int)(value)));		\
} while (0)

#define read_c0_index()		__read_32bit_c0_register($0, 0)
#define write_c0_index(val)	__write_32bit_c0_register($0, 0, val)

#define read_c0_random()	__read_32bit_c0_register($1, 0)
#define write_c0_random(val)	__write_32bit_c0_register($1, 0, val)

#define read_c0_entrylo0()	__read_32bit_c0_register($2, 0)
#define write_c0_entrylo0(val)	__write_32bit_c0_register($2, 0, val)

#define read_c0_entrylo1()	__read_32bit_c0_register($3, 0)
#define write_c0_entrylo1(val)	__write_32bit_c0_register($3, 0, val)

#define read_c0_context()	__read_32bit_c0_register($4, 0)
#define write_c0_context(val)	__write_32bit_c0_register($4, 0, val)

#define read_c0_userlocal()	__read_32bit_c0_register($4, 2)
#define write_c0_userlocal(val)	__write_32bit_c0_register($4, 2, val)

#define read_c0_pagemask()	__read_32bit_c0_register($5, 0)
#define write_c0_pagemask(val)	__write_32bit_c0_register($5, 0, val)

#define read_c0_pagegrain()	__read_32bit_c0_register($5, 1)
#define write_c0_pagegrain(val)	__write_32bit_c0_register($5, 1, val)

#define read_c0_wired()		__read_32bit_c0_register($6, 0)
#define write_c0_wired(val)	__write_32bit_c0_register($6, 0, val)

#define read_c0_info()		__read_32bit_c0_register($7, 0)

#define read_c0_cache()		__read_32bit_c0_register($7, 0)	/* TX39xx */
#define write_c0_cache(val)	__write_32bit_c0_register($7, 0, val)

#define read_c0_badvaddr()	__read_32bit_c0_register($8, 0)
#define write_c0_badvaddr(val)	__write_32bit_c0_register($8, 0, val)

#define read_c0_count()		__read_32bit_c0_register($9, 0)
#define write_c0_count(val)	__write_32bit_c0_register($9, 0, val)

#define read_c0_entryhi()	__read_32bit_c0_register($10, 0)
#define write_c0_entryhi(val)	__write_32bit_c0_register($10, 0, val)

#define read_c0_status()	__read_32bit_c0_register($12, 0)
#define write_c0_status(val)	__write_32bit_c0_register($12, 0, val)

#define read_c0_cause()		__read_32bit_c0_register($13, 0)
#define write_c0_cause(val)	__write_32bit_c0_register($13, 0, val)

#define read_c0_epc()		__read_32bit_c0_register($14, 0)
#define write_c0_epc(val)	__write_32bit_c0_register($14, 0, val)

#define read_c0_prid()		__read_32bit_c0_register($15, 0)



#define read_c0_intctl()	__read_32bit_c0_register($12, 1)
#define write_c0_intctl(val)	__write_32bit_c0_register($12, 1, val)

#define read_c0_ebase()		__read_32bit_c0_register($15, 1)
#define write_c0_ebase(val)	__write_32bit_c0_register($15, 1, val)

#endif
