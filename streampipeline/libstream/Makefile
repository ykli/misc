X264_DIR = ../../../../opensource/x264
FRAMESOURCE_DIR = ../../framesource/libframesource
IVS_DIR = ../../ivs
OPENCV_DIR = ../../../../opensource/opencv/out
IPC_DIR = ../../ipc

CC = mips-linux-gnu-gcc
CPLUSPLUS = mips-linux-gnu-g++
LD = mips-linux-gnu-ld
AR = mips-linux-gnu-ar cr

all: libstream.a

INCLUDES = -I$(X264_DIR) \
	-I$(FRAMESOURCE_DIR) \
	-I$(IVS_DIR) \
	-I$(OPENCV_DIR)/include \
	-I$(IPC_DIR)

LIBS =	-L$(X264_DIR) -lx264 \
	-L$(FRAMESOURCE_DIR) -lframesource \
	-L$(IPC_DIR) -lipc

CFLAGS = $(INCLUDES) -g -Wall -march=mips32r2 -Wa,-mmxu

OBJS =  common.o StreamSource.o StreamList.o WorkThread.o HandlerList.o Handler.o FrameSource.o StreamSink.o \
	Filter.o WriteResult.o V4L2Input.o FormatConvertorFilter.o H264Encoder.o T05MultiInput.o \
	SampleIVSFilter.o LineDetectFilter.o ShadeDetectFilter.o FaceDetectFilter.o SnapShotFilter.o

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

%.o:%.cpp
	$(CPLUSPLUS) -c $(CFLAGS) $< -o $@

libstream.a: $(OBJS)
	$(AR) $@ $(OBJS)

.PHONY: clean distclean
clean:
	rm -f *.o *~ *.a
