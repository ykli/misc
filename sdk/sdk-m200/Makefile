LIB_VERSION = 1.0.0

Target = ./doc			\
	./src/libimp		\
	./src/libsysutils	\
	./src/tools		\
	./adapter/adapter01

SDK_TOP = $(shell pwd)
BUILD_DIR := $(SDK_TOP)
IMP_DIR = $(SDK_TOP)/src/libimp
SYSUTILS_DIR = $(SDK_TOP)/src/libsysutils
INSTALL_DIR = $(SDK_TOP)/_install

ifeq ("$(origin V)", "command line")
  BUILD_VERBOSE = $(V)
endif
ifndef BUILD_VERBOSE
  BUILD_VERBOSE = 0
endif

ifeq ($(BUILD_VERBOSE),1)
  Q =
else
  Q = @
  MAKE = make -s
endif

export LIB_VERSION Q MAKE BUILD_DIR SDK_TOP IMP_DIR SYSUTILS_DIR INSTALL_DIR

#all:  modules pdf
all: modules
	@echo \"make install\" should be done before \"make test\"

modules:
	@$(foreach m, $(Target), $(MAKE) -C $(m);)

doc:
	@$(MAKE) -C $@

test:
	@$(MAKE) -C ./test

test-clean:
	@$(MAKE) -C ./test clean

clean:
	@$(foreach m, $(Target), $(MAKE) -C $(m) clean;)
	@$(MAKE) -C ./test clean

install:
	@$(foreach m, $(Target), $(MAKE) -s -C $(m) install;)
	@mkdir -p $(INSTALL_DIR)/include
	@cp -r ./include/* $(INSTALL_DIR)/include

distclean: clean
	@rm $(INSTALL_DIR) -rf
	@$(foreach m, $(Target), $(MAKE) -C $(m) distclean;)
	@$(MAKE) -C ./test distclean

.PHONY: doc test modules
