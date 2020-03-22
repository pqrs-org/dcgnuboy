#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>

typedef struct
{
  unsigned char id[3]; // 'NES'
  unsigned char ctrl_z; // control-z
  unsigned char dummy;
  unsigned char num_8k_vrom_banks;
  unsigned char flags_1;
  unsigned char flags_2;
  unsigned char reserved[8];
  unsigned int num_16k_rom_banks;
} NES_header;

enum {
  MASK_VERTICAL_MIRRORING = 0x01,
  MASK_HAS_SAVE_RAM       = 0x02,
  MASK_HAS_TRAINER        = 0x04,
  MASK_4SCREEN_MIRRORING  = 0x08
};


int 
has_trainer(NES_header *nh)
{
  return nh->flags_1 & MASK_HAS_TRAINER;
}


int
calc_crc32 (const char *filename)
{
  FILE* fp;
  unsigned char *ROM_banks;
  int image_type;
  NES_header header;
  int nread;
  int crc32;
  
  fp         = NULL;
  ROM_banks  = NULL;
  image_type = 0;
  
  fp = fopen(filename, "rb");
  if(fp == NULL)
    goto error;
  
  if(fread(&header, 1, 16, fp) != 16)
    goto error;
  
  
  // patch for 260-in-1(#235)
  header.num_16k_rom_banks = (!header.dummy) ? 256 : header.dummy;
  
  if((!memcmp(header.id, "NES", 3) && (header.ctrl_z == 0x1A)) ||
     (!memcmp(header.id, "NEZ", 3) && (header.ctrl_z == 0x1A)))
  {
    // allocate memory
    ROM_banks = malloc(header.num_16k_rom_banks * (16 * 1024));
    if (!ROM_banks)
      goto error;
    
    // load trainer if present
    if(has_trainer(&header))
      fseek(fp, 512, SEEK_CUR);
    
    nread = header.num_16k_rom_banks * (16 * 1024);
    if(fread(ROM_banks, 1, nread, fp) != nread)
      goto error;
  }

  {
    int i, j;
    unsigned long c, crctable[256];
    
    crc32 = 0;
    for(i = 0; i < 256; i++)
    {
      c = i;
      for (j = 0; j < 8; j++)
      {
        if (c & 1)
          c = (c >> 1) ^ 0xedb88320;
        else
          c >>= 1;
      }
      crctable[i] = c;
    }
    
    for(i = 0; i < header.num_16k_rom_banks; i++)
    {
      c = ~crc32;
      for(j = 0; j < 0x4000; j++)
        c = crctable[(c ^ ROM_banks[i*0x4000+j]) & 0xff] ^ (c >> 8);
      crc32 = ~c;
    }
  }
  
  fclose(fp);
  free(ROM_banks);
  return crc32;
  
error:
  if (fp) fclose(fp);
  if (ROM_banks) free(ROM_banks);
  return 0;
}


int
main(int argc, char **argv)
{
  if (argc != 2)
  {
    fprintf(stderr, "Usage: calc_crc32 filename\n");
    exit(1);
  }
  
  printf("%s: %x\n", basename(argv[1]), calc_crc32(argv[1]));
  return 0;
}
