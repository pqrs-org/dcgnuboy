#include <stdio.h>
#include <stdlib.h>

#define POLYNOMIAL 0x04c11db7L

int crc_table[256];

int 
reflect_bit(int ref, char ch)
{
  int value = 0;
  int i;
  
  for(i = 1; i < (ch + 1); i++)
  {
    if(ref & 1)
      value |= 1 << (ch - i);
    ref >>= 1;
  }
  return value;
}

/* generate the table of CRC remainders for all possible bytes */
void
init_crc32table(void)
{
  int i, j;  
  int crc_accum;
  
  for (i = 0; i < 256; i++)
  {
    crc_accum = reflect_bit(i, 8) << 24;
    
    for (j = 0; j < 8; j++)
    { 
      if (crc_accum & 0x80000000L)
        crc_accum = (crc_accum << 1) ^ POLYNOMIAL;
      else
        crc_accum = (crc_accum << 1);
    }
    crc_table[i] = reflect_bit(crc_accum, 32);
  }
  return; 
}

int
calc_crc32(int crc_accum, const char *data_blk_ptr, int data_blk_size)
  /* update the CRC on the data block one byte at a time */
{ 
  int j;
  for (j = 0; j < data_blk_size; j++)
    crc_accum = (crc_accum>>8) ^ crc_table[(crc_accum & 0xFF) ^ *data_blk_ptr++];
  return crc_accum; 
}


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
calc_nes_entire_crc32 (const char *filename)
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
  
  calc_crc32 (0, ROM_banks, nread);
  
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
  
  printf("%s: %x\n", basename(argv[1]), calc_nes_entire_crc32(argv[1]));
  return 0;
}
