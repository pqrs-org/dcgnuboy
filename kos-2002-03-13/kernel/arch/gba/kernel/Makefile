# KallistiOS ##version##
#
# arch/gba/kernel/Makefile
# (c)2000 Dan Potter
#   
# Makefile,v 1.1.1.1 2001/09/26 07:05:11 bardtx Exp

# Generic kernel abstraction layer: this provides a set of routines
# that the portable part of the kernel expects to find on every
# target processor. Other routines may be present as well, but
# that minimum set must be present.

COPYOBJS = dbgio.o main.o mm.o panic.o
COPYOBJS += syscall.o timer.o
OBJS = $(COPYOBJS) startup.o
SUBDIRS = 

myall: $(OBJS)
	cp $(COPYOBJS) $(KOS_BASE)/kernel/build/

include $(KOS_BASE)/Makefile.prefab

