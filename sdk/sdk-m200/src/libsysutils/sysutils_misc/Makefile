include $(SDK_TOP)/config.mk

OBJS =  key.o led.o lumina.o

sysutils_misc.o: $(OBJS)
	@echo "  [LD]	$(subst $(BUILD_DIR)/,,$(shell pwd)/$@)"
	$(LD) $(OBJS) $(LDFLAG) -o $@
