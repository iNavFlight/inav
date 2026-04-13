/* This is a test application to send rpmsgs in flood mode.
 * That is it will keep sending messages until there is no available
 * buffers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "platform_info.h"
#include "rpmsg-ping.h"

#define APP_EPT_ADDR    0
#define LPRINTF(format, ...) printf(format, ##__VA_ARGS__)
#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

struct _payload {
	unsigned long num;
	unsigned long size;
	unsigned char data[];
};

static int err_cnt;

#define PAYLOAD_MIN_SIZE 1
#define NUMS_PACKAGES 0x100000

/* Globals */
static struct rpmsg_endpoint lept;
static struct _payload *i_payload;
static int rnum = 0;
static int err_cnt = 0;
static int ept_deleted = 0;

/* External functions */
extern int init_system();
extern void cleanup_system();

/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
			     uint32_t src, void *priv)
{
	struct _payload *r_payload = (struct _payload *)data;

	(void)ept;
	(void)src;
	(void)priv;
	(void)len;

	if (r_payload->size == 0) {
		LPERROR(" Invalid size of package is received.\n");
		err_cnt++;
		return RPMSG_SUCCESS;
	}
	/* Validate data buffer integrity. */
	if (memcmp(r_payload->data, i_payload->data, r_payload->size) != 0) {
		LPRINTF("Data corruption %lu, size %lu\n",
			r_payload->num, r_payload->size);
		err_cnt++;
	}
	rnum = r_payload->num + 1;
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
	if (strcmp(name, RPMSG_SERVICE_NAME))
		LPERROR("Unexpected name service %s.\n", name);
	else
		(void)rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME,
				       APP_EPT_ADDR, dest,
				       rpmsg_endpoint_cb,
				       rpmsg_service_unbind);

}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
int app (struct rpmsg_device *rdev, void *priv)
{
	int ret;
	int i, s, max_size;
	int num_pkgs;

	LPRINTF(" 1 - Send data to remote core, retrieve the echo");
	LPRINTF(" and validate its integrity ..\n");

	num_pkgs = NUMS_PACKAGES;
	max_size = rpmsg_virtio_get_buffer_size(rdev);
	if (max_size < 0) {
		LPERROR("No avaiable buffer size.\n");
		return -1;
	}
	i_payload = (struct _payload *)metal_allocate_memory(max_size);

	if (!i_payload) {
		LPERROR("memory allocation failed.\n");
		return -1;
	}
	max_size -= sizeof(struct _payload);

	/* Create RPMsg endpoint */
	ret = rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME, APP_EPT_ADDR,
			       RPMSG_ADDR_ANY,
			       rpmsg_endpoint_cb, rpmsg_service_unbind);
	if (ret) {
		LPERROR("Failed to create RPMsg endpoint.\n");
		return ret;
	}

	while (!is_rpmsg_ept_ready(&lept))
		platform_poll(priv);
	LPRINTF("RPMSG endpoint is binded with remote.\n");

	memset(&(i_payload->data[0]), 0xA5, max_size);
	for (s = PAYLOAD_MIN_SIZE; s <= max_size; s++) {
		int size;

		i_payload->size = s;
		size = sizeof(struct _payload) + s;
		LPRINTF("echo test: package size %d, num of packages: %d\n",
			size, num_pkgs);
		rnum = 0;
		for (i = 0; i < num_pkgs; i++) {
			i_payload->num = i;
			while (!err_cnt && !ept_deleted) {
				ret = rpmsg_trysend(&lept, i_payload, size);
				if (ret == RPMSG_ERR_NO_BUFF) {
					platform_poll(priv);
				} else if (ret < 0) {
					LPERROR("Failed to send data...\n");
					break;
				} else {
					break;
				}
			}
			if (ret < 0 || err_cnt || ept_deleted)
				break;
		}
		if (ret < 0)
			break;
		while (rnum < num_pkgs && !err_cnt && !ept_deleted)
			platform_poll(priv);

		if (err_cnt || ept_deleted)
			break;
	}

	if (ept_deleted)
		LPRINTF("Remote RPMsg endpoint is destroyed unexpected.\r\n");

	LPRINTF("**********************************\n");
	LPRINTF(" Test Results: Error count = %d \n", err_cnt);
	LPRINTF("**********************************\n");
	/* Destroy the RPMsg endpoint */
	rpmsg_destroy_ept(&lept);
	LPRINTF("Quitting application .. Echo test end\n");

	metal_free_memory(i_payload);
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

