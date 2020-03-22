/* KallistiOS ##version##

   gettimeofday.c
   (c)2002 Dan Potter

   gettimeofday.c,v 1.1 2002/02/24 01:24:00 bardtx Exp
*/

#include <assert.h>
#include <time.h>
#include <arch/timer.h>
#include <arch/rtc.h>

/* This is kind of approximate and works only with "localtime" */
int gettimeofday(struct timeval *tv, struct timezone *tz) {
	uint32	m, s;
	
	assert( tv != NULL );

	timer_ms_gettime(&s, &m);
	tv->tv_sec = rtc_unix_secs() + s;
	tv->tv_usec = m;

	return 0;
}
