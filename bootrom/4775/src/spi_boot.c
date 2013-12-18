/*
 * spi_boot.c
 *
 * Internal BootROM c-code for JZ4775.
 * Supports booting spi nor flash.
 *
 * Copyright (C) 2005 - 2012 Ingenic Semiconductor Inc.
 *
 * Authors: twxie <twxie@ingenic.cn>
 *
 *  boot_sel[2:0]
 *
 *	  000	SPI Nor Flash boot
 *
 * Revision history:
 *  - 2012/03/21: Init version from jz4775 spi_boot. <Twxie>
 *  - 2012/04/11: Adjust copy data length, and add check sum handle. <Twxie>
 *  - 2012/04/14: Change spi pin to PA 18/20/21/23. Code style adjust. <Twxie>
 *  - 2012/04/28: Adjust some explanation. <Twxie>
 *  - 2012/05/18: Add macro.
 *                Add ssi_get_word function.
 *                Add P0 in sleep mode support.
 *                Fix volatile variable bug.
 *                Adjust copy data process.
 *                Adjust spl program check function. <Twxie>
 *  - 2012/05/31: Fix fault handle and bug.
 *                Adjust check function and spi boot flag. <Twxie>
 *  - 2012/06/05: Remove debug code. 
 *                Fix P0 in sleep mode support.
 *                Adjust timeout achieve. <Twxie>
 */

#include <irom.h>
#include <jz4775misc.h>
#include <jz4775ssi.h>
#include <jz4775gpio.h>
#include <jz4775cpm.h>
#include <common.h>

#ifdef SERIAL_DEBUG
#define DBG(s)		serial_puts(s)
#define DBG_HEX(a)	serial_put_hex(a)
#define DBG_DEC(a)	serial_put_dec(a)
#else 
#define DBG(s) 	
#define DBG_HEX(a) 	
#define DBG_DEC(a) 	
#endif

#define CMD_WREN 	0x06	/* Write Enable */
#define CMD_WRDI 	0x04	/* Write Disable */
#define CMD_RDSR 	0x05	/* Read Status Register */
#define CMD_WRSR 	0x01	/* Write Status Register */
#define CMD_READ 	0x03	/* Read Data */
#define CMD_FASH_READ 	0x0B	/* Read Data at high speed */
#define CMD_PP 		0x02	/* Page Program(write data) */
#define CMD_SE 		0xD8	/* Sector Erase */
#define CMD_BE 		0xC7	/* Bulk or Chip Erase */
#define CMD_DP 		0xB9	/* Deep Power-Down */
#define CMD_RES 	0xAB	/* Release from Power-Down and Read Electronic Signature */
#define CMD_RDID	0x9F	/* Read Identification */

#define SPI_TXRX_BYTE_WAIT_TIME (20 * 1)

extern u32 start_addr;
extern int cpustate;
extern int sd_boot(int);

#define __disable_ssi()					\
do{							\
	REG_GPIO_PXPENC(0) = BIT18|BIT19|BIT20|BIT23;/* enable the pulling of SSI pins */\
	__ssi_disable(0);				\
}while (0)

#define __enable_ssi()					\
do{							\
	__ssi_enable(0);				\
	REG_GPIO_PXPENS(0) = BIT18|BIT19|BIT20|BIT23;/* disable the pulling of SSI pins */\
}while (0)

#define __ssi_boot_fault()	\
do{				\
	__disable_ssi();	\
	return sd_boot(1);	\
}while (0)

#define __ssi_flush_txrxfifo()		\
do{					\
	__ssi_flush_fifo(0);		\
	while(!__ssi_rxfifo_empty(0));	\
}while (0)

#define __ssi_reg_init()			\
do{						\
	if ((cpustate & RSR_WR) || (cpustate & RSR_PR) )\
		REG_SSI_GR(0) = 11; /* 24MHz / (1MHz * 2) - 1 */ \
						\
	REG_SSI_CR1(0) = SSI_CR1_TFVCK_3 |	\
			SSI_CR1_TCKFI_3 |	\
			SSI_CR1_FMAT_SPI |	\
			SSI_CR1_FLEN_8BIT;	\
	REG_SSI_CR0(0) = SSI_CR0_EACLRUN |	\
			SSI_CR0_TFLUSH |	\
			SSI_CR0_RFLUSH;		\
}while (0)

static unsigned char ssi_get_byte(void)
{
	REG_SSI_DR(0) = 0;

	xtimeout(SPI_TXRX_BYTE_WAIT_TIME , 1);
	while (__ssi_rxfifo_empty(0)) {
		if ( !xtimeout(SPI_TXRX_BYTE_WAIT_TIME, 0) ) {
			DBG("ssi get a byte timeout.\n");
			__ssi_boot_fault();
		}
	}

	return REG_SSI_DR(0);
}

static unsigned int ssi_get_word(void)
{
	volatile int i;
	unsigned int word = 0;
	unsigned char *p = (unsigned char *)&word;
	for (i = 3; i >= 0 ; i--) {
		*(p++) = ssi_get_byte();
	}
	
	return word;
}

#if 0
void debug_read(void)
{
	int i;
	volatile unsigned int addr = 0xf4000800;
	unsigned int t;

	for (i = 0; i < 32; i++) {
		t = *(volatile unsigned int *)(addr + i * 4);
	}
}
#endif

int spi_boot(void)
{
	int i;
	unsigned int length, check_zero;
	unsigned char ssi_gr, num_addr;
	volatile unsigned char *addr;
	
	DBG("spi_boot. \n");

#if 0	/* test gpio */
	/*
	 * _|-------_|-------_|-------_		~ clk
	 * _|_-----__|_-----__|_-----__		~ dr
	 * _|__---___|__---___|__---___		~ dt
	 * _|___-____|___-____|___-____		~ ce0
	 */
	while (1) {
		__gpio_port_as_output1(0, 18);	//clk
		__gpio_port_as_output1(0, 20);	//dr
		__gpio_port_as_output1(0, 21);	//dt
		__gpio_port_as_output1(0, 23);	//ce0

		__gpio_port_as_output0(0, 23);	//ce0
		__gpio_port_as_output0(0, 21);	//dt
		__gpio_port_as_output0(0, 20);	//dr
		__gpio_port_as_output0(0, 18);	//clk
	}
#endif
	/* Setup GPIO pins: SSI0_CE0, SSI0_CLK, SSI0_DT, SSI0_DR */
	__gpio_as_spi_boot();

	/* Configure SSI controller */
	__disable_ssi();
	__ssi_reg_init();
	__enable_ssi();

	/* Manually set SSI Control Register1 UNFIN bit */
	REG_SSI_CR1(0) |= SSI_CR1_UNFIN ;

	//--------------------------------------------------------------
	// Read the first 4 words in spi nor to get the information of it,
	// and change SSI clock according to the value of SSI_GR in the 7th
	// byte. If there is no boot codes in SPI NOR, jump to sd boot(MSC1).
	// Spl code length in the third word. Check sum in the 4th word.
	//
	// The content of the 4 words:
	//	  | 0x4 | 0x3 | 0x2 | 0x55 | 0xaa | 0x55 | 0xaa | SSI_GR |
	// index     0     1     2      3      4      5      6      7
	//	  | len(LSB) | len | len | len(MSB) | check(LSB) | check | check | check(MSB) |
	// index       8        9     a        b          c          d       e         f 
	//
	// The first byte read from spi nor is the number of address
	// bytes, which will be 4, 3 or 2, and it will be stored to num_addr.
	//--------------------------------------------------------------
	/* firstly, read it assuming it's a 4-bytes-address spi nor */
	REG_SSI_DR(0) = CMD_READ;			//read command
	REG_SSI_DR(0) = 0x00;				//4 bytes address
	REG_SSI_DR(0) = 0x00;
	REG_SSI_DR(0) = 0x00;
	REG_SSI_DR(0) = 0x00;
	
	/* timeout handle : wait receive 5byte */
	xtimeout( (5*SPI_TXRX_BYTE_WAIT_TIME) , 1);
	while (__ssi_get_rxfifo_count(0) < 5) {
		if ( !xtimeout( (5*SPI_TXRX_BYTE_WAIT_TIME) , 0) ) {
			DBG("timeout at read 5 bytes. \n");
			__ssi_boot_fault();
		}
	}

	__ssi_flush_txrxfifo();				// clean rx fifo

	num_addr = ssi_get_byte();			// get nor flash address num

	/* check SPI nor flash boot flag */ 
	if (num_addr == 0x4) {
		ssi_get_byte();
		ssi_get_byte();
	} else if (num_addr == 0x3) {
		ssi_get_byte();
	} else if (num_addr != 0x2) {
		DBG("spi number of address fales. \n");	
		__ssi_boot_fault();
	}

	if ( !((ssi_get_byte() == 0x55) && (ssi_get_byte() == 0xaa) &&
	     (ssi_get_byte() == 0x55) && (ssi_get_byte() == 0xaa))) {	
		DBG("spi nor flash boot flag invalid. \n");
		__ssi_boot_fault();
	}

	/* get ssi clock generator value */
	ssi_gr = ssi_get_byte();
	
	/* get spl length */
	length = ssi_get_word();
	if (length > 14 * 1024) {
		DBG("Length of Load data is too much.\n");
		length = 0x3800;			// force set 14K bytes
	}

	/* get check_zero */
	check_zero = ssi_get_word();

	/* CPU state is Power ON, need to reset ssi clock */
	__disable_ssi();
	if ((cpustate & RSR_WR) || (cpustate & RSR_PR) )
		REG_SSI_GR(0) = ssi_gr;
	__enable_ssi();
	
	/* copy lengths bytes to target addr */
	REG_SSI_DR(0) = CMD_READ;			// send addr
	REG_SSI_DR(0) = 0x00;
	REG_SSI_DR(0) = 0x00;
	if (num_addr == 0x4) {
		REG_SSI_DR(0) = 0x00;
		REG_SSI_DR(0) = 0x00;
	} else if (num_addr == 0x3) {
		REG_SSI_DR(0) = 0x00;
	}

	xtimeout( (num_addr + 1)*SPI_TXRX_BYTE_WAIT_TIME , 1);// timeout prepare
	while(__ssi_get_rxfifo_count(0) != (num_addr + 1)) {
		if ( !xtimeout( ((num_addr +1)*SPI_TXRX_BYTE_WAIT_TIME) , 0) ) {
			DBG("timeout at data address. \n");
			__ssi_boot_fault();
		}
	}

	__ssi_flush_txrxfifo();				// clean rx fifo

	for (i = 0; i < 128; ++i)			// prepare get data
		REG_SSI_DR(0) = 0x0;

	addr = (unsigned char *)start_addr;

	for (i = 0; i < length; ++i) { 
		while(__ssi_rxfifo_empty(0));

		*(addr++) = REG_SSI_DR(0);
		REG_SSI_DR(0) = 0x00;
	}

	/* check handle */
	addr = (unsigned char *)start_addr + 16;
	length = (length - 16) / 4;
	for (i = 0; i < length; i++) {
		check_zero += *((volatile unsigned int *)addr);
		addr += 4;
	}

	if (check_zero)
		__ssi_boot_fault();
	
	__disable_ssi();
	
	/* just jump to start_addr +16 run. */
	return xfer_d2i(start_addr + 16, 0);
}


