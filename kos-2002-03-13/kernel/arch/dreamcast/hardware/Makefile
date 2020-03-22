# KallistiOS ##version##
#
# arch/dreamcast/hardware/Makefile
# (c)2000-2001 Dan Potter
#
# Makefile,v 1.9 2002/02/22 07:33:23 bardtx Exp

# Init wrapper
OBJS = hardware.o

# BIOS services
OBJS += biosfont.o cdrom.o

# Sound
OBJS += spu.o spudma.o

# Bus support
OBJS += asic.o g2bus.o

# Video-related
OBJS += video.o

# CPU-related
OBJS += sq.o

SUBDIRS = pvr network maple

include $(KOS_BASE)/Makefile.prefab



