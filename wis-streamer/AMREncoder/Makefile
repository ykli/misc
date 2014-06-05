LIVE_DIR = ../live

all:	libAMREncoder.a

CC = gcc
CPLUSPLUS = g++
LD = ld -r -Bstatic

INCLUDES = -I .

CFLAGS = $(INCLUDES) -D_LINUX -g -Wall

OBJS = fixed.o interf_enc.o sp_enc.o table.o

libAMREncoder.a: $(OBJS)
	$(LD) -o libAMREncoder.a $(OBJS)

interf_enc.c:				interf_enc.h interf_rom.h
interf_enc.h:				sp_enc.h
sp_enc.h:				typedef.h
sp_enc.c:				sp_enc.h rom_enc.h fixed.h
rom_enc.h:				sp_enc.h fixed.h
fixed.c:				fixed.h
table.c:				fixed.h

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

.cpp.o:
	$(CPLUSPLUS) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o *~
	rm -f libAMREncoder.a
