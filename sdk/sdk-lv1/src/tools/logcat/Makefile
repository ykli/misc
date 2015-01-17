OBJS = logcat.o
INCLUDES = -I../../lib/imp_log -I./
LIBS = -L../../../out/lib -limp

%.o:%.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -DHAVE_IOCTL -DHAVE_SYS_UIO_H $< -o $@

%.o:%.cpp
	$(CPLUSPLUS) -c $(CFLAGS) $(INCLUDES) -DHAVE_IOCTL -DHAVE_SYS_UIO_H $< -o $@

logcat:$(OBJS)
	$(CPLUSPLUS) $(CFLAGS) $(LDFLAG) $< ../../lib/imp_log/imp_log.o -o $@

.PHONY:clean distclean
clean:
	@rm -f *.o
distclean:clean
	@rm -f logcat
