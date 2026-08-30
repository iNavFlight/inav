/* This is a sample demonstration application that showcases usage of rpmsg
This application is meant to run on the remote CPU running baremetal code.
This application echoes back data that was sent to it by the master core. */

#include <stdio.h>
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "platform_info.h"
#include "rpmsg-ping.h"

#define SHUTDOWN_MSG	0xEF56A55A

#define LPRINTF(format, ...) printf(format, ##__VA_ARGS__)
//#define LPRINTF(format, ...)
#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

static struct rpmsg_endpoint lept;
static int shutdown_req = 0;

/* External functions */
extern int init_system(void);
extern void cleanup_system(void);

/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
			     uint32_t src, void *priv)
{
	(void)priv;
	(void)src;

	/* On reception of a shutdown we signal the application to terminate */
	if ((*(unsigned int *)data) == SHUTDOWN_MSG) {
		LPRINTF("shutdown message is received.\n");
		shutdown_req = 1;
		return RPMSG_SUCCESS;
	}

	/* Send data back to master */
	while (1) {
		int ret;

		ret = rpmsg_send(ept, data, len);
		if (ret == RPMSG_ERR_NO_BUFF) {
			LPRINTF("%s, wait for buffer\n", __func__);
			continue;
		} else {
			if (ret < 0)
				LPERROR("rpmsg_send, size %lu failed %d\r\n",
					(unsigned long)len, ret);
			break;
		}
	}
	return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	(void)ept;
	LPRINTF("unexpected Remote endpoint destroy\n");
	shutdown_req = 1;
}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
int app(struct rpmsg_device *rdev, void *priv)
{
	int ret;

	/* Initialize RPMSG framework */
	LPRINTF("Try to create rpmsg endpoint.\n");

	ret = rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME,
			       0, RPMSG_ADDR_ANY, rpmsg_endpoint_cb,
			       rpmsg_service_unbind);
	if (ret) {
		LPERROR("Failed to create endpoint.\n");
		return -1;
	}

	LPRINTF("Successfully created rpmsg endpoint.\n");
	while(1) {
		platform_poll(priv);
		/* we got a shutdown request, exit */
		if (shutdown_req) {
			break;
		}
	}
	rpmsg_destroy_ept(&lept);

	return 0;
}

/*-----------------------------------------------------------------------------*
 *  Application entry point
 *-----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	void *platform;
	struct rpmsg_device *rpdev;
	int ret;

	LPRINTF("Starting application...\n");

	/* Initialize platform */
	ret = platform_init(argc, argv, &platform);
	if (ret) {
		LPERROR("Failed to initialize platform.\n");
		ret = -1;
	} else {
		rpdev = platform_create_rpmsg_vdev(platform, 0,
						   VIRTIO_DEV_SLAVE,
						   NULL, NULL);
		if (!rpdev) {
			LPERROR("Failed to create rpmsg virtio device.\n");
			ret = -1;
		} else {
			app(rpdev, platform);
			platform_release_rpmsg_vdev(rpdev);
			ret = 0;
		}
	}

	LPRINTF("Stopping application...\n");
	platform_cleanup(platform);

	return ret;
}
