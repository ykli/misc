OUTPUT_ARCH(mips)
ENTRY(_start)
MEMORY
{
	rom0	: ORIGIN = 0xbfc00000 , LENGTH = 0x380
	repair	: ORIGIN = 0xbfc00380 , LENGTH = 0xa0
	rom1	: ORIGIN = 0xbfc00420 , LENGTH = 0x6be0
}

SECTIONS
{
	. = ALIGN(4);
	.startext : { *(.start_text*) } > rom0

	.reptext : { *(.reptext*) } > repair
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

