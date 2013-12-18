/*
 * linux/include/asm-mips/mach-jz4780/jz4780gpio.h
 *
 * JZ4780 GPIO register definition.
 *
 * Copyright (C) 2012 Ingenic Semiconductor Co., Ltd.
 */

#ifndef __JZ4780GPIO_H__
#define __JZ4780GPIO_H__

/***************************************************************************
 * GPIO
 ***************************************************************************/

//------------------------------------------------------
// GPIO Pins Description
//
// PORT A:
//
// PIN/BIT N	FUNC0		FUNC1		FUNC2		FUNC3		NOTE
//	0	SD0		-		-		-
//	1	SD1		-		-		-
//	2	SD2		-		-		-
//	3	SD3		-		-		-
//	4	SD4		MSC0_D4		-		-
//	5	SD5		MSC0_D5		-		-
//	6	SD6		MSC0_D6		-		-
//	7	SD7		MSC0_D7		-		-
//      16      RD_             -		-		-
//      17      WE_             -		-		-
//      18      FRE_            MSC0_D0		SSI0_DR		-
//      19      FWE_		MSC0_CMD	-		-
//      20      -		MSC0_CLK	SSI0_CLK	-		1
//      21      CS1_            MSC0_D1         SSI0_DT		-
//      22      CS2_            MSC0_D2		-		-
//      23      CS3_		MSC0_D3		SSI0_CE0_	-
//      24      CS4_ 		MSC0_RST_	-		-
//      25      CS5_		-		SSI0_CE0_	-
//      26      CS6_		RDWR_		-		-
//      27      WAIT_		-		SSI0_DR		-
//      28      -		-		SSI0_DT		-
//      29      NDQS		-		-		-
//      30	-		-		-		-		6
//      31	-		-		-		-		7,11
//Note1.     If NAND flash is used, this pin must be used as NAND FRB.
//Note6.     PA30: GPIO group A bit 30 can only be used as input and interrupt,
//       no pull-up and pull-down.
//           It is also used to select the function between PS2 function and
//       JTAG function of JTAG/UART3/PS2 Pins( TCK_UART3_RTS_PS2_MCLK,
//       TMS_UART3_CTS_PS2_MDATA, TDI_UART3_RxD_PS2_KCLK and TDO_UART3_TxD_PS2_KDATA),
//       which share the same set of pins.
//Note7.     PA31: GPIO group A bit 31. No corresponding pin exists for this GPIO.
//           It is only used to select the function between UART and JTAG of
//       JTAG/UART3/PS2 Pins (TCK_UART3_RTS_PS2_MCLK,TMS_UART3_CTS_PS2_MDATA,
//       TDI_UART3_RxD_PS2_KCLK and TDO_UART3_TxD_PS2_KDATA), which share the same
//       set of pins, by using register PASEL [31].
//           When PA31.function1 is false, select JTAG function.
//           When PA31.function1 is true, select UART function.
//Note11.    The pull enable of PA31 is used to control UART and JTAG of JTAG/UART3/PS2 Pins.

//------------------------------------------------------
// PORT B:
//
// PIN/BIT N	FUNC0		FUNC1	       FUNC2		FUNC3		NOTE
//	0	SA0_CL		-		-		-		8
//	1	SA1_AL		-		-		-		9
//	2	SA2		-		-		-
//	3	SA3		-		-		-
//	4	SA4		-		MMI_CRS		-
//	5	SA5		SSI0_CLK	-		-
//	6	CIM_PCLK 	TS1_CLK		-		-
//	7	CIM_HSYN  	TS1_STR		-		-
//	8	CIM_VSYN 	TS1_FAIL	-		-
//	9	CIM_MCLK 	TS1_FRM		-		EPD_PWC
//	10	CIM_D0 	 	TS1_DI0		-		EPD_PWR0
//	11	CIM_D1 	 	TS1_DI1		-		EPD_PWR1
//	12	CIM_D2 	 	TS1_DI2		-		EPD_SCE2
//	13	CIM_D3 		TS1_DI3		-		EPD_SCE3_
//	14	CIM_D4 		TS1_DI4		-		EPD_SCE4_
//	15	CIM_D5		TS1_DI5		-		EPD_SCE5_
//	16 	CIM_D6 		TS1_DI6		-		EPD_PWR2
//	17 	CIM_D7 		TS1_DI7		-		EPD_PWR3
//	18	CIM_D8          DMIC_CLK	-		EPD_BD0
//	19	CIM_D9          DMIC_IN		-		EPD_BD1
//	20 	MSC2_D0 	SSI0_DR		SSI1_DR		TSD0
//	21 	MSC2_D1 	SSI0_CE1_	SSI1_CE1_	TSD1
//	22 	TSDI2		-		-		-
//	23 	TSDI3		-		-		-
//	24 	TSDI4		-		-		-
//	25 	TSDI5		-		-		-
//	26 	TSDI6		-		-		-
//	27 	TSDI7		-		-		-
//	28 	MSC2_CLK        SSI0_CLK	SSI1_CLK	-
//	29 	MSC2_CMD        SSI0_DT		SSI1_DT		-
//	30 	MSC2_D2         SSI0_GPC	SSI1_GPC	-
//	31 	MSC2_D3         SSI0_CE0_	SSI1_CE0	-
//Note8.    If NAND flash is used, this pin must be used as NAND CLE.
//Note9.    If NAND flash is used, this pin must be used as NAND ALE.
//------------------------------------------------------
// PORT C:
// PIN/BIT N	FUNC0		FUNC1		FUNC2 		FUNC3		NOTE
//	0	LCD_B0 (O)	LCD_REV (O)	-               -
//	1	LCD_B1 (O)	LCD_PS (O)	-               -
//	2	LCD_B2 (O)	-               -	        -
//	3	LCD_B3 (O)	-               -	        -
//	4	LCD_B4 (O)	-               -	        -
//	5	LCD_B5 (O)	-               -	        -
//	6	LCD_B6 (O)	-               -	        -
//	7	LCD_B7 (O)	-               -	        -
//	8	LCD_PCLK (O)	-               -	        -
//	9	LCD_DE (O)	-               -	        -
//	10	LCD_G0 (O)	LCD_SPL (O)	-               -
//	11	LCD_G1 (O)	-               -	        -
//	12	LCD_G2 (O)	-               -	        -
//	13	LCD_G3 (O)	-               -	        -
//	14	LCD_G4 (O)	-               -	        -
//	15	LCD_G5 (O)	-               -	        -
//	16	LCD_G6 (O)	-               -	        -
//	17	LCD_G7 (O)	-               -	        -
//	18	LCD_HSYN (IO)	-               -	        -
//	19	LCD_VSYN (IO)	-               -	        -
//	20	LCD_R0 (O)	LCD_CLS (O)	-               -
//	21	LCD_R1 (O)	-               -	        -
//	22	LCD_R2 (O)	-               -	        -
//	23	LCD_R3 (O)	-               -	        -
//	24	LCD_R4 (O)	-               -	        -
//	25	LCD_R5 (O)	-               -	        -
//	26	LCD_R6 (O)	-               -	        -
//	27	LCD_R7 (O)	-               -	        -

//------------------------------------------------------
// PORT D:
//
// PIN/BIT N	FUNC0		FUNC1		FUNC2 		FUNC3		NOTE
//	0 	PCM0_DO		-		-		-
//	1 	PCM0_CLK	-		-		-
//	2 	PCM0_SYN	-		-		-
//	3 	PCM0_DI		-		-		-
//	4 	PS2_MCLK	UART2_RTS_	-		-
//	5	PS2_MDATA	UART2_CTS_	-		-
//	6 	PS2_KCLK	UART2_RXD	-		-
//	7 	PS2_KDATA	UART2_TXD	-		-
//	8 	SCC_DATA	-		-		-
//	9 	SCC_CLK		CLK48M		-	        -
//	10 	PWM6		I2C3_SDA	-		-
//	11 	PWM7		I2C3_SCK	-		-
//	12 	UART3_RXD	BCLK0		-		EPD_PWR4
//	13 	LRCLK0		-		-		EPD_PWR5
//	14 	-		-		-		-		10
//	15 	EXCLK		-		-	        -
//	16 	-		-		-	        -
//	17 	-		-		-		-		2,5
//	18 	-		-		-		-		3,5
//	19 	-		-		-		-		4,5
//	20 	MSC1_D0		SSI0_DR		SSI1_DR	        -
//	21	MSC1_D1		SSI0_CE1_	SSI1_CE1_	-
//	22 	MSC1_D2		SSI0_GPC	SSI1_GPC	-
//	23 	MSC1_D3		SSI0_CE0_	SSI1_CE0_	-
//	24 	MSC1_CLK	SSI0_CLK	SSI1_CLK	-
//	25 	MSC1_CMD	SSI0_DT		SSI1_DT		-
//	26 	UART1_RXD	MII_RXD2	-		-
//	27 	UART1_CTS_	MII_RXD3	-		-
//	28 	UART1_TXD	MII_TXD2	-		-
//	29 	UART1_RTS_	MII_TXD3	-		-
//	30 	I2C0_SDA	-		-		-
//	31 	I2C0_SCK	-		-		-
//
// Note2.     PD17: GPIO group D bit 17 is used as BOOT_SEL0 input during boot.
// Note3.     PD18: GPIO group D bit 18 is used as BOOT_SEL1 input during boot.
// Note4.     PD19: GPIO group D bit 19 is used as BOOT_SEL2 input during boot.
// Note5.     BOOT_SEL2, BOOT_SEL1, BOOT_SEL0 are used to select boot source and
//        function during the processor boot.
// Note10.    PD14,PF26-30: this pin is generally used from RTC. If this pin is
//        intended to be used as GPIO or interrupt, please refer to RTC spec to
//        configure the corresponding registers.
//
//------------------------------------------------------
// PORT E:
//
// PIN/BIT N	FUNC0		FUNC1		FUNC2		FUNC3		NOTE
//	0  	PWM0		- 		- 		-
//	1  	PWM1		- 		- 		-
//	2  	PWM2 		-		- 		-
//	3  	PWM3 		I2C4_SDA	- 		-
//	4  	PWM4 		I2C4_SCK	-		-
//	5  	PWM5 		UART3_TXD	SCLK_RSTN 	-
//	6  	AIC0_SDATI	- 		- 		EPD_PWR6
//	7  	AIC0_SDATO	- 		- 		EPD_PWR7-
//	8  	UART3_CTS_ 	BCLK0_AD 	- 		-
//	9  	UART3_RTS_ 	LRCLK0_AD 	- 		-
//	10  	DRVVBUS		-		-		-
//	12  	BCLK1_AD	I2C4_SDA	-		-
//	13  	LRCLK1_AD	I2C4_SCK	-		-
//	14  	SSI0_DR		SSI1_DR		-		-
//	15  	SSI0_CLK	SSI1_CLK	-		-
//	16  	SSI0_CE0_	SSI1_CE0_	-		-
//	17  	SSI0_DT		SSI1_DT		-		-
//	18  	SSI0_CE1_	SSI1_CE1_	-		-
//	19  	SSI0_GPC	SSI1_GPC	-		-
//	20  	MSC0_D0		MSC1_D0		MSC2_D0		-
//	21  	MSC0_D1		MSC1_D1		MSC2_D1		-
//	22  	MSC0_D2		MSC1_D2		MSC2_D2		-
//	23  	MSC0_D3		MSC1_D3		MSC2_D3		-
//	28  	MSC0_CLK	MSC1_CLK	MSC2_CLK	-
//	29  	MSC0_CMD	MSC1_CMD	MSC2_CMD	-
//	30  	I2C1_SDA	-		-		-
//	31  	I2C1_SCK	-		-		-
//
//------------------------------------------------------
// PORT F:
//
// PIN/BIT N	FUNC0		FUNC1		FUNC2 		FUNC3		NOTE
//	0	UART0_RXD	GPS_CLK		-		-
//	1	UART0_CTS_	GPS_MAG		-		-
//	2	UART0_RTS_	GPS_SIG		-		-
//	3	UART0_TXD	-		-		-
//	4	MII_TXD0	CIM_D0		-		-
//	5	MII_TXD1	CIM_D1		-		-
//	6	MII_TXCLK	CIM_MCLK	-		-
//	7	MII_RXCLK	CIM_PCLK	-		-
//	8	MII_RXER	CIM_HSYN	-		-
//	9	MII_RXDV	CIM_VSYN	-		-
//	10	MII_RXD0	CIM_D2		-		-
//	11	MII_RXD1	CIM_D3		-		-
//	12	MII_TXEN	CIM_D4		-		-
//	13	MII_MDC		CIM_D5		-		-
//	14	MII_MDIO	CIM_D6		-		-
//	15	MII_COL		CIM_D7		-		-
//	16	CIM_D10		-		I2C2_SDA	EPD_BD2
//	17	CIM_D11		-		I2C2_SCK	EPD_BD3
//	18	-		-		-		-
//	19	-		-		-		-
//	20	-		-		-		-
//	21	-		-		-		-
//	22	-		-		-		-
//	23	CEC		-		-		-
//	24	DDCSCL		I2C4_SCK	-		-
//	25	DDCSDA		I2C4_SDA	-		-
//	26	-		-		-		-		10
//	27	-		-		-		-		10
//	28	-		-		-		-		10
//	29	-		-		-		-		10
//	30	-		-		-		-		10
// Note10.    PD14,PF26-30: this pin is generally used from RTC. If this pin is
//        intended to be used as GPIO or interrupt, please refer to RTC spec to
//        configure the corresponding registers.
//
//////////////////////////////////////////////////////////



#define	GPIO_BASE	0xB0010000

/* GPIO group offset */
#define GPIO_GOS	0x100


#define GPIO_PXFUNS_OFFSET	(0x44)  /*  w, 32, 0x???????? */
#define GPIO_PXSELC_OFFSET	(0x58)  /*  w, 32, 0x???????? */
#define GPIO_PXPES_OFFSET	(0x34)  /*  w, 32, 0x???????? */
#define GPIO_PXFUNC_OFFSET	(0x48)  /*  w, 32, 0x???????? */
#define GPIO_PXPEC_OFFSET	(0x38)  /*  w, 32, 0x???????? */
#define GPIO_PXIMC_OFFSET	(0x28)  /*  w, 32, 0x???????? */
#define GPIO_PXDATC_OFFSET	(0x18)  /*  w, 32, 0x???????? */

#define GPIO_PXDATC(n)	(GPIO_BASE + (n)*GPIO_GOS + GPIO_PXDATC_OFFSET)
#define GPIO_PXIMC(n)	(GPIO_BASE + (n)*GPIO_GOS + GPIO_PXIMC_OFFSET)
#define GPIO_PXPEC(n)	(GPIO_BASE + (n)*GPIO_GOS + GPIO_PXPEC_OFFSET)
#define GPIO_PXFUNC(n)	(GPIO_BASE + (n)*GPIO_GOS + GPIO_PXFUNC_OFFSET)


#define GPIO_PXFUNS(n)	(GPIO_BASE + (n)*GPIO_GOS + GPIO_PXFUNS_OFFSET)
#define GPIO_PXSELC(n)	(GPIO_BASE + (n)*GPIO_GOS + GPIO_PXSELC_OFFSET)
#define GPIO_PXPES(n)	(GPIO_BASE + (n)*GPIO_GOS + GPIO_PXPES_OFFSET)

#define REG_GPIO_PXFUNS(n)	REG32(GPIO_PXFUNS(n))
#define REG_GPIO_PXFUNC(n)	REG32(GPIO_PXFUNC(n))
#define REG_GPIO_PXSELC(n)	REG32(GPIO_PXSELC(n))
#define REG_GPIO_PXPES(n)	REG32(GPIO_PXPES(n))
#define REG_GPIO_PXPEC(n)	REG32(GPIO_PXPEC(n))

/*************************************************************************
 * GPIO (General-Purpose I/O Ports)
 *************************************************************************/
#define GPIO_BASEA GPIO_BASE +(0)*0x100
#define GPIO_BASEB GPIO_BASE +(1)*0x100
#define GPIO_BASEC GPIO_BASE +(2)*0x100
#define GPIO_BASED GPIO_BASE +(3)*0x100
#define GPIO_BASEE GPIO_BASE +(4)*0x100
#define GPIO_BASEF GPIO_BASE +(5)*0x100

#define GPA(x) 		(32 * 0 + (x))
#define GPB(x) 		(32 * 1 + (x))
#define GPC(x) 		(32 * 2 + (x))
#define GPD(x) 		(32 * 3 + (x))
#define GPE(x) 		(32 * 4 + (x))
#define GPF(x) 		(32 * 5 + (x))

#define GPIO_PORT_NUM   6
#define MAX_GPIO_NUM	192
#define GPIO_WAKEUP     (30)  //WAKE_UP key PA30

//n = 0,1,2,3,4,5 (PORTA, PORTB, PORTC, PORTD, PORTE, PORTF)
#define GPIO_PXPIN(n)	(GPIO_BASE + (0x00 + (n)*0x100)) /* PIN Level Register */

#define GPIO_PXINT(n)	(GPIO_BASE + (0x10 + (n)*0x100)) /* Port Interrupt Register */
#define GPIO_PXINTS(n)	(GPIO_BASE + (0x14 + (n)*0x100)) /* Port Interrupt Set Register */
#define GPIO_PXINTC(n)	(GPIO_BASE + (0x18 + (n)*0x100)) /* Port Interrupt Clear Register */

#define GPIO_PXMASK(n)	(GPIO_BASE + (0x20 + (n)*0x100)) /* Port Interrupt Mask Register */
#define GPIO_PXMASKS(n)	(GPIO_BASE + (0x24 + (n)*0x100)) /* Port Interrupt Mask Set Reg */
#define GPIO_PXMASKC(n)	(GPIO_BASE + (0x28 + (n)*0x100)) /* Port Interrupt Mask Clear Reg */

#define GPIO_PXPAT1(n)	(GPIO_BASE + (0x30 + (n)*0x100)) /* Port Pattern 1 Register */
#define GPIO_PXPAT1S(n)	(GPIO_BASE + (0x34 + (n)*0x100)) /* Port Pattern 1 Set Reg. */
#define GPIO_PXPAT1C(n)	(GPIO_BASE + (0x38 + (n)*0x100)) /* Port Pattern 1 Clear Reg. */

#define GPIO_PXPAT0(n)	(GPIO_BASE + (0x40 + (n)*0x100)) /* Port Pattern 0 Register */
#define GPIO_PXPAT0S(n)	(GPIO_BASE + (0x44 + (n)*0x100)) /* Port Pattern 0 Set Register */
#define GPIO_PXPAT0C(n)	(GPIO_BASE + (0x48 + (n)*0x100)) /* Port Pattern 0 Clear Register */

#define GPIO_PXFLG(n)	(GPIO_BASE + (0x50 + (n)*0x100)) /* Port Flag Register */
#define GPIO_PXFLGC(n)	(GPIO_BASE + (0x58 + (n)*0x100)) /* Port Flag clear Register */

#define GPIO_PXPEN(n)	(GPIO_BASE + (0x70 + (n)*0x100)) /* Port Pull Disable Register */
#define GPIO_PXPENS(n)	(GPIO_BASE + (0x74 + (n)*0x100)) /* Port Pull Disable Set Register */
#define GPIO_PXPENC(n)	(GPIO_BASE + (0x78 + (n)*0x100)) /* Port Pull Disable Clear Register */

#if 0	//fixme twxie
#define GPIO_PXDS(n)	(GPIO_BASE + (0x80 + (n)*0x100)) /* Port Drive Strength Register */
#define GPIO_PXDSS(n)	(GPIO_BASE + (0x84 + (n)*0x100)) /* Port Drive Strength set Register */
#define GPIO_PXDSC(n)	(GPIO_BASE + (0x88 + (n)*0x100)) /* Port Drive Strength clear Register */
#endif


#define REG_GPIO_PXPIN(n)	REG32(GPIO_PXPIN((n)))  /* PIN level */
#define REG_GPIO_PXINT(n)	REG32(GPIO_PXINT((n)))  /* 1: interrupt pending */
#define REG_GPIO_PXINTS(n)	REG32(GPIO_PXINTS((n)))
#define REG_GPIO_PXINTC(n)	REG32(GPIO_PXINTC((n)))

#define REG_GPIO_PXMASK(n)	REG32(GPIO_PXMASK((n)))   /* 1: mask pin interrupt */
#define REG_GPIO_PXMASKS(n)	REG32(GPIO_PXMASKS((n)))
#define REG_GPIO_PXMASKC(n)	REG32(GPIO_PXMASKC((n)))

#define REG_GPIO_PXPAT1(n)	REG32(GPIO_PXPAT1((n)))   /* 1: disable pull up/down */
#define REG_GPIO_PXPAT1S(n)	REG32(GPIO_PXPAT1S((n)))
#define REG_GPIO_PXPAT1C(n)	REG32(GPIO_PXPAT1C((n)))

#define REG_GPIO_PXPAT0(n)	REG32(GPIO_PXPAT0((n)))  /* 0:GPIO/INTR, 1:FUNC */
#define REG_GPIO_PXPAT0S(n)	REG32(GPIO_PXPAT0S((n)))
#define REG_GPIO_PXPAT0C(n)	REG32(GPIO_PXPAT0C((n)))

#define REG_GPIO_PXFLG(n)	REG32(GPIO_PXFLG((n))) /* 0:GPIO/Fun0,1:intr/fun1*/
#define REG_GPIO_PXFLGC(n)	REG32(GPIO_PXFLGC((n)))

#define REG_GPIO_PXPEN(n)	REG32(GPIO_PXPEN((n)))
#define REG_GPIO_PXPENS(n)	REG32(GPIO_PXPENS((n))) /* 0:Level-trigger/Fun0, 1:Edge-trigger/Fun1 */
#define REG_GPIO_PXPENC(n)	REG32(GPIO_PXPENC((n)))

#if 0	//fixme twxie
#define REG_GPIO_PXDS(n)	REG32(GPIO_PXDS((n)))
#define REG_GPIO_PXDSS(n)	REG32(GPIO_PXDSS((n))) /* interrupt flag */
#define REG_GPIO_PXDSC(n)	REG32(GPIO_PXDSC((n))) /* interrupt flag */
#endif

#ifndef __MIPS_ASSEMBLER

/*----------------------------------------------------------------
 * p is the port number (0,1,2,3,4,5)
 * o is the pin offset (0-31) inside the port
 * n is the absolute number of a pin (0-127), regardless of the port
 */

//----------------------------------------------------------------
// Function Pins Mode
#define __gpio_as_func0(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	REG_GPIO_PXINTC(p) = (1 << o);		\
	REG_GPIO_PXMASKC(p) = (1 << o);		\
	REG_GPIO_PXPAT1C(p) = (1 << o);		\
	REG_GPIO_PXPAT0C(p) = (1 << o);		\
} while (0)

#define __gpio_as_func1(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	REG_GPIO_PXINTC(p) = (1 << o);		\
	REG_GPIO_PXMASKC(p) = (1 << o);		\
	REG_GPIO_PXPAT1C(p) = (1 << o);		\
	REG_GPIO_PXPAT0S(p) = (1 << o);		\
} while (0)

#define __gpio_as_func2(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	REG_GPIO_PXINTC(p) = (1 << o);		\
	REG_GPIO_PXMASKC(p) = (1 << o);		\
	REG_GPIO_PXPAT1S(p) = (1 << o);		\
	REG_GPIO_PXPAT0C(p) = (1 << o);		\
} while (0)

#define __gpio_as_func3(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	REG_GPIO_PXINTC(p) = (1 << o);		\
	REG_GPIO_PXMASKC(p) = (1 << o);		\
	REG_GPIO_PXPAT1S(p) = (1 << o);		\
	REG_GPIO_PXPAT0S(p) = (1 << o);		\
} while (0)

#if 0
/*
 * MII_TXD0- D3 MII_TXEN MII_TXCLK MII_COL
 * MII_RXER MII_RXDV MII_RXCLK MII_RXD0 - D3
 * MII_CRS MII_MDC MII_MDIO
 */
#define __gpio_as_eth()				\
do {						\
	REG_GPIO_PXINTC(1) =  0x00000010;	\
	REG_GPIO_PXMASKC(1) = 0x00000010;	\
	REG_GPIO_PXPAT1S(1) = 0x00000010;	\
	REG_GPIO_PXPAT0C(1) = 0x00000010;	\
	REG_GPIO_PXINTC(3) =  0x3c000000;	\
	REG_GPIO_PXMASKC(3) = 0x3c000000;	\
	REG_GPIO_PXPAT1C(3) = 0x3c000000;	\
	REG_GPIO_PXPAT0S(3) = 0x3c000000;	\
	REG_GPIO_PXINTC(5) =  0x0000fff0;	\
	REG_GPIO_PXMASKC(5) = 0x0000fff0;	\
	REG_GPIO_PXPAT1C(5) = 0x0000fff0;	\
	REG_GPIO_PXPAT0C(5) = 0x0000fff0;	\
} while (0)
#endif

#define __gpio_as_eth()				\
do {						\
	REG_GPIO_PXINTC(1) =  0x00000010;	\
	REG_GPIO_PXMASKC(1) = 0x00000010;	\
	REG_GPIO_PXPAT1S(1) = 0x00000010;	\
	REG_GPIO_PXPAT0C(1) = 0x00000010;	\
	REG_GPIO_PXINTC(3) =  0x3c000000;	\
	REG_GPIO_PXMASKC(3) = 0x3c000000;	\
	REG_GPIO_PXPAT1C(3) = 0x3c000000;	\
	REG_GPIO_PXPAT0S(3) = 0x3c000000;	\
	REG_GPIO_PXINTC(5) =  0x0000fff0;	\
	REG_GPIO_PXMASKC(5) = 0x0000fff0;	\
	REG_GPIO_PXPAT1C(5) = 0x0000fff0;	\
	REG_GPIO_PXPAT0C(5) = 0x0000fff0;	\
} while (0)

/*
 * GPS_CLK GPS_MAG GPS_SIG
 */
#define __gpio_as_gps()			\
do {						\
	REG_GPIO_PXFUNS(5) = 0x00000007;	\
	REG_GPIO_PXTRGC(5) = 0x00000007;	\
	REG_GPIO_PXSELS(5) = 0x00000007;	\
	REG_GPIO_PXPENS(5) = 0x00000007;		\
} while (0)

/*
 * UART0_TxD, UART0_RxD
 */
#define __gpio_as_uart0()			\
do {						\
	REG_GPIO_PXINTC(5) = (0x9 << 0);	\
	REG_GPIO_PXMASKC(5) = (0x9 << 0);	\
	REG_GPIO_PXPAT0C(5) = (0x9 << 0);	\
	REG_GPIO_PXPAT1C(5)  = (0x9 << 0);	\
} while (0)
/*
 * UART0_TxD, UART0_RxD, UART0_CTS, UART0_RTS
 */
#define __gpio_as_uart0_ctsrts()		\
do {						\
	REG_GPIO_PXINTC(5) = (0xf << 0);	\
	REG_GPIO_PXMASKC(5) = (0xf << 0);	\
	REG_GPIO_PXPAT0C(5) = (0xf << 0);	\
	REG_GPIO_PXPAT1C(5)  = (0xF << 0);	\
} while (0)

/*
 * UART1_TxD, UART1_RxD
 */
#define __gpio_as_uart1()			\
do {						\
	REG_GPIO_PXINTC(1) = (0x9 << 14);	\
	REG_GPIO_PXMASKC(1) = (0x9 << 14);	\
	REG_GPIO_PXPAT0S(1) = (0x9 << 14);	\
	REG_GPIO_PXPAT1C(1)  = (0x9 << 14);	\
} while (0)
/*
 * UART1_TxD, UART1_RxD, UART1_CTS, UART1_RTS
 */
#define __gpio_as_uart1_ctsrts()		\
do {						\
	REG_GPIO_PXINTC(3) = (0xf << 26);	\
	REG_GPIO_PXMASKC(3) = (0xf << 26);	\
	REG_GPIO_PXPAT0C(3) = (0xf << 26);	\
	REG_GPIO_PXPAT1C(3)  = (0xf << 26);	\
} while (0)

/*
 * UART2_TxD, UART2_RxD
 */
#define __gpio_as_uart2()			\
do {						\
	REG_GPIO_PXINTC(3) = (0xc << 4);	\
	REG_GPIO_PXMASKC(3) = (0xc << 4);	\
	REG_GPIO_PXPAT0C(3) = (0xc << 4);	\
	REG_GPIO_PXPAT1S(3)  = (0xc << 4);	\
} while (0)
/*
 * UART2_TxD, UART2_RxD, UART2_CTS, UART2_RTS
 */
#define __gpio_as_uart2_ctsrts()		\
do {						\
	REG_GPIO_PXINTC(3) = (0xf << 4);	\
	REG_GPIO_PXMASKC(3) = (0xf << 4);	\
	REG_GPIO_PXPAT0C(3) = (0xf << 4);	\
	REG_GPIO_PXPAT1S(3)  = (0xf << 4);	\
} while (0)

/*
 * UART3_TxD, UART3_RxD
 */
#define __gpio_as_uart3()			\
do {						\
	REG_GPIO_PXINTC(3) = (0x1 << 12);	\
	REG_GPIO_PXMASKC(3) = (0x1 << 12);	\
	REG_GPIO_PXPAT0C(3) = (0x1 << 12);	\
	REG_GPIO_PXPAT1C(3)  = (0x1 << 12);	\
	REG_GPIO_PXINTC(4) = (0x1 << 5);	\
	REG_GPIO_PXMASKC(4) = (0x1 << 5);	\
	REG_GPIO_PXPAT0C(4) = (0x1 << 5);	\
	REG_GPIO_PXPAT1S(4)  = (0x1 << 5);	\
} while (0)
/*
 * UART3_TxD, UART3_RxD, UART3_CTS, UART3_RTS
 */
#define __gpio_as_uart3_ctsrts()			\
do {							\
	REG_GPIO_PXINTC(3) = (0x1 << 12);		\
	REG_GPIO_PXMASKC(3) = (0x1 << 12);		\
	REG_GPIO_PXPAT0C(3) = (0x1 << 12);		\
	REG_GPIO_PXPAT1C(3)  = (0x1 << 12);		\
	REG_GPIO_PXINTC(4) = (0x1 << 5) | (0x3 << 8);	\
	REG_GPIO_PXMASKC(4) = (0x1 << 5) | (0x3 << 8);	\
	REG_GPIO_PXPAT0C(4) = (0x1 << 5) | (0x3 << 8);	\
	REG_GPIO_PXPAT1S(4)  = (0x1 << 5);		\
	REG_GPIO_PXPAT1C(4)  = (0x3 << 8);		\
} while (0)

//fixme twxie
#define __gpio_as_uart4()			\
do {					\
} while (0)

#ifdef __BOOT_ROM__
/*
 * NAND GPIO INIT, n = GP_CLE_ALE
 *
 * Setup GPIO pins: D0 ~ D15, SD0 ~ SD15, CLE, ALE, CS1#, FRE#,
 * FWE#, FRB#, RDWE#/BUFD#. Disable all pull-up resistors.
 */
#define __gpio_as_nand()			\
do {						\
	/* CS1#, FRE#, FWE# */			\
	REG_GPIO_PXINTC(0) = 0x002C0000;	\
	REG_GPIO_PXMASKC(0) = 0x002C0000;	\
	REG_GPIO_PXPAT1C(0) = 0x002C0000;	\
	REG_GPIO_PXPAT0C(0) = 0x002C0000;	\
	REG_GPIO_PXPENS(0) = 0x002C0000;	\
	/* CLE, ALE */				\
	REG_GPIO_PXINTC(1) = 0x00000003;	\
	REG_GPIO_PXMASKC(1) = 0x00000003;	\
	REG_GPIO_PXPAT1C(1) = 0x00000003;	\
	REG_GPIO_PXPAT0C(1) = 0x00000003;	\
	REG_GPIO_PXPENS(1) = 0x00000003;	\
	/* FRB#(input) */			\
	REG_GPIO_PXINTC(0) = 0x00100000;	\
	REG_GPIO_PXMASKS(0) = 0x00100000;	\
	REG_GPIO_PXPAT1S(0) = 0x00100000;	\
} while (0)

/*
 * NAND_SD0 ~ NAND_SD7,
 */
#define __gpio_as_nand_8bit()			\
do {						\
	/* SD0 ~ SD7 */				\
        REG_GPIO_PXINTC(0) = 0x000000FF;	\
        REG_GPIO_PXMASKC(0) = 0x000000FF;	\
        REG_GPIO_PXPAT1C(0) = 0x000000FF; 	\
        REG_GPIO_PXPAT0C(0) = 0x000000FF;	\
} while (0)

#define __gpio_as_nand_16bit()			\
do {						\
	/* SD0 ~ SD15 */			\
        REG_GPIO_PXINTC(0) = 0x0000FFFF;	\
        REG_GPIO_PXMASKC(0) = 0x0000FFFF;	\
        REG_GPIO_PXPAT1C(0) = 0x0000FFFF;	\
        REG_GPIO_PXPAT0C(0) = 0x0000FFFF;	\
} while (0)

/* disable NAND nDQS */
#define __gpio_as_common_nand()			\
do { 						\
	/* nDQS Disable */			\
	REG_GPIO_PXMASKS(0) = 0x20000000;	\
	REG_GPIO_PXPAT1S(0) = 0x20000000;	\
	REG_GPIO_PXPAT0S(0) = 0x20000000;	\
} while (0)

#else

/*
 * SD0 ~ SD7, CS1#, CLE, ALE, FRE#, FWE#, FRB#
 * @n: chip select number(1 ~ 6)
 */
#define __gpio_as_nand_8bit(n)						\
do {		              						\
	REG_GPIO_PXINTC(0) = 0x000c00ff; /* SD0 ~ SD7, CS1#, FRE#, FWE# */ \
	REG_GPIO_PXMASKC(0) = 0x000c00ff;				\
	REG_GPIO_PXPAT1C(0) = 0x000c00ff;				\
	REG_GPIO_PXPAT0C(0) = 0x000c00ff;				\
	REG_GPIO_PXPENS(0) = 0x000c00ff;				\
									\
	REG_GPIO_PXINTC(1) = 0x00000003; /* CLE(SA2), ALE(SA3) */	\
	REG_GPIO_PXMASKC(1) = 0x00000003;				\
	REG_GPIO_PXPAT1C(1) = 0x00000003;				\
	REG_GPIO_PXPAT0C(1) = 0x00000003;				\
	REG_GPIO_PXPENS(1) = 0x00000003;				\
									\
	REG_GPIO_PXINTC(0) = 0x00200000 << ((n)-1); /* CSn */		\
	REG_GPIO_PXMASKC(0) = 0x00200000 << ((n)-1);			\
	REG_GPIO_PXPAT1C(0) = 0x00200000 << ((n)-1);			\
	REG_GPIO_PXPAT0C(0) = 0x00200000 << ((n)-1);			\
	REG_GPIO_PXPENS(0) = 0x00200000 << ((n)-1);			\
									\
	REG_GPIO_PXINTC(0) = 0x00100000; /* FRB#(input) */		\
	REG_GPIO_PXMASKS(0) = 0x00100000;				\
	REG_GPIO_PXPAT1S(0) = 0x00100000;				\
	REG_GPIO_PXPENS(0) = 0x00100000;				\
} while (0)

#define __gpio_as_nand_16bit(n)						\
do {		              						\
	REG_GPIO_PXINTC(0) = 0x000cffff; /* SD0 ~ SD15, CS1#, FRE#, FWE# */ \
	REG_GPIO_PXMASKC(0) = 0x000cffff;				\
	REG_GPIO_PXPAT1C(0) = 0x000cffff;				\
	REG_GPIO_PXPAT0C(0) = 0x000cffff;				\
	REG_GPIO_PXPENS(0) = 0x000cffff;				\
									\
	REG_GPIO_PXINTC(1) = 0x00000003; /* CLE(SA2), ALE(SA3) */	\
	REG_GPIO_PXMASKC(1) = 0x00000003;				\
	REG_GPIO_PXPAT1C(1) = 0x00000003;				\
	REG_GPIO_PXPAT0C(1) = 0x00000003;				\
	REG_GPIO_PXPENS(1) = 0x00000003;				\
									\
	REG_GPIO_PXINTC(0) = 0x00200000 << ((n)-1); /* CSn */		\
	REG_GPIO_PXMASKC(0) = 0x00200000 << ((n)-1);			\
	REG_GPIO_PXPAT1C(0) = 0x00200000 << ((n)-1);			\
	REG_GPIO_PXPAT0C(0) = 0x00200000 << ((n)-1);			\
	REG_GPIO_PXPENS(0) = 0x00200000 << ((n)-1);			\
									\
	REG_GPIO_PXINTC(0) = 0x00100000; /* FRB#(input) */		\
	REG_GPIO_PXMASKS(0) = 0x00100000;				\
	REG_GPIO_PXPAT1S(0) = 0x00100000;				\
	REG_GPIO_PXPENS(0) = 0x00100000;				\
} while (0)

#endif	/* __BOOT_ROM__ */

/* Toggle NAND nDQS */
#define __gpio_as_toggle_nand()						\
do {									\
	/* nDQS */							\
	REG_GPIO_PXINTC(0) = 0x20000000;				\
	REG_GPIO_PXMASKC(0) = 0x20000000;				\
	REG_GPIO_PXPAT1C(0) = 0x20000000;				\
	REG_GPIO_PXPAT0C(0) = 0x20000000;				\
	REG_GPIO_PXPENC(0) = 0x20000000;				\
} while (0)

/*
 * CS4#, RD#, WR#, WAIT#, A0 ~ A22, D0 ~ D7
 * @n: chip select number(1 ~ 4)
 */
#define __gpio_as_nor_8bit(n)						\
do {								        \
	/* 32/16-bit data bus */					\
	REG_GPIO_PXFUNS(0) = 0x000000ff;				\
	REG_GPIO_PXSELC(0) = 0x000000ff;				\
	REG_GPIO_PXPENS(0) = 0x000000ff;					\
									\
	REG_GPIO_PXFUNS(2) = 0x00200000 << ((n)-1); /* CSn */		\
	REG_GPIO_PXSELC(2) = 0x00200000 << ((n)-1);			\
	REG_GPIO_PXPENS(2) = 0x00200000 << ((n)-1);			\
									\
	REG_GPIO_PXFUNS(1) = 0x0000ffff; /* A0~A15 */			\
	REG_GPIO_PXSELC(1) = 0x0000ffff;				\
	REG_GPIO_PXPENS(1) = 0x0000ffff;					\
	REG_GPIO_PXFUNS(2) = 0x06110007; /* RD#, WR#, WAIT#, A20~A22 */	\
	REG_GPIO_PXSELC(2) = 0x06110007;				\
	REG_GPIO_PXPENS(2) = 0x06110007;					\
	REG_GPIO_PXFUNS(2) = 0x000e0000; /* A17~A19 */	        	\
	REG_GPIO_PXSELS(2) = 0x000e0000;				\
	REG_GPIO_PXPENS(2) = 0x000e0000;					\
} while (0)

/*
 * CS4#, RD#, WR#, WAIT#, A0 ~ A22, D0 ~ D15
 * @n: chip select number(1 ~ 4)
 */
#define __gpio_as_nor_16bit(n)						\
do {	               							\
	/* 32/16-bit data normal order */				\
	REG_GPIO_PXFUNS(0) = 0x0000ffff;				\
	REG_GPIO_PXSELC(0) = 0x0000ffff;				\
	REG_GPIO_PXPENS(0) = 0x0000ffff;					\
									\
	REG_GPIO_PXFUNS(2) = 0x00200000 << ((n)-1); /* CSn */		\
	REG_GPIO_PXSELC(2) = 0x00200000 << ((n)-1);			\
	REG_GPIO_PXPENS(2) = 0x00200000 << ((n)-1);			\
									\
	REG_GPIO_PXFUNS(1) = 0x0000ffff; /* A0~A15 */			\
	REG_GPIO_PXSELC(1) = 0x0000ffff;				\
	REG_GPIO_PXPENS(1) = 0x0000ffff;					\
	REG_GPIO_PXFUNS(2) = 0x06110007; /* RD#, WR#, WAIT#, A20~A22 */	\
	REG_GPIO_PXSELC(2) = 0x06110007;				\
	REG_GPIO_PXPENS(2) = 0x06110007;					\
	REG_GPIO_PXFUNS(2) = 0x000e0000; /* A17~A19 */	        	\
	REG_GPIO_PXSELS(2) = 0x000e0000;				\
	REG_GPIO_PXPENS(2) = 0x000e0000;					\
} while (0)

/*
 * LCD_D0~LCD_D7, LCD_PCLK, LCD_HSYNC, LCD_VSYNC, LCD_DE
 */
#define __gpio_as_lcd_8bit()			\
do {						\
	REG_GPIO_PXINTC(2) = 0x000c03ff;	\
	REG_GPIO_PXMASKC(2)  = 0x000c03ff;		\
	REG_GPIO_PXPAT0C(2) = 0x000c03ff;	\
	REG_GPIO_PXPAT1C(2) = 0x000c03ff;		\
} while (0)

/*
 * LCD_R3~LCD_R7, LCD_G2~LCD_G7, LCD_B3~LCD_B7,
 * LCD_PCLK, LCD_HSYNC, LCD_VSYNC, LCD_DE
 */
#define __gpio_as_lcd_16bit()			\
do {						\
	REG_GPIO_PXINTC(2) = 0x0f8ff3f8;	\
	REG_GPIO_PXMASKC(2) = 0x0f8ff3f8;	\
	REG_GPIO_PXPAT0C(2) = 0x0f8ff3f8;	\
	REG_GPIO_PXPAT1C(2) = 0x0f8ff3f8;		\
} while (0)

/*
 * LCD_R2~LCD_R7, LCD_G2~LCD_G7, LCD_B2~LCD_B7,
 * LCD_PCLK, LCD_HSYNC, LCD_VSYNC, LCD_DE
 */
#define __gpio_as_lcd_18bit()			\
do {						\
	REG_GPIO_PXINTC(2) = 0x0fcff3fc;	\
	REG_GPIO_PXMASKC(2) = 0x0fcff3fc;	\
	REG_GPIO_PXPAT0C(2) = 0x0fcff3fc;	\
	REG_GPIO_PXPAT1C(2) = 0x0fcff3fc;		\
} while (0)

/*
 * LCD_R0~LCD_R7, LCD_G0~LCD_G7, LCD_B0~LCD_B7,
 * LCD_PCLK, LCD_HSYNC, LCD_VSYNC, LCD_DE
 */
#define __gpio_as_lcd_24bit()			\
do {						\
	REG_GPIO_PXINTC(2) = 0x0fffffff;	\
	REG_GPIO_PXMASKC(2)  = 0x0fffffff;	\
	REG_GPIO_PXPAT0C(2) = 0x0fffffff;	\
	REG_GPIO_PXPAT1C(2) = 0x0fffffff;		\
} while (0)

/*
 *  LCD_CLS, LCD_SPL, LCD_PS, LCD_REV
 */
#define __gpio_as_lcd_special()			\
do {						\
	REG_GPIO_PXINTC(2) = 0x0fffffff;	\
	REG_GPIO_PXMASKC(2) = 0x0fffffff;	\
	REG_GPIO_PXPAT0C(2) = 0x0feffbfc;	\
	REG_GPIO_PXPAT0S(2) = 0x00100403;	\
	REG_GPIO_PXPAT1C(2) = 0x0fffffff;		\
} while (0)

/*
 * CIM_D0~CIM_D7, CIM_MCLK, CIM_PCLK, CIM_VSYNC, CIM_HSYNC
 */
#define __gpio_as_cim()					\
	do {						\
		REG_GPIO_PXINTC(1) = 0x0003ffc0;	\
		REG_GPIO_PXMASKC(1) = 0x0003ffc0;		\
		REG_GPIO_PXPAT1C(1) = 0x0003ffc0;	\
		REG_GPIO_PXPAT0C(1) = 0x0003ffc0;	\
	} while (0)

#define __gpio_as_cim_10bit()					\
	do {						\
		REG_GPIO_PXINTC(1) = 0x000fffc0;	\
		REG_GPIO_PXMASKC(1) = 0x000fffc0;	\
		REG_GPIO_PXPAT1C(1) = 0x000fffc0;	\
		REG_GPIO_PXPAT0C(1) = 0x000fffc0;	\
	} while (0)

/*
 * SCLK_RSTN, BCLK_AD, SYNC_AD, SDATI, BCLK, SYNC,
 * SDATO, SDATO1, SDATO2, SDATO3
 */
#if 1
#define __gpio_as_aic()					\
	do {						\
		__gpio_as_func2(GPE(5));		\
		__gpio_as_func0(GPE(6));		\
		__gpio_as_func0(GPE(7));		\
		__gpio_as_func1(GPE(8));		\
		__gpio_as_func1(GPE(9));		\
		__gpio_as_func1(GPD(12));		\
		__gpio_as_func0(GPD(13));		\
		__gpio_as_func0(GPE(11));		\
		__gpio_as_func0(GPE(12));		\
		__gpio_as_func0(GPE(13));		\
	} while (0)
#else

#define __gpio_as_aic()				\
do {						\
	REG_GPIO_PXFUNS(4) = 0x16c00000;	\
	REG_GPIO_PXTRGC(4) = 0x02c00000;	\
	REG_GPIO_PXTRGS(4) = 0x14000000;	\
	REG_GPIO_PXSELC(4) = 0x14c00000;	\
	REG_GPIO_PXSELS(4) = 0x02000000;	\
	REG_GPIO_PXPENS(4)  = 0x16c00000;	\
} while (0)
#endif

#define __gpio_as_pcm0()			\
	do {					\
		__gpio_as_func0(GPD(0));	\
		__gpio_as_func0(GPD(1));	\
		__gpio_as_func0(GPD(2));	\
		__gpio_as_func0(GPD(3));	\
	} while (0)

#define __gpio_as_pcm1()			\
	do {					\
		__gpio_as_func1(GPF(12));	\
		__gpio_as_func1(GPF(13));	\
		__gpio_as_func1(GPF(14));	\
		__gpio_as_func1(GPF(15));	\
	} while (0)

/*
 * MSC0_CMD, MSC0_CLK, MSC0_D0 ~ MSC0_D7
 */
#define __gpio_a_as_msc0_8bit()			\
do {						\
	REG_GPIO_PXINTC(0)  = 0x01fc00f0;	\
	REG_GPIO_PXMASKC(0) = 0x01fc00f0;	\
	REG_GPIO_PXPAT1C(0) = 0x01fc00f0;	\
	REG_GPIO_PXPAT0S(0) = 0x01fc00f0;	\
	REG_GPIO_PXPENC(0)   = 0x01000000;	\
} while (0)

#define __gpio_a_as_msc0_1bit()			\
do {						\
	REG_GPIO_PXINTC(0)  = 0x011c0000;	\
	REG_GPIO_PXMASKC(0) = 0x011c0000;	\
	REG_GPIO_PXPAT1C(0) = 0x011c0000;	\
	REG_GPIO_PXPAT0S(0) = 0x011c0000;	\
	REG_GPIO_PXPENC(0)  = 0x01000000;	\
} while (0)

#define __gpio_d_as_msc1_4bit()			\
do {						\
	REG_GPIO_PXINTC(3)  = 0x03f00000;	\
	REG_GPIO_PXMASKC(3) = 0x03f00000;	\
	REG_GPIO_PXPAT1C(3) = 0x03f00000;	\
	REG_GPIO_PXPAT0C(3) = 0x03f00000;	\
} while (0)

#define __gpio_e_as_msc2_4bit()			\
do {						\
	REG_GPIO_PXINTC(4) = 0x30f00000;	\
	REG_GPIO_PXMASKC(4)  = 0x30f00000;	\
	REG_GPIO_PXPAT0C(4) = 0x30f00000;	\
	REG_GPIO_PXPAT1S(4) = 0x30f00000;	\
} while (0)

/*
 * MSC0_CMD, MSC0_CLK, MSC0_D0 ~ MSC0_D3
 */
#define __gpio_as_msc 	__gpio_a_as_msc0_8bit /* default as msc0 4bit */
#define __gpio_as_msc0 	__gpio_a_as_msc0_8bit /* msc0 default as 4bit */
#define __gpio_as_msc1 	__gpio_d_as_msc1_4bit /* msc1 only support 4bit */

/*
 * TSCLK, TSSTR, TSFRM, TSFAIL, TSDI0~7
 */
#define __gpio_as_tssi()                        \
do {                                            \
        REG_GPIO_PXINTC(1) = 0xf0300000;        \
        REG_GPIO_PXMASKC(1)  = 0xf0300000;      \
        REG_GPIO_PXPAT0S(1) = 0xf0300000;       \
        REG_GPIO_PXPAT1S(1) = 0xf0300000;       \
                                                \
        REG_GPIO_PXINTC(1) = 0x0fc00000;        \
        REG_GPIO_PXMASKC(1)  = 0x0fc00000;      \
        REG_GPIO_PXPAT0C(1) = 0x0fc00000;       \
        REG_GPIO_PXPAT1C(1) = 0x0fc00000;       \
} while (0)

/*
 * SSI0_CE0, SSI0_CLK, SSI0_DT, SSI0_DR
 */
//fixme twxie
#if 1
#define __gpio_as_ssi0()				\
	do {						\
		REG_GPIO_PXINTC(4) = (0xf << 14);	\
		REG_GPIO_PXMASKC(4) = (0xf << 14);	\
		REG_GPIO_PXPAT1C(4) = (0xf << 14);	\
		REG_GPIO_PXPAT0C(4) = (0xf << 14);	\
	} while (0)
#else
#define __gpio_as_ssi0()
#endif
#define __gpio_as_ssi() __gpio_as_ssi0()

#define __gpio_as_spi_boot()				\
	do {						\
		REG_GPIO_PXINTC(0) = (0x7 << 18);	\
		REG_GPIO_PXMASKC(0) = (0x7 << 18);	\
		REG_GPIO_PXPAT1S(0) = (0x7 << 18);	\
		REG_GPIO_PXPAT0C(0) = (0x7 << 18);	\
	} while (0)

/*
 * SSI1_CE1, SSI1_CLK, SSI1_DT, SSI1_DR
 */
//fixme twxie
#if 1
#define __gpio_as_ssi1()				\
	do {						\
		REG_GPIO_PXINTC(3) = (0x33 << 20);	\
		REG_GPIO_PXMASKC(3) = (0x33 << 20);	\
		REG_GPIO_PXPAT1S(3) = (0x33 << 20);	\
		REG_GPIO_PXPAT0C(3) = (0x33 << 20);	\
	} while (0)
#else
#define __gpio_as_ssi1()
#endif
#define __gpio_as_ssi_1() __gpio_as_ssi1()

/*
 * SSI2_CE0, SSI2_CE2, SSI2_GPC, SSI2_CLK, SSI2_DT, SSI12_DR
 */
//fixme twxie
#if 0
#define __gpio_as_ssi2_1()			\
do {						\
	REG_GPIO_PXFUNS(5) = 0xf0300000;	\
	REG_GPIO_PXTRGC(5) = 0xf0300000;	\
	REG_GPIO_PXSELS(5) = 0xf0300000;	\
	REG_GPIO_PXPENS(5)  = 0xf0300000;	\
} while (0)
#endif
#define __gpio_as_ssi2_1()

/*
 * I2C_SCK, I2C_SDA
 */
#define __gpio_as_i2c(n)				\
	do {						\
		REG_GPIO_PXINTC(3 + (n)) = 0xC0000000;	\
		REG_GPIO_PXMASKC(3 + (n)) = 0xC0000000;	\
		REG_GPIO_PXPAT1C(3 + (n)) = 0xC0000000;	\
		REG_GPIO_PXPAT0C(3 + (n)) = 0xC0000000;	\
	} while (0)

#define __gpio_as_i2c2()				\
	do {						\
		REG_GPIO_PXINTC(5) = 0x00030000;	\
		REG_GPIO_PXMASKC(5) = 0x00030000;	\
		REG_GPIO_PXPAT1S(5) = 0x00030000;	\
		REG_GPIO_PXPAT0C(5) = 0x00030000;	\
	} while(0)
/*
 * PWM0
 */
#define __gpio_as_pwm0()			\
do {						\
	REG32(GPIO_PXDATC(4)) = 0x1;		\
	REG32(GPIO_PXIMC(4)) = 0x1;		\
	REG32(GPIO_PXPEC(4)) = 0x1;		\
	REG32(GPIO_PXFUNC(4)) = 0x1;		\
} while (0)

/*
 * PWM1
 */
#define __gpio_as_pwm1()			\
do {						\
	REG32(GPIO_PXDATC(4)) = 0x2;		\
	REG32(GPIO_PXIMC(4)) = 0x2;		\
	REG32(GPIO_PXPEC(4)) = 0x2;		\
	REG32(GPIO_PXFUNC(4)) = 0x2;		\
} while (0)

/*
 * PWM2
 */
#define __gpio_as_pwm2()			\
do {						\
	REG32(GPIO_PXDATC(4)) = 0x4;		\
	REG32(GPIO_PXIMC(4)) = 0x4;		\
	REG32(GPIO_PXPEC(4)) = 0x4;		\
	REG32(GPIO_PXFUNC(4)) = 0x4;		\
} while (0)

/*
 * PWM3
 */
#define __gpio_as_pwm3()			\
do {						\
	REG32(GPIO_PXDATC(4)) = 0x8;		\
	REG32(GPIO_PXIMC(4)) = 0x8;		\
	REG32(GPIO_PXPEC(4)) = 0x8;		\
	REG32(GPIO_PXFUNC(4)) = 0x8;		\
} while (0)

/*
 * PWM4
 */
#define __gpio_as_pwm4()			\
do {						\
	REG32(0xB0010418) = 0x10;		\
	REG32(0xB0010428) = 0x10;		\
	REG32(0xB0010438) = 0x10;		\
	REG32(0xB0010448) = 0x10;		\
} while (0)

/*
 * PWM5
 */
#define __gpio_as_pwm5()			\
do {						\
	REG32(GPIO_PXDATC(4)) = 0x20;		\
	REG32(GPIO_PXIMC(4)) = 0x20;		\
	REG32(GPIO_PXPEC(4)) = 0x20;		\
	REG32(GPIO_PXFUNC(4)) = 0x20;		\
} while (0)

#define __gpio_as_pwm6()			\
do {						\
	REG32(GPIO_PXDATC(3)) = 0x1 << 10;	\
	REG32(GPIO_PXIMC(3)) = 0x1 << 10;	\
	REG32(GPIO_PXPEC(3)) = 0x1 << 10;	\
	REG32(GPIO_PXFUNC(3)) = 0x1 << 10;	\
} while (0)

#define __gpio_as_pwm7()			\
do {						\
	REG32(GPIO_PXDATC(3)) = 0x1 << 11;	\
	REG32(GPIO_PXIMC(3)) = 0x1 << 11;	\
	REG32(GPIO_PXPEC(3)) = 0x1 << 11;	\
	REG32(GPIO_PXFUNC(3)) = 0x1 << 11;	\
} while (0)

/*
 * n = 0 ~ 7
 */
#define __gpio_as_pwm(n)	__gpio_as_pwm##n()

/*
 * OWI - PA29 function 1
 */
#define __gpio_as_owi()				\
do {						\
	REG_GPIO_PXFUNS(0) = 0x20000000;	\
	REG_GPIO_PXTRGC(0) = 0x20000000;	\
	REG_GPIO_PXSELS(0) = 0x20000000;	\
} while (0)

/*
 * SCC - PD08 function 0
 *       PD09 function 0
 */
#define __gpio_as_scc()				\
do {						\
	REG_GPIO_PXFUNS(3) = 0xc0000300;	\
	REG_GPIO_PXTRGC(3) = 0xc0000300;	\
	REG_GPIO_PXSELC(3) = 0xc0000300;	\
} while (0)

#define __gpio_as_otg_drvvbus()	\
do {	\
	__gpio_as_func0(4*32+10);		\
} while (0)

//-------------------------------------------
// GPIO or Interrupt Mode

#define __gpio_get_port(p)	(REG_GPIO_PXPIN(p))
#define __gpio_port_as_output0(p, o)		\
do {						\
    REG_GPIO_PXINTC(p) = (1 << (o));		\
    REG_GPIO_PXMASKS(p) = (1 << (o));		\
    REG_GPIO_PXPAT1C(p) = (1 << (o));		\
    REG_GPIO_PXPAT0C(p) = (1 << (o));		\
} while (0)

#define __gpio_port_as_output1(p, o)		\
do {						\
    REG_GPIO_PXINTC(p) = (1 << (o));		\
    REG_GPIO_PXMASKS(p) = (1 << (o));		\
    REG_GPIO_PXPAT1C(p) = (1 << (o));		\
    REG_GPIO_PXPAT0S(p) = (1 << (o));		\
} while (0)

#define __gpio_port_as_input(p, o)		\
do {						\
    REG_GPIO_PXINTC(p) = (1 << (o));		\
    REG_GPIO_PXMASKS(p) = (1 << (o));		\
    REG_GPIO_PXPAT1S(p) = (1 << (o));		\
    REG_GPIO_PXPAT0C(p) = (1 << (o));		\
} while (0)


#define __gpio_as_output0(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	__gpio_port_as_output0(p, o);		\
} while (0)

#define __gpio_as_output1(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	__gpio_port_as_output1(p, o);		\
} while (0)

#define __gpio_as_output(n)	__gpio_as_output0(n)

#define __gpio_as_input(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	__gpio_port_as_input(p, o);		\
} while (0)

#define __gpio_set_pin(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	REG_GPIO_PXPAT0S(p) = (1 << o);		\
} while (0)

#define __gpio_clear_pin(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	REG_GPIO_PXPAT0C(p) = (1 << o);		\
} while (0)

#define __gpio_get_pin(n)			\
({						\
	unsigned int p, o, v;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	if (__gpio_get_port(p) & (1 << o))	\
		v = 1;				\
	else					\
		v = 0;				\
	v;					\
})

#define __gpio_as_irq_high_level(n)		\
	do {					\
		unsigned int p, o;		\
		p = (n) / 32;			\
		o = (n) % 32;			\
		REG_GPIO_PXINTS(p) = (1 << o);	\
		REG_GPIO_PXMASKC(p) = (1 << o);	\
		REG_GPIO_PXPAT1C(p) = (1 << o);	\
		REG_GPIO_PXPAT0S(p) = (1 << o);	\
	} while (0)

#define __gpio_as_irq_low_level(n)		\
	do {					\
		unsigned int p, o;		\
		p = (n) / 32;			\
		o = (n) % 32;			\
		REG_GPIO_PXINTS(p) = (1 << o);	\
		REG_GPIO_PXMASKC(p) = (1 << o);	\
		REG_GPIO_PXPAT1C(p) = (1 << o);	\
		REG_GPIO_PXPAT0C(p) = (1 << o);	\
	} while (0)

#define __gpio_as_irq_rise_edge(n)		\
	do {					\
		unsigned int p, o;		\
		p = (n) / 32;			\
		o = (n) % 32;			\
		REG_GPIO_PXINTS(p) = (1 << o);	\
		REG_GPIO_PXMASKC(p) = (1 << o);	\
		REG_GPIO_PXPAT1S(p) = (1 << o);	\
		REG_GPIO_PXPAT0S(p) = (1 << o);	\
	} while (0)

#define __gpio_as_irq_fall_edge(n)		\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	REG_GPIO_PXINTS(p) = (1 << o);  \
        REG_GPIO_PXMASKC(p) = (1 << o);   \
        REG_GPIO_PXPAT1S(p) = (1 << o); \
        REG_GPIO_PXPAT0C(p) = (1 << o); \
} while (0)

#define __gpio_mask_irq(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	REG_GPIO_PXMASKS(p) = (1 << o);		\
} while (0)

#define __gpio_unmask_irq(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	REG_GPIO_PXMASKC(p) = (1 << o);		\
} while (0)

#define __gpio_ack_irq(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	REG_GPIO_PXFLGC(p) = (1 << o);		\
} while (0)

#define __gpio_group_irq(n)			\
({						\
	register int tmp, i;			\
	tmp = REG_GPIO_PXFLG((n));		\
	for (i=31;i>=0;i--)			\
		if (tmp & (1 << i))		\
			break;			\
	i;					\
})

#define __gpio_enable_pull(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	REG_GPIO_PXPEC(p) = (1 << o);		\
} while (0)

#define __gpio_disable_pull(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	REG_GPIO_PXPENS(p) = (1 << o);		\
} while (0)


#endif /* __MIPS_ASSEMBLER */

#endif /* __JZ4780GPIO_H__ */

