#ifndef _PDMA_H_
#define _PDMA_H_

#define SE_PASS 0
#define SE_FAILURE 1

//register definition 
#define PDMA_BASE 0xB3420000

#define DMCS_OFF 0x1030

#define TCSM_BANK_LEN  0x0800

#define TCSM_BANK0_OFF 0x2000
#define TCSM_BANK1_OFF 0x2800
#define TCSM_BANK2_OFF 0x3000
#define TCSM_BANK3_OFF 0x3800
#define TCSM_BANK4_OFF 0x4000
#define TCSM_BANK5_OFF 0x4800
#define TCSM_BANK6_OFF 0x5000
#define TCSM_BANK7_OFF 0x5800

#define TCSM_SYNC_OFF 0xfc

#define DMCS (PDMA_BASE + DMCS_OFF)

#define TCSM_BANK0 (PDMA_BASE + TCSM_BANK0_OFF)
#define TCSM_BANK1 (PDMA_BASE + TCSM_BANK1_OFF)
#define TCSM_BANK(x) (PDMA_BASE + TCSM_BANK##x##_OFF)


#define TCSM_SYNC (TCSM_BANK0 + TCSM_SYNC_OFF)
#define TCSM_SE_ARG (TCSM_BANK0 + 0x100 + 0x20)

#define TRAP_ENTRY (TCSM_BANK0 + 0x100)
#define TRAP_HANDLER (TCSM_BANK0 + 0x200)
#define reset_mcu() (REG32(DMCS) = 1)
#define boot_up_mcu() (REG32(DMCS) = 0)

#endif
