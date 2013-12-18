#include <irom.h>
#include <jz4780gpio.h>
#include <jz4780misc.h>
#include <jz4780cpm.h>
#include <jz4780otp.h>
#include <common.h>
#include "secall.h"
#include "pdma.h"
#include "cache.h"
#include "mipsop.h"
#include "firmware.h"
#define CPU_TCSM    0xF4000800
#define TCSM_SIGNATURE_OFFSET 0x3778
#define TCSM_SIGNATURE_SRC (CPU_TCSM + TCSM_SIGNATURE_OFFSET)
#define TCSM_SE_DATA  TCSM_BANK(1)

volatile static struct args *b_args;
extern int cpufreq;

struct cache_info {
	unsigned int size;
	unsigned int ways;
	unsigned int linesize;
}; 

volatile static struct cache_info L1_dcache;
	
__attribute__ ((__section__ (".start_text"))) void power_off_device()
{
	while((*(volatile unsigned int *)0xb0003000 & (1 << 7)) == 0);
	*(volatile unsigned int *)0xb000303c = 0xa55a;
	while((*(volatile unsigned int *)0xb0003000 & (1 << 7)) == 0);
	*(volatile unsigned int *)0xb0003020 = 1;
	while((*(volatile unsigned int *)0xb0003000 & (1 << 7)) == 0);
}
void load_firmware()
{
	unsigned int *src = reset_handler;
        unsigned int *dst = (unsigned int *)PHY2VIRT(TCSM_BANK0);
        int i;
	for(i = 0;i < RESET_LEN;i++)
	{
		*dst++ = *src++;
	} 
	flush_dcache_range((unsigned int)PHY2VIRT(TCSM_BANK0),(unsigned int)PHY2VIRT(TCSM_BANK0 + RESET_LEN * 4));
	jz_sync();
}


int cache_init(void)
{
	L1_dcache.linesize = 32;
	L1_dcache.size = 1024 * 32;
	L1_dcache.ways = 8;
	return 0;
}

void flush_dcache_range(unsigned int start,unsigned int end)
{
	start &= ~(CFG_CACHELINE_SIZE-1);
	while (start <= end) {
		cache_unroll(start,Hit_Writeback_Inv_D);
		start += L1_dcache.linesize;
	}
	__asm__ volatile ("sync");
}

int init_se_mcu()
{
	unsigned int *d = (unsigned int *)TCSM_SE_ARG;
	int i;
	b_args = (struct args *)TCSM_SE_ARG;
	for(i = 0;i < sizeof(struct args) / 4;i++)
	{
		*d++ = 0;
	}
	b_args->arg[0] = get_secall_off(TCSM_SE_DATA);       
	i = secall(b_args,SC_FUNC_RSA_KEY_INIT,0);
	return i;
}

void move_data(void *src,int slen,void *target,int tlen)
{
	unsigned int *msrc = (unsigned int *)src;
	unsigned int *mtar = (unsigned int *)PHY2VIRT(target);
	unsigned int i;
	unsigned int len = (slen + 3) / 4;
	for(i = 0; i < len;i++)
	{
		*(mtar + i) = *(msrc + i);
	}
	flush_dcache_range((unsigned int)mtar, (unsigned int)mtar + len * 4);
	jz_sync();
}

#define UPLOAD_MAX_LENS 1024 * 14
int upload_data(void *d,int len)
{
	int i;
	int ret;
	int tran_len = 0;
	for(i = 0; i < len; i+= tran_len)
	{
		tran_len = (len-i)  > UPLOAD_MAX_LENS ? UPLOAD_MAX_LENS : (len-i);
		move_data((void *)((unsigned int)d + i),tran_len,(void *)TCSM_SE_DATA,tran_len);
		b_args->arg[0] = i;
		b_args->arg[1] = len;
		b_args->arg[2] = tran_len;
		b_args->arg[3] = get_secall_off(TCSM_SE_DATA);
		ret = secall(b_args,SC_FUNC_CHECK_DATA,SC_STATE_UPDATE);		
		if(ret != 0)
		{
			return ret;
		}
	}
	return 0;
}

int load_key(void *target,int *tlen)
{
	unsigned int length;
	unsigned int *msrc = (unsigned int *)(TCSM_SIGNATURE_SRC + 2 * 4);
	unsigned int *mtar = (unsigned int *)PHY2VIRT(target);
	length = *msrc;
	if(length == 0)
	{
		return 0;
	}
	int i;
	for(i = 0;i < length;i++)
	{
		*(mtar + i) = *(msrc + i + 1);
	}
	*tlen = length;
	flush_dcache_range((unsigned int)mtar, (unsigned int)mtar + length * 4);
	jz_sync();
	return length;
}

int check_data()
{
	int tlen = 0;
	int i;
	i = load_key((void *)TCSM_SE_DATA,&tlen);
	if( i == 0)
	{
		return -1;
	}
	for(i = 0;i < 8;i++)
	{
		b_args->arg[i] = 0;
	}
	b_args->arg[0] = tlen;
	b_args->arg[1] = get_secall_off(TCSM_SE_DATA);
	i = secall(b_args,SC_FUNC_CHECK_DATA,SC_STATE_UPDATECHECK);
	return  i;
}
void get_length_offset(unsigned int *off,unsigned int *len)
{
	unsigned int * dst = (unsigned int *)(TCSM_SIGNATURE_SRC);
	*off = *dst;
	*len = *(dst+1);	
}
void read_efuse_protect()
{
	//setting the efuse control register
	REG_OTP_CTRL = (0x1e0 << 21) | (0 << 16) | (1 << 0);
	while(!(REG_OTP_STATE & OTP_STATE_RD_DONE));
}
//divice the clk,make sure AHB2 clk is 200MHZ
void div_clock()
{
	unsigned int tmp;
	//init the APLL
	//write CPM.CPAPCR
	//APLLM = 0xC7(199),APLLN = 0x2,APLLOD = 0x1;APLLEN = 0x1;
	REG_CPM_CPAPCR |= (0xc7 << 19)|(2 << 13)|(1 << 9)|(1 << 0);
	while(!(REG_CPM_CPAPCR &(1 << 4)));	

	//write CPM.CPCCR
	//PDIV=3,H2DIV=1,H0DIV=1,L2CDIV=1;CDIV=0;
	//CE_CPU=CE_AHB0=CE_AHB2=1,other bits keep the original value
	tmp = REG_CPM_CPCCR & 0xfffffff0;
	tmp |= (1 << 22)|(1 << 21)|(1 << 20)|(3 << 16)|(1 << 12)|(1 << 8)|(1 << 4);
	REG_CPM_CPCCR = tmp;

	while(REG_CPM_CPCSR & 7);

	tmp = REG_CPM_CPCCR;
	tmp &= ~(0xff << 24);
	tmp |= (1 << 24) | (1 << 26) | (1 << 28) | (1 << 30);
	REG_CPM_CPCCR = tmp;
	
	//Setting the REG_OTP_EFUSE
	tmp = (RD_ADJ << 20) | (RD_STROBE << 16) | (WR_ADJ << 12) | (WR_STROBE << 0);
	REG_OTP_CFG = tmp;
	cpufreq = 400;
}

int sec_boot(void)
{
	int ret;
	unsigned int offset,length;
	//Initialize the d-cache
	cache_init();
	//Adjust the h2clk to 200MHZ
	div_clock();
	//load the firmware to TCSM,and reset the MCU
	reset_mcu();
	load_firmware();	
	b_args = (struct args *)TCSM_SE_ARG;
	b_args->func = 1;
	b_args->retval = 0xffffffff;
	boot_up_mcu();
	ret = init_se_mcu();
	if(ret != 0)
	{
		return -1;
	}
	//Upload the SPL to MCU
	get_length_offset(&offset,&length);
	//if check length == 0,return check failed
	if(length == 0)
	{
		return -1;
	}
	ret = upload_data((void *)(CPU_TCSM + offset),length);
	if(ret != 0)
	{
		return -1;
	}
	//Load the Signature to MCU
	ret = check_data();
	if(ret != 0)
	{
		return -1;
	}
	read_efuse_protect();
	return 0;
}
