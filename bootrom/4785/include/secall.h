#ifndef _SECALL_H_
#define _SECALL_H_

#include "pdma.h"
#define SC_FUNC_CHECK_DATA 	0x1
#define SC_FUNC_RSA		0x3
#define SC_FUNC_MD5		0x7
#define SC_FUNC_BURN		0xf
#define SC_FUNC_RSAEQFR	 	0x1f
#define SC_FUNC_RSAEQ	 	0x3f
#define SC_FUNC_RSA_KEY_INIT	0xff
#define SC_FUNC_TEST		0x1ff

#define SC_STATE_UPDATE		0x3
#define SC_STATE_UPDATECHECK 0x7

#define SC_BURN_STAGE_0		0x1
#define SC_BURN_STAGE_1		0x3
#define SC_BURN_STAGE_2		0x7


#define ERR_SC_ALREAD_ENABLED 		0x0e300000
#define ERR_DES_BUSY			0x0e310000
#define ERR_DES_TIMEOUT			0x0e310001
#define ERR_INVALID_NK			0x0e320000

#define ERR_CHECK_DATA_FAILED		0x0e330000
#define ERR_CHECK_DATA_FAILED_MAX   0x0e331000

#define ERR_INVALID_INPUT		0x0e340000
#define ERR_INVALID_STEP		0x0e350000

struct args {
	unsigned int func;
	unsigned int state;
	unsigned int retval;
	unsigned int arg[8];
};

#define send_secall(func) ( REG32(DMCS) = (REG32(DMCS) & 0xff0000ff) | 0x8 | ((func)<<8) )

#define polling_done() ({ \
			do { } while (args->retval & 0x80000000);	\
    })

static int secall(volatile struct args *argsx,unsigned int func,unsigned state){
	argsx->func = (func);
	argsx->state = (state);
	argsx->retval = 0x80000000;
	send_secall(1);
	while (argsx->retval & 0x80000000);
	return argsx->retval;
}

#define PHY2VIRT(x) (((unsigned int)x & (~0xe0000000)) | 0x80000000)
#define VIRT2PHY(x) ((unsigned int)x & (~0xe0000000))

#define get_secall_off(x) ((unsigned int)x - TCSM_BANK(0))
#endif /* _SECALL_H_ */
