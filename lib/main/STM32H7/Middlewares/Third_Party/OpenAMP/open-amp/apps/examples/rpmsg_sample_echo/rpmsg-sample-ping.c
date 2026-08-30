/*
 * This is a sample demonstration application that showcases usage of rpmsg
 * This application is meant to run on the remote CPU running baremetal code.
 * This application simulate linux sample rpmsg driver. For this it echo 100
 * time message sent by the rpmsg sample client available in linux kernel
 * distribution.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "platform_info.h"

#define RPMSG_SERV_NAME "rpmsg-client-sample"

#define HELLO_MSG	"hello world!"
#define BYE_MSG		"goodbye!"
#define MSG_LIMIT	100

#define APP_EPT_ADDR    0
#define LPRINTF(format, ...) printf(format, ##__VA_ARGS__)
#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

static int err_cnt;


/* Globals */
static struct rpmsg_endpoint lept;
static int rnum = 0;
static int err_cnt = 0;
static int ept_deleted = 0;

/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
			     uint32_t src, void *priv)
{
	char payload[RPMSG_BUFFER_SIZE];
	char seed[20];

	(void)ept;
	(void)src;
	(void)priv;

	memset(payload, 0, RPMSG_BUFFER_SIZE);
	memcpy(payload, data, len);
	LPRINTF("received message %d: %s of size %lu \r\n",
		rnum + 1, payload, (unsigned long)len);

	if (rnum == (MSG_LIMIT - 1))
		sprintf (seed, "%s", BYE_MSG);
	else
		sprintf (seed, "%s", HELLO_MSG);

	LPRINTF(" seed %s: \r\n", seed);

	if (strncmp(payload, seed, len)) {
		LPERROR(" Invalid message is received.\n");
		err_cnt++;
		return RPMSG_SUCCESS;
	}
	LPRINTF(" rnum %d: \r\n", rnum);
	rnum++;

	return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	(void)ept;
	rpmsg_destroy_ept(&lept);
	LPRINTF("echo test: service is destroyed\n");
	ept_deleted = 1;
}

static void rpmsg_name_service_bind_cb(struct rpmsg_device *rdev,
				       const char *name, uint32_t dest)
{
	LPRINTF("new endpoint notification is received.\n");
	if (strcmp(name, RPMSG_SERV_NAME))
		LPERROR("Unexpected name service %s.\n", name);
	else
		(void)rpmsg_create_ept(&lept, rdev, RPMSG_SERV_NAME,
				       APP_EPT_ADDR, dest,
				       rpmsg_endpoint_cb,
				       rpmsg_service_unbind);

}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
int app(struct rpmsg_device *rdev, void *priv)
{
	int ret;
	int i;

	LPRINTF(" 1 - Send data to remote core, retrieve the echo");
	LPRINTF(" and validate its integrity ..\n");

	/* Create RPMsg endpoint */
	ret = rpmsg_create_ept(&lept, rdev, RPMSG_SERV_NAME, APP_EPT_ADDR,
			       RPMSG_ADDR_ANY,
			       rpmsg_endpoint_cb, rpmsg_service_unbind);

	if (ret) {
		LPERROR("Failed to create RPMsg endpoint.\n");
		return ret;
	}

	while (!is_rpmsg_ept_ready(&lept))
		platform_poll(priv);

	LPRINTF("RPMSG endpoint is binded with remote.\n");
	for (i = 1; i <= MSG_LIMIT; i++) {


		if (i < MSG_LIMIT)
			ret = rpmsg_send(&lept, HELLO_MSG, strlen(HELLO_MSG));
		else
			ret = rpmsg_send(&lept, BYE_MSG, strlen(BYE_MSG));

		if (ret < 0) {
			LPERROR("Failed to send data...\n");
			break;
		}
		LPRINTF("rpmsg sample test: message %d sent\n", i);

		do {
			platform_poll(priv);
		} while ((rnum < i) && !err_cnt);

	}

	LPRINTF("**********************************\n");
	LPRINTF(" Test Results: Error count = %d\n", err_cnt);
	LPRINTF("**********************************\n");
	while (!ept_deleted)
		platform_poll(priv);
	LPRINTF("Quitting application .. rpmsg sample test end\n");

	return 0;
}

int main(int argc, char *argv[])
{
	void *platform;
	struct rpmsg_device *rpdev;
	int ret;

	/* Initialize platform */
	ret = platform_init(argc, argv, &platform);
	if (ret) {
		LPERROR("Failed to initialize platform.\n");
		ret = -1;
	} else {
		rpdev = platform_create_rpmsg_vdev(platform, 0,
						  VIRTIO_DEV_MASTER,
						  NULL,
						  rpmsg_name_service_bind_cb);
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

