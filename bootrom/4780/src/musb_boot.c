/*	
 * musb_boot.c
 *	
 * Internal BootROM c-code for JZ4780.
 * Supports booting USB device.
 *
 * Copyright (C) 2005 - 2012 Ingenic Semiconductor Inc.
 *
 * Authors: twxie <twxie@ingenic.cn>
 *
 *  boot_sel[2:0]
 *
 * 	001	   USB boot  @ USB 2.0 device
 *
 * Revision history:
 *  - 2012/03/21: Init version from jz4780 usb_boot. <Twxie>
 *  - 2012/04/07: Cut ch9.h included and adjust usb_boot.h. <Twxie>
 *  - 2012/04/14: Adjust code style for usb_boot. <Twxie>
 *  - 2012/06/07: Adjust timeout achieve. <Twxie>
 *  - 2012/06/12: Adjust code of cpm . <Twxie>
 *  - 2012/06/18: Insert extern flush_dcache_all, delete verbose.
 *                Fix a bug. Add set UTMI width is 8 bits <Twxie>
 */

#include <irom.h>
#include <common.h>
#include <regs.h>
#include <jz4780misc.h>
#include <jz4780cpm.h>
#include <jz4780tcu.h>
#include <jz4780intc.h>
#include <jz4780gpio.h>
#include <jz4780otg_musb.h>
#include <jz4780otp.h>
#include <usb_boot.h>
#include <pmon.h>

#ifdef SERIAL_DEBUG
#define DBG(s)		serial_puts(s)
#define DBG_HEX(a)	serial_put_hex(a)

#define DBG_DEC(a)	serial_put_dec(a)
#else 
#define DBG(s) 	
#define DBG_HEX(a) 	
#define DBG_DEC(a) 	
#endif

/*-------------------------------------------------------------------------*/
extern void div_clock(void);
extern void flush_dcache_all(void);

static void write_fifo (u32 fifo_addr, u32 src, u16 len)
{
	u32 len_word = len / 4;
	u32 len_byte = len % 4;

	while (len_word-- > 0) {
		REG32(fifo_addr) = REG32(src);
		src +=4;
	}
	
	while (len_byte-- >0) 
		REG8(fifo_addr) = REG8(src++);
	return;
}

static void read_fifo (u32 fifo_addr, u32 dest, u16 len)
{
	u32 len_word = len / 4;
	u32 len_byte = len % 4;

	while (len_word-- > 0) {
		REG32(dest) = REG32(fifo_addr);
		dest +=4;
	}
	
	while (len_byte-- >0) 
		REG8(dest++) = REG8(fifo_addr);
	return;
}

static void handle_epin1_intr(USB_STATUS * status)
{
	u16 length;
	
	DBG("handle_epin1_intr. \n");
	
	REG8(USB_REG_INDEX) = 1;

	if (REG8(USB_REG_INCSR) & USB_INCSR_FFNOTEMPT)
		return;

	length = REG16(USB_REG_INMAXP);
	if (status->length < length) {
		__asm__ volatile("nop\n\t");
		length = status->length;
	}

	if (length) {
		status->length -= length;
		write_fifo(USB_FIFO_EP1, (u32)status->addr, length);
		REG8(USB_REG_INCSR) = USB_INCSR_INPKTRDY;
	}
	
	return;
}

static void handle_epout1_intr(USB_STATUS * status)
{
	DBG("Handle_epout1_intr. \n");
	
	REG8(USB_REG_INDEX) = 1;
	
	if (!(REG8(USB_REG_OUTCSR) & USB_OUTCSR_OUTPKTRDY))
		return;

	read_fifo(USB_FIFO_EP1, (u32)status->addr, REG16(USB_REG_OUTCOUNT));
	
	REG8(USB_REG_OUTCSR) = 0;
	
	return;
}

static void ep0_tx_state(USB_STATUS *status)
{
	u16 record, length;
	
	if (status->length > 64)
		length = 64;				// max packetsize
	else
		length = status->length;
	
	record = length;
	if (length) {
		status->length -= length;
		write_fifo(USB_FIFO_EP0, (u32)status->addr, length);
	
		if (record == 64) {
			REG8(USB_REG_INCSR) = USB_CSR0_INPKTRDY;
			return;
		}
	}

	REG8(USB_REG_INCSR) = USB_CSR0_INPKTRDY | USB_CSR0_DATAEND;
	status->data_tran_flag = 0;
	return;
}

static int handle_request(USB_STATUS * status)
{
	u32 i, no_data_stage = 1;
	u32 addr;
	u32 word1, word2;
	
	word1 = REG32(USB_FIFO_EP0);			// first word of setup packet
	word2 = REG32(USB_FIFO_EP0);			// second word of setup packet
	
	if (word1 & 0x60) {
		/* vendor_request handle */
		DBG("Vendor_request : ");
		
		status->data_tran_flag = 0;

		switch((word1 >> 8) & 0xff) {
		case EP0_GET_CPU_INFO:
			DBG("EP0_GET_CPU_INFO \n");
			status->addr = cpu_info_data;
			status->length = 8;
			status->data_tran_flag = 1;
			no_data_stage = 0;
			break;
			
		case EP0_SET_DATA_ADDRESS:
			status->addr = (u8 *)((word1 & 0xffff0000) | (word2 & 0xffff));
			DBG("EP0_SET_DATA_ADDRESS. Data Addr = 0x");
			DBG_HEX((u32)status->addr);
			break;
			
		case EP0_SET_DATA_LENGTH:
			status->length = (word1 & 0xffff0000) | (word2 & 0xffff);
			REG8(USB_REG_CSR0) = USB_CSR0_SVDOUTPKTRDY | USB_CSR0_DATAEND;
			DBG("EP0_SET_DATA_LENGTH. Data length = ");
			DBG_DEC(status->length);
			return -GO_HANDLE_EPIN1_INTR;
			
		case EP0_FLUSH_CACHES:
			DBG("EP0_FLUSH_CACHE \n");
			flush_dcache_all();
			break;
			
		case EP0_PROG_STAGE1:
			REG8(USB_REG_CSR0) = USB_CSR0_SVDOUTPKTRDY|USB_CSR0_DATAEND;
			addr = (word1 & 0xffff0000) | (word2 & 0xffff);
			DBG("EP0_PROG_START1. Start addr = 0x");
			DBG_HEX(addr);
			return xfer_d2i(addr, 0x4000);
			
		case EP0_PROG_STAGE2:
			REG8(USB_REG_CSR0) = USB_CSR0_SVDOUTPKTRDY|USB_CSR0_DATAEND;
			addr = ((word1 & 0xffff0000)|(word2 & 0xffff));
			DBG("EP0_PROG_START2. Start addr = 0x");
			DBG_HEX(addr);

			{
				void (*func_usb)(unsigned int);
				func_usb = (void (*)(unsigned int)) addr;
				(*func_usb) (addr);
			}

			return -GO_USB_BOOT_RETURN;
			
		default:
			DBG("Not contain.\n");
			break;
		}
	} else {
		/* standard_request handle */
		DBG("Standard_request : ");

		switch((word1 >> 8) & 0xff) {
		case USB_REQ_SET_ADDRESS:
			status->addr_num = word1 >> 16;
			DBG("SET_ADDRESS. Address = ");
			DBG_DEC(status->addr_num);
			break;
			
		case USB_REQ_SET_CONFIGURATION:
			DBG("SET_CONFIGURATION. Configuration = ");
			DBG_DEC(word1 >> 16);
			break;
			
		case USB_REQ_GET_DESCRIPTOR:
			DBG("GET_DESCRIPTOR - ");

			no_data_stage = 0;
			status->data_tran_flag = 1;
			status->length = word2 >> 16;
			
			switch(word1 >> 24) {
			case USB_DT_DEVICE:
				DBG("DEVICE. \n");
				status->addr = device_desc;
				if (status->length > USB_DT_DEVICE_SIZE)
					status->length = USB_DT_DEVICE_SIZE;// max length of device_desc
				break;
				
			case USB_DT_CONFIG:
				DBG("CONFIG. \n");
				status->addr = fs_desc;
				if (REG8(USB_REG_POWER) & USB_POWER_HSMODE)
					status->addr = hs_desc;
				if (status->length > 32)
					status->length = 32;// max length of device_desc
				break;
				
			case USB_DT_STRING:
				DBG("STRING. \n");
				i = (word1 >>16) & 0xff;
				
				if (i == 1) {
					status->addr = string_manufacture;
					status->length = 16;
				} else if (i == 2) {
					status->addr = string_product;
					status->length = 46;
				} else {
					status->addr = string_lang_ids;
					status->length = 4;
				}
				break;
				
			case USB_DT_DEVICE_QUALIFIER:
				DBG("DEVICE_QUALIFIER. \n");
				status->addr = dev_qualifier;
				if (status->length > 10)
					status->length = 10;// max length of device_desc
				break;
				
			default:
				DBG("Not contain.\n");
				status->data_tran_flag = 0;
				break;
			}
			break;
			
		default:
			DBG("Not contain.\n");
			break; 
		}
	}

	if (no_data_stage)
		REG8(USB_REG_CSR0) = USB_CSR0_SVDOUTPKTRDY|USB_CSR0_DATAEND;
	else 
		REG8(USB_REG_CSR0) = USB_CSR0_SVDOUTPKTRDY;
	
	return 0;
}	/* handle_request */

static int handle_epin0_intr(USB_STATUS *status)
{
	u8 csr0;
	int ret;
	
	DBG("Handle_epin0_intr. \n");

	REG8(USB_REG_INDEX) = 0;

	csr0 = REG8(USB_REG_CSR0);	

	if (csr0 & USB_CSR0_SENTSTALL) {
		REG8(USB_REG_CSR0) = csr0 & ~(USB_CSR0_SENTSTALL | USB_CSR0_SENDSTALL);
		status->data_tran_flag = 0;
	}
	
	if (csr0 & USB_CSR0_SETUPEND) {
		REG8(USB_REG_CSR0) = csr0 | USB_CSR0_SVDSETUPEND;
		status->data_tran_flag = 0;
	}
	
	if (!status->data_tran_flag) {
		if (status->addr_num) {
			REG8(USB_REG_FADDR) = status->addr_num;
			status->addr_num = 0;
		}
		
		if (csr0 & USB_CSR0_OUTPKTRDY) {
			ret = handle_request(status);
			if (ret)
				return ret;
		} else
			return 0;
	}
	
	if (status->data_tran_flag)
		ep0_tx_state(status);
	return 0;
}	/* handle_epin0_intr */

static void handle_reset_intr(int *flag)
{
	u16 i = 0;

	DBG("Handle_reset_intr called \n");

	*flag = 1;

	REG8(USB_REG_FADDR) = 0;

	if (REG8(USB_REG_POWER) & USB_POWER_HSMODE)
		i = 512;
	else
		i = 64;
	
	REG16(USB_REG_INDEX) = 1;
	
	/* set max pkt size */
	REG16(USB_REG_INMAXP) = i;
	REG8(USB_REG_INCSRH) = 0;
	REG16(USB_REG_OUTMAXP) = i;
	REG8(USB_REG_OUTCSRH) = 0;
	
	/* usb_flush_fifo */
	REG8(USB_REG_INCSR) = USB_INCSR_CDT|USB_INCSR_FF;
	REG8(USB_REG_OUTCSR) = USB_OUTCSR_CDT|USB_OUTCSR_FF;
	
	return;
}	/* handle_reset_intr */

/*-------------------------------------------------------------------------*/
static void musb_otg_cpm_init(void)
{
	/* enable mentor otg controller */
	REG_CPM_USBPCR1 &= ~(1 << 28);

	/* DP & DM pull enable */
	REG_CPM_USBPCR1 |= (0x3 << 22);

	/* set UTMI bus width is 8bit */
	REG_CPM_USBPCR1 &= ~(1 << 18);

	/* set iddig jitter filter time */
	REG32(CPM_USBVBFIL) = 0x80;

	/* set usb detect time */
	REG32(CPM_USBRDT) = 0x96;

	REG32(CPM_USBRDT) |= USBRDT_VBFIL_LD_EN;

	/* set TXVREFTUNE TXHSXVTUNE default */
	REG32(CPM_USBPCR) &= ~0x3f;
	REG32(CPM_USBPCR) |= 0x35;

	/* set work as otg model */
	REG32(CPM_USBPCR) &= ~USBPCR_USB_MODE;
	REG32(CPM_USBPCR) |= USBPCR_VBUSVLDEXT;		// Let UDC PHY to enable pullup of D+ by set bit 24 of CPM_USBPCR
	
	/* otg PHY power on reset */
	REG32(CPM_USBPCR) |= USBPCR_POR;
	xudelay(20);					// Keep reset state for about 20us(10us is enough) assuming CCLK=192MHz
	REG32(CPM_USBPCR) &= ~USBPCR_POR;
	xmdelay(1);					// Delay about 1ms(300us is enough) assuming CCLK=192MHz to wait UDC PHY to be stable
	
	return;
}

#define __musb_regs_init() 				\
do{							\
	REG8(USB_REG_INTRUSBE) = 0; /* disable common USB interrupts */	\
	REG16(USB_REG_INTRINE) = 0; /* disable EPIN interrutps */	\
	REG16(USB_REG_INTROUTE) = 0;/* disable EPOUT interrutps */	\
	REG8(USB_REG_POWER)=USB_POWER_HSENAB|USB_POWER_SOFTCONN;	\
}while(0)

/*-------------------------------------------------------------------------*/

int musb_boot(void)
{
	u16 intr_in;
	volatile int ret;
	int usb_reset_flag = 0;
	USB_STATUS usb_status = {NULL, 0, 0, 0};

	DBG("musb_boot. \n");			

usb_boot_loop:
	DBG("musb_boot_loop. \n");
	
	/* init */
	div_clock();
	
	musb_otg_cpm_init();
	
	__cpm_enable_otg_phy();

	__musb_regs_init();
	
	xtimeout(10*1000*1000, 1);
	
	/* Main loop of polling the usb commands */
	while (1) {

		if (!(usb_reset_flag)) {
			if ( !xtimeout(10*1000*1000, 0)) {
				DBG("*! USB idle more than 10s !*\n\n");
				xmdelay(10);
				return -GO_RESTART_BOOT;
			}
		}
		
		/* reset interrupt handle */
		if (REG8(USB_REG_INTRUSB) & USB_INTR_RESET) {
			handle_reset_intr(&usb_reset_flag);
		}
		
		intr_in = REG16(USB_REG_INTRIN);
		
		/* Endpoint0 handle */
		ret = 0;
		if (intr_in & BIT0) {
			ret = handle_epin0_intr(&usb_status);
			if (ret == -GO_USB_BOOT_RETURN)
				goto usb_boot_loop;
		}
		
		/* Endpoint1 handle */
		if ( (intr_in & BIT1) || (ret == -GO_HANDLE_EPIN1_INTR) )
			handle_epin1_intr(&usb_status);

		if (REG16(USB_REG_INTROUT) & BIT1)
			handle_epout1_intr(&usb_status);
	}
	
	return 0;
}	/* usb_boot end */

