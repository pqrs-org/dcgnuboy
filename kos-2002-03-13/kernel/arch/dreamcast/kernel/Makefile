# KallistiOS ##version##
#
# arch/dreamcast/kernel/Makefile
# (c)2000 Dan Potter
#   
# Makefile,v 1.7 2002/02/15 20:20:34 bardtx Exp

# Generic kernel abstraction layer: this provides a set of routines
# that the portable part of the kernel expects to find on every
# target processor. Other routines may be present as well, but
# that minimum set must be present.

COPYOBJS = banner.o cache.o dbgio.o entry.o irq.o main.o mm.o panic.o
COPYOBJS += rtc.o ser_console.o syscall.o timer.o initall.o
COPYOBJS += mmu.o itlb.o crtbegin.o crtend.o atexit.o
OBJS = $(COPYOBJS) startup.o
SUBDIRS = 

myall: $(OBJS)
	-cp $(COPYOBJS) $(KOS_BASE)/kernel/build/
	-rm banner.c banner.o

include $(KOS_BASE)/Makefile.prefab

banner.o: banner.c

banner.c: make_banner.sh
	./make_banner.sh


