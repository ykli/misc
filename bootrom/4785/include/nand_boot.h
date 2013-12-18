/*
 * nand_boot.h
 *
 * Authors:
 * 		<xlsu@ingenic.cn>
 * header of nand_boot.c
 */
#ifndef __NAND_BOOT_H__
#define __NAND_BOOT_H__

#define EFUSE_NAND_REGBIT		11
#define EFUSE_NAND_BOOTSEL()		(REG_OTP_STATE & OTP_STATE_NEMCNFI_SEL)
#define EFUSE_NAND_NFIBOOT		(0x0 << EFUSE_NAND_REGBIT)
#define EFUSE_NAND_NEMCBOOT		(0x1 << EFUSE_NAND_REGBIT)
#define BCHBIT_SWITCH_NEMC_NFI		3
#define BCH_SWITCH_NEMC_NFI		(REG_BCH_CR)
#define NAND_SWITH_NFI			1
#define NAND_SWITH_NEMC			0
#define NAND_CS1				0xBB000000 /* read-write area in static bank 1 */
#define NAND_CS2				0xBA000000 /* read-write area in static bank 2 */
#define NAND_CS3				0xB9000000 /* read-write area in static bank 3 */
#define NAND_CS4				0xB8000000 /* read-write area in static bank 4 */
#define NAND_CS5				0xB7000000 /* read-write area in static bank 5 */
#define NAND_CS6				0xB6000000 /* read-write area in static bank 6 */

/* NAND Command */
#define NAND_CMD_READ0			0x00
#define NAND_CMD_READSTART		0x30
#define NAND_CMD_READID			0x90
#define NAND_CMD_RESET			0xFF

#define NAND_CMD_OFFSET			0x400000
#define NAND_ADDR_OFFSET		0x800000

#define NAND_DATA_PORT			NAND_CS1
#define NAND_CMD_PORT			(NAND_CS1 | NAND_CMD_OFFSET)
#define NAND_ADDR_PORT			(NAND_CS1 | NAND_ADDR_OFFSET)

/* NEMC NAND Control */
#define __nand_smcr_8bit()		(REG_NEMC_SMCR1 = 0x0FFF7700)
#define __nand_smcr_16bit()		(REG_NEMC_SMCR1 = 0x0FFF7740)
#define __nand_nemc_cmd(n)		(REG8(NAND_CMD_PORT) = (n))
#define __nand_nemc_addr(n)		(REG8(NAND_ADDR_PORT) = (n))

/* NFI NAND Control */
#define __nand_nfi_cmd(n)		(REG32(NAND_CMD_PORT) = ((n) | 1 << 21 | 1 << 10))
#define __nand_nfi_addr(n)		(REG32(NAND_ADDR_PORT) = ((n) | 1 << 21 | 1<< 10))


/* public Control */
#define __nand_data_out8(n)		((n) = REG8(NAND_DATA_PORT))
#define __nand_data_out16(n)	((n) = REG16(NAND_DATA_PORT))

#define MAX_PAGESIZE			(16 * 1024) /* supported Maximum Page Size of NAND */
#define FLAG_SIZE				256

#define BACKUP_OFFSET			128
#define BACKUP_NUM				8
#define BACKUP_PAGE				(BACKUP_OFFSET * BACKUP_NUM)

#ifdef FPGA_TEST
/* ECC Level: 24 bit/256bytes */
#define ECC_SIZE		256
#define ECC_BYTES		42 /* 14 * ECC Level / 8 */
#define ECCBUF_SIZE		ECC_BYTES
#else
/* ECC Level: 64 bit/256bytes */
#define ECC_SIZE		256
#define ECC_BYTES		112 /* 14 * ECC Level / 8 */
#define ECCBUF_SIZE		ECC_BYTES
#endif

/* NAND Initial Value */
#define PAGESIZE_DEFVALUE	512
#define BUSWIDTH_DEFVALUE	8 /* 8 buswidth 8bit, 16 buswidth 16bit */
#define ROWCYCLE_DEFVALUE	3
#define NANDTYPE_DEFVALUE	0 /* Common NAND default */

#define NANDTYPE_COMMON		0
#define NANDTYPE_TOGGLE		1

#define UNCOR_ECC			0xFFFF
#define TOGGLE_DQS_DELAY	0x1F

#define BUSWIDTH_FLAG_OFFSET	0				/* [0 : 63] */
#define NANDTYPE_FLAG_OFFSET	(BUSWIDTH_FLAG_OFFSET + 64)	/* [64 : 127] */
#define ROWCYCLE_FLAG_OFFSET	(NANDTYPE_FLAG_OFFSET + 64)	/* [128 : 159] */
#define PAGESIZE_FLAG2_OFFSET	(ROWCYCLE_FLAG_OFFSET + 32)	/* [160 : 191] */
#define PAGESIZE_FLAG1_OFFSET	(ROWCYCLE_FLAG_OFFSET + 32 * 2)	/* [192 : 223] */
#define PAGESIZE_FLAG0_OFFSET	(ROWCYCLE_FLAG_OFFSET + 32 * 3)	/* [224 : 255] */

#define FLAG_BUSWIDTH_8BIT		0x55
#define FLAG_BUSWIDTH_16BIT		0xaa
#define FLAG_NANDTYPE_COMMON	0x55
#define FLAG_NANDTYPE_TOGGLE	0xaa
#define FLAG_ROWCYCLE_2			0x55
#define FLAG_ROWCYCLE_3			0xaa
#define FLAG_PAGESIZE_512		0x555555
#define FLAG_PAGESIZE_2K		0x55aa55
#define FLAG_PAGESIZE_4K		0xaa5555
#define FLAG_PAGESIZE_8K		0xaaaa55
#define FLAG_PAGESIZE_16K		0xaaaaaa
#define FLAG_NULL				0

#define USE_PN					1

#define FLAG_STORE_ARRAY		(0x800)

struct nand_info {
	u32 page_size;
	int bus_width;
	int row_cycle;
	int nand_type; /* 0: Common NAND; 1: Toggle NAND */
};

#endif /* __NAND_BOOT_H__ */
