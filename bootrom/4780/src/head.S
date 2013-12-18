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
        .extern create_watchlist    // For code fixed
        .extern watch_list          // For code fixed

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
	addu	$9, $8, 32768

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
	// setup stack, jump to C code
	//----------------------------------------------------
#if 1
	la	$29, TCSM_STACK_TOP	// sp
	la      $2,  create_watchlist
	jalr	$31, $2   // scan the watch point and create watch list table
	nop
	la      $3, watch_list
	//lw      $2, 0($3)		// ???
	lw      $4, 0($3)
	ori	$4, $4, 4              // For I watch (1 << 2)
	mtc0	$4, $18, 0             // set watch lo
	lui	$4, 0x4000
	mtc0	$4, $19, 0             // set watch hi
#endif
	//----------------------------------------------------
        // jump to main
	//----------------------------------------------------
	la	$29, TCSM_STACK_TOP	// sp
	j	main
	nop

	.set reorder
	.end _start

#define k0 $26
#define k1 $27
#define EPC $14

        .section .reptext,"ax",@progbits
        .extern except_reg_stack 
	.globl _watch_except
        .ent   _watch_except
        .type  _watch_except, @function
	.set noreorder
_watch_except:
// save the scratch register
        la    k1, except_reg_stack
	sw    $2,   8(k1)
	sw    $3,  12(k1)
	sw    $4,  16(k1)
	sw    $5,  20(k1)
	sw    $18, 24(k1)
	sw    $31, 28(k1)
// find current watch point from watch list table
	la    k0, watch_list
// determine whether a branch delay 
	mfc0  $4, $13, 0
	mfc0  $3, EPC, 0
	bgez  $4, 8f
	nop
	addi  $3, $3, 4 
8:
	lw    $2, 0(k0)
	beqz  $2, 9f       // list end
	nop
	bne   $2, $3, 8b
	addiu  k0, k0, 8
// call fix code
	lw    $3, -4(k0)
	jalr  $31, $3
	move  $2, $0        // $2, return val, is next watch pc

	bnez  $2, 9f       // list end
	nop
	lw    $2, 0(k0)
9:
// set the next watch point
	ori   $2, $2, 4        // I watch (1 << 2)
	mtc0  $2, $18, 0       // set watch lo

// Restore the scratch register
	lw    $2,   8(k1)
	lw    $3,  12(k1)
	lw    $4,  16(k1)
	lw    $5,  20(k1)
	lw    $18, 24(k1)
	lw    $31, 28(k1)
	eret
	nop
	nop
	.set reorder
	.end _watch_except


