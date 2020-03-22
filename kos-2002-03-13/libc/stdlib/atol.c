/* KallistiOS ##version##

   atol.c
   (c)2001 Vincent Penne
   Modified from atoi.c by Brian Peek

   atol.c,v 1.1 2002/02/16 23:05:06 bardtx Exp
*/

long atol(const char * s) {
	long m, v;

	v = 0;

	if (*s == '-') {
		m = -1;
		s++;
	} else {
		m = 1;
		if (*s == '+')
			s++;
	}

	while (*s >= '0' && *s <= '9') {
		v = v*10 + *s-'0';
		s++;
	}

	return m * v;
}

