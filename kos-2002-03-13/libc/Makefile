# KOS libc ##version##
#
# Makefile
# (c)2002 Dan Potter
#   
# Makefile,v 1.4 2002/03/04 06:22:49 bardtx Exp

# gen
OBJS_GEN := $(patsubst %.c,%.o,$(wildcard gen/*.c))
OBJS     := $(OBJS_GEN)

# locale
OBJS_LOCALE := $(patsubst %.c,%.o,$(wildcard locale/*.c))
OBJS        += $(OBJS_LOCALE)

# net
OBJS_NET := $(patsubst %.c,%.o,$(wildcard net/*.c))
OBJS     += $(OBJS_NET)

# stdio
OBJS_STDIO := $(patsubst %.c,%.o,$(wildcard stdio/*.c))
OBJS       += $(OBJS_STDIO)

# stdlib
OBJS_STDLIB := $(patsubst %.c,%.o,$(wildcard stdlib/*.c))
OBJS        += $(OBJS_STDLIB)

# string
OBJS_STRING := $(patsubst %.c,%.o,$(wildcard string/*.c))
OBJS        += $(OBJS_STRING)

# time
OBJS_TIME := $(patsubst %.c,%.o,$(wildcard time/*.c))
OBJS      += $(OBJS_TIME)

SUBDIRS =

all: $(OBJS)
	-rm -f $(KOS_BASE)/kernel/build/libc/*.o
	-cp $(OBJS) $(KOS_BASE)/kernel/build/libc/

# Grab the shared Makefile pieces
include $(KOS_BASE)/Makefile.prefab


