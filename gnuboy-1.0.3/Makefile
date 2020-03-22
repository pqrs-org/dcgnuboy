# Generated automatically from Makefile.in by configure.

prefix = /usr/local
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin

CC = cc
LD = $(CC)
AS = $(CC)
INSTALL = /usr/bin/install -c -o root -g wheel

CFLAGS = -O2 -pipe -march=k6 -DALLOW_UNALIGNED_IO -fstrength-reduce -fthread-jumps  -fcse-follow-jumps -fcse-skip-blocks -frerun-cse-after-loop  -fexpensive-optimizations -fforce-mem -fforce-addr -fomit-frame-pointer
LDFLAGS = $(CFLAGS)  -s
ASFLAGS = $(CFLAGS)

TARGETS =  xgnuboy sgnuboy sdlgnuboy

ASM_OBJS = asm/i386/cpu.o asm/i386/lcd.o asm/i386/refresh.s

SYS_DEFS = -DHAVE_CONFIG_H -DIS_LITTLE_ENDIAN -DUSE_ASM -I./asm/i386 -DIS_FBSD
SYS_OBJS = sys/nix/nix.o $(ASM_OBJS)
SYS_INCS = -I/usr/local/include -I/usr/X11R6/include -I./sys/nix

FB_OBJS =  sys/dummy/nojoy.o sys/oss/oss.o
FB_LIBS = 

SVGA_OBJS = sys/svga/svgalib.o sys/pc/keymap.o sys/dummy/nojoy.o sys/oss/oss.o
SVGA_LIBS = -L/usr/local/lib -lvga

SDL_OBJS = sys/sdl/sdl.o sys/sdl/keymap.o
SDL_LIBS = -L/usr/local/lib -Wl,-rpath,/usr/local/lib -lSDL-1.1 -pthread -lm -L/usr/local/lib -lesd -laudiofile -lm -L/usr/X11R6/lib -lX11 -lXext -lXxf86vm -lXxf86dga -lXv -lXinerama -lvga -laa
SDL_CFLAGS = -I/usr/local/include -I/usr/local/include/SDL11 -D_REENTRANT -D_THREAD_SAFE

X11_OBJS = sys/x11/xlib.o sys/x11/keymap.o sys/dummy/nojoy.o sys/oss/oss.o
X11_LIBS = -L/usr/X11R6/lib -lX11 -lXext

all: $(TARGETS)

include Rules

fbgnuboy: $(OBJS) $(SYS_OBJS) $(FB_OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(SYS_OBJS) $(FB_OBJS) -o $@ $(FB_LIBS)

sgnuboy: $(OBJS) $(SYS_OBJS) $(SVGA_OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(SYS_OBJS) $(SVGA_OBJS) -o $@ $(SVGA_LIBS)

sdlgnuboy: $(OBJS) $(SYS_OBJS) $(SDL_OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(SYS_OBJS) $(SDL_OBJS) -o $@ $(SDL_LIBS)

sys/sdl/sdl.o: sys/sdl/sdl.c
	$(MYCC) $(SDL_CFLAGS) -c $< -o $@

sys/sdl/keymap.o: sys/sdl/keymap.c
	$(MYCC) $(SDL_CFLAGS) -c $< -o $@

xgnuboy: $(OBJS) $(SYS_OBJS) $(X11_OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(SYS_OBJS) $(X11_OBJS) -o $@ $(X11_LIBS)

dcgnuboy: $(OBJS) $(SYS_OBJS) $(DC_OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(SYS_OBJS) $(DC_OBJS) -o $@ $(DC_LIBS)

install: all
	$(INSTALL) -d $(bindir)
	$(INSTALL) -m 755 $(TARGETS) $(bindir)

clean:
	rm -f *gnuboy gmon.out *.o sys/*.o sys/*/*.o asm/*/*.o

distclean: clean
	rm -f config.* sys/nix/config.h Makefile




