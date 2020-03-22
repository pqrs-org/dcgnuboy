#include "gnuboy.h"

#include "defs.h"
#include "regs.h"
#include "mem.h"
#include "hw.h"
#include "rtc.h"
#include "rc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <time.h>
#include "unzip.h"

#include "sys/dc/dc_utils.h"
#include "sys/dc/dc_vmu.h"

static const int mbc_table[256] =
{
	0, 1, 1, 1, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 3,
	3, 3, 3, 3, 0, 0, 0, 0, 0, 5, 5, 5, MBC_RUMBLE, MBC_RUMBLE, MBC_RUMBLE, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, MBC_HUC3, MBC_HUC1
};

static const int rtc_table[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0
};

static const int batt_table[256] =
{
	0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0,
	1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0,
	0
};

static const int romsize_table[256] =
{
	2, 4, 8, 16, 32, 64, 128, 256, 512,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 128, 128, 128
	/* 0, 0, 72, 80, 96  -- actual values but bad to use these! */
};

static const int ramsize_table[256] =
{
	1, 1, 1, 4, 16,
	4 /* FIXME - what value should this be?! */
};


static uLong rom_crc32 = 0;
static char sramfile[256];
static char dc_vmu_savefile_ss[256];
static char dc_vmu_desc_long[256];
static char dc_vmu_desc_long_ss[256];

static int nobatt;
static int forcedmg, gbamode;

static int memfill = -1, memrand = -1;


static void initmem(void *mem, int size)
{
	char *p = mem;
	if (memrand >= 0)
	{
		srand(memrand ? memrand : time(0));
		while(size--) *(p++) = rand();
	}
	else if (memfill >= 0)
		memset(p, memfill, size);
	else
	{
		memset(p, 0, size);
	}
}

static byte *loadfile(FILE *f, int *len)
{
	int size;
	byte *buf;
	
	fseek (f, 0, SEEK_END);
	size = ftell (f);
	fseek (f, 0, SEEK_SET);
	
	buf = malloc (size);
	fread (buf, size, 1, f);
	
	*len = size;
	return buf;
}

#if 1
static byte *inf_buf;
static int inf_pos, inf_len;

static void inflate_callback(byte b)
{
	if (inf_pos >= inf_len)
	{
		inf_len += 512;
		inf_buf = realloc(inf_buf, inf_len);
#if 0
		if (!inf_buf) die("out of memory inflating file @ %d bytes\n", inf_pos);
#endif
	}
	inf_buf[inf_pos++] = b;
}

static byte *decompress(byte *data, int *len)
{
	unsigned long pos = 0;
	if (data[0] != 0x1f || data[1] != 0x8b)
		return data;
	inf_buf = 0;
	inf_pos = inf_len = 0;
	if (unzip(data, &pos, inflate_callback) < 0)
		return data;
	*len = inf_pos;
	return inf_buf;
}
#endif


int rom_load(const char *romfile)
{
	FILE *f;
	byte c, *data, *header;
	int len = 0, rlen;
	
	f = fopen(romfile, "rb");
        if (!f) return 0;
	
	header = data = loadfile(f, &len);
#if 1
	header = data = decompress(data, &len);
#endif
        
        rom_crc32 = crc32 (0, Z_NULL, 0);
        rom_crc32 = crc32 (rom_crc32, data, len);
        
	memcpy(rom.name, header+0x0134, 16);
	if (rom.name[14] & 0x80) rom.name[14] = 0;
	if (rom.name[15] & 0x80) rom.name[15] = 0;
	rom.name[16] = 0;
	
	c = header[0x0147];
	mbc.type = mbc_table[c];
	mbc.batt = (batt_table[c] && !nobatt);
	rtc.batt = rtc_table[c];
	mbc.romsize = romsize_table[header[0x0148]];
	mbc.ramsize = ramsize_table[header[0x0149]];
        
#if 0
	if (!mbc.romsize) die("unknown ROM size %02X\n", header[0x0148]);
	if (!mbc.ramsize) die("unknown SRAM size %02X\n", header[0x0149]);
#endif
	
	c = header[0x0143];
	hw.cgb = ((c == 0x80) || (c == 0xc0)) && !forcedmg;
	hw.gba = (hw.cgb && gbamode);
        
	rlen = 16384 * mbc.romsize;
	rom.bank = realloc(data, rlen); /* data & header will be broken */
	data = header = NULL;
	if (rlen > len) memset (rom.bank + len, 0xff, rlen - len);
	
	ram.sbank = malloc(8192 * mbc.ramsize);
	
	initmem(ram.sbank, 8192 * mbc.ramsize);
	initmem(ram.ibank, 4096 * 8);
	
	mbc.rombank = 1;
	mbc.rambank = 0;

	return 1;
}

extern uint8 dc_mvmu;

int sram_load()
{
	if (!dc_mvmu) return 0;
	
	if (!mbc.batt) return 0;
	
	/* Consider sram loaded at this point, even if file doesn't exist */
	ram.loaded = 1;
	
        char comp_buf[512 * 200]; /* entire VMU block size */
        uLongf comp_len;
        uLongf sram_len;
        int code;
        
        dc_print ("Loading SRAM...");
        
        comp_len = sizeof(comp_buf);
        if (ndc_vmu_load(comp_buf, &comp_len, dc_mvmu, sramfile) < 0) 
        {
          dc_print ("No savefile");
          return 0;
        }
        
        sram_len = 8192 * mbc.ramsize;
        code = uncompress (ram.sbank, &sram_len, comp_buf, comp_len);
        if (code != Z_OK)
        {
          dc_print ("uncompress failed");
          return 0;
        }
	
	return 1;
}


int sram_save()
{
	if (!dc_mvmu) return 0;
	
	/* If we crash before we ever loaded sram, DO NOT SAVE! */
	if (!mbc.batt || !ram.loaded || !mbc.ramsize)
		return 0;
	
        char comp_buf[512 * 200]; /* entire VMU blocks */
        uLongf comp_len;
        uLongf sram_len;
        int code;
        
        dc_print ("Saving SRAM...");
        
        comp_len = sizeof(comp_buf);
        sram_len = 8192 * mbc.ramsize;
        
        code = compress (comp_buf, &comp_len, ram.sbank, sram_len);
        if (code != Z_OK)
        {
          dc_put_error ("compress failed");
	  return 0;
        }

        if (ndc_vmu_save(comp_buf, comp_len, dc_mvmu, 
                         sramfile, "gnuboy/DC SRAM", 
                         dc_vmu_desc_long, NULL) < 0)
        {
          dc_put_error ("save failed");
	  return 0;
        }
        
	return 1;
}


int
statesave_to_vmu ()
{
	FILE *fp = NULL;
	char buf[max_statefile_size];
	uLongf buf_len;
	char comp_buf[512 * 200]; /* extire VMU blocks */
	uLongf comp_len;
	int code;
	
	if (!dc_mvmu) goto error;
	
	fp = fopen("/md/quick", "r");
	if (!fp)
		goto error;
	
	savestate (fp);
	
	fseek(fp, 0, SEEK_END);
	buf_len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (fread(buf, buf_len, 1, fp) != 1)
		goto error;
	fclose(fp);
	fp = NULL;
	
	comp_len = sizeof(comp_buf);
	code = compress (comp_buf, &comp_len, buf, buf_len);
	if (code != Z_OK)
		goto error;
	
	if (ndc_vmu_save(comp_buf, comp_len, dc_mvmu, 
			 dc_vmu_savefile_ss, "gnuboy/DC snap", 
			 dc_vmu_desc_long_ss, NULL) < 0)
		goto error;
	
	return 1;
error:
	if (fp) fclose(fp);
	return 0;
}


int
stateload_from_vmu ()
{
	FILE *fp = NULL;
	char buf[max_statefile_size];
	uLongf buf_len;
	char comp_buf[512 * 200]; /* entire VMU blocks */
	uLongf comp_len;
	int code;
	
	if (!dc_mvmu) goto error;
	
	if (ndc_vmu_load(comp_buf, &comp_len, dc_mvmu, dc_vmu_savefile_ss) < 0)
		goto error;
	
	buf_len = sizeof(buf);
	code = uncompress (buf, &buf_len, comp_buf, comp_len);
	if (code != Z_OK)
		goto error;
	
	fp = fopen ("/md/quick", "w");
	if (!fp)
		goto error;
	if (fwrite(buf, buf_len, 1, fp) != 1)
		goto error;
	
	fseek (fp, 0, SEEK_SET);
	fclose (fp);
	
	fp = fopen ("/md/quick", "r");
	if (!fp)
		goto error;
	loadstate (fp);
	
	fclose (fp);
	return 1;
	
error:
	if (fp) fclose(fp);
	return 0;
}


void rtc_save()
{
#if 0
	FILE *f;
	if (!rtc.batt) return;
	if (!(f = fopen(rtcfile, "wb"))) return;
	rtc_save_internal(f);
	fclose(f);
#endif
}

void rtc_load()
{
#if 0
	FILE *f;
	if (!rtc.batt) return;
	if (!(f = fopen(rtcfile, "r"))) return;
	rtc_load_internal(f);
	fclose(f);
#endif
}


void loader_unload()
{
	sram_save();
	if (rom.bank) free(rom.bank);
	if (ram.sbank) free(ram.sbank);
	rom.bank = NULL;
	ram.sbank = NULL;
	mbc.type = mbc.romsize = mbc.ramsize = mbc.batt = 0;
}

static const char *base(const char *s)
{
	const char *p;
	
	p = strrchr(s, '/');
	if (p) return p+1;
	return s;
}

static void cleanup()
{
	sram_save();
	rtc_save();
	/* IDEA - if error, write emergency savestate..? */
}

int loader_init(const char *romfile)
{
	char name[256], *p;
        
	if (!rom_load(romfile))
		return 0;
        
	vid_settitle(rom.name);
	
	sprintf (name, "%s", base(romfile));
	p = strchr(name, '.');
	if (p) *p = '\0';
        
        sprintf (sramfile, "GB_%lx", rom_crc32);
	sprintf (dc_vmu_savefile_ss, "GB_S%lx", rom_crc32);
        sprintf (dc_vmu_desc_long, "gnuboy/DC %s", name);
	sprintf (dc_vmu_desc_long_ss, "gnuboy/DC snap %s", name);
	
	sram_load();
        
        return 1;
}

rcvar_t loader_exports[] =
{
	RCV_BOOL("nobatt", &nobatt),
	RCV_BOOL("forcedmg", &forcedmg),
	RCV_BOOL("gbamode", &gbamode),
	RCV_INT("memfill", &memfill),
	RCV_INT("memrand", &memrand),
	RCV_END
};

