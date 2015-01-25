include $(SDK_TOP)/config.mk

OBJS = logcat.o
CFLAGS += -I$(IMP_DIR)/imp_log -I./
CFLAGS +=  -DHAVE_IOCTL -DHAVE_SYS_UIO_H
LIBS = -L$(IMP_DIR) -limp

logcat: $(OBJS)
	$(CPLUSPLUS) $(CFLAGS) $(LDFLAG) $< $(LIBS) -o $@
	@echo "  [GEN]	$(subst $(BUILD_DIR)/,,$(shell pwd)/$@)"

.PHONY:clean distclean

clean:
	@rm -f *.o

distclean:clean
	@rm -f logcat
