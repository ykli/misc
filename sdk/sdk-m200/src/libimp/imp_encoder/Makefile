include $(SDK_TOP)/config.mk

CFLAGS += -I./libx264/include
LDFLAG += -static -L./libx264/lib -lx264

OBJS =  encoder.o

imp_encoder.o: $(OBJS)
	@echo "  [LD]	$(subst $(BUILD_DIR)/,,$(shell pwd)/$@)"
	$(LD) $(OBJS) $(LDFLAG) -o $@
