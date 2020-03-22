# KallistiOS ##version##
#
# Root Makefile
# (c)2000-2001 Dan Potter
#   
# Makefile,v 1.4 2002/03/04 06:07:58 bardtx Exp

all: build-utils build-libc build-libm build-kernel build-addons

clean:
	-rm -f lib/*
	$(KOS_MAKE) -C utils clean
	$(KOS_MAKE) -C libc clean
	$(KOS_MAKE) -C libm clean
	$(KOS_MAKE) -C kernel clean
	$(KOS_MAKE) -C addons clean

build-utils:
	$(KOS_MAKE) -C utils

build-libc:
	$(KOS_MAKE) -C libc

build-libm:
	$(KOS_MAKE) -C libm

build-kernel:
	$(KOS_MAKE) -C kernel

build-addons:
	$(KOS_MAKE) -C addons

build-examples:
	$(KOS_MAKE) -C examples


