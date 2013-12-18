/*
 * JZ4785 NEMC register definition.
 *
 * Copyright (C) 2012 Ingenic Semiconductor Co., Ltd.
 */
#ifndef __JZ4785NEMC_H__
#define __JZ4785NEMC_H__

#define NEMC_BASE	0xB3410000

/*************************************************************************
 * NEMC (External Memory Controller for NAND)
 *************************************************************************/
#define NEMC_SMCR1	(NEMC_BASE + 0x14) /* Static Memory Control Register 1 */
#define NEMC_SMCR2	(NEMC_BASE + 0x18)
#define NEMC_SMCR3	(NEMC_BASE + 0x1c)
#define NEMC_SMCR4	(NEMC_BASE + 0x20)
#define NEMC_SMCR5	(NEMC_BASE + 0x24)
#define NEMC_SMCR6	(NEMC_BASE + 0x28)
#define NEMC_SACR1	(NEMC_BASE + 0x34) /* Static Bank Address Configuration Register 1 */
#define NEMC_SACR2	(NEMC_BASE + 0x38)
#define NEMC_SACR3	(NEMC_BASE + 0x3c)
#define NEMC_SACR4	(NEMC_BASE + 0x40)
#define NEMC_SACR5	(NEMC_BASE + 0x44)
#define NEMC_SACR6	(NEMC_BASE + 0x48)
#define NEMC_NFCSR	(NEMC_BASE + 0x50) /* NAND Flash Control/Status Register */
#define NEMC_PNCR 	(NEMC_BASE + 0x100) /* NAND PN Control Register */
#define NEMC_PNDR 	(NEMC_BASE + 0x104) /* NAND PN Data Register */
#define NEMC_BITCNT	(NEMC_BASE + 0x108) /* NAND Bit Counter */

/* NEMC for TOGGLE NAND */
#define NEMC_TGWE	(NEMC_BASE + 0x10C) /* Toggle NAND Data Write Access */
#define NEMC_TGCR1	(NEMC_BASE + 0x110) /* Toggle NAND Control Register 1 */
#define NEMC_TGCR2	(NEMC_BASE + 0x114)
#define NEMC_TGCR3	(NEMC_BASE + 0x118)
#define NEMC_TGCR4	(NEMC_BASE + 0x11C)
#define NEMC_TGCR5	(NEMC_BASE + 0x120)
#define NEMC_TGCR6	(NEMC_BASE + 0x124)
#define NEMC_TGSR	(NEMC_BASE + 0x128) /* Toggle NAND RD# to DQS and DQ delay Register */
#define NEMC_TGFL	(NEMC_BASE + 0x12C) /* Toggle NAND ALE Fall to DQS Rise (bank 1/2/3 TGFL) */
#define NEMC_TGFH	(NEMC_BASE + 0x130) /* Toggle NAND ALE Fall to DQS Rise (bank 4/5/6 TGFH) */
#define NEMC_TGCL	(NEMC_BASE + 0x134) /* Toggle NAND CLE to RD# Low (bank 1/2/3 TGCL) */
#define NEMC_TGCH	(NEMC_BASE + 0x138) /* Toggle NAND CLE to RD# low (bank 4/5/6 TGCH) */
#define NEMC_TGPD	(NEMC_BASE + 0x13C) /* Toggle NAND Data Postamble Hold Time Done */
#define NEMC_TGSL	(NEMC_BASE + 0x140) /* Toggle NAND DQS Setup Time for Data Input Start (bank 1/2/3 TGSL) */
#define NEMC_TGSH	(NEMC_BASE + 0x144) /* Toggle NAND DQS Setup Time for Data Input Start (bank 4/5/6 TGSH) */
#define NEMC_TGRR	(NEMC_BASE + 0x148) /* Toggle NAND Timer for Random Data Input and Register Read Out */
#define NEMC_TGDR	(NEMC_BASE + 0x14C) /* Toggle NAND DQS Delay Control Register */

#define REG_NEMC_SMCR1	REG32(NEMC_SMCR1)
#define REG_NEMC_SMCR2	REG32(NEMC_SMCR2)
#define REG_NEMC_SMCR3	REG32(NEMC_SMCR3)
#define REG_NEMC_SMCR4	REG32(NEMC_SMCR4)
#define REG_NEMC_SMCR5	REG32(NEMC_SMCR5)
#define REG_NEMC_SMCR6	REG32(NEMC_SMCR6)
#define REG_NEMC_SACR1	REG32(NEMC_SACR1)
#define REG_NEMC_SACR2	REG32(NEMC_SACR2)
#define REG_NEMC_SACR3	REG32(NEMC_SACR3)
#define REG_NEMC_SACR4	REG32(NEMC_SACR4)
#define REG_NEMC_SACR5	REG32(NEMC_SACR5)
#define REG_NEMC_SACR6	REG32(NEMC_SACR6)
#define REG_NEMC_NFCSR	REG32(NEMC_NFCSR)
#define REG_NEMC_PNCR	REG32(NEMC_PNCR)
#define REG_NEMC_PNDR	REG32(NEMC_PNDR)
#define REG_NEMC_BITCNT	REG32(NEMC_BITCNT)

#define REG_NEMC_TGWE	REG32(NEMC_TGWE)
#define REG_NEMC_TGCR1	REG32(NEMC_TGCR1)
#define REG_NEMC_TGCR2	REG32(NEMC_TGCR2)
#define REG_NEMC_TGCR3	REG32(NEMC_TGCR3)
#define REG_NEMC_TGCR4	REG32(NEMC_TGCR4)
#define REG_NEMC_TGCR5	REG32(NEMC_TGCR5)
#define REG_NEMC_TGCR6	REG32(NEMC_TGCR6)
#define REG_NEMC_TGSR	REG32(NEMC_TGSR)
#define REG_NEMC_TGFL	REG32(NEMC_TGFL)
#define REG_NEMC_TGFH	REG32(NEMC_TGFH)
#define REG_NEMC_TGCL	REG32(NEMC_TGCL)
#define REG_NEMC_TGCH	REG32(NEMC_TGCH)
#define REG_NEMC_TGPD	REG32(NEMC_TGPD)
#define REG_NEMC_TGSL	REG32(NEMC_TGSL)
#define REG_NEMC_TGSH	REG32(NEMC_TGSH)
#define REG_NEMC_TGRR	REG32(NEMC_TGRR)
#define REG_NEMC_TGDR	REG32(NEMC_TGDR)

/* NAND Flash Control/Status Register */
#define NEMC_NFCSR_DAEC		(1 << 31) /* Toggle NAND Data Access Enabel Clear */
#define NEMC_NFCSR_TNFE(n)	(1 << ((n) + 15)) /* Toggle NAND Flash CSn Enable */
#define NEMC_NFCSR_TNFE6	(1 << 21)
#define NEMC_NFCSR_TNFE5	(1 << 20)
#define NEMC_NFCSR_TNFE4	(1 << 19)
#define NEMC_NFCSR_TNFE3	(1 << 18)
#define NEMC_NFCSR_TNFE2	(1 << 17)
#define NEMC_NFCSR_TNFE1	(1 << 16)
#define NEMC_NFCSR_NFCE(n)	(1 << ((((n) - 1) << 1) + 1)) /* NAND Flash CSn Enable */
#define NEMC_NFCSR_NFE(n)	(1 << (((n) -1) << 1)) /* NAND Flash CSn FCE# Assertion Enable */
#define NEMC_NFCSR_NFCE6	(1 << 11)
#define NEMC_NFCSR_NFE6		(1 << 10)
#define NEMC_NFCSR_NFCE5	(1 << 9)
#define NEMC_NFCSR_NFE5		(1 << 8)
#define NEMC_NFCSR_NFCE4	(1 << 7)
#define NEMC_NFCSR_NFE4		(1 << 6)
#define NEMC_NFCSR_NFCE3	(1 << 5)
#define NEMC_NFCSR_NFE3		(1 << 4)
#define NEMC_NFCSR_NFCE2	(1 << 3)
#define NEMC_NFCSR_NFE2		(1 << 2)
#define NEMC_NFCSR_NFCE1	(1 << 1)
#define NEMC_NFCSR_NFE1		(1 << 0)

/* NAND PN Control Register */
// PN(bit 0):0-disable, 1-enable
// PN(bit 1):0-no reset, 1-reset
// (bit 2):Reserved
// BITCNT(bit 3):0-disable, 1-enable
// BITCNT(bit 4):0-calculate, 1's number, 1-calculate 0's number
// BITCNT(bit 5):0-no reset, 1-reset bitcnt
#define NEMC_PNCR_BITRST	(1 << 5)
#define NEMC_PNCR_BITSEL	(1 << 4)
#define NEMC_PNCR_BITEN		(1 << 3)
#define NEMC_PNCR_PNRST		(1 << 1)
#define NEMC_PNCR_PNEN		(1 << 0)

/* Toggle NAND Data Write Access */
#define NEMC_TGWE_DAE		(1 << 31) /* Toggle NAND Data Access Enabel */
#define NEMC_TGWE_WCD		(1 << 16) /* DQS Setup Time for data input start Done */
#define NEMC_TGWE_SDE(n)	(1 << ((n) - 1))
#define NEMC_TGWE_SDE6		(1 << 5) /* Set DQS output enable bank6 */
#define NEMC_TGWE_SDE5		(1 << 4)
#define NEMC_TGWE_SDE4		(1 << 3)
#define NEMC_TGWE_SDE3		(1 << 2)
#define NEMC_TGWE_SDE2		(1 << 1)
#define NEMC_TGWE_SDE1		(1 << 0)

/* Toggle NAND RD# to DQS and DQ delay Register */
#define NEMC_TGSR_DQSRE6(n)	((n) << 20) /* Toggle NAND Flash RD# to DQS and DQ delay bank6 */
#define NEMC_TGSR_DQSRE5(n)	((n) << 16)
#define	NEMC_TGSR_DQSRE4(n)	((n) << 12)
#define NEMC_TGSR_DQSRE3(n)	((n) << 8)
#define NEMC_TGSR_DQSRE2(n)	((n) << 4)
#define NEMC_TGSR_DQSRE1(n)	((n) << 0)

/* Toggle NAND ALE Fall to DQS Rise (bank 3/2/1 TGFL) */
#define NEMC_TGFL_FDA3(n)	((n) << 16) /* Toggle NAND Flash ALE Fall to DQS Rise Bank3 */
#define NEMC_TGFL_FDA2(n)	((n) << 8)
#define NEMC_TGFL_FDA1(n)	((n) << 0)
/* Toggle NAND ALE Fall to DQS Rise (bank 4/5/6 TGFH) */
#define NEMC_TGFH_FDA6(n)	((n) << 16) /* Toggle NAND Flash First ALE Fall to DQS Rise Bank6 */
#define NEMC_TGFH_FDA5(n)	((n) << 8)
#define NEMC_TGFH_FDA4(n)	((n) << 0)

/* Toggle NAND CLE to RD# Low (bank 1/2/3 TGCL) */
#define NEMC_TGCL_CLR3(n)	((n) << 16) /* Toggle NAND Flash CLE to RE_n Low Bank3 */
#define NEMC_TGCL_CLR2(n)	((n) << 8)
#define NEMC_TGCL_CLR1(n)	((n) << 0)
/* Toggle NAND CLE to RD# low (bank 4/5/6 TGCH) */
#define NEMC_TGCH_CLR6(n)	((n) << 16) /* Toggle NAND Flash CLE to RE_n Low Bank6 */
#define NEMC_TGCH_CLR5(n)	((n) << 8)
#define NEMC_TGCH_CLR4(n)	((n) << 0)

/* Toggle NAND Data Postamble Hold Time Done */
#define NEMC_TGPD_DPHTD(n)	(1 << ((n) - 1))
#define NEMC_TGPD_DPHTD6	(1 << 5) /* Toggle NAND Flash Data Postamble Hold Time Done Bank6 */
#define NEMC_TGPD_DPHTD5	(1 << 4)
#define NEMC_TGPD_DPHTD4	(1 << 3)
#define NEMC_TGPD_DPHTD3	(1 << 2)
#define NEMC_TGPD_DPHTD2	(1 << 1)
#define NEMC_TGPD_DPHTD1	(1 << 0)

/* Toggle NAND DQS Setup Time for Data Input Start (bank 1/2/3 TGSL) */
#define NEMC_TGSL_CQDSS3(n)	((n) << 16) /* DQS Setup Time for data input start (bank3) */
#define NEMC_TGSL_CQDSS2(n)	((n) << 8)
#define NEMC_TGSL_CQDSS1(n)	((n) << 0)
/* Toggle NAND DQS Setup Time for Data Input Start (bank 4/5/6 TGSH) */
#define NEMC_TGSH_CQDSS6(n)	((n) << 16) /* DQS Setup Time for data input start (bank6) */
#define NEMC_TGSH_CQDSS5(n)	((n) << 8)
#define NEMC_TGSH_CQDSS4(n)	((n) << 0)

/* Toggle NAND Timer for Random Data Input and Register Read Out */
#define NEMC_TGRR_TD_MASK	(1 << 16) /* Timer Done */
#define NEMC_TGRR_CWAW_MASK	0xFF /* Command Write Cycle to Address Write Cycle Time */

/* Toggle NAND DQS Delay Control */
#define NEMC_TGDR_ERR_MASK	(1 << 29) /* DQS Delay Detect ERROR */
#define NEMC_TGDR_DONE_MASK	(1 << 28) /* Delay Detect Done */
#define NEMC_TGDR_DET		(1 << 23) /* Start Delay Detecting */
#define NEMC_TGDR_AUTO		(1 << 22) /* Hardware Auto-detect & Set Delay Line */
#define NEMC_TGDR_RDQS_BIT	0 /* Number of Delay Elements Used on the Read DQS Delay-line */
#define NEMC_TGDR_RDQS_MASK	(0x1F << NEMC_TGDR_RDQS_BIT)

#ifndef __MIPS_ASSEMBLER

#define __pn_nemc_enable()           (REG_NEMC_PNCR = NEMC_PNCR_PNRST | NEMC_PNCR_PNEN)
#define __pn_nemc_disable()          (REG_NEMC_PNCR = 0x0)

#ifdef __BOOT_ROM__
#define __nand_enable()		(REG_NEMC_NFCSR = NEMC_NFCSR_NFE1 | NEMC_NFCSR_NFCE1)
#define __nand_disable()	(REG_NEMC_NFCSR = 0)

#define __tnand_dphtd_sync()	while (!(REG_NEMC_TGPD & NEMC_TGPD_DPHTD1))

/* Toggle NAND CE# Control */
#define __tnand_enable() \
do { \
        REG_NEMC_NFCSR = NEMC_NFCSR_TNFE1 | NEMC_NFCSR_NFE1; \
        __tnand_dphtd_sync(); \
        REG_NEMC_NFCSR |= NEMC_NFCSR_NFCE1 | NEMC_NFCSR_DAEC; \
} while (0)

#define __tnand_disable() \
do { \
        REG_NEMC_NFCSR &= ~NEMC_NFCSR_NFCE1; \
        __tnand_dphtd_sync(); \
        REG_NEMC_NFCSR = 0; \
} while (0)

#define __tnand_datard_perform() \
do { \
        REG_NEMC_TGWE |= NEMC_TGWE_DAE; \
        __tnand_dae_sync(); \
} while (0)

#define __tnand_dqsdelay_init(n) \
do { \
        unsigned int tmp; \
        tmp = REG_NEMC_TGDR & (~NEMC_TGDR_RDQS_MASK); \
        REG_NEMC_TGDR = tmp | ((n) & NEMC_TGDR_RDQS_MASK); \
} while (0)

#else

#define __tnand_dphtd_sync(n)	while (!(REG_NEMC_TGPD & NEMC_TGPD_DPHTD(n)))

#endif /* __BOOT_ROM__ */

#define __tnand_dae_sync()	while (!(REG_NEMC_TGWE & NEMC_TGWE_DAE))
#define __tnand_dae_clr()	while (REG_NEMC_TGWE & NEMC_TGWE_DAE)
#define __tnand_wcd_sync()	while (!(REG_NEMC_TGWE & NEMC_TGWE_WCD))
#define __tnand_dr_sync(tout)	while (!(REG_NEMC_TGDR & NEMC_TGDR_DONE) && (tout)--)
#define __tnand_delay_sync()	while (!(REG_NEMC_TGDR & NEMC_TGDR_DONE))

#define __tnand_dqsdelay_done() (REG_NEMC_TGDR & NEMC_TGDR_DONE)

#define __tnand_dqsdelay_probe() \
do { \
	REG_NEMC_TGDR |= NEMC_TGDR_DET | NEMC_TGDR_AUTO; \
	__tnand_delay_sync(); \
} while (0)

#define __tnand_fce_set() \
do { \
        __tnand_dphtd_sync(); \
        REG_NEMC_NFCSR |= NEMC_NFCSR_NFCE1; \
} while (0)

#define __tnand_fce_clear() \
do { \
        REG_NEMC_NFCSR |= NEMC_NFCSR_DAEC; \
        REG_NEMC_NFCSR &= ~NEMC_NFCSR_NFCE1; \
        __tnand_dphtd_sync(); \
} while (0)

#endif /* __MIPS_ASSEMBLER */
#endif /* __JZ4785NEMC_H__ */
