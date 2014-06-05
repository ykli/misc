#CONFIG_SW_X264 = y

LIVE_DIR = ../live555
X264_DIR = ../x264
IVS_DIR = ../../ingenic/ivs

all:	wis-streamer

CC = mips-linux-gnu-gcc
CPLUSPLUS = mips-linux-gnu-g++
STRIP = mips-linux-gnu-strip

INCLUDES = -I . \
	-I$(LIVE_DIR)/BasicUsageEnvironment/include \
	-I$(LIVE_DIR)/UsageEnvironment/include \
	-I$(LIVE_DIR)/groupsock/include \
	-I$(LIVE_DIR)/liveMedia/include \
	-I$(X264_DIR) \
	-I$(IVS_DIR)

LIBS =	-L$(LIVE_DIR)/liveMedia -lliveMedia \
	-L$(LIVE_DIR)/BasicUsageEnvironment -lBasicUsageEnvironment \
	-L$(LIVE_DIR)/UsageEnvironment -lUsageEnvironment \
	-L$(LIVE_DIR)/groupsock -lgroupsock \
	-L$(X264_DIR) -lx264 \
	-L$(IVS_DIR) -livs

ifeq ($(CONFIG_SW_X264), y)
FFMPEG_DIR = ../ffmpeg
INCLUDES += -I$(FFMPEG_DIR)
LIBS +=	-L$(FFMPEG_DIR)/libavcodec -lavcodec \
	-L$(FFMPEG_DIR)/libswscale -lswscale \
	-L$(FFMPEG_DIR)/libavutil -lavutil
CFLAGS += -DSW_X264
endif

OBJS = wis-streamer.o Options.o Err.o WISInput.o WISServerMediaSubsession.o \
	UnicastStreaming.o \
	WISH264VideoServerMediaSubsession.o H264VideoEncoder.o FormatConvertor.o IVSFramedFilter.o

CFLAGS += $(INCLUDES) -D_LINUX -O2 -Wall -march=mips32r2 -Wa,-mmxu

wis-streamer: $(OBJS)
	$(CPLUSPLUS) $(CFLAGS) $(LDFLAG) -o $@ $(OBJS) $(LIBS) -lm -lpthread -ldl -lrt
	$(STRIP) $@

AMREncoder/libAMREncoder.a:
	cd AMREncoder; $(MAKE)

AACEncoder/libAACEncoder.a:
	cd AACEncoder; $(MAKE)

wis-streamer.cpp:				Options.hh Err.hh UnicastStreaming.hh \
						MulticastStreaming.hh DarwinStreaming.hh
Options.hh:					MediaFormat.hh
UnicastStreaming.hh:			WISInput.hh
MulticastStreaming.hh:			WISInput.hh
DarwinStreaming.hh:			WISInput.hh

Options.cpp:				Options.hh TV.hh Err.hh
TV.cpp:					TV.hh Err.hh
Err.cpp:				Err.hh

WISInput.cpp:				WISInput.hh Options.hh Err.hh

WISServerMediaSubsession.cpp:		WISServerMediaSubsession.hh

UnicastStreaming.cpp:			UnicastStreaming.hh Options.hh \
					WISMPEG2TransportStreamServerMediaSubsession.hh \
					WISJPEGVideoServerMediaSubsession.hh \
					WISMPEG1or2VideoServerMediaSubsession.hh \
					WISMPEG4VideoServerMediaSubsession.hh \
					WISPCMAudioServerMediaSubsession.hh
WISMPEG2TransportStreamServerMediaSubsession.hh:	WISServerMediaSubsession.hh
WISJPEGVideoServerMediaSubsession.hh:	WISServerMediaSubsession.hh
WISServerMediaSubsession.hh:		WISInput.hh
WISMPEG1or2VideoServerMediaSubsession.hh:	WISServerMediaSubsession.hh
WISMPEG4VideoServerMediaSubsession.hh:	WISServerMediaSubsession.hh
WISPCMAudioServerMediaSubsession.hh:	WISServerMediaSubsession.hh MediaFormat.hh

MulticastStreaming.cpp:			MulticastStreaming.hh Options.hh AudioRTPCommon.hh \
					WISJPEGStreamSource.hh \
					MPEG2TransportStreamAccumulator.hh
WISJPEGStreamSource.hh:			WISInput.hh

DarwinStreaming.cpp:			DarwinStreaming.hh Options.hh AudioRTPCommon.hh \
					WISJPEGStreamSource.hh \
					MPEG2TransportStreamAccumulator.hh

AudioRTPCommon.cpp:			AudioRTPCommon.hh Options.hh WISInput.hh \
					MPEGAudioEncoder.hh AMRAudioEncoder.hh \
					AACAudioEncoder.hh

WISJPEGStreamSource.cpp:		WISJPEGStreamSource.hh

WISJPEGVideoServerMediaSubsession.cpp:	WISJPEGVideoServerMediaSubsession.hh WISJPEGStreamSource.hh

WISMPEG1or2VideoServerMediaSubsession.cpp:	WISMPEG1or2VideoServerMediaSubsession.hh

WISMPEG4VideoServerMediaSubsession.cpp:	WISMPEG4VideoServerMediaSubsession.hh

WISPCMAudioServerMediaSubsession.cpp:	WISPCMAudioServerMediaSubsession.hh Options.hh AudioRTPCommon.hh

MPEGAudioEncoder.cpp:			MPEGAudioEncoder.hh avcodec.h mpegaudio.h
avcodec.h:				mpegaudiocommon.h
mpegaudiocommon.h:			bswap.h
mpegaudio.c:				avcodec.h mpegaudio.h mpegaudiocommon.h
mpegaudiocommon.c:			avcodec.h

AMRAudioEncoder.cpp:			AMRAudioEncoder.hh AMREncoder/interf_enc.h AMREncoder/interf_rom.h

AACAudioEncoder.cpp:			AACAudioEncoder.hh AACEncoder/faac.h

MPEG2TransportStreamAccumulator.cpp:	MPEG2TransportStreamAccumulator.hh

WISMPEG2TransportStreamServerMediaSubsession.cpp:	WISMPEG2TransportStreamServerMediaSubsession.hh Options.hh MPEGAudioEncoder.hh MPEG2TransportStreamAccumulator.hh

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

.cpp.o:
	$(CPLUSPLUS) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o *~
	rm -f wis-streamer
	cd AMREncoder; $(MAKE) clean
	cd AACEncoder; $(MAKE) clean
