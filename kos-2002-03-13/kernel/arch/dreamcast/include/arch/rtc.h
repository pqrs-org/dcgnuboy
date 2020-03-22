/* KallistiOS ##version##

   arch/dreamcast/include/rtc.h
   (c)2000-2001 Dan Potter

   rtc.h,v 1.3 2002/02/24 02:14:04 bardtx Exp

 */

#ifndef __ARCH_RTC_H
#define __ARCH_RTC_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <time.h>

/* Returns the date/time value as a UNIX epoch time stamp */
time_t rtc_unix_secs();

__END_DECLS

#endif	/* __ARCH_RTC_H */

