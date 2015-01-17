include $(LIB_TOP)/config.mk

OBJS =  emu-framesource.o emu-enc.o

emulator.o: $(OBJS)
	@echo "  [LD]	$(subst $(BUILD_DIR)/,,$(shell pwd)/$@)"
	$(LD) $(LDFLAG) $(OBJS) -o $@
