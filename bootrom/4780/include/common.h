/*
 *	common.h
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#ifndef __BOOT_ROM__
#define __BOOT_ROM__
#endif /* __BOOT_ROM__ */

#ifdef DEBUG_IN_FPGA
extern void serial_init(void);
extern void serial_puts (const char *s);
extern void serial_put_hex(unsigned int d);
extern void serial_put_dec(unsigned int d);
#endif

extern int xfer_d2i(u32 jumb_addr, u32 length);
extern inline void jz_flush_dcache(void);
extern inline void jz_flush_icache(void);
extern inline void jz_flush_cache(void);
extern inline void jz_init_caches(void);

#endif
