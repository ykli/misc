#Define compiler
CROSS_COMPILE ?= mips-linux-gnu-

CC = $(CROSS_COMPILE)gcc
CPLUSPLUS = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar cr
STRIP = $(CROSS_COMPILE)strip

SDK_TOP =  $(shell pwd)/..

CFLAGS = $(INCLUDES) -O2 -Wall -march=mips32r2 -muclibc

CUNIT_DIR = ./extlib/out/cunit
NCURSES_DIR = ./extlib/out/ncurses

INCLUDES  = -I${CUNIT_DIR}/include \
	-I$(SDK_TOP)/out/include

LIBS =	-L$(CUNIT_DIR)/lib -lcunit \
	-L$(NCURSES_DIR)/lib -lncurses \
	-L$(SDK_TOP)/out/lib -limp

OBJS =	test_main.o \
	test_log.o test_module.o \
	test_emulator.o \
	test_framesource.o test_encoder.o \
	test_entire.c test_error.c

all:	test-sdk

Libncurses = $(NCURSES_DIR)
Libcunit = $(CUNIT_DIR)

$(Libncurses):
	@mkdir -p $(NCURSES_DIR)
	@cd extlib;ln -s $(SDK_TOP)/../../../opensource/ncurses;cd ncurses;./configure --prefix=$(shell pwd)/$(NCURSES_DIR) --host=mips-linux-gnu --without-ada --without-manpages --without-progs --without-tests CFLAGS=-muclibc LDFLAGS=-muclibc CPPFLAGS=-muclibc CXXFLAGS=-muclibc
	@make -C extlib/ncurses -j
	@make -C extlib/ncurses install

$(Libcunit):
	@mkdir -p $(CUNIT_DIR)
	@cd extlib;ln -s $(SDK_TOP)/../../../opensource/CUnit;cd CUnit;aclocal;autoconf;autoheader;libtoolize;automake --add-missing;automake -Wno-portability;chmod u+x configure;./configure --prefix=$(shell pwd)/$(CUNIT_DIR) --host=mips-linux-gnu --enable-static --disable-shared CFLAGS=-muclibc LDFLAGS=-muclibc CPPFLAGS=-muclibc
	@make -C extlib/CUnit -j
	@make -C extlib/CUnit install

test-sdk: $(Libncurses) $(Libcunit) $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAG) -o $@ $(OBJS) $(LIBS) -lm -ldl -lrt -lpthread

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

.cpp.o:
	$(CPLUSPLUS) -c $(CFLAGS) $< -o $@

clean:
	@rm -rf test-sdk *.xml *.o

distclean: clean
	@rm -rf extlib
