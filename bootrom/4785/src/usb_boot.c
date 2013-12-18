/*
 * usb_boot.c
 *
 * Internal BootROM c-code for JZ4780B.
 * Supports booting USB device.
 *
 * Copyright (C) 2005 - 2012 Ingenic Semiconductor Inc.
 *
 * Authors: cli <cli@ingenic.cn>
 *
 * Revision history:
 *  - 2013/09/02: modify jz4780B usb_boot base on jz4780 boot. <cli>
 *  - 2012/03/21: Init version from jz4780 usb_boot. <Twxie>
 *  - 2012/04/07: Cut ch9.h included and adjust usb_boot.h. <Twxie>
 *  - 2012/04/14: Adjust code style for usb_boot. <Twxie>
 */

#include <irom.h>
#include <common.h>
#include <jz4785cpm.h>
#include <usb/otg_dwc2.h>
#include <usb/usb_boot.h>
#include <pmon.h>

#define BOOTROM_CPUFREQ	(200)
static int volatile fout = BOOTROM_CPUFREQ;	/*cpm fout : cpufreq*/
static u32 status_map;

enum {
	HIGH_SPEED = 0,
	FULL_SPEED,
	LOW_SPEED
};

int enum_done_speed = HIGH_SPEED;

static inline int DEP_EP_MAXPKT_SIZE(int n)
{
	if (n) {
		if (enum_done_speed == HIGH_SPEED)
			return DEP_HS_PKTSIZE;
		else if (enum_done_speed == FULL_SPEED)
			return DEP_FS_PKTSIZE;
		else
			return 0;
	}
	return 64;
}

void dwc_read_out_packet(USB_STATUS *status, u16 count,u32 epnum)
{
	int dwords = (count+3) / 4;
	int i;

	DBG("epnum:");
	DBG_DEC(epnum);
	DBG("count:");
	DBG_DEC(count);
	DBG("dst addr:");
	DBG_HEX((u32)status->addr_out);

	if (epnum == 1 && status->addr_out != 0) {
		for (i = 0; i < dwords; i++)
			REG32((unsigned int *)(status->addr_out) + i) = REG_EP_FIFO(1);
		status->addr_out += count;
	} else {
		/*discard out*/
		u32 discard = 0;
		for (i = 0; i < dwords; i++)
			discard = REG_EP_FIFO(epnum);
	}

	return;
}

static void dwc_otg_flush_rx_fifo(void)
{
	REG_GRST_CTL = RSTCTL_RXFIFO_FLUSH;

	DBG("dwc_otg_flush_rx_fifo :\n");
	while (REG_GRST_CTL & RSTCTL_RXFIFO_FLUSH);
	return;
}

static void dwc_otg_flush_tx_fifo(unsigned char txf_num)
{
	unsigned int gintsts;
	unsigned int grstctl;
	gintsts = REG_GINT_STS;

	/*
	 * Step1: Check that GINTSTS.GinNakEff=0 if this
	 *		  bit is cleared then set Dctl.SGNPInNak = 1.
	 *        Nak effective interrupt = H indicating the core
	 *        is not reading from fifo
	 */

	DBG("dwc_otg_flush_tx_fifo :\n");
	if ((gintsts & GINTSTS_GINNAK_EFF))
	{
		REG_OTG_DCTL |= DCTL_SET_GNPINNAK;

		/* Step2: wait for GINTSTS.GINNakEff=1,which indicates
		 *        the NAK setting has taken effect to all IN endpoints */
		while(!(REG_GINT_STS & GINTSTS_GINNAK_EFF))
			xudelay(1);
	}

	/* Step3: wait for ahb master idle state */
	while (!(REG_GRST_CTL & RSTCTL_AHB_IDLE));

	/* Step4: Check that GrstCtl.TxFFlsh=0, if it is 0, then write
	 *        the TxFIFO number you want to flush to GrstCTL.TxFNum*/
	grstctl = REG_GRST_CTL;
	if (!(grstctl & RSTCTL_TXFIFO_FLUSH))
	{
		REG_GRST_CTL |= (txf_num << 6);
	}

	/* Step5: Set GRSTCTL.TxFFlsh=1 and wait for it to clear */
	REG_GRST_CTL |= RSTCTL_TXFIFO_FLUSH;

	while (REG_GRST_CTL & RSTCTL_TXFIFO_FLUSH);

	/* Step6: Set the DCTL.GCNPinNak bit */
	REG_OTG_DCTL |= DCTL_CLR_GNPINNAK;
}

static void dwc_set_in_nak(int epnum)
{
	int  timeout = 5000;

	if (!(REG_DIEP_CTL(epnum) & DEP_ENA_BIT))
		return ;

	REG_DIEP_CTL(epnum) |= DEP_SET_NAK;
	do {
		xudelay(1);
		if (timeout < 2) {
			DBG("dwc set in nak timeout\n");
		}
	} while ( (!(REG_DIEP_INT(epnum) & DEP_INEP_NAKEFF)) && (--timeout > 0));
}

static void dwc_disable_in_ep(int epnum)
{
	int  timeout = 100000;

	if (!(REG_DIEP_CTL(epnum) & DEP_ENA_BIT))
		return ;

	REG_DIEP_CTL(epnum) |= DEP_DISENA_BIT;

	do
	{
		xudelay(1);
		if (timeout < 2) {
			DBG("dwc disable in ep timeout\n");
		}
	} while ( (!(REG_DIEP_INT(epnum) & DEP_EPDIS_INT)) && (--timeout > 0));

	REG_DIEP_INT(epnum) = DEP_EPDIS_INT;

	dwc_otg_flush_tx_fifo(epnum);
}

static void dwc_stop_in_transfer(int epnum)
{
	dwc_set_in_nak(epnum);
	dwc_disable_in_ep(epnum);
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
		if (cnt++ > 10000 ) {
			DBG("GRESET wait reset timeout.\n");
			return;
		}
		xudelay(10);
	} while (greset & RSTCTL_CORE_RST);

	/* wait for 3 phy clocks */
	xudelay(100);

}

void dwc_otg_core_init(void)
{
	u32 gusbcfg;
	u32 reset = 0;

	DBG("dwc_otg_core_init :\n");

	/* AHB config Slave mode ,Unmask globle inter*/
	REG_GAHB_CFG = AHBCFG_GLOBLE_INTRMASK;

	/*Mask RxfvlMsk Intr*/
	REG_GINT_MASK &= ~(GINTMSK_RXFVL);

	/*HNP SRP not support , usb2.0 , utmi+, 16bit phy*/
	gusbcfg = REG_GUSB_CFG;
	if (!(gusbcfg | USBCFG_16BIT_PHY) ||
			(gusbcfg | USBCFG_PHY_INF_UPLI)) {
		/*When we change phy config, we must reset core*/
		reset = 1;
	}
	gusbcfg &= ~( USBCFG_HNP_EN | USBCFG_SRP_EN | USBCFG_PHY_SEL_USB1 |
			USBCFG_TRDTIME_MASK | USBCFG_PHY_INF_UPLI);
	gusbcfg |= USBCFG_16BIT_PHY | USBCFG_TRDTIME(5);
	REG_GUSB_CFG = gusbcfg;

	if (reset) {
		dwc_otg_core_reset();
		REG_GAHB_CFG = AHBCFG_GLOBLE_INTRMASK;
	}

	REG_GINT_STS = GINTSTS_START_FRAM;
	REG_GINT_MASK = GINTMSK_OTG;
}

void dwc_fifo_allocate(void)
{
	u16 epinfobase, gdfifocfg;

	/* rxfifo size */
	REG_GRXFIFO_SIZE = DEP_RXFIFO_SIZE;
	DBG("DEP_RXFIFO_SIZE: \n");
	DBG_HEX(DEP_RXFIFO_SIZE);
	DBG("REG_GRXFIFO_SIZE :\n");
	DBG_HEX(REG_GRXFIFO_SIZE);

	/* txfifo0 size */
	REG_GNPTXFIFO_SIZE = (DEP_NPTXFIFO_SIZE << 16) | DEP_RXFIFO_SIZE;
	DBG("(DEP_NPTXFIFO_SIZE << 16) | DEP_RXFIFO_SIZE :\n");
	DBG_HEX((DEP_NPTXFIFO_SIZE << 16) | DEP_RXFIFO_SIZE);
	DBG("REG_GNPTXFIFO_SIZE :\n");
	DBG_HEX(REG_GNPTXFIFO_SIZE);

	/* txfifo1 size */
	REG_DIEPTXF(1) = (DEP_DTXFIFO_SIZE << 16) | (DEP_RXFIFO_SIZE + DEP_NPTXFIFO_SIZE);
	DBG(" (DEP_DTXFIFO_SIZE << 16) | (DEP_RXFIFO_SIZE + DEP_NPTXFIFO_SIZE)");
	DBG_HEX( (DEP_DTXFIFO_SIZE << 16) | (DEP_RXFIFO_SIZE + DEP_NPTXFIFO_SIZE));
	DBG("REG_DIEPTXF(1):\n");
	DBG_HEX(REG_DIEPTXF(1));

	gdfifocfg = REG_GHW_CFG3 >> 16;
	epinfobase = (REG_DIEPTXF(1) & 0xffff) + (REG_DIEPTXF(1) >> 16);
	REG_GDFIFO_CFG = (epinfobase << 16) | gdfifocfg;

	dwc_otg_flush_tx_fifo(0x10);
	dwc_otg_flush_rx_fifo();
}

static void dwc_otg_device_init(void)
{
	DBG("bootrom device init!!!!\n");

	/* fifo init here*/
	dwc_fifo_allocate();

	/* dma disable ,High speed , stall no zero handshack*/
	REG_OTG_DCFG = DCFG_HANDSHAKE_STALL_ERR_STATUS;

	/* Soft Disconnect connect*/
	REG_OTG_DCTL &= ~DCTL_SOFT_DISCONN;
	REG_OTG_DCTL |= DCTL_NAK_ON_BBLE;

	/* Unmask suspend earlysuspend reset enumdone sof intr*/
	REG_GINT_MASK = GINTSTS_USB_SUSPEND | GINTSTS_USB_RESET | GINTSTS_ENUM_DONE |
		GINTSTS_USB_EARLYSUSPEND;

	return;
}

void handle_in_transfer(USB_STATUS *status, int epnum)
{
	int mps = DEP_EP_MAXPKT_SIZE(epnum);
	int pktcnt = 1;

	DBG("handle_in_transfer ,epnum:");
	DBG_DEC(epnum);
	DBG("mps :");
	DBG_DEC(mps);
	if (REG_DIEP_CTL(epnum) & DEP_ENA_BIT) {
		DBG("handle in transfer when endpoint enable\n");
		dwc_stop_in_transfer(epnum);
	}

	pktcnt = (status->length + mps - 1)/mps;

	if (status->length) {
		DBG("no zero packt:");
		DBG_HEX((pktcnt << DXEPSIZE_PKTCNT_BIT | status->length));
		REG_DIEP_SIZE(epnum) = (1 << DXEPSIZE_PKTCNT_BIT) | status->length;
		REG_DIEP_CTL(epnum) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
		REG_DIEP_EMPMSK |= (1 << epnum);
	} else {
		DBG("zero packet\n");
		REG_DIEP_SIZE(epnum) = DIEPSIZE0_PKTCNT_BIT;
		REG_DIEP_CTL(epnum) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
	}
}

void write_tranfer_fifo(USB_STATUS *status, int epnum)
{
	u32 dwords = 0;
	u32 txstatus = 0;
	u32 timeout = 0xfffff;
	u32 intsts = 0;
	int i,breakout = 3;

	dwords = (status->length + 3) / 4;
	DBG("xfer length = :");
	DBG_DEC(status->length);
	do {
		txstatus = REG_DIEP_TXFSTS(epnum);
		if (!(timeout--)) {
			intsts = REG_GINT_STS;
			if (intsts & GINTSTS_USB_RESET) {
				DBG("WARN: time out host reset\n");
				return;
			}
			timeout = 0xfffff;
			if (txstatus == 0) {
				if (breakout--)
					xudelay(1);
				else {
					DBG("May be some error happen\n");
					return;
				}
			}
		}
	} while (txstatus < dwords);

	for (i = 0; i < dwords; i++) {
		REG_EP_FIFO(epnum) = REG32((unsigned int *)(status->addr_in)+i);
	}
	status->length = 0;

	return;
}

static void handle_start_frame_intr(void)
{
	REG_GINT_STS = GINTSTS_START_FRAM;
	return;
}

void dwc_init(USB_STATUS *status,int *flag)
{
	memset((int *)status, 0,2*sizeof(USB_STATUS));
	dwc_otg_core_init();
	dwc_otg_device_init();
	*flag = 0;
	xtimeout(0, 1);
	return;
}

static void handle_early_suspend_intr(int *flag,USB_STATUS *status)
{
	u32 dsts = REG_OTG_DSTS;

	DBG("Handle early suspend intr.\n");
	REG_GINT_STS = GINTSTS_USB_EARLYSUSPEND;
	if (dsts & DSTS_ERRATIC_ERROR) {
		/*handle babble conditional,software reset*/
		dwc_otg_core_reset();
		dwc_init(status,flag);
	}
	return;
}

static void handle_reset_intr(void)
{
	DBG("Handle_reset_intr called \n");

	/* step1 :NAK OUT ep */
	REG_DOEP_CTL(0) |= DEP_SET_NAK;
	REG_DOEP_CTL(1) |= DEP_SET_NAK;

	/* step2: unmask inter */
	REG_DAINT_MASK = DAINT_IEP_MASK(0) | DAINT_OEP_MASK(0);
	REG_DOEP_MASK = DEP_SETUP_PHASE_DONE | DEP_XFER_COMP;
	REG_DIEP_MASK = DEP_XFER_COMP;

	/* step3: device init nothing to do */

	/* step4: dfifo dynamic allocated */
	//dwc_fifo_allocate();

	/* step5: Reset Device Address */
	REG_OTG_DCFG &= ~DCFG_DEV_ADDR_MASK;

	/* step6: setup EP0 to receive SETUP packets */
	REG_DOEP_SIZE(0) = DOEPSIZE0_SUPCNT_3;
	REG_DOEP_CTL(0) |= DEP_ENA_BIT | DEP_CLEAR_NAK;

	REG_GINT_STS = GINTSTS_USB_RESET;

	return;
}	/* handle_reset_intr */

static void handle_enum_done_intr(int *flag)
{
	u32 dsts = REG_OTG_DSTS;
	*flag = 1;

	DBG("Handle enum done intr.\n");
	switch(dsts & DSTS_ENUM_SPEED_MASK) {
	case DSTS_ENUM_SPEED_HIGH:
		DBG("High speed.\n");
		enum_done_speed = HIGH_SPEED;
		break;
	case DSTS_ENUM_SPEED_FULL_30OR60:
	case DSTS_ENUM_SPEED_FULL_48:
		DBG("Full speed.\n");
		enum_done_speed = FULL_SPEED;
		break;
	case DSTS_ENUM_SPEED_LOW:
		enum_done_speed = LOW_SPEED;
	default:
		DBG("Fault speed enumration\n");
		return;
	}

	REG_OTG_DCTL |= DCTL_CLR_GNPINNAK;
	REG_DIEP_CTL(0) |= DEP_EP0_MPS_64 | DEP_IN_FIFO_SEL(0);
	REG_DOEP_CTL(0) |= DEP_EP0_MPS_64 | DEP_CLEAR_NAK;
	REG_GINT_MASK |= GINTSTS_RXFIFO_NEMPTY | GINTSTS_IEP_INTR |
		GINTSTS_OEP_INTR | GINTSTS_START_FRAM;
	REG_GINT_STS = GINTSTS_ENUM_DONE;

	return;
}

void handle_rxfifo_nempty(USB_STATUS *status)
{
	u32 rxsts_pop = REG_GRXSTS_POP;
	u16 count = (rxsts_pop & GRXSTSP_BYTE_CNT_MASK) >> GRXSTSP_BYTE_CNT_BIT;
	u32 epnum = (rxsts_pop & 0xf);

	REG_GINT_MASK &= ~GINTSTS_RXFIFO_NEMPTY;
	switch(rxsts_pop & GRXSTSP_PKSTS_MASK) {
	case GRXSTSP_PKSTS_GOUT_NAK:
		DBG("GRXSTSP_PKSTS_GOUT_NAK.\n");
		break;
	case GRXSTSP_PKSTS_TX_COMP:
		DBG("GRXSTSP_PKSTS_TX_COMP.\n");
		break;
	case GRXSTSP_PKSTS_GOUT_RECV:
		DBG("GRXSTSP_PKSTS_GOUT_RECV.\n");
		dwc_read_out_packet(&status[epnum], count, epnum);
		break;
	case GRXSTSP_PKSTS_SETUP_COMP:
		DBG("GRXSTSP_PKSTS_SETUP_COMP.\n");
		break;
	case GRXSTSP_PKSTS_SETUP_RECV:
		DBG("GRXSTSP_PKSTS_SETUP_RECV.\n");
		status[epnum].setup_packet[0] = REG_EP_FIFO(epnum);
		status[epnum].setup_packet[1] = REG_EP_FIFO(epnum);
		break;
	default:
		break;
	}

	REG_GINT_STS = GINTSTS_RXFIFO_NEMPTY;
	REG_GINT_MASK |= GINTSTS_RXFIFO_NEMPTY;
	return;
}

void dwc_ep0_stall(void)
{
	DBG("ep0 stall\n");
	/*stall ep0*/
	REG_DIEP_CTL(0) |= DEP_SET_STALL;
	REG_DOEP_CTL(0) |= DEP_SET_STALL;

	/*restart ep0 out*/
	REG_DOEP_SIZE(0) = DOEPSIZE0_SUPCNT_3;
	REG_DOEP_CTL(0) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
}

void dwc_set_config(USB_STATUS *status)
{
	int max_pkt_size = DEP_EP_MAXPKT_SIZE(1);

	REG_DOEP_INT(1) = 0xffff;
	REG_DIEP_INT(1) = 0xffff;

	REG_DIEP_CTL(1) = USB_ACTIVE_EP | DEP_TYPE_BULK | max_pkt_size | DEP_RESET_DATA0 | DEP_IN_FIFO_SEL(1);
	REG_DOEP_CTL(1) = USB_ACTIVE_EP | DEP_TYPE_BULK | max_pkt_size | DEP_RESET_DATA0;
	REG_DIEP_SIZE(1) = 0;
	REG_DOEP_SIZE(1) = (1 << 19) | max_pkt_size;
	REG_DOEP_CTL(1) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
	REG_DAINT_MASK  |=  (1 << 1) | (1 << 17);    //umask ep1 intr;
}

int handle_setup_packet(USB_STATUS *status)
{
	u32 i;
	u32 addr;
	u32 word1 = status[0].setup_packet[0];
	u32 word2 = status[0].setup_packet[1];
	u32 usb_stall = 0;

	status[0].length = 0;
	status[0].data_trans = 0;

	if ((word1 & 0x60) == 0x40) {
		/* vendor_request handle */
		DBG("Vendor_request case :");
		switch((word1 >> 8) & 0xff) {
		case EP0_GET_CPU_INFO:
			DBG("EP0_GET_CPU_INFO \n");
			status[0].addr_in = (u8* )cpu_info_data;
			status[0].length = 8;
			break;

		case EP0_SET_DATA_ADDRESS:
			status[1].addr_out = (u8 *)((word1 & 0xffff0000) | (word2 & 0xffff));
			status[1].addr_in = status[1].addr_out;
			DBG("EP0_SET_DATA_ADDRESS. Data Addr = 0x");
			DBG_HEX((u32)status[1].addr_out);
			break;

		case EP0_SET_DATA_LENGTH:
			status[1].length = (word1 & 0xffff0000) | (word2 & 0xffff);
			DBG("EP0_SET_DATA_LENGTH. Data length = ");
			DBG_DEC(status[1].length);
			break;

		case EP0_PROG_START1:
			addr = (word1 & 0xffff0000) | (word2 & 0xffff);
			DBG("EP0_PROG_START1. Start addr = 0x");
			DBG_HEX(addr);
			handle_in_transfer(&status[0], 0);
			{
				int timeout = 0xffff;
				while ((~(REG_DIEP_INT(0) & DEP_XFER_COMP)) && (timeout--) );
				REG_DIEP_INT(0) = DEP_XFER_COMP;
			}
			DBG("jump addr\n");
			return xfer_d2i(addr, SPL_SIZE);

		case EP0_PROG_START2:
			addr = ((word1 & 0xffff0000)|(word2 & 0xffff));
			DBG("EP0_PROG_START2. Start addr = 0x");
			DBG_HEX(addr);
			handle_in_transfer(&status[0], 0);
			{
				int timeout = 0xffff;
				void (*func_usb)(unsigned int);

				while ((~(REG_DIEP_INT(0) & DEP_XFER_COMP)) && (timeout--) );
				REG_DIEP_INT(0) = DEP_XFER_COMP;

				func_usb = (void (*)(unsigned int)) addr;
				DBG("bootrom start stage 2\n");
				(*func_usb) (addr);
			}
			break;
		default:
			usb_stall = 1;
			break;
		}
	} else if (((word1 & 0x60) == 0x0)) {
		/* standard device request handle */
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
			dwc_set_config(&status[1]);
			break;

		case USB_REQ_GET_STATUS:
			DBG("GET_STATUS\n");
			status[0].length = 2;
			switch (word1 & 0x1f) {
			case 0x2:	/*endpoint*/
			case 0x1:	/*interface*/
				if (!(REG_DOEP_CTL(1) & USB_ACTIVE_EP)) {
					status[0].length = 0;
					usb_stall = 1;
					break;
				}
			case 0x0:	/*device*/
				status[0].addr_in = (u8*)&status_map;	//default map
				break;
			default:
				status[0].length = 0;
				usb_stall = 1;
				break;
			}
			break;
		case USB_REQ_GET_DESCRIPTOR:
			DBG("GET_DESCRIPTOR - ");
			status[0].length = word2 >> 16;
			switch(word1 >> 24) {
			case USB_DT_DEVICE:
				DBG("DEVICE. \n");
				status[0].addr_in = (u8*)device_desc;
				if (status[0].length > USB_DT_DEVICE_SIZE)
					status[0].length = USB_DT_DEVICE_SIZE;// max length of device_desc
				break;

			case USB_DT_CONFIG:
				DBG("CONFIG. \n");
				if ((REG_OTG_DSTS & DSTS_ENUM_SPEED_MASK) == DSTS_ENUM_SPEED_HIGH)
					status[0].addr_in = (u8 *)hs_desc;
				else
					status[0].addr_in = (u8 *)fs_desc;
				if (status[0].length > 32)
					status[0].length = 32;// max length of device_desc
				break;

			case USB_DT_STRING:
				DBG("STRING. \n");
				i = (word1 >>16) & 0xff;
				if (i == 1) {
					status[0].addr_in = (u8 *)string_manufacture;
					status[0].length = 16;
				} else if (i == 2) {
					status[0].addr_in = (u8 *)string_product;
					status[0].length = 46;
				} else {
					status[0].addr_in = (u8 *)string_lang_ids;
					status[0].length = 4;
				}
				break;

			case USB_DT_DEVICE_QUALIFIER:
				DBG("DEVICE_QUALIFIER. \n");
				status[0].addr_in = (u8 *)dev_qualifier;
				if (status[0].length > 10)
					status[0].length = 10;// max length of device_desc
				break;

			default:
				DBG("Not contain.\n");
				status[0].length = 0;
				usb_stall = 1;
				break;
			}
			break;

		default:
			DBG("Not contain.\n");
			usb_stall = 1;
			break;
		}
	} else {
		DBG("class request,reserve request will be stall\n");
		usb_stall = 1;
	}

	if (!usb_stall) {
		if (status[0].length != 0)
			status[0].data_trans = 1;
		handle_in_transfer(&status[0], 0);
	} else {
		dwc_ep0_stall();
	}

	return 0;
}

void handle_outep_intr(USB_STATUS *status)
{
	u32 ep_intr, intr;
	int epnum = 0;

	ep_intr = (REG_OTG_DAINT & 0x30000) >> 16;

	while (ep_intr) {
		if (ep_intr & 0x1) {
			DBG("Handle out epnum:");
			DBG_DEC(epnum);
			intr = REG_DOEP_INT(epnum);

			if (intr & DEP_XFER_COMP) {
				REG_DOEP_INT(epnum) = DEP_XFER_COMP;
				if (epnum) {
					DBG("out xfer complex .\n");
					REG_DOEP_SIZE(epnum) = (1 << 19) | DEP_EP_MAXPKT_SIZE(1);
					REG_DOEP_CTL(epnum) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
				} else {
					DBG("Handle epnum 0 xfer complex .\n");
					REG_DOEP_SIZE(0) = DOEPSIZE0_SUPCNT_3;
					REG_DOEP_CTL(0) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
				}
			}

			if (intr & DEP_SETUP_PHASE_DONE) {
				DBG("Handle setup done inter.\n");
				DBG_HEX(epnum);
				if (!epnum) {
					REG_DOEP_INT(0) = DEP_SETUP_PHASE_DONE;
					if (!(REG_DOEP_CTL(0) & DEP_ENA_BIT)) {
						REG_DOEP_SIZE(0) = DOEPSIZE0_SUPCNT_3;
						REG_DOEP_CTL(0) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
					}
					handle_setup_packet(status);
				}
			}

			if (intr & DEP_EPDIS_INT) {
				DBG("out disabled.\n");
				REG_DOEP_INT(epnum) = DEP_EPDIS_INT;
			}

			if (intr & DEP_BABBLE_ERR_INT) {
				DBG("out DEP_BABBLE_ERR_INT.\n");
				REG_DOEP_INT(epnum) = DEP_BABBLE_ERR_INT;
			}

			if (intr & DEP_NAK_INT) {
				DBG("out DEP_NAK_INT.\n");
				REG_DOEP_INT(epnum) = DEP_NAK_INT;
			}

			if (intr & DEP_NYET_INT) {
				DBG("out DEP_NYET_INT.\n");
				REG_DOEP_INT(epnum) = DEP_NYET_INT;
			}
		}
		epnum++;
		ep_intr >>= 1;
	}
	return;
}

void handle_inep_intr(USB_STATUS *status)
{
	u32 ep_intr;
	int epnum = 0;

	ep_intr = (REG_OTG_DAINT & 0x3);
	while(ep_intr) {
		if (ep_intr & 0x1) {
			u32 intr = REG_DIEP_INT(epnum);
			DBG("Handle in epnum:");
			DBG_DEC(epnum);

			if (intr & DEP_XFER_COMP) {
				DBG("in xfer complex.\n");
				REG_DIEP_INT(epnum) = DEP_XFER_COMP;
			}

			if (intr & DEP_EPDIS_INT)
				REG_DIEP_INT(epnum) = DEP_EPDIS_INT;

			if (intr & DEP_NAK_INT)
				REG_DIEP_INT(epnum) = DEP_NAK_INT;

			if (intr & DEP_NYET_INT)
				REG_DIEP_INT(epnum) = DEP_NYET_INT;

			if ((intr & DEP_TXFIFO_EMPTY) && (REG_DIEP_EMPMSK & (1 << epnum))) {
				DBG("in fifo empty.\n");
				REG_DIEP_EMPMSK &= ~(1 << epnum);
				if (status[epnum].length)
					write_tranfer_fifo(&status[epnum], epnum);
				REG_DIEP_INT(epnum) = DEP_TXFIFO_EMPTY;
			}
		}
		epnum++;
		ep_intr >>= 1;
	}
	return ;
}

static void dwc_phy_init(void)
{
#ifndef FPGA_TEST
	DBG("bootrom cpm init!!!!\n");
	int clk = jz_extal/1000000;

	REG_CPM_CLKGR1  &= ~(1 << 8);			//open otg1 clock gate
	REG_CPM_USBPCR1 &= ~(0x1f << 23);
	REG_CPM_USBPCR1 |= ((0x2 << 26)|(0x1 << 31));	//usb phy use extal crystal

	switch (clk) {
	case 9:
		break;
	case 10:
		REG_CPM_USBPCR1 |= (1 << 23);
		break;
	case 12:
		REG_CPM_USBPCR1 |= (2 << 23);
		break;
	case 19:
		REG_CPM_USBPCR1 |= (3 << 23);
		break;
	case 20:
		REG_CPM_USBPCR1 |= (4 << 23);
		break;
	case 50:
		REG_CPM_USBPCR1 |= (7 << 23);
		break;
	default:
	case 24:
		REG_CPM_USBPCR1 |= (5 << 23);
		break;
	}

	REG_CPM_USBPCR &= ~(1 << 31);
	REG_CPM_USBPCR |= USBPCR_VBUSVLDEXT;
	REG_CPM_USBPCR |= USBPCR_POR;
	xudelay(30);
	REG_CPM_USBPCR &= ~USBPCR_POR;
	xudelay(300);
	/*select 16bit phy*/
	REG_CPM_USBPCR1 |= (1 << 19);
	/*enable otg phy*/
	REG_CPM_OPCR |=  OPCR_OTGPHY_ENABLE;
#endif	/*nFPGA_TEST*/
}

int usb_boot(void)
{
	int usb_reset_flag = 0;
	u32 intsts;
	USB_STATUS status[2];

	status_map = 0;
	enum_done_speed = HIGH_SPEED;
	DBG("bootrom test !!!!\n");

	fout = BOOTROM_CPUFREQ;
	set_cpufreq(fout);
	dwc_phy_init();

	DBG_HEX(REG_GHW_CFG1);
	DBG_HEX(REG_GHW_CFG2);
	DBG_HEX(REG_GHW_CFG3);
	DBG_HEX(REG_GHW_CFG4);

	dwc_init(status,&usb_reset_flag);

	/* Main loop of polling the usb commands */
	while (1) {

		if (!(usb_reset_flag)) {
			/* Reset doesn't receive from host */
			if (!xtimeout(10 * 1000 * 1000, 0)) {
				DBG("*! USB idle more than 10s !*\n\n");
				return -GO_RESTART_BOOT;
			}
		} else {
			pmon_clear_cnt();
			pmon_stop();
		}

		intsts = REG_GINT_STS;

		if ((intsts & GINTSTS_USB_EARLYSUSPEND & REG_GINT_MASK))
			handle_early_suspend_intr(&usb_reset_flag,status);

		if ((intsts & GINTSTS_START_FRAM) & REG_GINT_MASK)
			handle_start_frame_intr();

		if ((intsts & GINTSTS_USB_RESET) & REG_GINT_MASK)
			handle_reset_intr();

		if ((intsts & GINTSTS_ENUM_DONE) & REG_GINT_MASK)
			handle_enum_done_intr(&usb_reset_flag);

		if ((intsts & GINTSTS_RXFIFO_NEMPTY) & REG_GINT_MASK)
			handle_rxfifo_nempty(status);

		if ((intsts & GINTSTS_OEP_INTR) & REG_GINT_MASK)
			handle_outep_intr(status);

		if ((intsts & GINTSTS_IEP_INTR) & REG_GINT_MASK)
			handle_inep_intr(status);
	}

	return 0;
}	/* usb_boot end */
