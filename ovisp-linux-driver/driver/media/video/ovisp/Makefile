# Makefile for OVISP video driver

ifeq ($(CONFIG_VIDEO_OVISP_STUB),y)
obj-$(CONFIG_VIDEO_OVISP) += ovisp-videostub.o ovisp-videobuf.o
else
obj-$(CONFIG_VIDEO_OVISP) += ovisp-video.o ovisp-videobuf.o ovisp-isp.o ovisp-csi.o isp-i2c.o isp-ctrl.o ovisp-debugtool.o
endif
