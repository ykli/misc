include $(SDK_TOP)/config.mk

OBJS =  emu-framesource.o emu-encoder.o emu-fakedev.o

emulator.o: $(OBJS)
	@echo "  [LD]	$(subst $(BUILD_DIR)/,,$(shell pwd)/$@)"
	$(LD) $(OBJS) $(LDFLAG) -o $@
