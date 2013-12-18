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


extern void serial_puts(const char *);
extern void serial_put_dec(unsigned int);
extern void serial_put_hex(unsigned int);

//#define BOOTROM_TEST 1		/*use 4780 grus board test 4780B usb boot code*/
//#define FPGA_TEST 1			/*fpga platform test*/

#define BOOTROM_CPUFREQ	(200)
static int volatile fout = BOOTROM_CPUFREQ;	/*cpm fout : cpufreq*/

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

static inline int DEP_EP_MAXPKT_CNT(int n)
{
	int v = 0;
	if (n)
		v = 1023;
	else
		v = 1;
	return v;
}

void flush_dcache_all(void)
{
	u32 addr;

	for (addr = K0BASE; addr < K0BASE + CFG_DCACHE_SIZE;
			addr += CFG_CACHELINE_SIZE) {
		asm volatile (
				" cache %0, 0(%1)\n\t"
				:
				: "I" (Index_Writeback_Inv_D), "r"(addr));
	}

	asm volatile ("sync");
}

/*-------------------------------------------------------------------------*/
static void dwc_read_out_packet(USB_STATUS *status, u16 count,u32 epnum)
{
	int dwords = (count+3) / 4;
	int i;

	for (i = 0; i < dwords; i++)
		REG32((unsigned int *)(status->addr) + i) = REG_EP_FIFO(1);

	status->addr += count;
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
		if (cnt++ > 10000 ) {
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
	/* This value caculate for 30M PHY and 30M AHB,our AHB is higher than 60M
	 * so there is no problem*/
	if (((REG_GHW_CFG2 >> 6) & 0x3) == 1) {
		REG_GUSB_CFG &= ~USBCFG_TRDTIME_MASK;
		if (dwc_get_utmi_width() == 0) {
			DBG("8BIT UTMI+.\n");
			REG_GUSB_CFG &= ~(USBCFG_16BIT_PHY);
			REG_GUSB_CFG |= USBCFG_TRDTIME(9);
			REG_CPM_USBPCR1 &= ~(1 << 19);
		} else if (dwc_get_utmi_width() == 1) {
			DBG("16BIT UTMI+.\n");
			REG_GUSB_CFG |= USBCFG_16BIT_PHY;
			REG_GUSB_CFG |= USBCFG_TRDTIME(5);
			REG_CPM_USBPCR1 |= (1 << 19);
		} else if (dwc_get_utmi_width() == 2) {
			DBG("8BIT or 16BIT UTMI+.\n");

			if (UTMI_PHY_WIDTH == 8) {
				/* 8bit */
				REG_GUSB_CFG &= ~(USBCFG_16BIT_PHY);
				REG_GUSB_CFG |= USBCFG_TRDTIME(9);
				REG_CPM_USBPCR1 &= ~(1 << 19);
			} else  {
				/* 16bit */
				REG_GUSB_CFG |= USBCFG_16BIT_PHY;
				REG_GUSB_CFG |= USBCFG_TRDTIME(5);
				REG_CPM_USBPCR1 |= (1 << 19);
			}
		}
	}  else
		DBG("Unkonwn PHY TYPE?.\n");
}

static void dwc_otg_core_init(void)
{
	u32 gusbcfg;

	DBG("dwc_otg_core_init :\n");
	/*HNP SRP not support , usb2.0 , utmi+*/
	gusbcfg = REG_GUSB_CFG;
	gusbcfg &= ~( USBCFG_HNP_EN | USBCFG_SRP_EN | USBCFG_PHY_SEL_USB1  | USBCFG_PHY_INF_UPLI);
	REG_GUSB_CFG = gusbcfg;

	REG_OTG_DCTL &= ~DCTL_SOFT_DISCONN;		// Soft Disconnect-> 0(normal mode)
	dwc_otg_select_phy_width();

	dwc_otg_core_reset();
	REG_GAHB_CFG = 1 | 1 << 7;			// Slave mode and Mask all intr
	REG_GINT_MASK = 0;			// Mask Intr
}

static void dwc_fifo_allocate(void)
{
	u16 epinfobase, gdfifocfg;
	REG_GRXFIFO_SIZE = DEP_RXFIFO_SIZE;
	DBG("DEP_RXFIFO_SIZE: \n");
	DBG_HEX(DEP_RXFIFO_SIZE);
	DBG("REG_GRXFIFO_SIZE :\n");
	DBG_HEX(REG_GRXFIFO_SIZE);
	REG_GNPTXFIFO_SIZE = (DEP_NPTXFIFO_SIZE << 16) | DEP_RXFIFO_SIZE;
	DBG("(DEP_NPTXFIFO_SIZE << 16) | DEP_RXFIFO_SIZE :\n");
	DBG_HEX((DEP_NPTXFIFO_SIZE << 16) | DEP_RXFIFO_SIZE);
	DBG("REG_GNPTXFIFO_SIZE :\n");
	DBG_HEX(REG_GNPTXFIFO_SIZE);
	REG_GDTXFIFO_SIZE = (DEP_DTXFIFO_SIZE << 16) | (DEP_RXFIFO_SIZE + DEP_NPTXFIFO_SIZE);
	DBG(" (DEP_DTXFIFO_SIZE << 16) | (DEP_RXFIFO_SIZE + DEP_NPTXFIFO_SIZE)");
	DBG_HEX( (DEP_DTXFIFO_SIZE << 16) | (DEP_RXFIFO_SIZE + DEP_NPTXFIFO_SIZE));
	DBG("REG_GDTXFIFO_SIZE :\n");
	DBG_HEX(REG_GDTXFIFO_SIZE);

	gdfifocfg = REG_GHW_CFG3 >> 16;
	epinfobase = (REG_GDTXFIFO_SIZE & 0xffff) + (REG_GDTXFIFO_SIZE >> 16);
	REG_GDFIFO_CFG = (epinfobase << 16) | gdfifocfg;

	dwc_otg_flush_tx_fifo(0x10);
	dwc_otg_flush_rx_fifo();
}

static void dwc_otg_device_init(void)
{
	DBG("bootrom device init!!!!\n");
	/* clear irq and mask all ep intr */
	REG_DOEP_MASK = 0;
	REG_DIEP_MASK = 0;
	REG_OTG_DAINT = 0xffffffff;
	REG_DAINT_MASK = 0;

	/* disable all in and out ep */
	REG_DIEP_INT(0) = 0xff;
	REG_DOEP_INT(0) = 0xff;
	REG_DIEP_INT(1) = 0xff;
	REG_DOEP_INT(1) = 0xff;
	REG_GINT_STS = 0xffffffff;

	dwc_fifo_allocate();

	REG_OTG_DCFG = DCFG_HANDSHAKE_STALL_ERR_STATUS;	// Slave mode and High speed to enum
	REG_OTG_DCTL &= ~DCTL_SOFT_DISCONN;				// Soft Disconnect-> 0(normal mode)
	REG_OTG_DCTL |= DCTL_NAK_ON_BBLE;
	REG_OTG_DSTS = GINTSTS_START_FRAM;

	REG_GAHB_CFG |= 1;
	REG_GINT_MASK = GINTSTS_USB_SUSPEND | GINTSTS_USB_RESET | GINTSTS_ENUM_DONE | GINTSTS_START_FRAM |
		GINTSTS_USB_EARLYSUSPEND;

	return;
}

void  handle_in_transfer(USB_STATUS *status, int epnum)
{
	u16 pktcnt;
	u32 max_pkt_size = 512;
	u32 max_pkt_cnt = 1;

	if (REG_DIEP_CTL(epnum) & DEP_ENA_BIT) {
		DBG("handle in transfer when endpoint enable\n");
		DBG("epnum:");
		DBG_DEC(epnum);
		dwc_stop_in_transfer(epnum);
	}

	DBG("handle_in_transfer : epnum:");
	DBG_DEC(epnum);
	DBG("status->length:");
	DBG_DEC(status->length);

	if (status->length) {
		max_pkt_size = DEP_EP_MAXPKT_SIZE(epnum);
		max_pkt_cnt = DEP_EP_MAXPKT_CNT(epnum);

#ifdef HAVE_IN_ZERO_PACKET
		if (status->length%max_pkt_size == 0)
			status->need_zpkt = 1;
		else
			status->need_zpkt = 0;
#endif

		if (status->length > max_pkt_size * max_pkt_cnt)
			status->xfer_size = max_pkt_size * max_pkt_cnt;
		else
			status->xfer_size = status->length;


		pktcnt = (status->xfer_size + max_pkt_size - 1)/max_pkt_size;

		status->xfer_count = 0;

		if (epnum == 0) {
			REG_DIEP_INT(0) = DEP_TIME_OUT;
			REG_DIEP_MASK |= DEP_TIME_OUT;
			//REG_DOEP_INT(0) = DEP_OUTTOKEN_RECV_EPDIS;
			//REG_DOEP_MASK |= DEP_OUTTOKEN_RECV_EPDIS;
		}

		DBG("no zero packt REG_DIEP_SIZE :");
		DBG_HEX((pktcnt << DXEPSIZE_PKTCNT_BIT | status->xfer_size));
		REG_DIEP_SIZE(epnum) = (pktcnt << DXEPSIZE_PKTCNT_BIT) | status->xfer_size;
		REG_DIEP_CTL(epnum) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
		REG_DIEP_EMPMSK |= (1 << epnum);
	} else {
		DBG("zero packet\n");
#ifdef HAVE_IN_ZERO_PACKET
		status->need_zpkt = 0;
#endif
		if (epnum == 0) {
			//REG_DIEP_INT(0) = DEP_INTOKEN_RECV_TXFIFO_EMPTY;
			//REG_DIEP_MASK |= DEP_INTOKEN_RECV_TXFIFO_EMPTY;
		}

		REG_DIEP_SIZE(epnum) = DIEPSIZE0_PKTCNT_BIT;
		REG_DIEP_CTL(epnum) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
	}
}

static void write_tranfer_fifo(USB_STATUS *status, int epnum)
{
	u32 xfersize = 0;
	u32 dwords = 0;
	u32 txstatus = 0;
	u32 timeout = 0xfffff;
	u32 intsts = 0;
	u32 max_pkt_size = DEP_EP_MAXPKT_SIZE(epnum);
	if (status->xfer_size > max_pkt_size)
		xfersize = max_pkt_size;
	else
		xfersize = status->xfer_size;

	dwords = (xfersize + 3) / 4;

	while (dwords) {
		int i,breakout = 0;
		DBG("xfer_size = :");
		DBG_DEC(status->xfer_size);
		do {
			if (!(timeout--)) {
				intsts = REG_GINT_STS;
				if (intsts & GINTSTS_USB_RESET) {
					status->length = 0;
#ifdef HAVE_IN_ZERO_PACKET
					status->need_zpkt = 0;
#endif
					status->xfer_count = 0;
					status->addr = 0;
					DBG("WARN: time out host reset\n");
					return;
				}
				timeout = 0xfffff;
			}
			txstatus = REG_DIEP_TXFSTS(epnum);
			if (txstatus == 0) {
				xudelay(1);
				breakout++;
			}
			if (breakout == 3)
				return;
		} while (txstatus < dwords);

		for (i = 0; i < dwords; i++) {
			REG_EP_FIFO(epnum) = REG32((unsigned int *)(status->addr)+i);
		}
		status->xfer_count += xfersize;
		status->addr += xfersize;
		status->xfer_size -= xfersize;

		if (status->xfer_size > max_pkt_size)
			xfersize = max_pkt_size;
		else
			xfersize = status->xfer_size;

		dwords = (xfersize + 3) / 4;
	}

	return;
}

static void handle_start_frame_intr(void)
{
	REG_GINT_STS = GINTSTS_START_FRAM;
	return;
}

void memclr (int *p, int size)
{
	int i;
	for (i = 0; i < size; i++)
		p[i] = 0;
}

static void handle_early_suspend_intr(int *flag,USB_STATUS *status)
{
	u32 dsts = REG_OTG_DSTS;

	DBG("Handle early suspend intr.\n");
	REG_GINT_STS = GINTSTS_USB_EARLYSUSPEND;
	if (dsts & DSTS_ERRATIC_ERROR) {
		/*handle babble conditional,software reset*/
		dwc_otg_core_init();
		dwc_otg_device_init();
		memclr((int *)status, 2*sizeof(USB_STATUS));
		*flag = 0;
		xtimeout(0, 1);
		xmdelay(1000);
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
	REG_DAINT_MASK = (1 << 0) | (1 << 16);	//open ep0 mask;
	REG_DOEP_MASK = DEP_SETUP_PHASE_DONE | DEP_XFER_COMP | DEP_B2B_SETUP_RECV;
	REG_DIEP_MASK = DEP_XFER_COMP;// | DEP_TIME_OUT;

	/* step3: device init nothing to do */

	/* step4: dfifo dynamic allocated */
	//dwc_fifo_allocate();

	/* step5: Reset Device Address */
	REG_OTG_DCFG &= ~DCFG_DEV_ADDR_MASK;

	/* step6: setup EP0 to receive SETUP packets */
	REG_DOEP_SIZE(0) = DOEPSIZE0_SUPCNT_3 | DOEPSIZE0_PKTCNT_BIT | (8 * 3);

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
	REG_GINT_MASK |= GINTSTS_RXFIFO_NEMPTY | GINTSTS_IEP_INTR | GINTSTS_OEP_INTR;
	REG_DIEP_CTL(0) |= DEP_EP0_MPS_64;
	REG_DOEP_CTL(0) |= DEP_ENA_BIT | DEP_CLEAR_NAK | DEP_EP0_MPS_64;
	REG_GINT_STS = GINTSTS_ENUM_DONE;

	return;
}

static void handle_rxfifo_nempty(USB_STATUS *status)
{
	u16 count;
	u32 rxsts_pop = REG_GRXSTS_POP;
	u32 epnum = 0;

	REG_GINT_MASK &= ~GINTSTS_RXFIFO_NEMPTY;
	switch(rxsts_pop & GRXSTSP_PKSTS_MASK) {
	case GRXSTSP_PKSTS_GOUT_NAK:
		DBG("GRXSTSP_PKSTS_GOUT_NAK.\n");
		break;
	case GRXSTSP_PKSTS_TX_COMP:
		DBG("GRXSTSP_PKSTS_TX_COMP.\n");
		break;
	case GRXSTSP_PKSTS_GOUT_RECV:
		count = (rxsts_pop & GRXSTSP_BYTE_CNT_MASK) >> GRXSTSP_BYTE_CNT_BIT;
		epnum = (rxsts_pop & 0xf);
		DBG("handle_rxfifo_nempty GRXSTSP_PKSTS_GOUT_RECV epnum :");
		DBG_DEC(epnum);
		if (count) {
			DBG("RXFIFO cnt:");
			DBG_HEX(count);
			DBG("status->addr:");
			DBG_HEX((unsigned int)(status[epnum].addr));
			dwc_read_out_packet(&status[epnum], count, epnum);
		}
		break;
	case GRXSTSP_PKSTS_SETUP_COMP:
		DBG("GRXSTSP_PKSTS_SETUP_COMP.\n");
		break;
	case GRXSTSP_PKSTS_SETUP_RECV:
		DBG("GRXSTSP_PKSTS_SETUP_RECV.\n");
		if ((rxsts_pop & 0xf) == 0) {
			u32 i = 0;
			count = (rxsts_pop & GRXSTSP_BYTE_CNT_MASK) >> GRXSTSP_BYTE_CNT_BIT;
			count = count/4;
			while (count--) {
				i = (status[0].back2backnum++)%2;
				status[0].setup_packet[i] = REG_EP_FIFO(0);
			}
		}
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
	u32 tmp = 0;
	REG_DIEP_SIZE(0) = 0;
	REG_DIEP_CTL(0) |= DEP_SET_STALL;
	tmp = REG_DIEP_CTL(0);
	if (tmp & DEP_ENA_BIT)
		REG_DIEP_CTL(0) |= DEP_DISENA_BIT;
	DBG("ep0 stall\n");

	REG_DOEP_SIZE(0) = DOEPSIZE0_SUPCNT_3 | DOEPSIZE0_PKTCNT_BIT | (8 * 3);
	REG_DOEP_CTL(0) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
}

void dwc_set_config(USB_STATUS *status)
{
	int max_pkt_size = DEP_EP_MAXPKT_SIZE(1);

	REG_DOEP_INT(1) = 0xffff;
	REG_DIEP_INT(1) = 0xffff;
	status->xfer_count = 0;
	status->length = 0;

	REG_DIEP_CTL(1) = USB_ACTIVE_EP | DEP_TYPE_BULK | max_pkt_size | DEP_RESET_DATA0 | DEP_IN_FIFO_SEL(1);
	REG_DOEP_CTL(1) = USB_ACTIVE_EP | DEP_TYPE_BULK | max_pkt_size | DEP_RESET_DATA0;
	REG_DIEP_SIZE(1) = 0;
	REG_DOEP_SIZE(1) = (1 << 19) | max_pkt_size;
	REG_DOEP_CTL(1) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
	REG_DAINT_MASK  |=  (1 << 1) | (1 << 17);    //umask ep1 intr;
}

int handle_setup_packet(USB_STATUS *status, int epnum)
{
	u32 i;
	u32 addr;
	u32 word1 = status[0].setup_packet[0];
	u32 word2 = status[0].setup_packet[1];
	u32 usb_stall = 0;

	status[0].back2backnum = 0;
	status[0].length = 0;
	status[0].trans_status = USB_STATU_STAGE;

	if (status[0].back2backnum%2) {
		DBG("WARNING :back2back num is oddly :");
		DBG_DEC(status[0].back2backnum);
	}

	if (word1 & 0x60) {
		/* vendor_request handle */
		DBG("Vendor_request case :");
		switch((word1 >> 8) & 0xff) {
		case EP0_GET_CPU_INFO:
			DBG("EP0_GET_CPU_INFO \n");
			status[0].addr = (u8* )cpu_info_data;
			status[0].length = 8;
			break;

		case EP0_SET_DATA_ADDRESS:
			status[1].addr = (u8 *)((word1 & 0xffff0000) | (word2 & 0xffff));
			DBG("EP0_SET_DATA_ADDRESS. Data Addr = 0x");
			DBG_HEX((u32)status[1].addr);
			break;

		case EP0_SET_DATA_LENGTH:
			status[1].length = (word1 & 0xffff0000) | (word2 & 0xffff);
			DBG("EP0_SET_DATA_LENGTH. Data length = ");
			DBG_DEC(status[1].length);
			break;

		case EP0_FLUSH_CACHES:
			DBG("EP0_FLUSH_CACHE \n");
			flush_dcache_all();		//twxie fixme
			break;

		case EP0_PROG_START1:
			addr = (word1 & 0xffff0000) | (word2 & 0xffff);
			DBG("EP0_PROG_START1. Start addr = 0x");
			DBG_HEX(addr);
			handle_in_transfer(&status[0], epnum);
#if 1
			{
				int timeout = 0xffff;
				while ((~(REG_DIEP_INT(0) & DEP_XFER_COMP)) && (timeout--) );
				if (!timeout)
					DBG("why !!!!!\n");
				REG_DIEP_INT(0) = DEP_XFER_COMP;
				status[0].trans_status = USB_IDLE_STAGE;
				REG_DOEP_SIZE(0) = DOEPSIZE0_SUPCNT_3 | DOEPSIZE0_PKTCNT_BIT | (8 * 3);
				REG_DOEP_CTL(0) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
				status[epnum].length = 0;
				status[epnum].addr = 0;
			}
#endif
#if 0
			DBG("addr :");
			DBG_HEX(addr);
			DBG("value:");
			DBG_HEX(REG32(addr));

			for (i = 26 ; i < 100;i++) {
				DBG("addr :");
				DBG_HEX(addr+(i*4));
				DBG("value");
				DBG_HEX(REG32(addr + (i*4)));
			}
#endif
			DBG("jump addr\n");
			return xfer_d2i(addr, SPL_SIZE);

		case EP0_PROG_START2:
			addr = ((word1 & 0xffff0000)|(word2 & 0xffff));
			DBG("EP0_PROG_START2. Start addr = 0x");
			DBG_HEX(addr);
			handle_in_transfer(&status[0], epnum);
#if 1
			{
				int timeout = 0xffff;
				while ((~(REG_DIEP_INT(0) & DEP_XFER_COMP)) && (timeout--) );
				if (!timeout)
					DBG("why !!!!!\n");
				REG_DIEP_INT(0) = DEP_XFER_COMP;
				status[0].trans_status = USB_IDLE_STAGE;
				REG_DOEP_SIZE(0) = DOEPSIZE0_SUPCNT_3 | DOEPSIZE0_PKTCNT_BIT | (8 * 3);
				REG_DOEP_CTL(0) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
				status[epnum].length = 0;
				status[epnum].addr = 0;
			}
#endif
			{
				void (*func_usb)(unsigned int);
				func_usb = (void (*)(unsigned int)) addr;
				DBG("bootrom start stage 2\n");
				(*func_usb) (addr);
			}
			break;
#if defined (BOOTROM_TEST) || defined(FPGA_TEST)
			/*Just a test mode , we use burntool on windows burn a binary file,
				and use this interface return the burned data for check*/
		case EP0_BOOTROM_TEST:
			DBG("EP0_BOOTROM_TEST\n");
			handle_in_transfer(&status[0],0);
			handle_in_transfer(&status[1],1);
			return 0;
#endif
		default:
			usb_stall = 1;
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
			dwc_set_config(&status[1]);
			break;

		case USB_REQ_GET_DESCRIPTOR:
			DBG("GET_DESCRIPTOR - ");
			status[0].length = word2 >> 16;
			switch(word1 >> 24) {
			case USB_DT_DEVICE:
				DBG("DEVICE. \n");
				status[0].addr = (u8*)device_desc;
				if (status[0].length > USB_DT_DEVICE_SIZE)
					status[0].length = USB_DT_DEVICE_SIZE;// max length of device_desc
				break;

			case USB_DT_CONFIG:
				DBG("CONFIG. \n");
				if ((REG_OTG_DSTS & DSTS_ENUM_SPEED_MASK) == DSTS_ENUM_SPEED_HIGH)
					status[0].addr = (u8 *)hs_desc;
				else
					status[0].addr = (u8 *)fs_desc;
				if (status[0].length > 32)
					status[0].length = 32;// max length of device_desc
				break;

			case USB_DT_STRING:
				DBG("STRING. \n");
				i = (word1 >>16) & 0xff;
				if (i == 1) {
					status[0].addr = (u8 *)string_manufacture;
					status[0].length = 16;
				} else if (i == 2) {
					status[0].addr = (u8 *)string_product;
					status[0].length = 46;
				} else {
					status[0].addr = (u8 *)string_lang_ids;
					status[0].length = 4;
				}
				break;

			case USB_DT_DEVICE_QUALIFIER:
				DBG("DEVICE_QUALIFIER. \n");
				status[0].addr = (u8 *)dev_qualifier;
				if (status[0].length > 10)
					status[0].length = 10;// max length of device_desc
				break;

			default:
				status[0].length = 0;
				usb_stall = 1;
				DBG("Not contain.\n");
				break;
			}
			break;

		default:
			usb_stall = 1;
			DBG("Not contain.\n");
			break;
		}
	}

	if (!usb_stall) {
		if (status[0].length != 0)
			status[0].trans_status = USB_DATA_STAGE;
		handle_in_transfer(&status[0], epnum);
	} else {
		status[0].trans_status = USB_IDLE_STAGE;
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
				if (!(REG_DIEP_CTL(0) & DEP_ENA_BIT)) {
					if (epnum) {
						DBG("out xfer complex .\n");
						REG_DOEP_SIZE(epnum) = (1 << 19) | DEP_EP_MAXPKT_SIZE(1);
						REG_DOEP_CTL(epnum) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
					} else if (status[0].trans_status == USB_STATU_STAGE) {
						DBG("Handle epnum 0 out status  phase xfer complex .\n");
						status[0].trans_status = USB_IDLE_STAGE;
						REG_DOEP_SIZE(0) = DOEPSIZE0_PKTCNT_BIT | (8 * 3);
						REG_DOEP_CTL(0) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
					} else if (status[0].trans_status == USB_DATA_STAGE){
						DBG("Handle epnum 0 out data phase xfer complex. \n");
						status[0].trans_status = USB_STATU_STAGE;
						handle_in_transfer(&status[0],0);
					} else {
						DBG("Handle epnum 0 out setup phase xfer complex .\n");
					}
				}
			}

			if (intr & DEP_SETUP_PHASE_DONE) {
				if (epnum == 0) {
					DBG("Handle epnum 0 setup done inter.\n");
					if (intr & DEP_B2B_SETUP_RECV) {
						DBG("Handle epnum 0 back to back .\n");
						REG_DOEP_INT(epnum) = DEP_B2B_SETUP_RECV;
					}
					REG_DOEP_INT(epnum) = DEP_SETUP_PHASE_DONE;
					handle_setup_packet(status, epnum);
				} else {
					DBG("unkown setup done.\n");
				}
			}

			if (intr & DEP_EPDIS_INT) {
				DBG("out disabled.\n");
				REG_DOEP_INT(epnum) = DEP_EPDIS_INT;
			}
			if (intr & DEP_AHB_ERR) {
				DBG("out AHB error.\n");
				REG_DOEP_INT(epnum) = DEP_AHB_ERR;
			}

			if (intr & DEP_BABBLE_ERR_INT) {
				DBG("out DEP_BABBLE_ERR_INT.\n");
				REG_DOEP_INT(epnum) = DEP_BABBLE_ERR_INT;
			}
#if 0
		/*if you want open DEP_OUTTOKEN_RECV_EPDIS, open all DEP_OUTTOKEN_RECV_EPDIS
		 * The core must stall IN/OUT tokens if, during the Data stage of a control transfer, the host sends
		 * more IN/OUT tokens than are specified in the SETUP packet. In this case, the application must to
		 * enable DIEPINTn.INTknTXFEmp and DOEPINTn.OUTTknEPdis interrupts during the Data stage of the control
		 * transfer, after the core has transferred the amount of data specified in the SETUP packet. Then,
		 * when the application receives this interrupt, it must set the STALL bit in the corresponding
		 * endpoint control register,and clear this interrupt.*/
			if ((intr & DEP_OUTTOKEN_RECV_EPDIS) & REG_DOEP_MASK) {
				DBG("outep DEP_OUTTOKEN_RECV_EPDIS.\n");
				if (!epnum) {
					u32 usb_stall = 1;
					u32 intr_in = REG_DIEP_INT(0);
					if (intr_in & (DEP_XFER_COMP | DEP_TIME_OUT))
						usb_stall = 0;
					if (usb_stall) {
						status[0].length = 0;
						REG_DIEP_MASK &= ~DEP_TIME_OUT;
						REG_DOEP_MASK &= ~DEP_OUTTOKEN_RECV_EPDIS;
						dwc_ep0_stall();
					}

				}
				REG_DOEP_INT(0) = DEP_OUTTOKEN_RECV_EPDIS;
			}
#endif

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

	ep_intr = (REG_OTG_DAINT & 0xffff);
	while(ep_intr) {
		if (ep_intr & 0x1) {
			u32 intr = REG_DIEP_INT(epnum);
			DBG("Handle in epnum:");
			DBG_DEC(epnum);

			if (intr & DEP_XFER_COMP) {
				REG_DIEP_INT(epnum) = DEP_XFER_COMP;
				if ((status[epnum].length > status[epnum].xfer_count)) {
					DBG("in xfer complex,start another.\n");
					status[epnum].length -= status[epnum].xfer_count;
					handle_in_transfer(&status[epnum],epnum);
				}
#ifdef HAVE_IN_ZERO_PACKET
				else if (status[epnum].need_zpkt) {
					status[epnum].length = 0;
					REG_DIEP_EMPMSK &= ~(1 << epnum);
					handle_in_transfer(&status[epnum],epnum);
				}
#endif
				else {
					DBG("in xfer complex.\n");
					REG_DIEP_EMPMSK &= ~(1 << epnum);
					if (epnum == 0) {
						if (status[0].trans_status == USB_DATA_STAGE) {
							status[0].trans_status = USB_STATU_STAGE;
							REG_DIEP_MASK &= ~DEP_TIME_OUT;
							REG_DOEP_SIZE(0) = DOEPSIZE0_PKTCNT_BIT;
							REG_DOEP_CTL(0) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
						} else if (status[0].trans_status == USB_STATU_STAGE) {
							status[0].trans_status = USB_IDLE_STAGE;
							REG_DOEP_SIZE(0) = DOEPSIZE0_SUPCNT_3 | DOEPSIZE0_PKTCNT_BIT | (8 * 3);
							REG_DOEP_CTL(0) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
						}
					}
					status[epnum].length = 0;
					status[epnum].addr = 0;
				}
			}

			if (intr & DEP_EPDIS_INT) {
				DBG("in disabled.\n");
				REG_DIEP_INT(epnum) = DEP_EPDIS_INT;
			}

			if (intr & DEP_AHB_ERR) {
				DBG("in AHB error.\n");
				REG_DIEP_INT(epnum) = DEP_AHB_ERR;
			}

			if (intr & DEP_NAK_INT)
				REG_DIEP_INT(epnum) = DEP_NAK_INT;

			if (intr & DEP_NYET_INT)
				REG_DIEP_INT(epnum) = DEP_NYET_INT;

			if ((intr & DEP_TIME_OUT) & REG_DIEP_MASK) {
				DBG("in tx timeout.\n");
				REG_DIEP_MASK &= ~DEP_TIME_OUT;
				REG_DIEP_INT(epnum) = DEP_TIME_OUT;
				if (!epnum) {
					if (status[0].trans_status == USB_DATA_STAGE) {
						status[0].trans_status = USB_STATU_STAGE;
						REG_DOEP_SIZE(0) = DOEPSIZE0_PKTCNT_BIT;
						REG_DOEP_CTL(0) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
						status[0].length = 0;
						status[0].addr = 0;
					} else if (status[0].trans_status == USB_STATU_STAGE) {
						status[0].trans_status = USB_IDLE_STAGE;
						REG_DOEP_SIZE(0) = DOEPSIZE0_SUPCNT_3 | DOEPSIZE0_PKTCNT_BIT | (8 * 3);
						REG_DOEP_CTL(0) |= DEP_ENA_BIT | DEP_CLEAR_NAK;
						status[0].length = 0;
						status[0].addr = 0;
					}
				}
			}

			if ((intr & DEP_TXFIFO_EMPTY) && (REG_DIEP_EMPMSK & (1 << epnum))) {
				DBG("in fifo empty.\n");
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

/*-------------------------------------------------------------------------*/
static void dwc_otg_cpm_init(void)
{
	DBG("bootrom cpm init!!!!\n");
	REG_CPM_USBPCR &= ~(1 << 31);
	REG_CPM_USBPCR |= USBPCR_VBUSVLDEXT;
	REG_CPM_USBPCR |= USBPCR_POR;
	xudelay(30);
	REG_CPM_USBPCR &= ~USBPCR_POR;
	xudelay(300);
	__cpm_enable_otg_phy();
}


enum pll_type {
	APLL = 0,
	MPLL,
};

void pll_init(int m, int n,int od, int pll)
{

	switch (pll) {
	case MPLL:
		REG_CPM_CPMPCR = (m << 24)|(n << 15)|(od << 11)|(1 << 0);
		while(!(REG_CPM_CPMPCR & (1 << 4)));
		break;
	default:
	case APLL:		/* m	n	od	en*/
		REG_CPM_CPAPCR = (m << 24)|(n << 15)|(od << 11)|(1 << 0);
		while(!(REG_CPM_CPAPCR & (1 << 4)));
		break;
	}
	return;
}

void set_cpufreq(unsigned int extclk)
{
	/* otg phy select clock from crystal*/
	unsigned int nf = 0,nr = 0,no = 0, od = 0;
	unsigned int fin = (extclk/1000000);
	unsigned int tmp = 0;
	unsigned int pll_sel = APLL;

	/*Caculate no : 500/fout <= no <= 1500/fout */
	if (fout >= 1500) {
		DBG("not support more than 1.5Ghz cpu speed\n");
		fout = 800;
		od = 0;
		no = 1;
	} if (fout >=  500) {
		od = 0;
		no = 1;
	} else if (fout >= 250) {
		od = 1;
		no = 2;
	} else if (fout >= 125) {
		od = 2;
		no = 4;
	} else if (fout >= 63) {
		od = 3;
		no = 8;
	} else {
		DBG("cpu speed is too low ,use crystal\n");
		return;
	}

	/*Caculate nr : 1 <= fin/nr <= 50 (nr = n)>= 1 */
	nr = 1;

	/*Caculate nf : nf >= 2 nf <= 255 nf = m*/
	nf = fout*no*nr/fin;
	if (nf > 255)
		nf = 255;
	if (nf < 2) {
		nf = 2;
		nr = 2;
	}

	/*Caculate fout : fout = fin*nf/nr/no*/
	fout = fin * nf /nr /no;
	notify_cpufreq_change(fout);
	DBG("pll init freq is :");
	DBG_DEC(fout);

	/*pll init*/
	pll_init(nf/*m*/,nr/*n*/,od, pll_sel);

	/* cpu freq init
	 *	PCLK : [19:16] H2CLK: [15:12] H0CLK: [11:8] L2CCLK:[7:4] CCLK [3:0]
	 *	CCCLK = 1,2,3 or 4 times of L2CCLK
	 *	H2CLK = 1 or 2 times of PCLK
	 */
	tmp = REG_CPM_CPCCR & 0xfffffff0;
	tmp |= (1 << 22) | (1 << 21) | (1 << 20) |
		(3 << 16) | (1 << 12) | (1 << 8) | (1 << 4);
	REG_CPM_CPCCR = tmp;
	while(REG_CPM_CPCSR & 7);

	tmp = REG_CPM_CPCCR;
	tmp &= ~(0xff << 24);
	if (pll_sel == APLL)		// select apll
		tmp |= (1 << 24) | (1 << 26) | (1 << 28) | (1 << 30);
	else				// select mpll
		tmp |= (2 << 24) | (2 << 26) | (2 << 28);
	REG_CPM_CPCCR = tmp;

	return;
}

int div_clock(void)
{
	int clk = jz_extal/1000000;
	int extclk = jz_extal;

	REG_CPM_CLKGR1  &= ~(1 << 8);			//open otg1 clock gate
	REG_CPM_USBPCR1 &= ~(0x1f << 23);
	REG_CPM_USBPCR1 |= ((0x2 << 26)|(0x1 << 31));	//usb phy use extal crystal

	switch (clk) {
	case 9:
		set_cpufreq(extclk);
		break;
	case 10:
		REG_CPM_USBPCR1 |= (1 << 23);
		set_cpufreq(extclk);
		break;
	case 12:
		REG_CPM_USBPCR1 |= (2 << 23);
		set_cpufreq(extclk);
		break;
	case 19:
		REG_CPM_USBPCR1 |= (3 << 23);
		set_cpufreq(extclk);
		break;
	case 20:
		REG_CPM_USBPCR1 |= (4 << 23);
		set_cpufreq(extclk);
		break;
	case 50:
		REG_CPM_USBPCR1 |= (7 << 23);
		set_cpufreq(extclk);
		break;
	default:
	case 24:
		REG_CPM_USBPCR1 |= (5 << 23);
		set_cpufreq(extclk);
		break;
	}
	return 0;
}


int usb_boot(void)
{
	int usb_reset_flag = 0;
	u32 intsts;
	USB_STATUS status[2];
	memclr((int *)status, 2*sizeof(USB_STATUS));

	DBG("bootrom test !!!!\n`");
#ifdef	BOOTROM_TEST
	REG_OTG_DCTL = DCTL_SOFT_DISCONN;		// Soft Disconnect-> 0(normal mode)
	dwc_otg_core_reset();
	xmdelay(1000);
#else
#ifndef FPGA_TEST
	div_clock();
#else
	REG_CPM_USBPCR1 &= ~(0x7 << 23);
	REG_CPM_USBPCR1 |= (0x2 << 23);
	REG_CPM_USBPCR1 |= ((0x2 << 26)|(0x1 << 31));
#endif
#endif
	dwc_otg_cpm_init();

	dwc_otg_core_init();

	DBG_HEX(REG_GHW_CFG1);
	DBG_HEX(REG_GHW_CFG2);
	DBG_HEX(REG_GHW_CFG3);
	DBG_HEX(REG_GHW_CFG4);

	dwc_otg_device_init();

	enum_done_speed = HIGH_SPEED;

	/* start counter */
	/*instruction time is not fixable on the bootrom,we use pmon for delay*/
	xtimeout(0, 1);

	/* Main loop of polling the usb commands */
	while (1) {

#ifndef BOOTROM_TEST
		if (!(usb_reset_flag)) {
			if (!xtimeout(10 * 1000 * 1000, 0)) {
				DBG("*! USB idle more than 10s !*\n\n");
				return -GO_RESTART_BOOT;
			}
		}
#else
		if (REG_GOTG_INTR & GOTGINT_SESSION_END) {
			DBG("session end detected\n");
			REG_GOTG_INTR = GOTGINT_SESSION_END;
		}
#endif
		intsts = REG_GINT_STS;

		if ((intsts & GINTSTS_USB_EARLYSUSPEND & REG_GINT_MASK))
			handle_early_suspend_intr(&usb_reset_flag,status);

		if ((intsts & GINTSTS_START_FRAM) & REG_GINT_MASK)
			handle_start_frame_intr();

		/* reset interrupt handle */
		if ((intsts & GINTSTS_USB_RESET) & REG_GINT_MASK)
			handle_reset_intr();

		/* enum done */
		if ((intsts & GINTSTS_ENUM_DONE) & REG_GINT_MASK) {
			pmon_clear_cnt();
			pmon_stop();
			handle_enum_done_intr(&usb_reset_flag);
		}


		if ((intsts & GINTSTS_RXFIFO_NEMPTY) & REG_GINT_MASK)
			handle_rxfifo_nempty(status);

		if ((intsts & GINTSTS_OEP_INTR) & REG_GINT_MASK)
			handle_outep_intr(status);

		if ((intsts & GINTSTS_IEP_INTR) & REG_GINT_MASK)
			handle_inep_intr(status);

	}
	return 0;
}	/* usb_boot end */
