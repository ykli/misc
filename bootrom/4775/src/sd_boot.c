/*
 *	sd_boot.c
 */
#include <irom.h>
#include <jz4775msc.h>
#include <jz4775gpio.h>
#include <jz4775misc.h>
#include <jz4775cpm.h>
#include <common.h>

static int rca;
static volatile int retry_times = 1;
static int current_boot;
static int ctl_num;
extern u32 start_addr;
extern int usb_boot(void);	
int sd_boot(int);
int alternative_boot(void);

#define SD_BOOT_ROM_SIZE 	0x1c * 512	//14k
#define EMMC_BOOT_ROM_SIZE 	0x1c * 512	//14k
#define SD_BOOT_ROM_DST  	0xf4000800

#define SD_BOOT                 1
#define ALTERNATIVE_BOOT        3

#define MMC_IRQ_MASK()					\
do {							\
	REG_MSC_IMASK(ctl_num) = 0xffffffff;		\
	REG_MSC_IREG(ctl_num) = 0xffffffff;		\
} while(0)

#define MMC_TO()				\
do {						\
	REG_MSC_RESTO(ctl_num) = 0x100;		\
	REG_MSC_RDTO(ctl_num) = 0x1ffffff;	\
} while(0)

static void sd_init(void)
{
	if (ctl_num == 0)
		__gpio_a_as_msc0_1bit();
	else
		__gpio_e_as_msc1_4bit();
	__msc_reset(ctl_num);
	MMC_IRQ_MASK();
	MMC_TO();
}

static inline void wait_prog_done(void)
{
	while (!(REG_MSC_STAT(ctl_num) & MSC_STAT_PRG_DONE))
		;
	REG_MSC_IREG(ctl_num) = MSC_IREG_PRG_DONE;	
}

static inline void jz_mmc_start(void)
{
	REG_MSC_STRPCL(ctl_num) = MSC_STRPCL_START_OP;
}

static u8* mmc_cmd(u16 cmd, u32 arg, u32 cmdat, u16 rtype)
{
	static u8 resp[6];
	int i;

	REG_MSC_CMD(ctl_num) = cmd;	
	REG_MSC_ARG(ctl_num) = arg;
	REG_MSC_CMDAT(ctl_num) = cmdat;

	jz_mmc_start();
	/* cmd0 with arg 0xfffffffa and set the MSC_CMDAT_BOOT_MODE_A can not wait for MSC_STAT_END_CMD_RES set */
	if ((arg == 0xfffffffa) && (cmd == 0) && (cmdat & MSC_CMDAT_BOOT_MODE_A))
		return 0;
	
	while (!(REG_MSC_STAT(ctl_num) & MSC_STAT_END_CMD_RES))
		;
	REG_MSC_IREG(ctl_num) = MSC_IREG_END_CMD_RES;
	
	if (rtype == MSC_CMDAT_RESPONSE_NONE)
		        return 0;

	for (i = 2; i >= 0; i--) {
		u16 res_fifo = REG_MSC_RES(ctl_num);
		int offset = i << 1;	
		
		resp[offset] = ((u8 *)&res_fifo)[0];
		resp[offset + 1] = ((u8 *)&res_fifo)[1];
	}
	
	return resp;
}

static int error_handler(u32 boot_mode)
{
	if (ctl_num == 1)
		return usb_boot();
	if (retry_times) {
		retry_times--;

		switch(boot_mode)
		{
		case SD_BOOT:
			sd_boot(0);
			break;
		case ALTERNATIVE_BOOT:
			alternative_boot();
			break;
		}
		return 0;
	}else {
		retry_times = 1;
		return sd_boot(1);	
	}
}

static int mmc_block_read(u32 size, u32 *dst)
{
	u8 *resp;
	u32 nob, cnt, stat;
		
	resp = mmc_cmd(16, 0x200, 0x1, MSC_CMDAT_RESPONSE_R1);

	REG_MSC_BLKLEN(ctl_num) = 0x200;
	REG_MSC_NOB(ctl_num) = size / 512;
	
	resp = mmc_cmd(18, 0x0, 0x9, MSC_CMDAT_RESPONSE_R1);	//more than 16
	nob = size / 64;
	
	for (; nob > 0; nob--) {
		while(1){
			stat = REG_MSC_STAT(ctl_num);	
			if (stat & (MSC_STAT_TIME_OUT_READ | MSC_STAT_CRC_READ_ERROR)) {
				goto err;
			}

			if(!(stat & MSC_STAT_DATA_FIFO_EMPTY))
				break;
		}
		
		cnt = 16;		//16 words
		while (cnt--) {
			while (!(REG_MSC_IREG(ctl_num) & MSC_IREG_RXFIFO_RD_REQ))
				;
			*dst++ = REG_MSC_RXFIFO(ctl_num);
		}
	}
	resp = mmc_cmd(12, 0, 0x1, MSC_CMDAT_RESPONSE_R1);
	return 0;
err:
	return 1;
}

static int mmc_block_readp(u32 size, u32 *dst)
{
	u32 nob, cnt, stat;
	
	nob = size / 64;
	
	xtimeout(50*1000,1);

	while (!(REG_MSC_STAT(ctl_num) & MSC_STAT_BAR)) 
		if (!xtimeout(51*1000, 0))
			goto err;

	if (REG_MSC_STAT(ctl_num) & MSC_STAT_BAE)
			goto err;

	for (; nob > 0; nob--) {
		while(1){
			stat = REG_MSC_STAT(ctl_num);	

			if (stat & (MSC_STAT_TIME_OUT_READ | MSC_STAT_CRC_READ_ERROR | MSC_STAT_BCE))
				goto err;

			if(!(stat & MSC_STAT_DATA_FIFO_EMPTY))
				break;
		}

		cnt = 16;		//16 words
		while (cnt--) {
			while (!(REG_MSC_IREG(ctl_num) & MSC_IREG_RXFIFO_RD_REQ))
				;
			*dst++ = REG_MSC_RXFIFO(ctl_num);
		}
	}

	return 0;
err:
	if (REG_MSC_STAT(ctl_num) & MSC_STAT_BCE)
		REG_MSC_CMDAT(ctl_num) |= (MSC_CMDAT_DIS_BOOT);

	return 1;
}

static int sd_found(void)
{
	u8 *resp;
	u32 cardaddr, timeout = 100;

	resp = mmc_cmd(55, 0, 0x1, MSC_CMDAT_RESPONSE_R1);
	resp = mmc_cmd(41, 0x40ff8000, 0x3, MSC_CMDAT_RESPONSE_R3);

	while (timeout-- && !(resp[4] & 0x80)) {
		xmdelay(10);
		resp = mmc_cmd(55, 0, 0x1, MSC_CMDAT_RESPONSE_R1);
		resp = mmc_cmd(41, 0x40ff8000, 0x3, MSC_CMDAT_RESPONSE_R3);
	}

	if (!(resp[4] & 0x80))
		return 1;		

	resp = mmc_cmd(2, 0, 0x2, MSC_CMDAT_RESPONSE_R2);
	resp = mmc_cmd(3, 0, 0x6, MSC_CMDAT_RESPONSE_R6);
	cardaddr = (resp[4] << 8) | resp[3];
	rca = cardaddr << 16;

	REG_MSC_CLKRT(ctl_num) = 1;
	resp = mmc_cmd(7, rca, 0x1, MSC_CMDAT_RESPONSE_R1);
	resp = mmc_cmd(55, rca, 0x1, MSC_CMDAT_RESPONSE_R1);
	resp = mmc_cmd(6, 0, 0x1, MSC_CMDAT_RESPONSE_R1);

	return 0;
}

static int mmc_found(void)
{
	u8 *resp;
	u32 timeout = 100;

	if (ctl_num == 1)
		return 1;

	resp = mmc_cmd(1, 0x40ff8000, 0x3, MSC_CMDAT_RESPONSE_R3);

	while (timeout-- && !(resp[4] & 0x80)) {
		xmdelay(10);
		resp = mmc_cmd(1, 0x40ff8000, 0x3, MSC_CMDAT_RESPONSE_R3);
	}

	if (!(resp[4] & 0x80))
		return 1;		

	resp = mmc_cmd(2, 0, 0x2, MSC_CMDAT_RESPONSE_R2);
	resp = mmc_cmd(3, 0x10, 0x1, MSC_CMDAT_RESPONSE_R1);

	REG_MSC_CLKRT(ctl_num) = 1;
	resp = mmc_cmd(7, 0x10, 0x1, MSC_CMDAT_RESPONSE_R1);
	resp = mmc_cmd(6, 0x3b70001, 0x41, MSC_CMDAT_RESPONSE_R1);

	wait_prog_done();

	return 0;
}

int sd_boot(num)
{
	u8 *resp;
	u32 ret;

	current_boot = SD_BOOT;
	ctl_num = num;

	if (ctl_num == 1) 
		REG_CPM_CLKGR &= ~(1 << 11);

	sd_init();
	REG_MSC_CLKRT(ctl_num) = 7;
	REG_MSC_LPM(ctl_num) = 1;

	/* cmd12 reset when we reading or writing from the card, send this cmd */
	resp = mmc_cmd(12, 0, 0x41, MSC_CMDAT_RESPONSE_R1);

	resp = mmc_cmd(0, 0, 0x80, MSC_CMDAT_RESPONSE_NONE);
	resp = mmc_cmd(8, 0x1aa, 0x1, MSC_CMDAT_RESPONSE_R1);

	resp = mmc_cmd(55, 0, 0x1, MSC_CMDAT_RESPONSE_R1);

	if (resp[0] & 0x20){
		if (resp[5] == 0x37){
			resp = mmc_cmd(41, 0x40ff8000, 0x3, MSC_CMDAT_RESPONSE_R3);
			if (resp[5] == 0x3f)
				ret = sd_found();
			else
				ret = mmc_found();
		} else {
			ret = mmc_found();
		}
	} else{
		ret = mmc_found();
	}

	if (ret)
		return error_handler(current_boot);
	
	ret = mmc_block_read(SD_BOOT_ROM_SIZE, (u32 *)SD_BOOT_ROM_DST);	//SDRAM ADDR

	if(!ret)
	{
		if (!(REG32(SD_BOOT_ROM_DST + 512) == 0x4d53504c))
			return sd_boot(1);
		else
			return xfer_d2i(start_addr + 516, 0);

	}else{
		return error_handler(current_boot);
	}
}

int alternative_boot(void)
{
	int ret;

	current_boot = ALTERNATIVE_BOOT;
	ctl_num = 0;

	sd_init();
	REG_MSC_CLKRT(ctl_num) = 1;
	REG_MSC_LPM(ctl_num) = 1;
	
	/* cmd12 reset when we reading or writing from the card, send this cmd */
	mmc_cmd(12, 0, 0x1, MSC_CMDAT_RESPONSE_R1);

	mmc_cmd(0, 0xf0f0f0f0, 0x80, MSC_CMDAT_RESPONSE_NONE);
	REG_MSC_BLKLEN(ctl_num) = 0x200;
	REG_MSC_NOB(ctl_num) = EMMC_BOOT_ROM_SIZE / 512;
	mmc_cmd(0, 0xfffffffa, ((MSC_CMDAT_INIT) | (MSC_CMDAT_EXP_BOOT_ACK) | (MSC_CMDAT_BOOT_MODE_A) | (MSC_CMDAT_DATA_EN)), MSC_CMDAT_RESPONSE_NONE);

	ret = mmc_block_readp(EMMC_BOOT_ROM_SIZE, (u32 *)SD_BOOT_ROM_DST);

	if(!ret){
		mmc_cmd(0, 0, 0x0, MSC_CMDAT_RESPONSE_NONE);

		if (!(REG32(SD_BOOT_ROM_DST + 512) == 0x4d53504c)){
			return sd_boot(1);
		}
		return xfer_d2i(start_addr + 516, 0);
	}else{
		return error_handler(current_boot);
	}
}
