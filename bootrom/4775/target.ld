OUTPUT_ARCH(mips)
ENTRY(_start)
MEMORY
{
	rom0	: ORIGIN = 0xbfc00000 , LENGTH = 0x480
	repair	: ORIGIN = 0xbfc00480 , LENGTH = 0x20 
	rom1	: ORIGIN = 0xbfc004a0 , LENGTH = 0x3b60
	tcsm	: ORIGIN = 0xf4000000 , LENGTH = 0x800
}

SECTIONS
{
	. = ALIGN(4);
	.startext : { *(.start_text*) } > rom0
	. = ALIGN(4);
	.tcsm_copy : { *(.tcsm_copy*) } > rom0

	.reptext : { *(.reptext*) } > repair
	.text : { *(.text*) } > rom1
	. = ALIGN(4);
	.rodata : { *(.rodata*) } > rom1

        .tcsm_ram :
        {
	  . = ALIGN(4);
	  *(.sdata*) 
	  . = ALIGN(4);
	  *(.data*) *(.scommon*) *(.reginfo*) 
        } > tcsm

       .bss : 
       {
          . = ALIGN(4);
          _fbss_start = .;
	  _gp = ABSOLUTE(.); /* Base of small data */
	  *(.got*)
	  *(.sbss*)
	  *(.bss*)
          . = ALIGN (4);
          _fbss_end = .;
       } >tcsm
}

