#CONFIG_SW_X264 = y

all:	testlibstream

CC = mips-linux-gnu-gcc
CPLUSPLUS = mips-linux-gnu-g++
STRIP = mips-linux-gnu-strip

INCLUDES = -I . \
	-I../libstream

LIBS =	-L../libstream -lstream

OBJS = main.o

CFLAGS += $(INCLUDES) -D_LINUX -g -Wall -march=mips32r2 -Wa,-mmxu

testlibstream: $(OBJS)
	$(CPLUSPLUS) $(CFLAGS) $(LDFLAG) -o $@ $(OBJS) $(LIBS) -lm -lpthread -ldl -lrt
	$(STRIP) $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

.cpp.o:
	$(CPLUSPLUS) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o *~
	rm -f testlibstream
