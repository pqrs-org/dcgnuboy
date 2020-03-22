/* KallistiOS ##version##

   maple_enum.c
   (c)2002 Dan Potter
 */

#include <dc/maple.h>

CVSID("maple_enum.c,v 1.1 2002/02/22 07:34:20 bardtx Exp");

/* Return the number of connected devices */
int maple_enum_count() {
	int p, u, cnt;

	for (cnt=0, p=0; p<MAPLE_PORT_COUNT; p++)
		for (u=0; u<MAPLE_UNIT_COUNT; u++) {
			if (maple_state.ports[p].units[u].valid)
				cnt++;
		}

	return cnt;
}

/* Return a raw device info struct for the given device */
maple_device_t * maple_enum_dev(int p, int u) {
	if (maple_dev_valid(p, u))
		return &maple_state.ports[p].units[u];
	else
		return NULL;
}

