CC = mips-linux-gnu-gcc
CPLUSPLUS = mips-linux-gnu-g++
LD = mips-linux-gnu-ld
AR = mips-linux-gnu-ar cr

all: libstream.a

INCLUDES = 
LIBS = 
CFLAGS = $(INCLUDES) -g -Wall

OBJS = StreamSource.o StreamList.o WorkThread.o

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

%.o:%.cpp
	$(CPLUSPLUS) -c $(CFLAGS) $< -o $@

libstream.a: $(OBJS)
	$(AR) $@ $(OBJS)

.PHONY: clean distclean
clean:
	-rm $(OBJS)

distclean: clean
	-rm libstream.a
