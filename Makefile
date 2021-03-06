STARTUP = $(KOS_BASE)/kernel/arch/dreamcast/kernel/startup.o
NESTERDC_LIBS = -L$(KOS_BASE)/lib -lkallisti -lz
NESTERDC_LIBS += -lsupc++ -lm -lc -lgcc

all: 
	$(KOS_MAKE) -C utils 
	$(KOS_MAKE) -C $(KOS_BASE)
	$(KOS_MAKE) -C $(NESTER_BASE) -f Makefile.dreamcast 
	$(KOS_CCPLUS) $(KOS_CPPFLAGS) $(KOS_LDFLAGS) -o obj/nester.elf $(STARTUP) $(NESTER_OBJ)/*.o $(NESTERDC_LIBS) 

clean: 
	-rm -f obj/*
	$(KOS_MAKE) -C utils clean
	$(KOS_MAKE) -C $(KOS_BASE) clean 
	$(KOS_MAKE) -C $(NESTER_BASE) -f Makefile.dreamcast clean 

install:
	tcpcat -h yuri:4711 obj/nester.elf

devel:
	$(KOS_MAKE) -C $(NESTER_BASE) -f Makefile.dreamcast clean
	$(KOS_MAKE) all

makeasm: disasm
	$(NDC_BASE)/utils/asmfilter/asmfilter < disasm > asm

disasm: force
	$(KOS_OBJDUMP) -D obj/nester.elf > disasm

1ST_READ.BIN: 
	$(KOS_OBJCOPY) -O binary obj/nester.elf obj/nester.bin
	$(NDC_BASE)/utils/scramble/scramble obj/nester.bin obj/1ST_READ.BIN

force: 

