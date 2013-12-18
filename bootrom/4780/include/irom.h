/*
 *	irom.h
 */

#ifndef __IROM_H__
#define __IROM_H__
#include "pmon.h"

/* Macro which used for code fixed by EFUSE */
#define EFUSE_FIXCODE_START 0xb3410228
#define MAX_WATCH_NUM   32 
#define MAX_SAVEREG_NUM 8

//============================================
#ifdef USE_TCSM_DEBUG
/*

   F400 0020:
   Code
   ReadOnly DATA
   DATA
   BSS

   F400 3000    <---- SP

   F400 3000 --- F400 4000   <---- SPL SPAC
   */
#define TCSM_DATA_START  0xF4000000
#define TCSM_DATA_MAXLEN 4
#define TCSM_STACK_TOP   0xF4003000
#define TCSM_BUFF_START  0xF4003000
#define TCSM_BUFF_LEN    0x1000

#else
/*
   BFC0 0000:
   Code
   ReadOnly DATA
   TCSM_COPY_DATA

   F400 0000:
   DATA (TCSM_COPY_DATA)
   BSS

   F400 2000    <---- SP

   F400 2000 --- F400 4000   <---- SPL SPAC
   */

#define TCSM_DATA_START  0xF4000000
#define TCSM_DATA_MAXLEN 512
#define TCSM_STACK_TOP   0xF4000800
#define TCSM_BUFF_START  0xF4000800
#define TCSM_BUFF_LEN    0x3800

#endif  /* USE_TCSM_DEBUG */

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
