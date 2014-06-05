LIVE_DIR = ../live

all:	libAACEncoder.a

CC = gcc
CPLUSPLUS = g++
LD = ld -r -Bstatic

INCLUDES = -I .

CFLAGS = $(INCLUDES) -D_LINUX -g -Wall

OBJS = aacquant.o bitstream.o channels.o fft.o filtbank.o fixed.o frame.o huffman.o table.o util.o

libAACEncoder.a: $(OBJS)
	$(LD) -o libAACEncoder.a $(OBJS)

aacquant.c:	fixed.h frame.h aacquant.h coder.h huffman.h util.h
frame.h:	coder.h channels.h aacquant.h fft.h
channels.h:	coder.h
aacquant.h:	coder.h
huffman.h:	bitstream.h coder.h
bitstream.h:	frame.h coder.h channels.h
bitstream.c:	fixed.h coder.h channels.h huffman.h bitstream.h util.h
channels.c:	fixed.h channels.h coder.h util.h
fft.c:		fixed.h fft.h util.h
filtbank.c:	fixed.h coder.h filtbank.h frame.h fft.h util.h
filtbank.h:	frame.h
fixed.c:	fixed.h
frame.c:	fixed.h frame.h coder.h channels.h bitstream.h filtbank.h aacquant.h util.h huffman.h version.h
huffman.h:	bitstream.h coder.h
huffman.c:	fixed.h huffman.h coder.h bitstream.h util.h
table.c:	fixed.h
util.c:		fixed.h util.h

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

.cpp.o:
	$(CPLUSPLUS) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o *~
	rm -f libAACEncoder.a
