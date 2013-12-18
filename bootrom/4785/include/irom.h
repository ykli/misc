/*
 *	irom.h
 */

#ifndef __IROM_H__
#define __IROM_H__
#include "pmon.h"

/* Macro which used for code fixed by EFUSE */
#define EFUSE_FIXCODE_START 0xb3540290
#define MAX_WATCH_NUM   32 
#define MAX_SAVEREG_NUM 8

//============================================
/*
   BFC0 0000:
   Code
   ReadOnly DATA
   CACHE_COPY_DATA

   8000 0000:
   DATA (COPY FROM CACHE_COPY_DATA)
   BSS

   8000 1000    <---- SP

   8000 1000 --- 8000 8000   <---- SPL SPAC
   */

#define DATA_START  0x80000000
#define STACK_TOP   0x80001000
#define START_ADDR  STACK_TOP
#define SPL_SIZE    (28*1024)

#ifndef __ASM__
typedef 		 char s8;
typedef unsigned char u8;

typedef 		 short s16;
typedef unsigned short u16;

typedef 		 int s32;
typedef unsigned int u32;

typedef u16 le16;

typedef unsigned long ulong;


#define GO_RESTART_BOOT			1
#define GO_USB_BOOT_RETURN		2
#define GO_HANDLE_EPIN1_INTR	3
#define CANCEL_USB_BOOT			4
extern int xudelay(int usec);

extern void xmdelay(int sdelay);
extern int xtimeout(int usec, int ret);

#endif
#endif	/* __IROM_H__ */
