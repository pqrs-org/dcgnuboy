# KallistiOS ##version##
#
# arch/dreamcast/hardware/pvr/Makefile
# (c)2002 Dan Potter
#   
# Makefile,v 1.4 2002/01/29 07:40:03 bardtx Exp

# Memory management
OBJS := pvr_mem_core.o pvr_mem.o

# Internal functions
OBJS += pvr_buffers.o pvr_irq.o

# Init / Shutdown / Globals / Misc
OBJS += pvr_init_shutdown.o pvr_globals.o pvr_misc.o

# Fog
OBJS += pvr_fog.o

# Palette
OBJS += pvr_palette.o

# Primitives / scene management
OBJS += pvr_prim.o pvr_scene.o

# Texture handling
OBJS += pvr_texture.o

# "ta" module compatibility
OBJS += ta_compat.o

include $(KOS_BASE)/Makefile.prefab


