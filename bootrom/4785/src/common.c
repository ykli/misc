#include <irom.h>
#include <common.h>
#include <jz4785uart.h>
#include <jz4785otp.h>
#include <jz4785gpio.h>
#include <jz4785cpm.h>

#include "regdef.h"

extern int cpustate;

unsigned int volatile jz_extal = 24000000;
unsigned int volatile cpufreq = 24;//Mhz

#ifdef SERIAL_DEBUG
void serial_putc (const char c)
{
	volatile u8 *uart_lsr = (volatile u8 *)(UART_BASE + OFF_LSR);
	volatile u8 *uart_tdr = (volatile u8 *)(UART_BASE + OFF_TDR);

	if (c == '\n') serial_putc ('\r');

	/* Wait for fifo to shift out some bytes */
	while ( !((*uart_lsr & (UART_LSR_TDRQ | UART_LSR_TEMT)) == 0x60) );

	*uart_tdr = (u8)c;
}

void serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}

int serial_tstc (void)
{
	volatile u8 *uart_lsr = (volatile u8 *)(UART_BASE + OFF_LSR);

	if (*uart_lsr & UART_LSR_DR) {
		/* Data in rfifo */
		return (1);
	}
	return 0;
}

int serial_getc (void)
{
	volatile u8 *uart_rdr = (volatile u8 *)(UART_BASE + OFF_RDR);

	while (!serial_tstc());

	return *uart_rdr;
}

static void serial_setbrg(void)
{
	volatile u8 *uart_lcr = (volatile u8 *)(UART_BASE + OFF_LCR);
	volatile u8 *uart_dlhr = (volatile u8 *)(UART_BASE + OFF_DLHR);
	volatile u8 *uart_dllr = (volatile u8 *)(UART_BASE + OFF_DLLR);
	volatile u8 *uart_umr = (volatile u8 *)(UART_BASE + OFF_UMR);
	volatile u8 *uart_uacr = (volatile u8 *)(UART_BASE + OFF_UACR);

	u32 baud_div, tmp;

#define DIV_BEST 13
#define UMR_BEST 16
#define UACR_BEST 0

	*uart_umr = UMR_BEST;
	*uart_uacr = UACR_BEST;
	baud_div = DIV_BEST;

	tmp = *uart_lcr;
	tmp |= UARTLCR_DLAB;
	*uart_lcr = tmp;

	*uart_dlhr = (baud_div >> 8) & 0xff;
	*uart_dllr = baud_div & 0xff;

	tmp &= ~UARTLCR_DLAB;
	*uart_lcr = tmp;
}

void serial_init(void)
{
	volatile u8 *uart_fcr = (volatile u8 *)(UART_BASE + OFF_FCR);
	volatile u8 *uart_lcr = (volatile u8 *)(UART_BASE + OFF_LCR);
	volatile u8 *uart_ier = (volatile u8 *)(UART_BASE + OFF_IER);
	volatile u8 *uart_sircr = (volatile u8 *)(UART_BASE + OFF_SIRCR);

	__gpio_as_uart0();

	/* Disable port interrupts while changing hardware */
	*uart_ier = 0;

	/* Disable UART unit function */
	*uart_fcr = ~UARTFCR_UUE;

	/* Set both receiver and transmitter in UART mode (not SIR) */
	*uart_sircr = ~(SIRCR_RSIRE | SIRCR_TSIRE);

	/* Set databits, stopbits and parity. (8-bit data, 1 stopbit, no parity) */
	*uart_lcr = UARTLCR_WLEN_8 | UARTLCR_STOP1;

	serial_setbrg();

	/* Enable UART unit, enable and clear FIFO */
	*uart_fcr = UARTFCR_UUE | UARTFCR_TFLS | UARTFCR_RFLS;

	serial_puts("serial_init ok\n");
}

void serial_put_hex(unsigned int  d)
{
	char c[12];
	char i;
	for(i = 0; i < 8;i++)
	{
		c[i] = (d >> ((7 - i) * 4)) & 0xf;
		if(c[i] < 10)
			c[i] += 0x30;
		else
			c[i] += (0x41 - 10);
	}
	c[8] = '\n';
	c[9] = 0;
	serial_puts(c);
}

void serial_put_dec(unsigned int  d)
{
	char c[13];
	int i, j;

	for (i = 0; i < 10 ;i++)
	{
		if (!d && !i) {
			c[0] = 0x30;
			break;
		}

		if (d) {
			c[i] = d % 10 + 0x30;
			d /= 10;
		} else {
			break;
		}
	}
	c[i] = '\n';
	c[i + 1] = 0;

	for (j = 0; i > j+1; j++) {
		c[j] ^= c[i-1];
		c[i-1] ^= c[j];
		c[j] ^= c[i-1];
		i--;
	}

	serial_puts(c);
}
#endif

extern u32 start_addr;

int xfer_d2i(u32 jump_addr, u32 length) {

	int return_val = -1;
#if 0	/*4780b does not have security boot function*/
	/*when p0 occurs, sec boot will not start*/
	int ret;
	if ((cpustate & RSR_WR) || (cpustate & RSR_PR))
	{
		if(REG_OTP_STATE & OTP_STATE_SECBOOT_EN)
		{
			ret = sec_boot();
			if(ret != 0)
			{
				//security boot failed,shut down the device
				//need to use shut down device function instead of this
				power_off_device();
				//	return -GO_USB_BOOT_RETURN;
			}
		}
	}
#endif

	pmon_stop();

	__asm__ volatile (
			".set noreorder\n\t"
			".set mips32\n\t"

			"la    $26,0x80001000\n\t"
			"add   $27,$26, %1\n\t"
			"xfer_a_word:\n\t"
			"lw    $8, 0($26)\n\t"
			"mtc0  $8, $28, 1\n\t"
			"cache 0xc,0($26)\n\t"
			"bne   $26, $27,  xfer_a_word\n\t"
			"addiu $26, $26,  4\n\t"
			"mtc0  $0, $26\n\t"
			"jal  %2\n\t"
			"nop\n\t"
			"move %0,$2\n\t"
			".set reorder\n\t"
			: "=r"(return_val)
			: "r"(length),"r"(jump_addr)
			: "memory", "$26","$27","$8","$2","$31"
	);

	return return_val;
}

static void pll_init(int m, int n,int od, int pll)
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

void notify_cpufreq_change(unsigned int speed)
{
    cpufreq = speed;
}

void set_cpufreq(unsigned int cpufreq)
{
#ifndef FPGA_TEST
	/* otg phy select clock from crystal*/
	unsigned int nf = 0,nr = 0,no = 0, od = 0;
	unsigned int extclk = jz_extal;
	unsigned int fin = (extclk/1000000);
	unsigned int tmp = 0;
	unsigned int pll_sel = APLL;
	unsigned int fout = cpufreq;

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
#endif
	return;
}

int xudelay(int usec)
{
	pmon_stop();
	pmon_clear_cnt();
	pmon_start();
	while (!((get_pmon_rc()) >= (cpufreq*usec)))
		;
	pmon_stop();
	return 0;
}

void xmdelay(int sdelay)
{
	xudelay(sdelay * 1000);
}

int xtimeout(int usec, int ret)
{
	volatile unsigned int rc;

	if (ret) {
		pmon_stop();
		pmon_clear_cnt();
		pmon_start();
		return 0;
	} else {
		rc = get_pmon_rc();
		if (rc>= (cpufreq*usec)) {
			pmon_stop();
			pmon_clear_cnt();
			return 0;
		} else
			return 1;
	}
}


