/* KallistiOS ##version##

   controller.c
   (C)2002 Dan Potter

 */

#include <dc/maple.h>
#include <dc/maple/controller.h>
#include <string.h>

CVSID("controller.c,v 1.1 2002/02/22 07:34:20 bardtx Exp");


static void cont_reply(maple_frame_t *frm) {
	maple_response_t	*resp;
	uint32			*respbuf;

	/* Unlock the frame now (it's ok, we're in an IRQ) */
	maple_frame_unlock(frm);

	/* Make sure we got a valid response */
	resp = (maple_response_t *)frm->recv_buf;
	if (resp->response != MAPLE_RESPONSE_DATATRF)
		return;
	respbuf = (uint32 *)resp->data;
	if (respbuf[0] != MAPLE_FUNC_CONTROLLER)
		return;

	/* Update the status area from the response */
	if (frm->dev) {
		memcpy(frm->dev->status, respbuf+1, (resp->data_len-1) * 4);
		frm->dev->status_valid = 1;
	}
}

static int cont_poll(maple_device_t *dev) {
	uint32 * send_buf;

	if (maple_frame_lock(&dev->frame) < 0)
		return 0;

	maple_frame_init(&dev->frame);
	send_buf = (uint32 *)dev->frame.recv_buf;
	send_buf[0] = MAPLE_FUNC_CONTROLLER;
	dev->frame.cmd = MAPLE_COMMAND_GETCOND;
	dev->frame.dst_port = dev->port;
	dev->frame.dst_unit = dev->unit;
	dev->frame.length = 1;
	dev->frame.callback = cont_reply;
	dev->frame.send_buf = send_buf;
	maple_queue_frame(&dev->frame);

	return 0;
}

static void cont_periodic(maple_driver_t *drv) {
	maple_driver_foreach(drv, cont_poll);
}

static int cont_attach(maple_driver_t *drv, maple_device_t *dev) {
	memset(dev->status, 0, sizeof(dev->status));
	dev->status_valid = 0;
	return 0;
}

static void cont_detach(maple_driver_t *drv, maple_device_t *dev) {
	memset(dev->status, 0, sizeof(dev->status));
	dev->status_valid = 0;
}

/* Device Driver Struct */
static maple_driver_t controller_drv = {
	functions:	MAPLE_FUNC_CONTROLLER,
	name:		"Controller Driver",
	periodic:	cont_periodic,
	attach:		cont_attach,
	detach:		cont_detach
};

/* Add the controller to the driver chain */
int cont_init() {
	return maple_driver_reg(&controller_drv);
}

void cont_shutdown() {
	maple_driver_unreg(&controller_drv);
}

