/*
** thinlib (c) 2001 Matthew Conte (matt@conte.com)
**
**
** tl_joy.h
**
** DOS joystick reading defines / protos
**
** tl_joy.h,v 1.1.1.1 2002/04/09 14:41:22 tekezo Exp
*/

#ifndef _TL_JOY_H_
#define _TL_JOY_H_

#define  JOY_MAX_BUTTONS   4

typedef struct joy_s
{
   int left, right, up, down;
   int button[JOY_MAX_BUTTONS];
} joy_t;

extern void thin_joy_shutdown(void);
extern int thin_joy_init(void);
extern int thin_joy_read(joy_t *joy);

#endif /* !_TL_JOY_H_ */

/*
** tl_joy.h,v
** Revision 1.1.1.1  2002/04/09 14:41:22  tekezo
**
**
*/
