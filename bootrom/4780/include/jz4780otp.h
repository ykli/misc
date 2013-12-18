/*
 * linux/include/asm-mips/mach-jz4780/jz4780msc.h
 *
 * JZ4780 MSC register definition.
 *
 * Copyright (C) 2010 Ingenic Semiconductor Co., Ltd.
 */

#ifndef __JZ4780OTP_H__
#define __JZ4780OTP_H__

#define OTP_CTRL       0xB34100D0
#define OTP_CFG        0xB34100D4
#define OTP_STATE      0xB34100D8
#define OTP_DATA_BASE  0xB34100DC

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
#define OTP_STATE_GLOBAL_PRT		(1 << 15) 
#define OTP_STATE_CHIPID_PRT  		(1 << 14) 
#define OTP_STATE_CUSTID_PRT		(1 << 13)
#define OTP_STATE_SECWR_EN		(1 << 12)
#define OTP_STATE_PC_PRT        	(1 << 11)
#define OTP_STATE_HDMIKEY_PRT     	(1 << 10)
#define OTP_STATE_SECKEY_PRT		(1 << 9)
#define OTP_STATE_SECBOOT_EN 		(1 << 8)
#define OTP_STATE_HDMI_BUSY		(1 << 2)
#define OTP_STATE_WR_DONE		(1 << 1)
#define OTP_STATE_RD_DONE         	(1 << 0)

#endif /* __JZ4780MSC_H__ */

