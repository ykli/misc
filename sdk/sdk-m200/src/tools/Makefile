#Define compiler
CROSS_COMPILE ?= mips-linux-gnu-

CC = $(CROSS_COMPILE)gcc
CPLUSPLUS = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar cr
STRIP = $(CROSS_COMPILE)strip

CFLAGS = $(INCLUDES) -O2 -Wall -march=mips32r2

all: TOOLS

export CFLAGS LDFLAG CC CPLUSPLUS LD AR STRIP
export LIB_VERSION Q MAKE BUILD_DIR SDK_TOP IMP_DIR SYSUTILS_DIR INSTALL_DIR

#Tools to be compiled.
TOOLS = logcat

S_ELF = $(foreach m, $(TOOLS), $(m)/$(m))

TOOLS:
	@$(foreach m, $(TOOLS), $(MAKE) -C $(m);)

.PHONY: clean distclean TOOLS
clean:
	@$(foreach m, $(TOOLS), $(MAKE) -C $(m) $@;)

distclean:
	@$(foreach m, $(TOOLS), $(MAKE) -C $(m) $@;)

install:
	@mkdir -p $(INSTALL_DIR)/tools/
	@$(foreach m, $(S_ELF), cp $(m) $(INSTALL_DIR)/tools/;)
