include $(SDK_TOP)/config.mk

OBJS =  framesource.o

imp_framesource.o: $(OBJS)
	@echo "  [LD]	$(subst $(BUILD_DIR)/,,$(shell pwd)/$@)"
	$(LD) $(LDFLAG) $(OBJS) -o $@
