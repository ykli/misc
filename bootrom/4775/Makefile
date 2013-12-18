#
# Copyright (C) 2006 Ingenic Semiconductor Inc.
#
# Makefile for the JZ4770 internal bootrom code.
#

DEBUG   := 0
CROSS	:= mipsel-linux-

ifeq ($(DEBUG),1)
CFLAGS	:= -O0 -g -mips32 -fno-pic -mno-abicalls -I./include -Wall -DUSE_TCSM_DEBUG
LDFLAGS	:= -nostdlib -EL -T targetdbg.ld
else
CFLAGS	:= -Os -mips32 -fno-pic -fno-schedule-insns -fno-schedule-insns2 -fno-builtin-memset -mno-abicalls -I./include -Wall
LDFLAGS	:= -nostdlib -EL -T target.ld
endif

#CFLAGS += -DSERIAL_DEBUG

OBJS	= ./src/head.o \
		  ./src/common.o \
		  ./src/irom.o \
		  ./src/sd_boot.o \
		  ./src/nand_boot.o \
		  ./src/spi_boot.o \
		  ./src/nor_boot.o \
		  ./src/usb_boot.o \

all: irom.bin

irom.bin: irom.elf
	
	gcc -O2 src/fixelf.c -o fixelf
	$(CROSS)readelf -S $<
	
ifeq ($(DEBUG),0)
	$(CROSS)objcopy --only-section   .tcsm_ram -O binary $< $@.tcsmcopy
# Check the copying data length > 512 ?
	@if [ `wc -c $@.tcsmcopy | awk '{print $$1}'` -gt 512 ]; then echo "********* Data is too large ( > 512) *********"; exit 1; fi
	./fixelf $< tcsm_copy_data 0 $@.tcsmcopy
endif
	$(CROSS)objcopy --remove-section .tcsm_ram -O binary $< $@
	
	$(CROSS)objdump -D $< > irom.dump
	$(CROSS)objdump -h $< > irom.map
	$(CROSS)nm -n $< > System.map
	
irom.elf: $(OBJS)
	$(CROSS)ld $(LDFLAGS) $(OBJS) -o $@

.c.o:
	$(CROSS)gcc $(CFLAGS) -c $< -o $@
#	$(CROSS)gcc -v -save-temps $(CFLAGS) -c $< -o $@
.S.o:
	$(CROSS)gcc $(CFLAGS) -D__ASM__ -c $< -o $@
clean:
	rm -fr *.elf *.bin *.dump *.map *.tcsmcopy fixelf *.i *.s ./src/*.o
