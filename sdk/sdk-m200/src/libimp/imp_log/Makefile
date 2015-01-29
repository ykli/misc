include $(SDK_TOP)/config.mk

CFLAGS += -DHAVE_PTHREADS -DHAVE_SYS_UIO_H

OBJS =  event_tag_map.o logd_write.o logprint.o implog.o

imp_log.o: $(OBJS)
	@echo "  [LD]	$(subst $(BUILD_DIR)/,,$(shell pwd)/$@)"
	$(LD) $(OBJS) $(LDFLAG) -o $@
