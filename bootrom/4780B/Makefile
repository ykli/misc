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
CFLAGS	:= -Os -mips32 -fno-pic -fno-schedule-insns -fno-schedule-insns2 -mno-abicalls -I./include -Wall
LDFLAGS	:= -nostdlib -EL -T target.ld
endif

#CFLAGS += -DSERIAL_DEBUG

OBJS	= ./src/head.o \
		  ./src/common.o \
		  ./src/irom.o \
		  ./src/sd_boot.o \
		  ./src/nand_boot_nfi.o \
		  ./src/spi_boot.o \
		  ./src/sec_boot.o \
		  ./src/nor_boot.o \
		  ./src/usb_boot.o

all: irom.bin

irom.bin: irom.elf
	
	$(CROSS)readelf -S $<
	
	$(CROSS)objdump -D $< > irom.dump
	$(CROSS)objdump -h $< > irom.map
	$(CROSS)nm -n $< > System.map
	$(CROSS)strip -s -g --strip-unneeded -N -X irom.elf
	$(CROSS)objcopy -O binary irom.elf irom.bin
	
irom.elf: $(OBJS)
	$(CROSS)ld $(LDFLAGS) $(OBJS) -o $@

.c.o:
	#$(CROSS)gcc -v -save-temps $(CFLAGS) -c -g $< -o $@
	$(CROSS)gcc $(CFLAGS) -c $< -o $@
.S.o:
	$(CROSS)gcc $(CFLAGS) -D__ASM__ -c $< -o $@
clean:
	rm -fr *.elf *.bin *.dump *.map *.i *.s ./src/*.o
