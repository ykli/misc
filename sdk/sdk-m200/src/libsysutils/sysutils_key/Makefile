include $(SDK_TOP)/config.mk

OBJS =  key.o

sysutils_key.o: $(OBJS)
	@echo "  [LD]	$(subst $(BUILD_DIR)/,,$(shell pwd)/$@)"
	$(LD) $(LDFLAG) $(OBJS) -o $@
