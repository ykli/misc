include $(SDK_TOP)/config.mk

OBJS =  module.o group.o device.o sys_interface.o sys_core.o vbm.o

imp_system.o: $(OBJS)
	@echo "  [LD]	$(subst $(BUILD_DIR)/,,$(shell pwd)/$@)"
	$(LD) $(OBJS) $(LDFLAG) -o $@
