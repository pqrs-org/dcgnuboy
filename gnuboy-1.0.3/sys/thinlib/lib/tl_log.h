/*
** thinlib (c) 2001 Matthew Conte (matt@conte.com)
**
**
** tl_log.h
**
** Error logging header file
**
** tl_log.h,v 1.1.1.1 2002/04/09 14:41:22 tekezo Exp
*/

#ifndef _TL_LOG_H_
#define _TL_LOG_H_

extern void thin_printf(const char *format, ... );
extern void thin_setlogfunc(int (*logfunc)(const char *string, ... ));
extern void thin_assert(int expr, int line, const char *file, char *msg);

#endif /* !_TL_LOG_H_ */

/*
** tl_log.h,v
** Revision 1.1.1.1  2002/04/09 14:41:22  tekezo
**
**
*/
