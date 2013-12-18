/*
 * jz4780nand.h
 *
 * Authors:
 * 		<xlsu@ingenic.cn>
 * header of nand_boot.c
 */
#ifndef __NAND_BOOT_H__
#define __NAND_BOOT_H__

/* NAND Command */
#define NAND_CMD_READ0		0x00
#define NAND_CMD_READSTART      0x30
#define NAND_CMD_READID		0x90
#define NAND_CMD_RESET          0xFF

#define NAND_CMD_OFFSET		0x400000
#define NAND_ADDR_OFFSET	0x800000

#define NAND_DATA_PORT		NEMC_CS1
#define NAND_CMD_PORT		(NEMC_CS1 | NAND_CMD_OFFSET)
#define NAND_ADDR_PORT		(NEMC_CS1 | NAND_ADDR_OFFSET)

/* NAND Control */
#define SMCR_VALUE		0x3FFF7700
#define __nand_smcr_8bit()	(REG_NEMC_SMCR1 = SMCR_VALUE | NEMC_SMCR_BW_8)
#define __nand_smcr_16bit()	(REG_NEMC_SMCR1 = SMCR_VALUE | NEMC_SMCR_BW_16)
#define __tnand_tgcr_init()	(REG_NEMC_TGCR1 = 0x0000220A)

#define __nand_cmd(n)		(REG8(NAND_CMD_PORT) = (n))
#define __nand_addr(n)		(REG8(NAND_ADDR_PORT) = (n))
#define __nand_data_out8(p)	(*(unsigned char *)(p) = REG8(NAND_DATA_PORT))
#define __nand_data_out16(p)	(*(unsigned short *)(p) = REG16(NAND_DATA_PORT))

struct nand_info {
	u32 page_size;
	int bus_width;
	int row_cycle;
	int nand_type; /* 0: Common NAND; 1: Toggle NAND */
};

#endif /* __NAND_BOOT_H__ */
