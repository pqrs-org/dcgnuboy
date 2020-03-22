/* KallistiOS ##version##

   kernel/thread_vars.c
   (c)2000-2001 Dan Potter
*/

#include <kos/thread.h>
#include <string.h>

CVSID("thread_vars.c,v 1.3 2002/01/14 05:56:19 bardtx Exp");

/* You may (rightfully) be wondering why the heck this module exists at all.
   Well, those of you not in the know about how a linker works anyway =). 
   What happens when you link a program is that any object file containing
   a requried symbol is automatically linked in, in full. So unless we
   seperate this out, any module that checks to see if you're using
   threads is going to force the inclusion of the entire threads package
   (yuck!).
*/

/* Are threads enabled? */
int thd_enabled = 0;

/* Static thread structure for storing things like PWD when threads are
   not actually enabled */
static kthread_t thd_static;

/* The currently executing thread */
kthread_t *thd_current = &thd_static;

/* Find the current thread */
kthread_t *thd_get_current() {
	return thd_current;
}

/* Set/Get thread pwd (in here so FS doesn't bring in threads) */
/* Retrieve / set thread pwd */   
const char *thd_get_pwd(kthread_t *thd) {
	return thd->pwd;
}

void thd_set_pwd(kthread_t *thd, const char *pwd) {
	strncpy(thd->pwd, pwd, sizeof(thd->pwd) - 1);
}
                

