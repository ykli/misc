/*
 * head.S
 *
 * Entry point of the firmware.
 * The firmware code are executed in the ICache.
 *
 * Copyright (C) 2006 Ingenic Semiconductor Inc.
 *
 */
#include "irom.h"

        .section .start_text,"ax",@progbits

	.extern main

        .extern _fbss_start
        .extern _fbss_end

        .extern tcsm_copy_data
        .extern tcsm_data_len

	.globl _start
	.set noreorder
        .ent  _start
        .type  _start, @function
_start:
        /* flush BTB */
	mfc0	$2, $16, 7
	nop
	ori	$2, $2, 2    /* bit 1: write 1 to BTB invalid */
	mtc0	$2, $16, 7
	nop

	/* open the bridge between cpu and busses */
	la	$8, 0xb0000024
	lw	$9, 0($8)
	ori	$9, $9, 0x2
	sw	$9, 0($8)
        /* init d-cache */
/*	.set	mips32  */
	mtc0    $0, $28
	mtc0    $0, $29

	li	$8, 0x80000000
	addu	$9, $8, 16384

0:
	cache   0x9, 0($8)
	bne     $8, $9, 0b
	addiu   $8, $8, 32

/*	.set	mips32*/


	mfc0	$2, $12, 0
	nop
	and	$2, $2, 0xffffffe0
	mtc0 	$2, $12, 0
	nop

	/* Init BSS data */
	la 	$2, _fbss_start
	la 	$3, _fbss_end
	beq	$2, $3, 2f
	nop
1:
	sw	$0, 0($2)
	addiu	$2, $2, 4
	bne	$2, $3, 1b
	nop
2:
	/* Copy data to  tcsm */
	la 	$2, tcsm_copy_data
	la 	$3, tcsm_data_len
	lw 	$3, 0($3)
	addu	$3, $2, $3
	la	$4, TCSM_DATA_START
3:
	lw	$5, 0($2)
	addiu	$2, $2, 4
	sw	$5, 0($4)
	addiu	$4, $4, 4
	bne	$2, $3, 3b
	nop

	//----------------------------------------------------
        // jump to main
	//----------------------------------------------------
	la	$29, TCSM_STACK_TOP	// sp
	j	main
	nop

	.set reorder
	.end _start

	.section .reptext,"ax",@progbits
	.globl _debug_except
        .ent   _debug_except
        .type  _debug_except, @function
	.set noreorder	
_debug_except:
	//Jump to  tcsm  the last 32 bytes 
	mtc0 $8, $31, 0
	la   $8, 0xf4003fe0
	lw   $8, 0($8)
	jr   $8
	nop
	
	.set reorder
	.end _debug_except
