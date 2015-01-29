include $(SDK_TOP)/config.mk

OBJS =  battery.o

sysutils_battery.o: $(OBJS)
	@echo "  [LD]	$(subst $(BUILD_DIR)/,,$(shell pwd)/$@)"
	$(LD) $(OBJS) $(LDFLAG) -o $@
