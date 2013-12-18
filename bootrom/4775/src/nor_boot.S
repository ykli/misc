//----------------------------------------------------------------------
// Boot from NOR flash CS2#
//----------------------------------------------------------------------
	.text
	.globl nor_boot
        .ent   nor_boot
        .type  nor_boot, @function
	.set noreorder
nor_boot:

	la	$29, 0xb0010000		// GPIO registers base address
	la	$30, 0xb3410000		// NEMC registers base address for static memory
	//--------------------------------------------------------------
	// Setup GPIO pins: SD0 ~ SD7, SA0 ~ SA5, CS2#, RD#, WR#, WAIT#	
	// Disable all pull-up resistors.
	//--------------------------------------------------------------
	li	$9, 0x00400000		// CS2#
	sw	$9, 0x44($29)		// PADATS, input -> output high -> function high	
	li	$9, 0x084300ff		// SD0 ~ SD7, RD#, WR#, CS2#, WAIT#
	sw	$9, 0x18($29)		// PAINTC
	sw	$9, 0x28($29)		// PAMASKC
	sw	$9, 0x38($29)		// PAPAT1C
	sw	$9, 0x48($29)		// PAPAT0C
	sw	$9, 0x74($29)		// PAPES as out put, no need pull up or pull down

	li	$9, 0x0000003f		// SA0 ~ SA5
	sw	$9, 0x118($29)		// PBINTC
	sw	$9, 0x128($29)		// PBMASKC
	sw	$9, 0x138($29)		// PBPAT1C
	sw	$9, 0x148($29)		// PBPAT0C
	sw	$9, 0x174($29)		// PBPES as out put, no need pull up or pull down
	
	li	$11, 0x0fff7700		// SMCR2 value for 8-bit flash

	//--------------------------------------------------------------
	// Read first byte from flash to check data bus width, 0x00-8bit,
	// 0x10-16bit, or else jump to usb_boot
	//--------------------------------------------------------------
	la	$12, 0xba000000		// CS2# base address
	lbu	$8, 0x0($12)		// load data byte

	beqz    $8, nor_boot_cs2
	nop
	li	$9, 0x10
	bne	$9, $8, usb_boot
	nop

nor_boot_16bit:
	li	$10, 0x0000ffff		// SD0 ~ SD15 for 16-bit flash

	sw	$10, 0x18($29)		// PAINTC
	sw	$10, 0x28($29)		// PAMASKC
	sw	$10, 0x38($29)		// PAPAT1C
	sw	$10, 0x48($29)		// PAPAT0C
	sw	$10, 0x74($29)		// PAPES as out put, no need pull up or pull down

	
	li	$11, 0x0fff7740		// SMCR2 value for 16-bit flash

nor_boot_cs2:
	sw	$11, 0x18($30)		// SMCR2
	addiu	$12, $12, 4		// CS2# + 4
	jr	$12			// jump to external memory
	nop

	.set reorder
