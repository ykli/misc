/*
 * usb_boot.c
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
 *      110        USB boot  @ USB 2.0 device, EXTCLK=12MHz
 * 	001	   USB boot  @ USB 2.0 device, EXTCLK=26MHz
 * 	010	   USB boot  @ USB 2.0 device, EXTCLK=19.2MHz	
 *
 * Revision history:
 *  - 2012/03/21: Init version from jz4780 usb_boot. <Twxie>
 *  - 2012/04/07: Cut ch9.h included and adjust usb_boot.h. <Twxie>
 *  - 2012/04/14: Adjust code style for usb_boot. <Twxie>
 */

#include <irom.h>
#include <common.h>
#include <regs.h>
#include <jz4780misc.h>
#include <jz4780cpm.h>
#include <jz4780ost.h>
#include <jz4780tcu.h>
#include <jz4780intc.h>
#include <jz4780gpio.h>
#include <jz4780otg_dwc.h>
#include <jz4780otp.h>
#include <susb_boot.h>
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

#define K0BASE 0x80000000
#define CFG_DCACHE_SIZE         (32*1024)
#define CFG_ICACHE_SIZE         (32*1024)
#define CFG_CACHELINE_SIZE      32

#define Index_Writeback_Inv_D   0x01
#define Index_Store_Tag_I       0x08

extern void div_clock(void);

void flush_dcache_all(void)
{
        u32 addr;

        for (addr = K0BASE; addr < K0BASE + CFG_DCACHE_SIZE;
             addr += CFG_CACHELINE_SIZE) {
                asm volatile (
                        ".set mips3\n\t"
                        " cache %0, 0(%1)\n\t"
                        ".set mips2\n\t"
                        :
                        : "I" (Index_Writeback_Inv_D), "r"(addr));
        }

        asm volatile ("sync");
}

/*-------------------------------------------------------------------------*/
static void dwc_read_out_packet(USB_STATUS *status, u16 count)
{
	int i;
	int dwords = (count + 3) / 4;
	
	for (i = 0; i < dwords; i++)
		REG32((unsigned int *)(status->addr) + i) = REG_EP0_FIFO;
	status->addr += count;
}


static void dwc_otg_flush_rx_fifo(void)
{
	REG_GRST_CTL = RSTCTL_RXFIFO_FLUSH;

	while (REG_GRST_CTL & RSTCTL_RXFIFO_FLUSH);

	return;
}

static void dwc_otg_flush_tx_fifo(void)
{
	REG_GRST_CTL = RSTCTL_TXFNUM_ALL | RSTCTL_TXFIFO_FLUSH;

	while (REG_GRST_CTL & RSTCTL_TXFIFO_FLUSH);

	return;
}

static void disable_all_ep(void)
{
	int i;

	for (i = 0; i < DEP_NUM; i++) {
		REG_DIEP_CTL(i) |= (DEP_DISENA_BIT | DEP_SET_NAK);
		REG_DOEP_CTL(i) |= (DEP_DISENA_BIT | DEP_SET_NAK);
		
		REG_DIEP_SIZE(i) = 0;
		REG_DOEP_SIZE(i) = 0;

		REG_DIEP_INT(i) = 0xff;
		REG_DOEP_INT(i) = 0xff;
	}
}

static void dwc_otg_device_init(void)
{
	u16 epinfobase, gdfifocfg;

	REG_GRXFIFO_SIZE = DEP_RXFIFO_SIZE;
	REG_GNPTXFIFO_SIZE = (DEP_NPTXFIFO_SIZE << 16) | DEP_RXFIFO_SIZE;
	REG_GDTXFIFO_SIZE = (DEP_DTXFIFO_SIZE << 16) | (DEP_RXFIFO_SIZE + DEP_NPTXFIFO_SIZE);

	gdfifocfg = REG_GHW_CFG3 >> 16;
	epinfobase = (REG_GRXFIFO_SIZE & 0xffff) + (REG_GNPTXFIFO_SIZE >> 16); 
	REG_GDFIFO_CFG = (epinfobase << 16) | gdfifocfg;

	dwc_otg_flush_tx_fifo();
	dwc_otg_flush_rx_fifo();

	/* clear irq and mask all ep intr */
	REG_DOEP_MASK = 0;
	REG_DIEP_MASK = 0;
	REG_OTG_DAINT = 0xffffffff;
	REG_DAINT_MASK = 0;

	/* disable all in and out ep */
	disable_all_ep();

	REG_GINT_STS = 0xffffffff;

	REG_OTG_DCFG = 0;		// Slave mode and High speed to enum	
	REG_OTG_DCTL = 0;		// Soft Disconnect-> 0(normal mode)

	REG_GINT_MASK |= (1 << 4);
	
	return;
}

static int dwc_get_utmi_width(void)
{
	return (REG_GHW_CFG4 >> 14) & 0x3;
}
static void dwc_otg_core_reset(void)
{
	u32 greset = 0;
	u32 cnt = 0;
	do {
		xudelay(10);
		greset = REG_GRST_CTL;
		if (cnt++ > 100000) {
			DBG("GRESET wait IDLE timeout.\n");
			return;
		}
	} while ((greset & RSTCTL_AHB_IDLE) == 0);

	cnt = 0;

	REG_GRST_CTL = greset | RSTCTL_CORE_RST;
	do {
		greset = REG_GRST_CTL;
		if (cnt++ > 10000  ) {
			DBG("GRESET wait reset timeout.\n");
			return;
		}
		xudelay(10);
	} while (greset & RSTCTL_CORE_RST);

	/* wait for 3 phy clocks */
	xudelay(100);
		
}

static void dwc_otg_select_phy_width(void)
{
// fixme full speed use 16bit phy can use trdtime_6?
	if (((REG_GHW_CFG2 >> 6) & 0x3) == 1) {
		/* UTMI+ WIDTH choose diff trdtiming */
		if (dwc_get_utmi_width() == 0) {
			DBG("8BIT UTMI+.\n");
			REG_GUSB_CFG &= ~(1 << 3);
			REG_GUSB_CFG |= USBCFG_TRDTIME_9;
			REG_CPM_USBPCR1 &= ~(3 << 18);
		} else if (dwc_get_utmi_width() == 1) {
			DBG("16BIT UTMI+.\n");
			REG_GUSB_CFG |= (1 << 3);
			REG_GUSB_CFG |= USBCFG_TRDTIME_6;
			REG_CPM_USBPCR1 |= (3 << 18);
		} else if (dwc_get_utmi_width() == 2) {
			DBG("8BIT or 16BIT UTMI+.\n");
			REG_GUSB_CFG &= ~USBCFG_TRDTIME_MASK;

			if (UTMI_PHY_WIDTH == 8) {
				/* 8bit */
				REG_GUSB_CFG &= ~(1 << 3);
				REG_GUSB_CFG |= USBCFG_TRDTIME_9;
				REG_CPM_USBPCR1 &= ~(3 << 18);
			} else  {
				/* 16bit */
				REG_GUSB_CFG |= (1 << 3);
				REG_GUSB_CFG |= USBCFG_TRDTIME_6;
				REG_CPM_USBPCR1 |= (3 << 18);
			}

		}
	}  else 
		DBG("Unkonwn PHY TYPE?.\n");
}

static void dwc_otg_core_init(void)
{
	u32 gusbcfg;

	gusbcfg = REG_GUSB_CFG;
	
	gusbcfg &= ~((1 << 4) | (1 << 6) | (1 << 8) | (1 << 9)); 
	REG_GUSB_CFG = gusbcfg;	// HNP SRP not support and select UTMI+

	dwc_otg_select_phy_width();

	dwc_otg_core_reset();

	REG_GAHB_CFG = 1 << 7;			// Slave mode and Mask all intr
	REG_GINT_MASK = 0;			// Mask Intr 
}

static void handle_ep0_status_in_phase(USB_STATUS *status, int epnum)
{
	REG_DIEP_SIZE(epnum) = DOEPSIZE0_PKTCNT_BIT; // pktcnt->1 xfersize->0
	REG_DIEP_CTL(epnum) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
}

static void handle_ep0_data_in_phase(USB_STATUS *status, int epnum)
{
	u16 pktcnt, xfersize;
	u16 txstatus = REG_DIEP_TXFSTS(epnum);

	if (status->length > DEP_EP0_MAXPKET_SIZE)
		xfersize = DEP_EP0_MAXPKET_SIZE;
	else
		xfersize = status->length;

	pktcnt = (xfersize + DEP_EP0_MAXPKET_SIZE - 1) / DEP_EP0_MAXPKET_SIZE;

	REG_DIEP_SIZE(epnum) = (pktcnt << 19) | xfersize;
	REG_DIEP_CTL(epnum) |= DEP_ENA_BIT | DEP_CLEAR_NAK;

	REG_DIEP_EMPMSK |= (1 << epnum);
}

static void write_ep0_packet(USB_STATUS *status, int epnum)
{
	u16 pktcnt, xfersize;
	u16 dwords;
	u16 txstatus = REG_DIEP_TXFSTS(epnum);
	int i;

	if (status->length > DEP_EP0_MAXPKET_SIZE)
		xfersize = DEP_EP0_MAXPKET_SIZE;
	else
		xfersize = status->length;

	dwords = (xfersize + 3) / 4;


	for (i = 0; i < dwords; i++) {
		REG_EP0_FIFO = REG32((unsigned int *)(status->addr)+i);
	}

	status->length -= xfersize;
	
	return;
}

static void handle_start_frame_intr(void)
{
	DBG("Handle start frame intr.\n");

	REG_GINT_STS |= GINTSTS_START_FRAM;
	return;
}

static void handle_early_suspend_intr(void)
{
	DBG("Handle early suspend intr.\n");
	
	REG_GINT_STS |= GINTSTS_USB_EARLYSUSPEND;
	
	return;
}

static void handle_reset_intr(void)
{
	u16 i = 0;

	DBG("Handle_reset_intr called \n");
	REG_DAINT_MASK |=  (1 << 0) | (1 << 16);	//inep0 outep0
	REG_DOEP_MASK |= (1 << 0) | (1 << 3);		// xfercompl setupdone 
	REG_DIEP_MASK |= (1 << 0) | (1 << 3);		// xfercompl intoken timeout 

	/* NAK OUT ep */
	REG_DOEP_CTL(0) |= DEP_SET_NAK; 
	REG_DOEP_CTL(1) |= DEP_SET_NAK; 

	/* flush all txfifos */
	dwc_otg_flush_tx_fifo();

	/* Reset Device Address */
	REG_OTG_DCFG &= ~DCFG_DEV_ADDR_MASK;

	/* setup EP0 to receive SETUP packets */
	REG_DOEP_SIZE(0) = DOEPSIZE0_SUPCNT_3 | DOEPSIZE0_PKTCNT_BIT | (8 * 3);

	REG_DOEP_CTL(0) = DEP_ENA_BIT  | DEP_CLEAR_NAK;

	REG_GINT_STS |= GINTSTS_USB_RESET;

	return;
}	/* handle_reset_intr */

static void handle_enum_done_intr(int *flag)
{
	int speed = 0;

	*flag = 1;

	u32 dsts = REG_OTG_DSTS;
	u32 diep0ctl = REG_DIEP_CTL(0);
	diep0ctl &= ~(0x3);

#define USB_SPEED_HIGH	0
#define USB_SPEED_FULL	1
#define USB_SPEED_LOW	2	

	REG_DIEP_CTL(1) &= ~DEP_PKTSIZE_MASK;
	REG_DOEP_CTL(1) &= ~DEP_PKTSIZE_MASK;
	REG_DIEP_CTL(1) &= ~DEP_TYPE_MASK;
	REG_DOEP_CTL(1) &= ~DEP_TYPE_MASK;

	DBG("Handle enum done intr.\n");

	switch(dsts & DSTS_ENUM_SPEED_MASK) {
	case DSTS_ENUM_SPEED_HIGH:
		DBG("High speed.\n");
		speed = USB_SPEED_HIGH;
		REG_DIEP_CTL(1) |= DEP_HS_PKTSIZE;
		REG_DOEP_CTL(1) |= DEP_HS_PKTSIZE;
		break;
	case DSTS_ENUM_SPEED_FULL_30OR60:
	case DSTS_ENUM_SPEED_FULL_48:
		DBG("Full speed.\n");
		speed = USB_SPEED_FULL;
		REG_DIEP_CTL(1) |= DEP_FS_PKTSIZE;
		REG_DOEP_CTL(1) |= DEP_FS_PKTSIZE;
		break;
	case DSTS_ENUM_SPEED_LOW:
		speed = USB_SPEED_LOW;
		diep0ctl |= DEP_EP0_MPS_8;
		break;
	default:
		DBG("Fault speed enumration\n");
		break;
	}

	REG_DIEP_CTL(0) = diep0ctl;
	REG_DOEP_CTL(0) |= DEP_ENA_BIT;
	REG_DIEP_CTL(1) |= USB_ACTIVE_EP | DEP_TYPE_BULK;
	REG_DOEP_CTL(1) |= USB_ACTIVE_EP | DEP_TYPE_BULK;

	REG_OTG_DCTL |= DCTL_CLR_GNPINNAK;

	REG_GINT_STS |= GINTSTS_ENUM_DONE;
	
	return;
}

static void handle_rxfifo_nempty(USB_STATUS *status)
{
	u16 count;
	u32 rxsts_pop = REG_GRXSTS_POP;

	DBG("Handle rxfifo nempty.\n");
	switch(rxsts_pop & GRXSTSP_PKSTS_MASK) {
	case GRXSTSP_PKSTS_GOUT_NAK:       
		DBG("GRXSTSP_PKSTS_GOUT_NAK.\n");
		break;
	case GRXSTSP_PKSTS_GOUT_RECV:         
		DBG("GRXSTSP_PKSTS_GOUT_RECV.\n");
		count = (rxsts_pop & GRXSTSP_BYTE_CNT_MASK) >> GRXSTSP_BYTE_CNT_BIT;
		if (count) {
			DBG("RXFIFO cnt:");
			DBG_HEX(count);
			DBG("\n");
			DBG("status->addr:");
			DBG_HEX((unsigned int)(status->addr));
			DBG("\n");
			dwc_read_out_packet(status, count);
		}
		break;
	case GRXSTSP_PKSTS_TX_COMP:           
		DBG("GRXSTSP_PKSTS_TX_COMP.\n");
		break;
	case GRXSTSP_PKSTS_SETUP_COMP:
		DBG("GRXSTSP_PKSTS_SETUP_COMP.\n");
		break;
	case GRXSTSP_PKSTS_SETUP_RECV:
		DBG("GRXSTSP_PKSTS_SETUP_RECV.\n");
		status->setup_packet[0] = REG_EP0_FIFO;
		status->setup_packet[1] = REG_EP0_FIFO;
		DBG("word1:");
		DBG_HEX(status->setup_packet[0]);
		DBG("word2:");
		DBG_HEX(status->setup_packet[1]);
		break;
	default:
		break;
	}

	REG_GINT_STS |= GINTSTS_RXFIFO_NEMPTY;

	return;
}
static int handle_setup_packet(USB_STATUS *status, int epnum)
{
	u32 i;
	u32 addr;

	u32 word1 = status->setup_packet[0];
	u32 word2 = status->setup_packet[1];
	status->data_tran_flag = 0;
	status->length = 0;

	if (word1 & 0x60) {
		/* vendor_request handle */
		DBG("Vendor_request : ");

		switch((word1 >> 8) & 0xff) {
		case EP0_GET_CPU_INFO:
			DBG("EP0_GET_CPU_INFO \n");
			status->addr = cpu_info_data;
			status->length = 8;
			status->data_tran_flag = 1;
			break;
			
		case EP0_SET_DATA_ADDRESS:
			status->addr = (u8 *)((word1 & 0xffff0000) | (word2 & 0xffff));
			DBG("EP0_SET_DATA_ADDRESS. Data Addr = 0x");
			DBG_HEX((u32)status->addr);
			break;
			
		case EP0_SET_DATA_LENGTH:
			status->length = (word1 & 0xffff0000) | (word2 & 0xffff);
			DBG("EP0_SET_DATA_LENGTH. Data length = ");
			DBG_DEC(status->length);
			return -GO_HANDLE_EPIN1_INTR;
			
		case EP0_FLUSH_CACHES:
			DBG("EP0_FLUSH_CACHE \n");
			flush_dcache_all();		//twxie fixme
			break;
			
		case EP0_PROG_START1:
			addr = (word1 & 0xffff0000) | (word2 & 0xffff);
			DBG("EP0_PROG_START1. Start addr = 0x");
			DBG_HEX(addr);
			handle_ep0_status_in_phase(status, epnum);
			return xfer_d2i(addr, 0x4000);
			
		case EP0_PROG_START2:
			addr = ((word1 & 0xffff0000)|(word2 & 0xffff));
			DBG("EP0_PROG_START2. Start addr = 0x");
			DBG_HEX(addr);

			handle_ep0_status_in_phase(status, epnum);
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
			addr = word1 >> 16;
			REG_OTG_DCFG &= ~DCFG_DEV_ADDR_MASK;
			REG_OTG_DCFG |= addr << DCFG_DEV_ADDR_BIT;
			DBG("SET_ADDRESS. Address = ");
			DBG_HEX(addr);
			break;
			
		case USB_REQ_SET_CONFIGURATION:
			DBG("SET_CONFIGURATION. Configuration = ");
			DBG_DEC(word1 >> 16);
			REG_DOEP_CTL(1) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
			break;
			
		case USB_REQ_GET_DESCRIPTOR:
			DBG("GET_DESCRIPTOR - ");

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
				if ((REG_OTG_DSTS & DSTS_ENUM_SPEED_MASK) == DSTS_ENUM_SPEED_HIGH)
					status->addr = hs_desc;
				else
					status->addr = fs_desc;
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
				break;
			}
			break;
			
		default:
			DBG("Not contain.\n");
			break; 
		}
	}

	if (status->data_tran_flag)
		handle_ep0_data_in_phase(status, epnum);
	else
		handle_ep0_status_in_phase(status, epnum);
	
	return 0;
}

static void handle_outep_intr(USB_STATUS *status)
{
	u32 ep_intr, intr;
	int epnum = 0;

	DBG("Handle outep intr.\n");

	ep_intr = (REG_OTG_DAINT & 0xffff0000) >> 16;
	while (ep_intr) {
		DBG("Handle outep:");
		DBG_HEX(epnum);

		intr = REG_DOEP_INT(epnum);
		// FIX ME
		if (intr & DEP_XFER_COMP) {
			DBG("outep xfer comp.\n");
			REG_DOEP_INT(epnum) |= DEP_XFER_COMP;
			REG_DOEP_CTL(epnum) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
		}
		// FIX ME
		if (intr & DEP_SETUP_PHASE_DONE) {
			DBG("DEP_SETUP_PHASE_DONE.\n");
			REG_DOEP_INT(epnum) |= DEP_SETUP_PHASE_DONE;
			handle_setup_packet(status, epnum);
		}
		if (intr & DEP_EPDIS_INT) {
			DBG("outep disabled.\n");
			REG_DOEP_INT(epnum) |= DEP_EPDIS_INT;
		}
		if (intr & DEP_AHB_ERR) {
			DBG("outep AHB error.\n");
			REG_DOEP_INT(epnum) |= DEP_AHB_ERR;
		}
		if (intr & DEP_STATUS_PHASE_RECV) {
			DBG("outep DEP_STATUS_PHASE_RECV.\n");
			REG_DOEP_INT(epnum) |= DEP_STATUS_PHASE_RECV;
		}

		if (intr & DEP_BABBLE_ERR_INT) {
			DBG("outep DEP_BABBLE_ERR_INT.\n");
			REG_DOEP_INT(epnum) |= DEP_BABBLE_ERR_INT;
		}
		if (intr & DEP_OUTTOKEN_RECV_EPDIS) {
			DBG("outep DEP_OUTTOKEN_RECV_EPDIS.\n");
			REG_DOEP_INT(epnum) |= DEP_OUTTOKEN_RECV_EPDIS;
		} 
		if (intr & DEP_NAK_INT) {
			DBG("outep DEP_NAK_INT.\n");
			REG_DOEP_INT(epnum) |= DEP_NAK_INT;
		}
		if (intr & DEP_NYET_INT) {
			DBG("outep DEP_NYET_INT.\n");
			REG_DOEP_INT(epnum) |= DEP_NYET_INT;
		}

		epnum++;
		ep_intr >>= 1;
	}

	return;
}

static void handle_inep_intr(USB_STATUS *status)
{
	u32 ep_intr, intr;	
	int epnum = 0;

	DBG("Handle inep intr.\n");
	ep_intr = (REG_OTG_DAINT & 0xffff);

	while(ep_intr) {
		DBG("Handle inep:");
		DBG_HEX(epnum);

		u32 intr = REG_DIEP_INT(epnum);
		// FIX ME
		if (intr & DEP_XFER_COMP) {
			DBG("inep xfer comp.\n");
			REG_DIEP_EMPMSK = 0;
			REG_DIEP_INT(epnum) |= DEP_XFER_COMP;
			if (intr & DEP_NAK_INT)
				REG_DIEP_INT(epnum) |= DEP_NAK_INT;
		}

		if (intr & DEP_EPDIS_INT) {
			DBG("inep disabled.\n");
			REG_DIEP_INT(epnum) |= DEP_EPDIS_INT;
		}

		if (intr & DEP_AHB_ERR) {
			DBG("inep AHB error.\n");
			REG_DIEP_INT(epnum) |= DEP_AHB_ERR;
		}

		if (intr & DEP_TIME_OUT) {
			DBG("inep tx timeout.\n");
			REG_DIEP_INT(epnum) |= DEP_TIME_OUT;
		}

		if (intr & DEP_INTOKEN_RECV_TXFIFO_EMPTY) {
			DBG("intoken recv while txfifo empty.\n");
			REG_DIEP_INT(epnum) |= DEP_INTOKEN_RECV_TXFIFO_EMPTY;
		}

		if (intr & DEP_TXFIFO_EMPTY) {
			DBG("TX FIFO EMPTY intr.\n");
			if (status->length)
				write_ep0_packet(status, epnum);
			REG_DIEP_INT(epnum) |= DEP_TXFIFO_EMPTY;
		}

		if (intr & DEP_NAK_INT) {
			DBG("inep DEP_NAK_INT.\n");
			REG_DIEP_INT(epnum) |= DEP_NAK_INT;
		}
		epnum++;
		ep_intr >>= 1;
	}

	return ;
}
static void dwc_otg_enable_common_irq(void)
{
	REG_GAHB_CFG |= 1;
	/*		CONIDSTS	OUTEP	   INEP		enum	  usbreset 	*/	
	REG_GINT_MASK |= (1 << 28) | (1 << 19) | (1 << 18) | (1 << 13) | (1 << 12);
	
	return; 
}

/*-------------------------------------------------------------------------*/
static void dwc_otg_cpm_init(void)
{
	REG_CPM_USBPCR1 |= (1 << 28);

	REG_CPM_USBPCR &= ~(1 << 31);
        REG_CPM_USBPCR |= USBPCR_VBUSVLDEXT;

	REG_CPM_USBPCR |= USBPCR_POR;
        xudelay(30);
        REG_CPM_USBPCR &= ~USBPCR_POR;
        xudelay(300);
}

void memset (unsigned char *p, unsigned char c, int size)
{
	int i;
	for (i = 0; i < size; i++)
		p[i] = c; 
}

int usb_boot(void)
{
	int usb_reset_flag = 0;
	u32 intsts;
	USB_STATUS status = {NULL, {0, 0}, 0, 0, 0};
	
	div_clock();

	dwc_otg_cpm_init();

	__cpm_enable_otg_phy();

	dwc_otg_core_init();	

	dwc_otg_device_init();

	dwc_otg_enable_common_irq();

	/* start counter */
	xtimeout(0, 1);
	
	/* Main loop of polling the usb commands */
	while (1) {
		if (!(usb_reset_flag)) {
			if (!xtimeout(10 * 1000 * 1000, 0)) {
				DBG("*! USB idle more than 10s !*\n\n");
				pmon_clear_cnt();
				pmon_stop();

				return -GO_RESTART_BOOT;
			}
		}
		
		intsts = REG_GINT_STS;
		if (intsts & GINTSTS_USB_EARLYSUSPEND)
			handle_early_suspend_intr();

		if (intsts & GINTSTS_START_FRAM)
			handle_start_frame_intr();

		/* reset interrupt handle */
		if (intsts & GINTSTS_USB_RESET) {
			handle_reset_intr();
		}

		/* enum done */	
		if (intsts & GINTSTS_ENUM_DONE) {
			pmon_clear_cnt();
			pmon_stop();
			handle_enum_done_intr(&usb_reset_flag);
		}


		if (intsts & GINTSTS_RXFIFO_NEMPTY)
			handle_rxfifo_nempty(&status);

		if (intsts & GINTSTS_IEP_INTR)
			handle_inep_intr(&status);

		if (intsts & GINTSTS_OEP_INTR)
			handle_outep_intr(&status);
	}
	
	return 0;
}	/* usb_boot end */

