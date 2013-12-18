OUTPUT_ARCH(mips)
ENTRY(_start)
MEMORY
{
	rom0	: ORIGIN = 0xbfc00000 , LENGTH = 0x380
	repair	: ORIGIN = 0xbfc00380 , LENGTH = 0x100
	debug	: ORIGIN = 0xbfc00480 , LENGTH = 0x20
	rom1	: ORIGIN = 0xbfc004a0 , LENGTH = 0x6b60
}

SECTIONS
{
	. = ALIGN(4);
	.startext : { *(.start_text*) } > rom0

	.reptext : { *(.reptext*) } > repair
	.dbgtest : { *(.dbgtest*) } > debug
	.text : { *(.text*) } > rom1
	. = ALIGN(4);
	.rodata : { *(.rodata*) } > rom1

    _data_start = .;
	_data_end = . + SIZEOF(.data);

    .data 0x80000000: AT(ADDR(.rodata) + SIZEOF(.rodata))
    {
	  *(.sdata*)
	  *(.data*)
	  *(.scommon*)
	  *(.reginfo*)
    }

    _fbss_start = .;
    .bss 0x80000000 + SIZEOF(.data) : AT(ADDR(.rodata) +SIZEOF(.rodata)+ SIZEOF(.data))
    {
	  *(.sbss*)
	  *(.bss*)
    }
    _fbss_end = .;
}

