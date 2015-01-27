include $(SDK_TOP)/config.mk

OBJS =  base.o

sysutils_base.o: $(OBJS)
	@echo "  [LD]	$(subst $(BUILD_DIR)/,,$(shell pwd)/$@)"
	$(LD) $(LDFLAG) $(OBJS) -o $@
