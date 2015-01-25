include $(SDK_TOP)/config.mk

OBJS =  encoder.o

imp_encoder.o: $(OBJS)
	@echo "  [LD]	$(subst $(BUILD_DIR)/,,$(shell pwd)/$@)"
	$(LD) $(LDFLAG) $(OBJS) -o $@
