#include <irom.h>
#include <common.h>
#include <jz4780misc.h>
#include <jz4780uart.h>
#include <jz4780otp.h>
#include <jz4780gpio.h>
#include <jz4780cpm.h>

extern int cpustate;

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

static void serial_setbrg_4780(void)
{
	volatile u8 *uart_lcr = (volatile u8 *)(UART_BASE + OFF_LCR);
	volatile u8 *uart_dlhr = (volatile u8 *)(UART_BASE + OFF_DLHR);
	volatile u8 *uart_dllr = (volatile u8 *)(UART_BASE + OFF_DLLR);
	volatile u8 *uart_umr = (volatile u8 *)(UART_BASE + OFF_UMR);
	volatile u8 *uart_uacr = (volatile u8 *)(UART_BASE + OFF_UACR);

	u32 baud_div, tmp;

	*uart_umr = 0x10;
	*uart_uacr = 0;
	//baud_div = (CFG_EXTAL / 16 / CONFIG_BAUDRATE);
	baud_div = 78;

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

	serial_setbrg_4780();

	/* Enable UART unit, enable and clear FIFO */
	*uart_fcr = UARTFCR_UUE | UARTFCR_TFLS | UARTFCR_RFLS;
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
	
	u32  i;
	int ret;

	/*when p0 occurs, sec boot will not start*/
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

	pmon_stop();
	__asm__ volatile (
		"# Embedded assembler by twxie. \n\t"
		".set noreorder\n\t"
		".set mips32\n\t"
		"mtc0 	$0, $26\n\t"					// CP0_ERRCTL, restore WST reset state
			
		"jalr 	%0\n\t"							// jump, and place the return address in $31
		"nop	\n\t"
		".set mips2\n\t"
		".set reorder\n\t"
		"# End of embedded assembler by twxie. \n\t"
		:
		: "r"(jump_addr)
		);
	
	return -GO_USB_BOOT_RETURN;
}
extern int cpufreq;
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
	} else {
		rc = get_pmon_rc();
		if (rc>= (cpufreq*usec)) {
			pmon_stop();
			return 0;
		} else
			return 1;
	}
}


