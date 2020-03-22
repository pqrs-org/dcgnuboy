/* KallistiOS ##version##

   malloc_debug.c
   (c)2001-2002 Dan Potter
*/

static char id[] = "KOS malloc_debug.c,v 1.2 2002/02/22 07:41:22 bardtx Exp";

#include <malloc.h>
#include <arch/types.h>
#include <kos/thread.h>
#include <arch/spinlock.h>

static spinlock_t mutex = SPINLOCK_INITIALIZER;

/* The memory pool will look something like this:
   memctl_t	(padded to 32 bytes long)
   char[1024]	pre-buffer no-touch zone
   char[]	allocated buffer space
   char[1024]	post-buffer no-touch zone
 */
typedef struct memctl {
	uint32		magic;
	uint32		size;
	tid_t		thread;
	uint32		addr;
	int		inuse, damaged;	
	uint32		*post;
	struct memctl	*next;
} memctl_t;

static memctl_t *first = NULL, *last = NULL;

void * sbrk(int amt);

void *malloc(size_t amt) {
	memctl_t *ctl;
	uint32 *nt1, *nt2;
	uint32 *space;
	uint32 pr;
	int i;

	__asm__ __volatile__("sts	pr,%0\n"
		: "=&z" (pr)
		: /* no inputs */
		: "memory" );

	spinlock_lock(&mutex);

	/* Get a control block space */
	ctl = sbrk(1024);
	memset(ctl, 0, 1024);
	ctl->magic = 0x00f00b44;
	ctl->size = amt;
	// ctl->thread = thd_get_tid();
	ctl->thread = thd_current->tid;
	ctl->addr = pr;
	ctl->inuse = 1;
	ctl->damaged = 0;

	if (!first)
		first = ctl;
	if (last)
		last->next = ctl;

	/* Fill pre-buffer no-touch zone */
	nt1 = ((uint32*)ctl);
	for (i=sizeof(memctl_t)/4; i<1024/4; i++) {
		nt1[i] = 0xdeadbeef;
	}

	/* Allocate actual buffer space */
	if (amt & 31) {
		amt = (amt & ~31) + 32;
	}
	space = sbrk(amt);

	/* Allocate post-buffer no-touch zone */
	nt2 = sbrk(1024);
	for (i=0; i<1024/4; i++) {
		nt2[i] = 0xfeedc0de;
	}

	ctl->post = nt2;
	ctl->next = NULL;
	last = ctl;

	spinlock_unlock(&mutex);

	printf("Thread %d/%08lx allocated %d bytes at %08lx; %08lx left\r\n",
		ctl->thread, ctl->addr, ctl->size, space, 0x8d000000 - (uint32)sbrk(0));

	return space;
}

void *memalign(size_t alignment, size_t amt) {
	uint32 sb, pr;

	__asm__ __volatile__("sts	pr,%0\n"
		: "=&z" (pr)
		: /* no inputs */
		: "memory" );
	printf("Real address for the following is %08x\r\n", pr);

	sb = (uint32)sbrk(0);
	if (sb & (alignment - 1)) {
		sbrk(alignment - (sb & (alignment - 1)));
	}
	return malloc(amt);
}

void free(void *block) {
	memctl_t *ctl;
	uint32 *nt1, *nt2, pr;
	int i;
	
	__asm__ __volatile__("sts	pr,%0\n"
		: "=&z" (pr)
		: /* no inputs */
		: "memory" );

	spinlock_lock(&mutex);

	printf("Thread %d/%08x freeing block @ %08x\r\n",
		thd_current->tid, pr, ((uint32)block) - 1024);
	ctl = (memctl_t*)( ((uint32)block) - 1024);
	if (ctl->magic != 0x00f00b44) {
		printf("  'magic' is not correct! %08x\r\n", ctl->magic);
		spinlock_unlock(&mutex);
		return;
	}

	if (ctl->inuse == 0) {
		printf("  block already freed!\r\n");
		ctl->damaged = 1;
	}

	nt1 = (uint32*)( ((uint32)block) - 1024 );
	for (i=sizeof(memctl_t)/4; i<1024/4; i++) {
		if (nt1[i] != 0xdeadbeef) {
			printf("  pre-magic is wrong at index %d (%08x)\r\n", i, nt1[i]);
			ctl->damaged = 1;
			break;
		}
	}

	nt2 = ctl->post;
	for (i=0; i<1024/4; i++) {
		if (nt2[i] != 0xfeedc0de) {
			printf("  post-magic is wrong at index %d (%08x)\r\n", i, nt2[i]);
			ctl->damaged = 1;
			break;
		}
	}

	ctl->inuse = 0;
	spinlock_unlock(&mutex);
}

void malloc_stats() {
	memctl_t *ctl;
	int dmgcnt, leaked;
	
	if (!first) {
		printf("NO MEMORY ALLOCATED\r\n");
		return;
	}

	printf("%d TOTAL BYTES OF MEMORY ALLOCATED\r\n",
		(uint32)sbrk(0) - (uint32)first);

	ctl = first;
	printf("Blocks still allocated:\r\n");
	dmgcnt = 0; leaked = 0;
	while (ctl) {
		if (ctl->inuse) {
			printf("  INUSE %08x: size %d, thread %d, addr %08x\r\n",
				ctl, ctl->size, ctl->thread, ctl->addr);
			leaked += ctl->size;
		}
		if (ctl->damaged) {
			printf("  DMGED %08x: size %d, thread %d, addr %08x\r\n",
				ctl, ctl->size, ctl->thread, ctl->addr);
			dmgcnt++;
		}
		ctl = ctl->next;
	}
	printf("End of list; %d damaged blocks and %d leaked bytes\r\n", dmgcnt, leaked);
}

void *realloc(void *ptr, size_t amt) {
	printf("UNSUPPORTED REALLOC CALLED\r\n");
	return NULL;
}
