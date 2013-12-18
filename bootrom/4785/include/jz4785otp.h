/*
 * linux/include/asm-mips/mach-jz4780/jz4780msc.h
 *
 * JZ4780 MSC register definition.
 *
 * Copyright (C) 2010 Ingenic Semiconductor Co., Ltd.
 */

#ifndef __JZ4780OTP_H__
#define __JZ4780OTP_H__

#define OTP_CTRL       0xB3540000
#define OTP_CFG        0xB3540004
#define OTP_STATE      0xB3540008
#define OTP_DATA_BASE  0xB354000C

#define OTP_DATA(n)   (OTP_DATA_BASE + (n)*4)

#define REG_OTP_CTRL      REG32(OTP_CTRL)
#define REG_OTP_CFG       REG32(OTP_CFG)
#define REG_OTP_STATE     REG32(OTP_STATE)
#define REG_OTP_DATA(n)   REG32(OTP_DATA(n)) 

#define RD_ADJ 15
#define RD_STROBE 7
#define WR_ADJ 1
#define WR_STROBE  333

/* EFUSE Status Register  (OTP_STATE) */
#define OTP_STATE_USERKEY_PRT		(1 << 23) 
#define OTP_STATE_MD5_PRT  		(1 << 22) 
#define OTP_STATE_AUTO_DET_EN		(1 << 21)
#define OTP_STATE_PC_PRT        	(1 << 20)
#define OTP_STATE_TRIM3_PRT        	(1 << 19)
#define OTP_STATE_WDT_ENABLE        	(1 << 18)
#define OTP_STATE_TRIM1_PRT        	(1 << 17)
#define OTP_STATE_TRIM0_PRT        	(1 << 16)
#define OTP_STATE_CUSTID_PRT     	(1 << 15)
#define OTP_STATE_CHIPID_PRT     	(1 << 14)
#define OTP_STATE_CDCBOOT_PRT     	(1 << 13)
#define OTP_STATE_SECBOOT_PRT     	(1 << 12)
#define OTP_STATE_NEMCNFI_SEL     	(1 << 11)
#define OTP_STATE_DIS_JTAG     		(1 << 10)
#define OTP_STATE_AUTOBOOT_EN     	(1 << 9)
#define OTP_STATE_SECBOOT_EN     	(1 << 8)
#define OTP_STATE_WR_DONE		(1 << 1)
#define OTP_STATE_RD_DONE         	(1 << 0)

#endif /* __JZ4780MSC_H__ */

