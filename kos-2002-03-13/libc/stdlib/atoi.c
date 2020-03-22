/* KallistiOS ##version##

   atoi.c
   (c)2001 Vincent Penne

   atoi.c,v 1.1 2002/02/09 06:15:43 bardtx Exp
*/

int atoi(const char * s) {
	int m, v;

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

