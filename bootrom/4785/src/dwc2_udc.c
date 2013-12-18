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
#include <regs.h>
#include <jz4780cpm.h>
#include <otg_dwc2.h>
#include <usb_boot.h>
#include <pmon.h>

#define BOOTROM_CPUFREQ	(200)

static u32 status_map;
enum control_state {
	EP0_SETUP = 0
	EP0_DATA,
	EP0_STATUS,
};

struct dwc2 {
	struct usb_gadget	gadget;
	struct usb_request	*req_attr;
	struct usb_ep		*ep_attr;
	struct usb_ctrlrequest  ctrl_req;
	enum control_state ep0_state;
	enum usb_device_speed	speed;
	bool usb_reset_flag;
};

/* Fix if we change this we must change usb_init function*/
#define DWC_MAX_EP_NUM	5
#define DWC_MAX_OUT_EP	2
#define DWC_MAX_IN_EP	3
#define DWC_MAX_REQ_NUM	(DWC_MAX_IN_EP+DWC_MAX_OUT_EP)
struct dwc2 m_dwc;
struct usb_ep	   ep_attr[DWC_MAX_EP_NUM];
struct usb_request req_attr[DWC_MAX_REQ_NUM];

void dwc_read_out_packet(struct usb_ep* usb_ep, u32 count, u32 epnum)
{
	struct usb_request *req = NULL;
	u32 dwords = (count+3) / 4;
	u32 i;

	DBG("epnum:");
	DBG_DEC(epnum);
	DBG("count:");
	DBG_DEC(count);

	if (usb_ep) {
		req = usb_ep->req;
		DBG("dst addr:");
		DBG_HEX((u32)req->buf);
	}

	if (usb_ep && req->buf) {
		for (i = 0; i < dwords; i++)
			REG32((u32*)(req->buf) + req->actual + i) = REG_EP_FIFO(epnum);
		req->actual += count;
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
	gusbcfg &= ~( USBCFG_HNP_EN | USBCFG_PHY_SEL_USB1 |
			USBCFG_TRDTIME_MASK | USBCFG_PHY_INF_UPLI);
	gusbcfg |= USBCFG_16BIT_PHY | USBCFG_SRP_EN | USBCFG_TRDTIME(5);
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
	struct dwc2 dwc = &m_dwc;
	u16 start_addr = 0;
	u16 fifo_size;

	/*rx fifo size*/
	fifo_size = 1047;	/* 4*1 + (7 + 1)*(512/4 + 1) + 2*2 + 1 fix*/
	REG_GRXFIFO_SIZE = fifo_size;
	DBG("rx fifo size: \n");
	DBG_HEX(fifo_size);
	DBG("REG_GRXFIFO_SIZE :\n");
	DBG_HEX(REG_GRXFIFO_SIZE);

	/* txfifo0 size */
	start_addr += fifo_size;
	fifo_size = 128;		/* (7 + 1)*(64/4) fix*/
	REG_GNPTXFIFO_SIZE = (fifo_size << 16) | start_addr;
	DBG("tx fifo 0  size:\n");
	DBG_HEX((fifo_size << 16) | start_addr);
	DBG("REG_GNPTXFIFO_SIZE :\n");
	DBG_HEX(REG_GNPTXFIFO_SIZE);

	/* txfifo1 size */
	start_addr += fifo_size;
	fifo_szie = 128;		/* (7 + 1)*(64/4) fix*/
	REG_DIEPTXF(1)	= (fifo_size << 16) | start_addr;
	DBG("tx fifo 1 size:");
	DBG_HEX((fifo_size << 16) | start_addr);
	DBG("REG_DIEPTXF(1) :\n");
	DBG_HEX(REG_DIEPTXF(1));

	/* txfifo2 size */
	start_addr += fifo_size;
	fifo_szie = 1024;		/* (7 + 1)*(512/4) fix*/
	REG_DIEPTXF(2)	= (fifo_size << 16) | start_addr;
	DBG("tx fifo 2 size:");
	DBG_HEX((fifo_size << 16) | start_addr);
	DBG("REG_DIEPTXF(2):\n");
	DBG_HEX(REG_DIEPTXF(2));

	/*ep info size*/
	start_addr += fifo_size;	/*3576*/
	fifo_size = REG_GHW_CFG3 >> 16;
	REG_GDFIFO_CFG = (start_addr << 16) | gdfifocfg;
	if (start_addr > gdfifocfg) {
		DBG("error we used too many fifo");
	}

	dwc_otg_flush_tx_fifo(0x10);
	dwc_otg_flush_rx_fifo();
}

static void dwc_otg_device_init(void)
{
	DBG("bootrom device init!!!!\n");

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

void start_in_transfer(struct usb_ep *usb_ep)
{
	struct usb_request *req = usb_ep->req;
	u32 epnum = EP_NUM(usb_ep);
	u32 xfer_len = req->length - req->actual;

	DBG("handle_in_transfer ,epnum:");
	DBG_DEC(epnum);

	if (REG_DIEP_CTL(epnum) & DEP_ENA_BIT) {
		DBG("handle in transfer when endpoint enable\n");
		dwc_stop_in_transfer(epnum);
	}

	if (xfer_len) {
		DBG("no zero packt size :");
		DBG_HEX(xfer_len);
		int pktcnt;
		/*We assume not many data with in transfer*/
		req->xfer_size = xfer_len;
		if (epnum == 0 && xfer_len >= 127) {
			req->xfer_size = usb_ep->mps;
		}
		pktcnt = (req->xfer_size + usb_ep->mps - 1)/usb_ep->mps;
		REG_DIEP_SIZE(epnum) = (pktcnt << DXEPSIZE_PKTCNT_BIT) | req->xfer_size;
		REG_DIEP_CTL(epnum) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
		REG_DIEP_EMPMSK |= (1 << epnum);
	} else {
		DBG("zero packet\n");
		req->xfer_size = 0;
		REG_DIEP_SIZE(epnum) = (1 << DXEPSIZE_PKTCNT_BIT);
		REG_DIEP_CTL(epnum) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
	}
}

void start_out_transfer(struct usb_ep *usb_ep)
{
	struct usb_request *req = usb_ep->req;
	u32 epnum = EP_NUM(usb_ep);
	u32 xfer_len = req->length - req->actual;
	u32 dosize = 0

	DBG("handle_out_transfer ,epnum:");
	DBG_DEC(epnum);

	if (xfer_len) {
		DBG("no zero packet size:");
		DBG_HEX(xfer_len);
		if (xfer_len >= usb_ep->mps)
			req->xfer_size = usb_ep->mps;
		else
			req->xfer_size = xfer_len;
		dosize = (1 << DXEPSIZE_PKTCNT_BIT) | req->xfer_size;
		if (!epnum)
			dosize |= DOEPSIZE0_SUPCNT_3;
		REG_DOEP_SIZE(epnum) = dosize;
		REG_DOEP_CTL(epnum) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
	} else {
		DBG("zero packet");
		req->xfer_size = 0;
		dosize = (1 << DXEPSIZE_PKTCNT_BIT);
		if (!epnum)
			dosize |= DOEPSIZE0_SUPCNT_3;
		REG_DOEP_SIZE(epnum) = dosize;
		REG_DOEP_CTL(epnum) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
	}
}

void inline start_out_status(struct usb_ep * ep0out) {
	ep0out->req->length = 0;
	ep0out->req->actual = 0;
	usb_ep->complete = NULL;
	start_out_transfer(usb_ep);
}

void inline start_in_status(struct usb_ep * ep0in) {
	ep0in->req->length = 0;
	ep0in->req->actual = 0;
	usb_ep->complete = NULL;
	start_in_transfer(usb_ep);
}

int write_tranfer_fifo(struct usb_ep *usb_ep)
{
	struct usb_request *req = usb_ep->req;
	u32 dwords = (status->xfer_size + 3)/4;
	u32 epnum = EP_NUM(usb_ep);
	u32 txstatus = 0;
	u32 timeout = 0xfffff;
	u32 intsts = 0;
	int i,breakout = 3;

	DBG("xfer size = :");
	DBG_DEC(status->xfer_size);
	do {
		txstatus = REG_DIEP_TXFSTS(epnum);
		if (!(timeout--)) {
			intsts = REG_GINT_STS;
			if (intsts & GINTSTS_USB_RESET) {
				DBG("WARN: time out host reset\n");
				return -1;
			}
			timeout = 0xfffff;
			if (txstatus == 0) {
				if (breakout--)
					xudelay(1);
				else {
					DBG("May be some error happen\n");
					return -1;
				}
			}
		}
	} while (txstatus < dwords);

	for (i = 0; i < dwords; i++)
		REG_EP_FIFO(epnum) = REG32((u32*)(req->buf)+ req->actual + i);

	req->actual += req->xfer_size;

	return 0;
}

static void handle_start_frame_intr(void)
{
	REG_GINT_STS = GINTSTS_START_FRAM;
	return;
}

void dwc_phy_init(void)
{
#ifdef FPGA_TEST
	DBG("bootrom cpm init!!!!\n");

	int clk = jz_extal/1000000;

	/*Enable dwc clk gate*/
	REG_CPM_CLKGR1  &= ~(1 << 8);

	/*Config usb phy clk*/
	REG_CPM_USBPCR1 &= ~(0x1f << 23);
	REG_CPM_USBPCR1 |= ((0x2 << 26)|(0x1 << 31));
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

	/*phy mode device only*/
	REG_CPM_USBPCR &= ~(1 << 31);
	REG_CPM_USBPCR |= USBPCR_VBUSVLDEXT;

	/*power on reset*/
	REG_CPM_USBPCR |= USBPCR_POR;
	xudelay(30);
	REG_CPM_USBPCR &= ~USBPCR_POR;
	xudelay(300);

	/*select 16bit phy*/
	REG_CPM_USBPCR1 |= (1 << 19);

	/*enable otg phy*/
	REG_CPM_OPCR |=  OPCR_OTGPHY_ENABLE;
#endif /*FPGA_TEST*/
}


void usb_init(struct usb_ep *ep,
		struct usb_request *req,
		struct dwc2 *dwc)
{
	int i = 0;


	/*dwc udw refer for pipe :control 0,bulk2 in, bulk1 out, int 1 in*/
	memset(ep,0,sizeof(MAX_EP_NUM*sizeof(struct usb_ep)));
	memset(req,0,sizeof(MAX_REQ_NUM*sizeof(struct usb_request)));

	/*ep init*/
	for (i = 0; i < DWC_MAX_REQ_NUM; i++) {
		req[i].is_used = false;
		req[i].need_zpkt = false;
	}

	ep[0].flags = (EP_USED|USB_ENDPOINT_XFER_CONTROL);
	ep[0].epaddr = 0x00;
	req[0].is_used = true;
	ep[0].req = &req[0];
	ep[0].mps = 64;
	ep[0].mps_limit = 64;

	ep[1].flags = (EP_USED|USB_ENDPOINT_XFER_CONTROL);
	ep[1].epaddr = 0x80;
	req[1].is_used = true;
	ep[1].req = &req[1];
	ep[1].mps = 64;
	ep[1].mps_limit = 64;

	ep[2].flags = USB_ENDPOINT_XFER_INT;
	ep[2].epaddr = 0x81;
	ep[2].mps_limit = 64;

	ep[3].flags = USB_ENDPOINT_XFER_BULK;
	ep[3].epaddr = 0x01;
	ep[2].mps = 512;
	ep[2].mps_limit = 512;

	ep[4].flags = USB_ENDPOINT_XFER_BULK;
	ep[4].epaddr = 0x82;
	ep[4].mps_limit = 512;
	ep[4].mps = 512;

	/*gadget init*/
	dwc->gadget.ep0out = &ep[0];
	dwc->gadget.next = &ep[0];
	dwc->gadget.ep0in = &ep[1];
	ep[0].next = &ep[1];
	dwc->gadget.ep_queue = usb_ep_queue;
	dwc->gadget.ep_enable = usb_ep_enable;
	dwc->gadget.get_ep = usb_ep_search;
	dwc->gadget.alloc_request = usb_alloc_request;
	dwc->gadget.pullup = usb_pull_up;

	dwc->ep_attr = ep_attr;
	dwc->req_attr = req_attr;
	dwc->usb_reset_flag = false;
	dwc->speed = USB_SPEED_HIGH;
	dwc->ep0_state = EP0_SETUP;

	return;
}

void dwc_udc_init(void)
{
	/*dwc core init*/
	dwc_otg_core_init();

	/*dwc fifo allocate*/
	dwc_fifo_allocate();

	/*dwc device init*/
	dwc_otg_device_init();

	return;
}

static int handle_early_suspend_intr(void)
{
	u32 dsts = REG_OTG_DSTS;

	DBG("Handle early suspend intr.\n");
	REG_GINT_STS = GINTSTS_USB_EARLYSUSPEND;
	if (dsts & DSTS_ERRATIC_ERROR) {
		/*handle babble conditional,software reset*/
		REG_OTG_DCTL |= DCTL_SOFT_DISCONN;
		dwc_otg_core_reset();
		return -EAGAIN;
	}
	return 0;
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
	REG_DOEP_CTL(0) |= DEP_EP0_MPS_64 | DEP_CLEAR_NAK | DEP_ENA_BIT;

	REG_GINT_STS = GINTSTS_USB_RESET;

	return;
}	/* handle_reset_intr */

static void handle_enum_done_intr(struct dwc2 *dwc)
{
	u32 dsts = REG_OTG_DSTS;

	dwc->usb_reset_flag = 1;

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

	/*cfg ep0 in*/
	REG_DIEP_CTL(0) |= DEP_EP0_MPS_64 | DEP_IN_FIFO_SEL(0);
	dwc->gadget.ep0in->mps = 64;
	dwc->gadget.ep0in->flags |= EP_ACTIVE;

	/*cfg ep0 out*/
	dwc->gadget.ep0out->mps = 64;
	dwc->gadget.ep0out->flags |= EP_ACTIVE;

	REG_GINT_MASK |= GINTSTS_RXFIFO_NEMPTY | GINTSTS_IEP_INTR |
		GINTSTS_OEP_INTR | GINTSTS_START_FRAM;
	REG_GINT_STS = GINTSTS_ENUM_DONE;

	return;
}

void handle_rxfifo_nempty(struct dwc2 *dwc)
{
	u32 rxsts_pop = REG_GRXSTS_POP;
	u16 count = (rxsts_pop & GRXSTSP_BYTE_CNT_MASK) >> GRXSTSP_BYTE_CNT_BIT;
	u32 epnum = (rxsts_pop & 0xf);
	struct usb_ep *usb_ep = NULL;
	int i = 0;

	for (i = 0; i < MAX_EP_NUM; i++) {
		if (!EP_IS_USED(&dwc->ep_attr[i]))
			continue;
		if (EP_IS_IN(&dwc->ep_attr[i]))
			continue;
		if (EP_NUM(&dwc->ep_attr[i]) == epnum)
			usb_ep = &dwc->ep_attr[i];
	}

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
		dwc_read_out_packet(usb_ep ,count, epnum);
		break;
	case GRXSTSP_PKSTS_SETUP_COMP:
		DBG("GRXSTSP_PKSTS_SETUP_COMP.\n");
		break;
	case GRXSTSP_PKSTS_SETUP_RECV:
		DBG("GRXSTSP_PKSTS_SETUP_RECV.\n");
		((u32 *)dwc->ctrl_req)[0] = REG_EP_FIFO(epnum);
		((u32 *)dwc->ctrl_req)[1] = REG_EP_FIFO(epnum);
		break;
	default:
		break;
	}

	REG_GINT_STS = GINTSTS_RXFIFO_NEMPTY;
	REG_GINT_MASK |= GINTSTS_RXFIFO_NEMPTY;
	return;
}

void start_setup_transfer(void)
{
	REG_DOEP_SIZE(0) = DOEPSIZE0_SUPCNT_3;
	REG_DOEP_CTL(0) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
	return;
}

void dwc_ep0_stall(int is_in)
{
	DBG("ep0 stall\n");
	/*stall ep0*/
	if (is_in) {
		REG_DIEP_SIZE(0) = 0;
		REG_DIEP_CTL(0) |= DEP_SET_STALL;
	} else {
		REG_DOEP_CTL(0) |= DEP_SET_STALL;
	}

	/*restart ep0 out*/
	mdwc.ep0_state = EP0_SETUP;
	start_setup_transfer();
}

int handle_setup_packet(struct usb_ep *usb_ep)
{
	struct dwc2 *dwc = &m_dwc;
	struct usb_request *req = NULL;
	u32 usb_stall = 0;
	int epnum = EP_NUM(usb_ep);
	DBG("Handle setup done inter.\n");

	if (!usb_ep && (ep_num = EP_NUM(usb_ep))) {
		DBG("handle set up error\n");
		return 0;
	}

	REG_DOEP_INT(0) = DEP_SETUP_PHASE_DONE;
	REG_DOEP_INT(0) = DEP_B2B_SETUP_RECV;

	if ((dwc->ctrl_req.bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD) {
		/* standard device request handle */
		DBG("Standard_request : ");
		switch((word1 >> 8) & 0xff) {
		case USB_REQ_SET_ADDRESS:
			addr = word1 >> 16;
			REG_OTG_DCFG &= ~DCFG_DEV_ADDR_MASK;
			REG_OTG_DCFG |= addr << DCFG_DEV_ADDR_BIT;
			DBG("SET_ADDRESS. Address = ");
			DBG_HEX(addr);
			usb_ep->req->length = 0;
			usb_ep->req->actual = 0;
			start_in_transfer(usb_ep);
			break;

		case USB_REQ_GET_STATUS:
			DBG("GET_STATUS\n");
			switch (word1 & 0x1f) {
			case 0x2:	/*endpoint*/
			case 0x1:	/*interface*/
				if (!(REG_DOEP_CTL(1) & USB_ACTIVE_EP)) {
					usb_ep->req->length = 0;
					usb_stall = 1;
					break;
				}
			case 0x0:	/*device*/
				usb_ep->req->length = 2;
				usb_ep->req->actual = 0;
				usb_ep->req->buf = &status_map;
				usb_ep->complete = NULL;
				start_in_transfer(usb_ep);
				break;
			default:
				usb_stall = 1;
				break;
			}
			break;
		default:
			if (dwc->gadget.setup(usb_ep,&dwc->ctrl_req))
				usb_stall = 1;
			break;
		}
	} else {
		if (dwc->gadget.setup(usb_ep,&dwc->ctrl_req))
			usb_stall = 1;
	}

	if (usb_stall)
		dwc_ep0_stall(1);

	if ((dwc->ctrl_req.wLength)) {
		dwc->ep0_state = EP0_DATA;
	} else {
		dwc->ep0_state = EP0_STATUS;
		start_setup_transfer();
	}

	return 0;
}


void handle_out_xfer_complete(struct usb_ep *usb_ep)
{
	struct usb_request *req;
	struct dwc2 *dwc = &mdwc;
	u32 epnum = 0;
	u32 rev_size = 0;

	REG_DOEP_INT(epnum) = DEP_XFER_COMP;

	if (!usb_ep)
		return;

	epnum = EP_NUM(usb_ep);

	if (epnum) {
		rev_size = REG_DOEP_SIZE(epnum) & 0x7ffff;
		if (rev_size || req->actual >= req->length) {
			/*short packet receive or complet transfer*/
			if (usb_ep->req->complete)
				usb_ep->req->complete(usb_ep,usb_ep->req);
		} else
			start_out_transfer(usb_ep);
	} else {
		/*if (dwc->ep0_state == EP0_SETUP) {	yan zheng complex zai setup done zhi qian*/
		if (dwc->ep0_state == EP0_DATA) {
			struct usb_ep *ep0in = dwc->gadget.ep0in;
			if (REG_DOEP_INT(0) & DEP_SETUP_OUT_DATA_DONE)
				REG_DOEP_INT(0) & DEP_SETUP_OUT_DATA_DONE;
			dwc->ep0_state = EP0_STATUS;
			if (usb->req->complete)
				usb_ep->req->complete(usb_ep,usb_ep->req);
			start_setup_transfer();
			/*we assume gadget must support this out data format*/
			start_in_status(ep0in);
		} else if (dwc->ep0_state == EP0_STATUS) {
			dwc->ep0_state = EP0_SETUP;
			start_setup_transfer();
		}
	}

	return;
}

void handle_outep_intr(struct dwc2 *dwc)
{
	u32 ep_intr, intr;
	int epnum = 0;
	int damask = ((1 << (DWC_MAX_OUT_EP + 1)) - 1);

	ep_intr = (REG_OTG_DAINT >> 16) & damask;
	while (ep_intr) {
		struct usb_ep *usb_ep = NULL;
		if (ep_intr & 0x1) {
			DBG("Handle out epnum:");
			DBG_DEC(epnum);

			for (i = 0; i < MAX_EP_NUM; i++) {
				if (!EP_IS_USED(&dwc->ep_attr[i]))
					continue;
				if (EP_IS_IN(&dwc->ep_attr[i]))
					continue;
				if (EP_NUM(&dwc->ep_attr[i]) == epnum)
					usb_ep = &dwc->ep_attr[i];
			}

			intr = REG_DOEP_INT(epnum);

			if (intr & DEP_XFER_COMP)
				handle_out_xfer_complete(usb_ep);

			if (intr & DEP_SETUP_PHASE_DONE)
				handle_setup_packet(usb_ep);

			if (intr & DEP_EPDIS_INT) {
				DBG("out disabled\n");
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

void handle_in_xfer_complete(struct usb_ep *usb_ep)
{
	struct usb_request *req = NULL;
	struct dwc2 *dwc = &m_dwc;
	u32 epnum = 0;
	DBG("in xfer complex.\n");

	if (!usb_ep)
		return;

	epnum = EP_NUM(usb_ep);
	REG_DIEP_INT(epnum) = DEP_XFER_COMP;
	req = usb_ep->req;

	if (epnum) {
		/*We don not support too many in transfer on bulkin*/
		if (usb_ep->req->complete)
			usb_ep->req->complete(usb_ep,usb_ep->req);
	} else {
		if (dwc->ep0_state == EP0_DATA) {
			struct usb_ep *ep0out = dwc->gadget.ep0out;
			dwc->ep0_state = EP0_STATUS;
			start_out_status(&dwc->gadget.ep0out);
		} else if (dwc->ep0_state == EP0_STATUS)
			dwc->ep0_state = EP0_SETUP;
	}
	return;
}

void handle_in_xfer_timeout(struct usb_ep *usb_ep)
{
	struct usb_request *req = NULL;
	struct dwc2 *dwc = &m_dwc;
	u32 epnum = 0;
	DBG("in timeout\n");
	if (!usb_ep)
		return;
	
	epnum = EP_NUM(usb_ep);
	REG_DIEP_INT(epnum) = DEP_TIME_OUT;
	req = usb_ep->req;

	if (!epnum && dwc->ep0_state == EP0_DATA) {
		struct usb_ep *ep0out = dwc->gadget.ep0out;
		dwc->ep0_state = EP0_STATUS;
		ep0out->req->length = 0;
		ep0out->req->actual = 0;
		start_out_transfer(&dwc->gadget.ep0out);
	}

	return;
}

void handle_in_fifo_empty(struct usb_ep *usb_ep)
{
	struct usb_request *req = NULL;
	struct dwc2 *dwc = &m_dwc;
	u32 epnum = 0;
	if (!usb_ep)
		return;
	epnum = EP_NUM(usb_ep);
	if (REG_DIEP_EMPMSK & (1 << epnum)) {
		DBG("in fifo empty.\n");
		REG_DIEP_EMPMSK &= ~(1 << epnum);
		write_tranfer_fifo(usb_ep);
	}

	REG_DIEP_INT(epnum) = DEP_TXFIFO_EMPTY;
}

void handle_inep_intr(struct dwc2* dwc)
{
	u32 ep_intr;
	int epnum = 0;
	int damask = ((1 << (DWC_MAX_IN_EP + 1)) - 1);

	ep_intr = (REG_OTG_DAINT & damask);
	while(ep_intr) {
		if (ep_intr & 0x1) {
			struct usb_ep *usb_ep = NULL;
			u32 intr = REG_DIEP_INT(epnum);
			DBG("Handle in epnum:");
			DBG_DEC(epnum);

			for (i = 0; i < MAX_EP_NUM; i++) {
				if (!EP_IS_USED(&dwc->ep_attr[i]))
					continue;
				if (!EP_IS_IN(&dwc->ep_attr[i]))
					continue;
				if (EP_NUM(&dwc->ep_attr[i]) == epnum)
					usb_ep = &dwc->ep_attr[i];
			}

			if (intr & DEP_XFER_COMP)
				handle_in_xfer_complete(usb_ep);

			if (intr & DEP_TIME_OUT)
				handle_in_xfer_timeout(usb_ep);

			if (intr & DEP_TXFIFO_EMPTY)
				handle_in_fifo_empty(usb_ep);

			if (intr & DEP_EPDIS_INT) {
				DBG("in disabled\n");
				REG_DIEP_INT(epnum) = DEP_EPDIS_INT;
			}

			if (intr & DEP_NAK_INT)	{
				DBG("in nak\n");
				REG_DIEP_INT(epnum) = DEP_NAK_INT;
			}

			if (intr & DEP_NYET_INT) {
				DBG("out nak\n");
				REG_DIEP_INT(epnum) = DEP_NYET_INT;
			}
		}
		epnum++;
		ep_intr >>= 1;
	}
	return ;
}

struct usb_ep *usb_ep_search(struct usb_endpoint_descriptor *desc)
{
	struct dwc2 *dwc = &m_dwc;
	struct usb_ep *usb_ep = NULL;
	u32 desc_type = desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;

	for (i = 2; i <= MAX_EP_NUM; i++) {
		if (EP_IS_USED(&dwc->ep_attr[i]))
			continue;
		if (desc->bEndpointAddress != dwc->ep_attr[i].epaddr)
			continue;
		if (desc_type == EP_TYEP(usb_ep)) {
			usb_ep = &dwc->ep_attr[i];
			usb_ep->flags |= EP_USED;
			break;
		}
	}

	return usb_ep;
}

struct usb_request *usb_alloc_request(void)
{
	struct dwc2 *dwc = &m_dwc;
	struct usb_request *req = NULL;

	for (i = 2; i <= MAX_REQ_NUM; i++) {
		if (dwc->req_attr[i].is_used == false) {
			req = &dwc->req_attr[i];
			req->is_used = true;
			break;
		}
	}
	return req;
}

int usb_ep_queue(struct usb_ep *usb_ep, struct usb_request *req)
{
	if (!usb_ep || !req)
		return -ENODEV;

	usb_ep->req = req;
	usb_ep->flags |= EP_BUSY;
	req->xfer_size = 0;
	req->actual = 0;

	if (EP_IS_IN(usb_ep))
		start_in_transfer(struct usb_ep *usb_ep);
	else
		start_out_transfer(struct usb_ep *usb_ep);

	return 0;
}

int usb_ep_enable(struct usb_ep *usb_ep,
		struct usb_endpoint_descriptor *desc)
{
	int mps = le16_to_cpu(desc->wMaxPacketSize);
	int epnum = EP_NUM(usb_ep);
	int ep_type = EP_TYEP(usb_ep);
	int is_in = EP_IS_IN(usb_ep);

	/*ep 0 is alreay enable*/
	if (!epnum)
		return 0;

	DBG("enable ep");
	DBG_HEX(usb_ep->epaddr);

	usb_ep->flags |= EP_ACTIVE;
	usb_ep->mps = mps;

	if (is_in) {
		REG_DIEP_INT(epnum) = 0xffff;
		REG_DIEP_CTL(epnum) = USB_ACTIVE_EP | DEP_TYPE(ep_type) | mps | DEP_RESET_DATA0 |
			DEP_IN_FIFO_SEL(epnum);
		REG_DIEP_SIZE(epnum) = 0;
		REG_DAINT_MASK |= DAINT_IEP_MASK(epnum);
	} else {
		REG_DOEP_INT(epnum) = 0xffff;
		REG_DOEP_CTL(epnum) = USB_ACTIVE_EP | DEP_TYPE(ep_type) | mps | DEP_RESET_DATA0;
		REG_DOEP_SIZE(epnum) = 0;
		REG_DAINT_MASK |= DAINT_OEP_MASK(epnum);
	}

	return 0;
}

int usb_pull_up(int pullup)
{
	if (pullup) {

	} else {

	}
}

int udc_init(int (*bind)(struct usb_gadget *gadget))
{
	int ret = 0;
	DBG("dwc udc init\n");

	/*dump dwc ghw cfg*/
	DBG_HEX(REG_GHW_CFG1);
	DBG_HEX(REG_GHW_CFG2);
	DBG_HEX(REG_GHW_CFG3);
	DBG_HEX(REG_GHW_CFG4);

	/*usb driver init*/
	usb_init(ep_attr,req_attr,&m_dwc);

	ret = bind(&m_dwc.gadget);
	if (ret) {
		DBG("Gadget driver bind failed\n");
		return ret;
	}

	/*cpu freq init*/
	set_cpufreq(BOOTROM_CPUFREQ);

	/*phy init*/
	dwc_phy_init();

	/*dwc udc init*/
	dwc_udc_init();

	/*start timeout counter*/
	xtimeout(0, 1);

	DBG("dwc udc init ok\n");

	return ret;
}

void handle_suspend_intr(void)
{
	DBG("handle_suspend_intr\n");
}

int usb_interrupt_handler(void)
{
	u32 intsts;
	status_map = 0;
	int ret = 0;

	if (!(usb_reset_flag)) {
		/* Reset doesn't receive from host */
		if (!xtimeout(10 * 1000 * 1000, 0)) {
			DBG("*! USB idle more than 10s !*\n\n");
			return -ETIMEDOUT;
		}
	} else {
		pmon_clear_cnt();
		pmon_stop();
	}

	intsts = REG_GINT_STS;

	if ((intsts & GINTSTS_USB_EARLYSUSPEND) & REG_GINT_MASK) {
		ret = handle_early_suspend_intr(void);
		if (ret)
			return ret;
	}

	if ((intsts & GINTSTS_START_FRAM) & REG_GINT_MASK)
		handle_start_frame_intr();

	if ((intsts & GINTSTS_USB_SUSPEND) & REG_GINT_MASK)
		handle_suspend_intr(void);

	if ((intsts & GINTSTS_USB_RESET) & REG_GINT_MASK)
		handle_reset_intr();

	if ((intsts & GINTSTS_ENUM_DONE) & REG_GINT_MASK)
		handle_enum_done_intr(&m_dwc);

	if ((intsts & GINTSTS_RXFIFO_NEMPTY) & REG_GINT_MASK)
		handle_rxfifo_nempty(&m_dwc);

	if ((intsts & GINTSTS_OEP_INTR) & REG_GINT_MASK)
		handle_outep_intr(&m_dwc);

	if ((intsts & GINTSTS_IEP_INTR) & REG_GINT_MASK)
		handle_inep_intr(&m_dwc);

	return 0;
}
