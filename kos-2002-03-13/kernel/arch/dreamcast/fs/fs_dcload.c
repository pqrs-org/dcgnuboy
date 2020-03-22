/* KallistiOS ##version##

   kernel/arch/dreamcast/fs/fs_dcload.c
   (c)2001 Andrew Kieschnick
   
*/

/*

This is a rewrite of Dan Potter's fs_serconsole to use the dcload / dc-tool
fileserver and console. 

printf goes to the dc-tool console
/pc corresponds to / on the system running dc-tool

This is the KOS 1.1.x version - a few things that used to be in this file had
to be moved elsewhere, so using fs_dcload now requires patching a file - 
kernel/arch/dreamcast/kernel/main.c. You should have gotten the patch along with this file...

*/

#include <dc/fs_dcload.h>
#include <kos/thread.h>
#include <arch/spinlock.h>
#include <arch/dbgio.h>
#include <kos/fs.h>

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>

CVSID("fs_dcload.c,v 1.6 2002/03/10 07:08:05 bardtx Exp");

static spinlock_t mutex = SPINLOCK_INITIALIZER;

/* Printk replacement */

void dcload_printk(const char *str) {
    spinlock_lock(&mutex);
    dcloadsyscall(DCLOAD_WRITE, 1, str, strlen(str));
    spinlock_unlock(&mutex);
}

static char *dcload_path = NULL;
uint32 dcload_open(const char *fn, int mode) {
    int hnd = 0;
    uint32 h;
    int dcload_mode = 0;
    
    spinlock_lock(&mutex);
    
    if (mode & O_DIR) {
        if (fn[0] == '\0') {
            fn = "/";
        }
	hnd = dcloadsyscall(DCLOAD_OPENDIR, fn);
	if (hnd) {
	    if (dcload_path)
		free(dcload_path);
	    if (fn[strlen(fn) - 1] == '/') {
		dcload_path = malloc(strlen(fn)+1);
		strcpy(dcload_path, fn);
	    } else {
		dcload_path = malloc(strlen(fn)+2);
		strcpy(dcload_path, fn);
		strcat(dcload_path, "/");
	    }
	}
    } else { /* hack */
	if ((mode & O_MODE_MASK) == O_RDONLY)
	    dcload_mode = 0;
	if ((mode & O_MODE_MASK) == O_RDWR)
	    dcload_mode = 2 | 0x0200;
	if ((mode & O_MODE_MASK) == O_WRONLY)
	    dcload_mode = 1 | 0x0200;
	if ((mode & O_MODE_MASK) == O_APPEND)
	    dcload_mode =  2 | 8 | 0x0200;
	if (mode & O_TRUNC)
	    dcload_mode |= 0x0400;
	hnd = dcloadsyscall(DCLOAD_OPEN, fn, dcload_mode, 0644);
	hnd++; /* KOS uses 0 for error, not -1 */
    }
    
    h = hnd;

    spinlock_unlock(&mutex);
    return h;
}

void dcload_close(uint32 hnd) {
    spinlock_lock(&mutex);
    
    if (hnd) {
	if (hnd > 100) /* hack */
	    dcloadsyscall(DCLOAD_CLOSEDIR, hnd);
	else {
	    hnd--; /* KOS uses 0 for error, not -1 */
	    dcloadsyscall(DCLOAD_CLOSE, hnd);
	}
    }
    spinlock_unlock(&mutex);
}

ssize_t dcload_read(uint32 hnd, void *buf, size_t cnt) {
    ssize_t ret = -1;
    
    spinlock_lock(&mutex);
    
    if (hnd) {
	hnd--; /* KOS uses 0 for error, not -1 */
	ret = dcloadsyscall(DCLOAD_READ, hnd, buf, cnt);
    }
    
    spinlock_unlock(&mutex);
    return ret;
}

ssize_t dcload_write(uint32 hnd, const void *buf, size_t cnt) {
    ssize_t ret = -1;
    	
    spinlock_lock(&mutex);
    
    if (hnd) {
	hnd--; /* KOS uses 0 for error, not -1 */
	ret = dcloadsyscall(DCLOAD_WRITE, hnd, buf, cnt);
    }

    spinlock_unlock(&mutex);
    return ret;
}

off_t dcload_seek(uint32 hnd, off_t offset, int whence) {
    off_t ret = -1;

    spinlock_lock(&mutex);

    if (hnd) {
	hnd--; /* KOS uses 0 for error, not -1 */
	ret = dcloadsyscall(DCLOAD_LSEEK, hnd, offset, whence);
    }

    spinlock_unlock(&mutex);
    return ret;
}

off_t dcload_tell(uint32 hnd) {
    off_t ret = -1;
    
    spinlock_lock(&mutex);

    if (hnd) {
	hnd--; /* KOS uses 0 for error, not -1 */
	ret = dcloadsyscall(DCLOAD_LSEEK, hnd, 0, SEEK_CUR);
    }

    spinlock_unlock(&mutex);
    return ret;
}

size_t dcload_total(uint32 hnd) {
    size_t ret = -1;
    size_t cur;
	
    spinlock_lock(&mutex);
	
    if (hnd) {
	hnd--; /* KOS uses 0 for error, not -1 */
	cur = dcloadsyscall(DCLOAD_LSEEK, hnd, 0, SEEK_CUR);
	ret = dcloadsyscall(DCLOAD_LSEEK, hnd, 0, SEEK_END);
	dcloadsyscall(DCLOAD_LSEEK, hnd, cur, SEEK_SET);
    }
	
    spinlock_unlock(&mutex);
    return ret;
}

/* Not thread-safe, but that's ok because neither is the FS */
static dirent_t dirent;
dirent_t *dcload_readdir(uint32 hnd) {
    dirent_t *rv = NULL;
    dcload_dirent_t *dcld;
    dcload_stat_t filestat;
    char *fn;

    if (hnd < 100) return NULL; /* hack */

    spinlock_lock(&mutex);

    dcld = (dcload_dirent_t *)dcloadsyscall(DCLOAD_READDIR, hnd);
    
    if (dcld) {
	rv = &dirent;
	strcpy(rv->name, dcld->d_name);
	rv->size = 0;
	rv->time = 0;
	rv->attr = 0; /* what the hell is attr supposed to be anyways? */

	fn = malloc(strlen(dcload_path)+strlen(dcld->d_name)+1);
	strcpy(fn, dcload_path);
	strcat(fn, dcld->d_name);

	if (!dcloadsyscall(DCLOAD_STAT, fn, &filestat)) {
	    if (filestat.st_mode & S_IFDIR)
		rv->size = -1;
	    else
		rv->size = filestat.st_size;
	    rv->time = filestat.st_mtime;
	    
	}
	
	free(fn);
    }
    
    spinlock_unlock(&mutex);
    return rv;
}

int dcload_rename(const char *fn1, const char *fn2) {
    int ret;

    spinlock_lock(&mutex);

    /* really stupid hack, since I didn't put rename() in dcload */

    ret = dcloadsyscall(DCLOAD_LINK, fn1, fn2);

    if (!ret)
	ret = dcloadsyscall(DCLOAD_UNLINK, fn1);

    spinlock_unlock(&mutex);
    return ret;
}

int dcload_unlink(const char *fn) {
    int ret;

    spinlock_lock(&mutex);

    ret = dcloadsyscall(DCLOAD_UNLINK, fn);

    spinlock_unlock(&mutex);
    return ret;
}

/* Pull all that together */
static vfs_handler vh = {
    { "/pc" },          /* path prefix */
    0, 0,		/* In-kernel, no cacheing */
    NULL,               /* linked list pointer */
    dcload_open, 
    dcload_close,
    dcload_read,
    dcload_write,
    dcload_seek,
    dcload_tell,
    dcload_total,
    dcload_readdir,
    NULL,               /* ioctl */
    dcload_rename,
    dcload_unlink,
    NULL                /* mmap */
};

/* Call this before arch_init_all (or any call to dbgio_*) to use dcload's
   console output functions. */
void fs_dcload_init_console() {
    /* Check for dcload */
    if (*DCLOADMAGICADDR != DCLOADMAGICVALUE)
	return;

    dbgio_set_printk(dcload_printk);
}

static int *dcload_wrkmem = NULL;

int fs_dcload_init() {
    
    /* Check for dcload */
    if (*DCLOADMAGICADDR != DCLOADMAGICVALUE)
	return -1;

    /* Give dcload the 64k it needs to compress data (if on serial) */
    dcload_wrkmem = malloc(65536);
    if (dcload_wrkmem)
    	if (dcloadsyscall(DCLOAD_ASSIGNWRKMEM, dcload_wrkmem) == -1)
    	    free(dcload_wrkmem);
    
    /* Register with VFS */
    return fs_handler_add("/pc", &vh);
}

int fs_dcload_shutdown() {
    /* Check for dcload */
    if (*DCLOADMAGICADDR != DCLOADMAGICVALUE)
	return -1;

    /* Free dcload wrkram */
    if (dcload_wrkmem) {
        dcloadsyscall(DCLOAD_ASSIGNWRKMEM, 0);
        free(dcload_wrkmem);
    }
    
    return fs_handler_remove(&vh);
}

