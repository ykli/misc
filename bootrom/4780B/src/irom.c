/*
 *	irom.c
 */

#include <irom.h>
#include <common.h>
#include <regs.h>
#include <jz4780cpm.h>
#include <jz4780ost.h>
#include <jz4780tcu.h>
#include <jz4780intc.h>
#include <jz4780rtc.h>
#include <jz4780gpio.h>
#include <jz4780uart.h>
#include <jz4780nfi.h>

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
u32 boot_sel = 0;
u32 start_addr, watch_list[MAX_WATCH_NUM * 2], except_reg_stack[MAX_SAVEREG_NUM];
u16  watch_order[MAX_WATCH_NUM];
int cpustate = 0;

extern int usb_boot(void);
extern int spi_boot(void);
extern int sd_boot(int);
extern int nand_boot(void);

#define argument_init() 		\
do{					\
	start_addr = START_ADDR;	\
	boot_sel = (REG32(GPIO_PXPIN(3)) >> 17) & 0x7;/* store BOOT_SEL2~0 */	\
}while(0)

#define gpio_init() 			\
do{					\
	REG32(GPIO_PXINTC(3)) =  (0x7 << 17);	\
	REG32(GPIO_PXMASKS(3)) = (0x7 << 17);	\
	REG32(GPIO_PXPAT1S(3)) = (0x7 << 17);	\
	REG32(GPIO_PXPAT0C(3)) = (0x7 << 17);	\
	REG32(GPIO_PXPENC(3)) =  (0x7 << 17);	/*pull up*/\
	/*boot select pin input PD17,PD18,PD19*/	\
}while(0)

int get_pll_freq(void)
{
        int freq;
        unsigned int apll_m =  __cpm_get_pllm();
        unsigned int apll_n =  __cpm_get_plln();
        unsigned int apll_od =  __cpm_get_pllod();
        unsigned int extclk0 = __cpm_get_extalclk0();

        freq = (extclk0 * (apll_m + 1)) / (apll_n + 1) / (apll_od + 1);

        return (freq / 1000000);
}

void create_watchlist()
{
	unsigned int pack_addr_csize, pack_seqno_csize, fixcode_size;
	unsigned int pcvalue, watchno, i, j, list_end;
	unsigned int *efuse_data;

	/* Init the watch_list table as zero.  */
	for (i = 0; i < MAX_WATCH_NUM * 2; i++)
		watch_list[i] = 0;
	for (i = 0; i < MAX_WATCH_NUM; i++)
		watch_order[i] = 0;
	list_end = 0;

	/* Initial efuse point */
	efuse_data = (unsigned int *)EFUSE_FIXCODE_START;

	while (*efuse_data != 0)
	{
		/* Fetch the watch_pc, watch_no and watch code size. */
		pack_addr_csize  = *efuse_data >> 16;
		pack_seqno_csize = *efuse_data & 0xffff;

		pcvalue = (pack_addr_csize >> 4) << 3;          // watch pc value
		watchno = (pack_seqno_csize >> 4);              // watch point sequence No.
		fixcode_size = ((pack_addr_csize & 0xf) << 4) | (pack_seqno_csize & 0xf);  // insn number, actual size / 4.

		/* Serach position in watch_list table for this watch point recording. */
		for (i = 0; i < MAX_WATCH_NUM; i++)
		{
			if (watch_list[i * 2] == 0)   // watch list table end
				break;
			if (watchno <= (watch_order[i] & 0xfff))
				break;
		}

		if (i == MAX_WATCH_NUM)
			break;

		/* Insert the watch point in list table. */
		if ((watch_list[i * 2] != 0) && (watchno < (watch_order[i] & 0xfff)))
		{
			for (j = list_end; j > i && j < MAX_WATCH_NUM; j--)
			{
				watch_list[j * 2] = watch_list[j * 2 - 2];
				watch_list[j * 2 + 1] = watch_list[j * 2 - 1];
				watch_order[j] = watch_order[j - 1];
			}
		}

		watch_list[i * 2] = pcvalue | 0xbfc00000;
		watch_list[i * 2 + 1] = (unsigned int)efuse_data + 4;
		watch_order[i] = watchno;

		/* to next watch point. */
		list_end++;
		efuse_data += fixcode_size;
		if (list_end >= MAX_WATCH_NUM)
			break;
	} // while (*efuse_data != 0)
}

int main(int argc, char **argv)
{

	static u32 boot_counter = 0;
	int SLPC = 0;
	int ret = 0;
	void (*p0wakeup)(void);
	p0wakeup = (void (*)(void))REG_CPM_SP;

restart_boot:
	// Enter hibernate mode because the times of jumping to usb_boot from other boot exceeds 3.
	if( ++boot_counter > 3) {
		power_off_device();
	}

	gpio_init();
	argument_init();
#ifdef SERIAL_DEBUG
	serial_init();
#endif
	cpustate = REG_CPM_RSR;
	if (!((cpustate & RSR_WR) || (cpustate & RSR_PR))) {
		if (cpustate & RSR_P0R) {
			if (REG_CPM_SBCR == 1) {
				(*p0wakeup)();
			} else {
				notify_cpufreq_change(get_pll_freq());
				boot_sel = REG_CPM_SP;
			}
		}
	}

	pmon_prepare(PMON_EVENT_CYCLE);

/*
 *						 PD19 PD18 PD17
 *  usb  boot (synopsis) 1   1   1
 *  nand boot            1   1   0
 *  msc0 boot            1   0   1
 *  msc1 boot            1   0   0
 *  emmc boot            0   1   1
 *  nor  boot            0   1   0
 *  spi  boot            0   0   0
 */

	switch (boot_sel) {
	case 0x6: 	             // if boot_sel[2:0]=110, NAND boot @ CS1
		ret = nand_boot();
		break;
	case 0x5: 	             // if boot_sel[2:0]=101, SD boot @ MSC0
		ret = sd_boot(0);
		break;
	case 0x4:		     // if boot_sel[2:0]=100, SD boot @ MSC1
		ret = sd_boot(1);
		break;
	case 0x3:
		ret = emmc_boot();
		break;
	case 0x2:               // if boot_sel[2:0]=010, NOR boot @ CS2
		ret = nor_boot();
		break;
	case 0x0:               // if boot_sel[2:0]=000, SPI boot @ SPI0/CE0
		ret = spi_boot();
		break;
	case 0x1:		//we should not come here!! flow boot will running into chip RAM not ROM space!
	default:
		ret = usb_boot();
		break;
	}

	DBG("boot end. \n");
	goto restart_boot;
	return 0;
} /* main end */

