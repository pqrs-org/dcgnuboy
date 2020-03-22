# KallistiOS ##version##
#
# libmp3/Makefile
# (c)2001 Dan Potter
#   
# Makefile,v 1.1 2002/02/10 06:40:06 bardtx Exp

OBJS = snd_sfxmgr.o snd_stream.o snd_stream_drv.o

# Only compile this if we have an ARM compiler handy
ifdef DC_ARM7CC
	SUBDIRS = arm
endif

include $(KOS_BASE)/Makefile.prefab

snd_stream_drv.o: arm/stream.drv
	$(KOS_BASE)/utils/bin2o/bin2o $< snd_stream_drv snd_stream_drv.o

