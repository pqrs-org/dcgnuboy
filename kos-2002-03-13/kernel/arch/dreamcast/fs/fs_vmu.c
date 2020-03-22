/* KallistiOS ##version##

   fs_vmu.c
   (C)2000-2002 Jordan DeLong and Dan Potter

*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <arch/types.h>
#include <arch/spinlock.h>
#include <dc/fs_vmu.h>
#include <dc/maple.h>
#include <dc/maple/vmu.h>

CVSID("fs_vmu.c,v 1.8 2002/03/10 07:08:05 bardtx Exp");

/*

This is the vmu filesystem module.  Because there are no directories on vmu's
it's pretty simple, however the filesystem uses a seperate directory for each
of the vmu slots, so if vmufs were mounted on /vmu, /vmu/a1/ is the dir for 
slot 1 on port a, and /vmu/c2 is slot 2 on port c, etc.

At the moment this FS is kind of a hack because of the simplicity (and weirdness)
of the VMU file system. For one, all files must be pretty small, so it loads
and caches the entire file on open. For two, all files are a multiple of 512
bytes in size (no way around this one). On top of it all, files may have an 
obnoxious header and you can't just read and write them with abandon like
a normal file system. We'll have to find ways around this later on, but for
now it gives the file data to you raw.

*/

#define VMU_DIR 	0
#define VMU_FILE	1

/* File handles */
typedef struct vmu_fh_str {
	uint32	strtype;		/* 0==dir, 1==file */
	struct vmu_fh_str *next; 	/* next in the list */

	int	mode;			/* mode the file was opened with */
	char	name[12];		/* name of the file */
	uint32	diridx;			/* index in the vmu directory: -1 means it needs to be created when closed */
	off_t	loc;			/* current position in the file (bytes) */
	uint8	blk;			/* first block of this file */
	uint8	addr;			/* maple address of the vmu to use */
	uint8	filesize;		/* file length from dirent (in 512-byte blks) */
	uint8	*data;			/* copy of the whole file */
} vmu_fh_t;

/* Directory handles */
typedef struct vmu_dh_str {
	uint32	strtype;		/* 0==dir, 1==file */
	struct vmu_dh_str *next;	/* next in the list */

	dirent_t dirent;		/* Dirent to pass back */
	uint8	*dirblocks;		/* Copy of all directory blocks */
	uint16	entry;			/* Current dirent */
	uint16	dircnt;			/* Count of dir entries */	
	uint8	addr;			/* VMU address */
} vmu_dh_t;

/* Linked list of open files (controlled by "mutex") */
static vmu_fh_t *vmu_fh = NULL;

/* Thread mutex for vmu_fh access */
static spinlock_t mutex;

/* Thread mutex for vmu writing operations */
static spinlock_t writemutex;

/* directory entries, 32 bytes each */
typedef struct {
	uint8	filetype;	/* 0x00 = no file; 0x33 = data; 0xcc = a game */
	uint8	copyprotect;	/* 0x00 = copyable; 0xff = copy protected */
	uint16	firstblk;	/* location of the first block in the file */
	char	filename[12];	/* there is no null terminator */
	uint8	cent;		/* these are all file creation date stuff, in BCD format */
	uint8	year;
	uint8	month;
	uint8	day;
	uint8	hour;
	uint8	min;
	uint8	sec;
	uint8	dow;		/* day of week (0 = monday, etc) */
	uint16	filesize;	/* size of the file in blocks */
	uint16	hdroff;		/* offset of header, in blocks from start of file */
	uint8	dummys[4];	/* unused */
} directory_t;

/* Convert a decimal number to BCD; max of two digits */
static uint8 dec_to_bcd(int dec) {
	uint8 rv = 0;

	rv = dec % 10;
	rv |= ((dec / 10) % 10) << 4;

	return rv;
}

/* Fill in the date on a directory_t for writing */
static void vmu_fill_dir_time(directory_t *d) {
	time_t t;
	struct tm tm;

	/* Get the time */
	t = time(NULL);
	localtime_r(&t, &tm);

	/* Fill in the struct, converting to BCD */
	d->cent = 0x20;		/* Change if neccessary ;-) */
	d->year = dec_to_bcd(tm.tm_year - 100);
	d->month = dec_to_bcd(tm.tm_mon + 1);
	d->day = dec_to_bcd(tm.tm_mday);
	d->hour = dec_to_bcd(tm.tm_hour);
	d->min = dec_to_bcd(tm.tm_min);
	d->sec = dec_to_bcd(tm.tm_sec);
	d->dow = dec_to_bcd((tm.tm_wday - 1) % 7);
}

/* Take a VMUFS path and return the requested address */
static uint8 vmu_path_to_addr(const char *p) {
	if (p[0] != '/') return 0;		/* Only absolute paths */
	if (p[1] < 'a' || p[1] > 'd') return 0;	/* Unit A-D, device 0-5 */
	if (p[2] < '0' || p[2] > '5') return 0; /* FIXME: make this case insensitive */
	
	return maple_create_addr(p[1] - 'a', p[2] - '0');
}

/* opendir function */
vmu_fh_t *vmu_open_dir(uint8 addr) {
	uint8 	buff[512]; 	/* for reading the directory entry and root block */
	uint16	*buff16;	/* 16-bit version of buff */
	uint16	dirblock;	/* the directory starting block */
	uint16	dirlength;	/* size of the directory in blocks */
	int	i, n;
	vmu_dh_t *dh;

	buff16 = (uint16*)buff;

	/* read the root block and find out where the directory is and how long it is */
	if ( (i=vmu_block_read(addr, 255, buff)) != 0) {
		dbglog(DBG_ERROR, "VMUFS: Can't read root block (%d)\r\n", i);
		return 0;
	}
	dirblock = buff16[0x4a/2];
	dirlength = buff16[0x4c/2];

	/* Allocate a buffer for the dir blocks */
	dh = malloc(sizeof(vmu_dh_t));
	dh->strtype = VMU_DIR;
	dh->dirblocks = malloc(512*dirlength);
	dh->entry = 0;
	dh->dircnt = (dirlength * 512) / 32;
	dh->addr = addr;
	
	/* Read all dir blocks */
	for (n=0; n<dirlength; n++) {
		if (vmu_block_read(addr, dirblock, dh->dirblocks + n*512) != 0) {
			dbglog(DBG_ERROR, "vmu_open_dir: Can't read dir block %d\r\n", dirblock);
			return 0;
		}
		dirblock--;
	}
	
	return (vmu_fh_t *)dh;
}

/* openfile function */
vmu_fh_t *vmu_open_file(uint8 addr, const char *path, int mode) {
	vmu_fh_t *fd;		/* file descriptor */
	int32	diridx;		/* directory number */
	uint8 	buff[512]; 	/* for reading the directory entry and root block */
	uint16	*buff16;	/* 16-bit version of buff */
	uint16	dirblock;	/* the directory starting block */
	uint16	dirlength;	/* size of the directory in blocks */
	directory_t dir;
	int	i, n;

	buff16 = (uint16*)buff;

	/* read the root block and find out where the directory is and how long it is */
	if ( (i=vmu_block_read(addr, 255, buff)) != 0) {
		dbglog(DBG_ERROR, "VMUFS: Can't read root block (%d)\r\n", i);
		return NULL;
	}
	dirblock = buff16[0x4a/2];
	dirlength = buff16[0x4c/2];

	/* search through the directory entries and find the first one with this filename */
	diridx = -1;
	for (n = dirlength; n > 0; n--) {
		if (vmu_block_read(addr, dirblock, buff) != 0) {
			dbglog(DBG_ERROR, "vmu_open_file: Can't read dir block %d\r\n", dirblock);
			return NULL;
		}

		for (i = 0; i < 16; i++) {
			memcpy(&dir, &buff[i * 32], sizeof(directory_t));
			if (strnicmp(&path[4], dir.filename, 12) == 0) {
				diridx = i;
				diridx += (dirblock - n) * 16;
				n = 0;		/* quit the outer for */
				break;
			}
		}
		dirblock--;
	}
	/* the direntry doesn't exist, so we need to make some dir stuff */
	if (diridx == -1) {
		/* if the file wasn't opened for writing we get upset now */
		if ((mode & O_MODE_MASK) != O_RDWR && (mode & O_MODE_MASK) != O_WRONLY) {
			return NULL;
		} else {
			dir.filesize = 1;
			dir.firstblk = 0;
		}
	}

	/* malloc a new fh struct */
	fd = malloc(sizeof(vmu_fh_t));

	/* fill in the filehandle struct */
	fd->strtype = VMU_FILE;
	fd->diridx = diridx;
	fd->blk = dir.firstblk;
	fd->loc = 0;
	fd->addr = addr;
	fd->mode = mode;
	strncpy(fd->name, &path[4], 12);
	fd->filesize = dir.filesize;
	if (fd->filesize > 200) {
		dbglog(DBG_WARNING, "VMUFS: file %s greater than 200 blocks: corrupt card?\r\n", path);
		free(fd);
		return NULL;
	}

	/* if the file doesn't exist we don't need to read it, so we malloc 512 and exit now */
	if (fd->diridx == -1) {
		fd->data = malloc(512);
		memset(fd->data, 0, 512);
		return fd;
	}
	
	/* Read the FAT */
	if (vmu_block_read(addr, 254, (uint8*)buff) < 0) {
		dbglog(DBG_ERROR, "Can't read VMU FAT (address %02x)\r\n", addr);
		free(fd);
		return NULL;
	}
	
	/* Follow the FAT, reading all blocks */
	fd->data = malloc(dir.filesize*512);
	dirblock = fd->blk;
	for (i=0; i<fd->filesize; i++) {
		dbglog(DBG_KDEBUG, "reading block: %d\r\n", dirblock);
		if (vmu_block_read(addr, dirblock, fd->data+i*512) < 0) {
			dbglog(DBG_ERROR, "Can't read block %d\r\n", dirblock);
			free(fd->data);
			free(fd);
			return NULL;
		}
		if (dirblock == 0xfffa && i < fd->filesize - 1) {
			dbglog(DBG_WARNING, "Warning: File shorter in FAT than DIR (%d vs %d)\r\n", i, fd->filesize);
			fd->filesize = i + 1; 
			break;
		}
		dirblock = buff16[dirblock];
	}
	
	return fd;
}

/* open function */
uint32 vmu_open(const char *path, int mode) {
	uint8		addr;		/* maple bus address of the vmu unit */
	vmu_fh_t	*fh;

	/* figgure out which vmu slot is being opened */
	addr = vmu_path_to_addr(path);
	/* printf("VMUFS: card address is %02x\r\n", addr); */
	if (addr == 0) return 0;

	/* Check for open as dir */
	if (strlen(path) == 3) {
		if (!(mode & O_DIR)) return 0;
		fh = vmu_open_dir(addr);
	} else {
		fh = vmu_open_file(addr, path, mode);
	}
	if (fh == NULL) return 0;

	/* link the fh onto the top of the list */
	spinlock_lock(&mutex);
	fh->next = vmu_fh;
	vmu_fh = fh;
	spinlock_unlock(&mutex);

	return (uint32)fh;
}

/* write a file out before closing it: we aren't perfect on error handling here */
int vmu_write_close(uint32 hnd) {
	uint8		buff[512];
	uint8		dirblockbuff[512];
	vmu_fh_t	*fh;
	directory_t	*dir;
	uint16		*buff16;
	int		dirblock, dirlength, currblk;
	int		i, n, lastblk;

	fh = (vmu_fh_t*)hnd;

	/* for now I only deal with creating NEW files */
	if (fh->diridx != -1)
		return -1;

	/* for (i = 0; i < fh->filesize * 512; i++)
		printf("%c", fh->data[i]); */

	buff16 = (uint16 *) buff;

	spinlock_lock(&writemutex);

	/* read the root block and find out where the directory is and how long it is */
	if ((i = vmu_block_read(fh->addr, 255, buff)) != 0) {
		dbglog(DBG_ERROR, "vmu_write_close: Can't read root block (%d)\r\n", i);
		goto return_error;
	}
	dirblock = buff16[0x4a/2];
	dirlength = buff16[0x4c/2];

	/* search through the directory entries and find the first free entry */
	dir = NULL;
	for (n = dirlength; n > 0; n--) {
		if (vmu_block_read(fh->addr, dirblock, dirblockbuff) != 0) {
			dbglog(DBG_ERROR, "vmu_write_close: Can't read dir block %d\r\n", dirblock);
			goto return_error;
		}

		for (i = 0; i < 16; i++) {
			dir = (directory_t *) &dirblockbuff[i * 32];
			if (dir->filetype == 0) {
				dbglog(DBG_KDEBUG, "Got empty directory ent (%d, %d)\r\n", n, i);
				n = 0;		/* quit the outer for */
				break;
			}
		}
		dirblock--;
	}
	dirblock++;

	if (dir == NULL) {
		dbglog(DBG_ERROR, "VMUFS: No empty directory slots in VMU!\n");
		goto return_error;
	}

	/* add check for not getting empty direntry */

	/* Read the FAT */
	if (vmu_block_read(fh->addr, 254, (uint8*)buff) < 0) {
		dbglog(DBG_ERROR, "Can't read VMU FAT (address %02x)\r\n", fh->addr);
		goto return_error;
	}

	/* zero out the dir struct */
	memset(dir, 0, sizeof(directory_t));

	/* Allocate blocks and write the data */
	lastblk = -1;
	for (n=0; n<fh->filesize; n++) {
		/* Find an open block */
		currblk = -1;
		for (i=0; i<256; i++) {
			if (buff16[i] == 0xfffc) {
				currblk = i;
				break;
			}
		}

		/* Didn't find one? */
		if (currblk == -1) {
			spinlock_unlock(&writemutex);
			goto return_error;
		}

		/* If this is the first block, set that in the header */
		if (lastblk == -1) {
			dir->firstblk = (uint16)currblk;
			dbglog(DBG_KDEBUG, "first dirblk: %d\n", dir->firstblk);
		}

		/* If this is the last block, then mark it as such in the FAT;
		   if not then we want to put something there in our buffer
		   so that the block allocator doesn't find an empty one. */
		if (n == fh->filesize - 1)
			buff16[currblk] = 0xfffa;
		else
			buff16[currblk] = 0xdead;

		/* If this isn't the first block, then link the last block
		   in the chain to us. */
		if (lastblk != -1)
			buff16[lastblk] = currblk;

		/* Write out the block */
		dbglog(DBG_KDEBUG, "writing blk %d to vmu at %d\n", n, currblk);
		if (vmu_block_write(fh->addr, currblk, fh->data + n*512) < 0) {
			dbglog(DBG_ERROR, "Can't write blk %d of file to VMU\n", n);
			goto return_error;
		}

		lastblk = currblk;
	}

	/* write the new fat */
	if (vmu_block_write(fh->addr, 254, buff) < 0) {
		dbglog(DBG_ERROR, "Can't write VMU FAT (address %02x)\r\n", fh->addr);
		goto return_error;
	}

	/* fill in the rest of the directory entry */
	dir->filetype = 0x33;
	dir->copyprotect = 0x00;
	strncpy(dir->filename, fh->name, 12);
	dir->filesize = fh->filesize;
	dir->hdroff = 0;	/* Header is at file beginning */

	/* I'd like to use rtc_unix_secs() eventually here, but we need
	   a bit more libc support first. For now, dump in an arbitrary
	   date so it's not just crap data. */
	vmu_fill_dir_time(dir);

	/* write the new directory */
	if (vmu_block_write(fh->addr, dirblock, dirblockbuff) != 0) {
		dbglog(DBG_ERROR, "vmu_write_close: Can't write dir block %d\r\n", dirblock);
		goto return_error;
	}

	spinlock_unlock(&writemutex);

	return 0;

return_error:
	spinlock_unlock(&writemutex);
	return -1;
}

/* close a file */
void vmu_close(uint32 hnd) {
	vmu_fh_t *last, *cur;
	vmu_fh_t *fh;

	last = NULL;
	fh = (vmu_fh_t *)hnd;

	/* if the file is open for writing we need to flush it to the vmu */
	if (fh->strtype == VMU_FILE) {
		if ((fh->mode & O_MODE_MASK) == O_WRONLY || (fh->mode & O_MODE_MASK) == O_RDWR)
			vmu_write_close(hnd);
	}

	/* Look for the one to get rid of */
	spinlock_lock(&mutex);
	cur = vmu_fh;
	while (cur != NULL) {
		if (cur == fh) {
			if (last == NULL)
				vmu_fh = cur->next;
			else
				last->next = cur->next;
				
			switch (cur->strtype) {
				case 0: {
					vmu_dh_t *dir = (vmu_dh_t*)cur;
					free(dir->dirblocks);
					break;
				}
				case 1:
					free(cur->data);
					break;
			}
			free(cur);
			break;
		}
		last = cur;
		cur = cur->next;
	}
	spinlock_unlock(&mutex);
}

/* Verify that a given hnd is actually in the list */
int vmu_verify_hnd(uint32 hnd, int type) {
	vmu_fh_t	*cur;
	int		rv;

	rv = 0;
	
	spinlock_lock(&mutex);
	cur = vmu_fh;
	while (cur != NULL) {
		if ((uint32)cur == hnd) {
			rv = 1;
			break;
		}
		cur = cur->next;
	}
	spinlock_unlock(&mutex);
	
	if (rv)
		return cur->strtype == type;
	else
		return 0;
}

/* read function */
ssize_t vmu_read(uint32 hnd, void *buffer, size_t cnt) {
	vmu_fh_t *fh;

	/* Check the handle */
	if (!vmu_verify_hnd(hnd, VMU_FILE))
		return -1;

	fh = (vmu_fh_t *)hnd;

	/* make sure we're opened for reading */
	if ((fh->mode & O_MODE_MASK) != O_RDONLY && (fh->mode & O_MODE_MASK) != O_RDWR)
		return 0;

	/* Check size */
	cnt = (fh->loc + cnt) > (fh->filesize*512) ?
		(fh->filesize*512 - fh->loc) : cnt;

	/* Copy out the data */
	memcpy(buffer, fh->data+fh->loc, cnt);
	fh->loc += cnt;
	
	return cnt;
}

/* write function */
ssize_t vmu_write(uint32 hnd, const void *buffer, size_t cnt) {
	vmu_fh_t	*fh;
	void		*tmp;
	int		n;

	/* Check the handle we were given */
	if (!vmu_verify_hnd(hnd, VMU_FILE))
		return -1;

	fh = (vmu_fh_t *)hnd;

	/* Make sure we're opened for writing */
	if ((fh->mode & O_MODE_MASK) != O_WRONLY && (fh->mode & O_MODE_MASK) != O_RDWR)
		return -1;

	/* Check to make sure we have enough room in data */
	if (fh->loc + cnt > fh->filesize * 512) {
		/* Figure out the new block count */
		n = ((fh->loc + cnt) - (fh->filesize * 512));
		if (n & 511)
			n = (n+512) & ~511;
		n = n / 512;

		dbglog(DBG_KDEBUG, "VMUFS: extending file's filesize by %d\n", n);
		
		/* We alloc another 512*n bytes for the file */
		tmp = realloc(fh->data, (fh->filesize + n) * 512);
		if (!tmp) {
			dbglog(DBG_ERROR, "VMUFS: unable to realloc another 512 bytes\r\n");
			return -1;
		}

		// Assign the new pointer and clear out the new space
		fh->data = tmp;
		memset(fh->data + fh->filesize * 512, 0, 512*n);
		fh->filesize += n;
	}

	/* insert the data in buffer into fh->data at fh->loc */
	/* memmove(&fh->data[fh->loc + cnt], &fh->data[fh->loc], cnt);
	memmove(&fh->data[fh->loc], buffer, cnt); */
	dbglog(DBG_KDEBUG, "VMUFS: adding %d bytes of data at loc %d (%d avail)\n",
		cnt, fh->loc, fh->filesize * 512);
	memcpy(fh->data + fh->loc, buffer, cnt);
	fh->loc += cnt;

	return cnt;
}

/* tell the current position in the file */
off_t vmu_tell(uint32 hnd) {
	/* Check the handle */
	if (!vmu_verify_hnd(hnd, VMU_FILE))
		return -1;

	return ((vmu_fh_t *) hnd)->loc;
}

/* return the filesize */
size_t vmu_total(uint32 fd) {
	/* Check the handle */
	if (!vmu_verify_hnd(fd, VMU_FILE))
		return -1;

	/* note that all filesizes are multiples of 512 for the vmu */
	return (((vmu_fh_t *) fd)->filesize) * 512;
}

/* read a directory handle */
dirent_t *vmu_readdir(uint32 fd) {
	vmu_dh_t	*dh;
	directory_t	*dir;

	/* Check the handle */
	if (!vmu_verify_hnd(fd, VMU_DIR)) {
		return NULL;
	}

	dh = (vmu_dh_t*)fd;

	/* printf("VMUFS: readdir on entry %d of %d\r\n", dh->entry, dh->dircnt); */

	/* Go to the next non-empty entry */
	while (dh->entry < dh->dircnt
		&& ((directory_t*)(dh->dirblocks + 32*dh->entry))->filetype == 0)
			dh->entry++;

	/* Check if we have any entries left */
	if (dh->entry >= dh->dircnt)
		return NULL;
	
	/* printf("VMUFS: reading non-null entry %d\r\n", dh->entry); */
		
	/* Ok, extract it and fill the dirent struct */
	dir = (directory_t*)(dh->dirblocks + 32*dh->entry);
	dh->dirent.size = dir->filesize*512;
	strncpy(dh->dirent.name, dir->filename, 12);
	dh->dirent.name[12] = 0;
	dh->dirent.time = 0;	/* FIXME */
	dh->dirent.attr = 0;

	/* Move to the next entry */
	dh->entry++;

	return &dh->dirent;
}

/* Delete a file (contributed by Brian Peek) */
int vmu_unlink(const char *path) {
	int32	diridx;		/* directory number */
	uint8 	buff[512]; 	/* for reading the directory entry and root block */
	uint16	*buff16;	/* 16-bit version of buff */
	uint16	dirblock;	/* the directory starting block */
	uint16	dirlength;	/* size of the directory in blocks */
	uint8	addr = 0;	/* address of VMU */
	uint8	empty_blk[512];	/* zero'ed block */
	uint16	tmp;		/* store previous FAT entry */
	directory_t dir;
	int	i, n;

	memset(empty_blk, 0, 512);

	/* convert path to valid VMU address */
	addr = vmu_path_to_addr(path);

	buff16 = (uint16*)buff;

	spinlock_lock(&writemutex);

	/* read the root block and find out where the directory is and how long it is */
	if ( (i=vmu_block_read(addr, 255, buff)) != 0) {
		dbglog(DBG_ERROR, "VMUFS: Can't read root block (%d)\r\n", i);
		goto return_error;
	}
	dirblock = buff16[0x4a/2];
	dirlength = buff16[0x4c/2];

	/* search through the directory entries and find the first one with this filename */
	diridx = -1;
	for (n = dirlength; n > 0; n--) {
		if (vmu_block_read(addr, dirblock, buff) != 0) {
			dbglog(DBG_ERROR, "vmu_unlink_file: Can't read dir block %d\r\n", dirblock);
			goto return_error;
		}

		for (i = 0; i < 16; i++) {
			memcpy(&dir, &buff[i * 32], sizeof(directory_t));
			if (strnicmp(&path[4], dir.filename, 12) == 0) {
				diridx = i;
				diridx += (dirblock - n) * 16;
				n = 0;		/* quit the outer for */
				break;
			}
		}
		dirblock--;
	}

	/* the direntry doesn't exist, so bail */
	if (diridx == -1) {
		dbglog(DBG_ERROR, "vmu_unlink: file not found: %s\r\n", path);
		goto return_error;
	}

	if (dir.filesize > 200) {
		dbglog(DBG_WARNING, "VMUFS: file %s greater than 200 blocks: corrupt card?\r\n", path);
		goto return_error;
	}

	dbglog(DBG_KDEBUG, "clearing directory block: %d\r\n", dirblock+1);
	memcpy(&buff[i*32], 0, sizeof(directory_t));
	if (vmu_block_write(addr, dirblock+1, buff) < 0) {
		spinlock_unlock(&writemutex);
		dbglog(DBG_ERROR, "Can't write block %d\r\n", dirblock);
		goto return_error;
	}

	/* Read the FAT */
	if (vmu_block_read(addr, 254, (uint8*)buff) < 0) {
		dbglog(DBG_ERROR, "Can't read VMU FAT (address %02x)\r\n", addr);
		goto return_error;
	}
	
	/* Follow the FAT, reading all blocks */
	dirblock = dir.firstblk;
	for (i=0; i<dir.filesize; i++) {
		dbglog(DBG_KDEBUG, "clearing block: %d\r\n", dirblock);
		if (vmu_block_write(addr, dirblock, empty_blk) < 0) {
			spinlock_unlock(&writemutex);
			dbglog(DBG_ERROR, "Can't write block %d\r\n", dirblock);
			goto return_error;
		}
		if (dirblock == 0xfffa && i < dir.filesize - 1) {
			dbglog(DBG_WARNING, "Warning: File shorter in FAT than DIR (%d vs %d)\r\n", i, dir.filesize);
			dir.filesize = i + 1; 
			goto return_error;
		}
		tmp = buff16[dirblock];
		buff16[dirblock] = 0xfffc;
		dirblock = tmp;
	}
	
	/* Write the FAT */
	if (vmu_block_write(addr, 254, (uint8*)buff) < 0) {
		spinlock_unlock(&writemutex);
		dbglog(DBG_ERROR, "Can't write VMU FAT (address %02x)\r\n", addr);
		goto return_error;
	}

	spinlock_unlock(&writemutex);
	return 0;

return_error:
	spinlock_unlock(&writemutex);
	return -1;
}

/* handler interface */
static vfs_handler vh = {
	{ "/vmu" },	/* path prefix */
	0, 1,		/* In-kernel, cache (not implemented yet, however) */
	NULL,		/* Linked list pointer */
	
	vmu_open,
	vmu_close,
	vmu_read,
	vmu_write,	/* the write function */
	NULL,		/* the seek function */
	vmu_tell,
	vmu_total,
	vmu_readdir,	/* readdir */
	NULL,		/* ioctl */
	NULL,		/* rename/move */
	vmu_unlink,	/* unlink */		
	NULL		/* mmap (need to implement this) */
};

int fs_vmu_init() {
	spinlock_init(&mutex);
	return fs_handler_add("/vmu", &vh);
}

int fs_vmu_shutdown() { return fs_handler_remove(&vh); }

