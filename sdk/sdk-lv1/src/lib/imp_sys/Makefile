include $(LIB_TOP)/config.mk

OBJS =  module.o group.o device.o sys_interface.o sys_core.o mem.o

imp_sys.o: $(OBJS)
	@echo "  [LD]	$(subst $(BUILD_DIR)/,,$(shell pwd)/$@)"
	$(LD) $(LDFLAG) $(OBJS) -o $@
