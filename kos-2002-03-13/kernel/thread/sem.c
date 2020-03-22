/* KallistiOS ##version##

   sem.c
   (c)2001 Dan Potter
*/

/* Defines semaphores */

/**************************************/

#include <string.h>
#include <malloc.h>

#include <kos/thread.h>
#include <kos/limits.h>
#include <kos/sem.h>
#include <sys/queue.h>
#include <arch/syscall.h>
#include <arch/spinlock.h>

CVSID("sem.c,v 1.2 2002/01/06 01:14:48 bardtx Exp");

/**************************************/

/* Semaphore list spinlock */
static spinlock_t mutex;

/* Global list of semaphores */
static struct semlist sem_list;

/* Allocate a new semaphore; the semaphore will be assigned
   to the calling process and when that process dies, the semaphore
   will also die. */
semaphore_t *sem_create(int value) {
	semaphore_t	*sm;

	/* Create a semaphore structure */
	sm = (semaphore_t*)malloc(sizeof(semaphore_t));
	sm->owner = thd_current->tid;
	sm->count = value;
	TAILQ_INIT(&sm->blocked_wait);

	/* Add to the global list */
	spinlock_lock(&mutex);
	LIST_INSERT_HEAD(&sem_list, sm, g_list);
	spinlock_unlock(&mutex);

	/* Add to the process' list of semaphores */
	/* XXX Do later */

	return sm;
}

/* Take care of destroying a semaphore */
void sem_destroy(semaphore_t *sm) {
	/* XXX Do something with queued threads */

	/* Remove it from the global list */
	spinlock_lock(&mutex);
	LIST_REMOVE(sm, g_list);
	spinlock_unlock(&mutex);

	/* Free the memory */
	free(sm);
}

/* Wait on a semaphore */
static void sc_sem_wait(semaphore_t *sm) {
	/* If there's enough count left, then let the thread proceed */
	if (sm->count > 0) {
		sm->count--;
		return;
	}

	/* Otherwise, block the thread on semaphore wait and run another */
	thd_current->state = STATE_WAITSEM;
	TAILQ_INSERT_TAIL(&sm->blocked_wait, thd_current, thdq);
	thd_schedule();
}
void sem_wait(semaphore_t *sm) {
	if (irq_inside_int()) return;
	SYSCALL(sc_sem_wait);
}

/* Signal a semaphore */ 
static void sc_sem_signal(semaphore_t *sm) {
	kthread_t	*thd;

	/* Is there anyone waiting? If so, pass off to them */
	if (!TAILQ_EMPTY(&sm->blocked_wait)) {
		/* Remove it from the queue */
		thd = TAILQ_FIRST(&sm->blocked_wait);
		TAILQ_REMOVE(&sm->blocked_wait, thd, thdq);

		/* Re-activate it */
		thd->state = STATE_READY;
		thd_add_to_runnable(thd);
	} else {
		/* No one is waiting, so just add another tick */
		sm->count++;
	}
}
void sem_signal(semaphore_t *sm) {
	if (irq_inside_int()) {
		sc_sem_signal(sm);
	} else {
		SYSCALL(sc_sem_signal);
	}
}

/* Return the semaphore count */
int sem_count(semaphore_t *sm) {
	/* Look for the semaphore */
	return sm->count;
}

/* Free all semaphores for the given process' pid */
/* void sem_freeall(pid_t pid) {
	semaphore_t *n1, *n2;

	n1 = LIST_FIRST(&sem_list);
	while (n1 != NULL) {
		n2 = LIST_NEXT(n1, g_list);
		if (n1->owner == pid) {
			LIST_REMOVE(n1, g_list);
			sem_destroy(n1);
		}
		n1 = n2;
	}
} */
      
/* Initialize semaphore structures */
int sem_init() {
	LIST_INIT(&sem_list);
	return 0;
}

/* Shut down semaphore structures */
void sem_shutdown() { }


