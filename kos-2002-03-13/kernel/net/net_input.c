/* KallistiOS ##version##

   kernel/net/net_input.c
   (c)2002 Dan Potter
*/

#include <stdio.h>
#include <kos/net.h>
#include "net_icmp.h"

CVSID("net_input.c,v 1.1 2002/02/10 20:31:43 bardtx Exp");

/*

  Main packet input system
 
*/

int net_input(netif_t *device, const uint8 *data, int len) {
	net_icmp_input(device, data, len);
	return 0;
}

