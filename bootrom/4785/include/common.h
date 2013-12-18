/*
 *	common.h
 */

#ifndef __COMMON_H__
#define __COMMON_H__

extern unsigned int volatile jz_extal;


#ifndef __BOOT_ROM__
#define __BOOT_ROM__
#endif

#ifdef SERIAL_DEBUG
void serial_init(void);
void serial_puts(const char *);
void serial_put_dec(unsigned int);
void serial_put_hex(unsigned int);
#define DBG(s)		serial_puts(s)
#define DBG_HEX(a)	serial_put_hex(a)
#define DBG_DEC(a)	serial_put_dec(a)
#else
#define DBG(s)
#define DBG_HEX(a)
#define DBG_DEC(a)
#endif

int xfer_d2i(u32 jumb_addr, u32 length);

#define REG8(addr)	*((volatile unsigned char *)(addr))
#define REG16(addr)	*((volatile unsigned short *)(addr))
#define REG32(addr)	*((volatile unsigned int *)(addr))

#define INREG8(x)               ((unsigned char)(*(volatile unsigned char *)(x)))
#define OUTREG8(x, y)           *(volatile unsigned char *)(x) = (y)
#define SETREG8(x, y)           OUTREG8(x, INREG8(x)|(y))
#define CLRREG8(x, y)           OUTREG8(x, INREG8(x)&~(y))
#define CMSREG8(x, y, m)        OUTREG8(x, (INREG8(x)&~(m))|(y))

#define INREG16(x)              ((unsigned short)(*(volatile unsigned short *)(x)))
#define OUTREG16(x, y)          *(volatile unsigned short *)(x) = (y)
#define SETREG16(x, y)          OUTREG16(x, INREG16(x)|(y))
#define CLRREG16(x, y)          OUTREG16(x, INREG16(x)&~(y))
#define CMSREG16(x, y, m)       OUTREG16(x, (INREG16(x)&~(m))|(y))

#define INREG32(x)              ((unsigned int)(*(volatile unsigned int *)(x)))
#define OUTREG32(x, y)          *(volatile unsigned int *)(x) = (y)
#define SETREG32(x, y)          OUTREG32(x, INREG32(x)|(y))
#define CLRREG32(x, y)          OUTREG32(x, INREG32(x)&~(y))
#define CMSREG32(x, y, m)       OUTREG32(x, (INREG32(x)&~(m))|(y))

/*
 * Define the bit field macro to avoid the bit mistake
 */
#define BIT0            (1 << 0)
#define BIT1            (1 << 1)
#define BIT2            (1 << 2)
#define BIT3            (1 << 3)
#define BIT4            (1 << 4)
#define BIT5            (1 << 5)
#define BIT6            (1 << 6)
#define BIT7            (1 << 7)
#define BIT8            (1 << 8)
#define BIT9            (1 << 9)
#define BIT10           (1 << 10)
#define BIT11           (1 << 11)
#define BIT12 	        (1 << 12)
#define BIT13 	        (1 << 13)
#define BIT14 	        (1 << 14)
#define BIT15 	        (1 << 15)
#define BIT16 	        (1 << 16)
#define BIT17 	        (1 << 17)
#define BIT18 	        (1 << 18)
#define BIT19 	        (1 << 19)
#define BIT20 	        (1 << 20)
#define BIT21 	        (1 << 21)
#define BIT22 	        (1 << 22)
#define BIT23 	        (1 << 23)
#define BIT24 	        (1 << 24)
#define BIT25 	        (1 << 25)
#define BIT26 	        (1 << 26)
#define BIT27 	        (1 << 27)
#define BIT28 	        (1 << 28)
#define BIT29 	        (1 << 29)
#define BIT30 	        (1 << 30)
#define BIT31 	        (1 << 31)

#define NULL 0
#define false 0
#define true 1

void set_cpufreq(unsigned int cpufreq);

/* Generate the bit field mask from msb to lsb */
#define BITS_H2L(msb, lsb)  ((0xFFFFFFFF >> (32-((msb)-(lsb)+1))) << (lsb))

#define le16_to_cpu (x) ((((x)&0xff) << 8 |(((x) >> 8) & 0xff)))
#define cpu_to_le16(x)	le16_to_cpu(x);

/* Get the bit field value from the data which is read from the register */
#define get_bf_value(data, lsb, mask)  (((data) & (mask)) >> (lsb))
static inline void * memset (void *p, int c,int size)
{
	int i;
	for (i = 0; i < size; i++)
		((u8 *)p)[i] = c;

	return p;
}

static inline char* strcpy (char *dest,char *src)
{
	int i = 0;
	do {
		dest[i++] = *src;
	} while (*src++ != '\0');

	return dest;
}

static inline void* memcpy(void * dest,const void *src,int count)
{
	char *tmp = (char *) dest, *s = (char *) src;

	while (count--)
		*tmp++ = *s++;

	return dest;
}

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})


enum {
	APLL = 0,
	MPLL,
};

void notify_cpufreq_change(unsigned int speed);

#endif	/*__COMMON_H__*/
