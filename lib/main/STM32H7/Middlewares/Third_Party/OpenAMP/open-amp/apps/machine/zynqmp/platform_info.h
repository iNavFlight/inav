/*
 * Copyright (c) 2016 Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* This file populates resource table for BM remote
 * for use by the Linux Master */

#ifndef PLATFORM_INFO_H_
#define PLATFORM_INFO_H_

#include <openamp/remoteproc.h>
#include <openamp/virtio.h>
#include <openamp/rpmsg.h>

#if defined __cplusplus
extern "C" {
#endif

struct remoteproc_priv {
	const char *ipi_name; /**< IPI device name */
	const char *ipi_bus_name; /**< IPI bus name */
	const char *rsc_name; /**< rsc device name */
	const char *rsc_bus_name; /**< rsc bus name */
	const char *shm_name; /**< shared memory device name */
	const char *shm_bus_name; /**< shared memory bus name */
	struct metal_device *ipi_dev; /**< pointer to IPI device */
	struct metal_io_region *ipi_io; /**< pointer to IPI i/o region */
	struct metal_device *shm_dev; /**< pointer to shared memory device */
	struct metal_io_region *shm_io; /**< pointer to shared memory i/o
						 region */
	struct remoteproc_mem shm_mem; /**< shared memory */
	unsigned int ipi_chn_mask; /**< IPI channel mask */
	atomic_int ipi_nokick;
};


/**
 * platform_init - initialize the platform
 *
 * It will initialize the platform.
 *
 * @argc: number of arguments
 * @argv: array of the input arguements
 * @platform: pointer to store the platform data pointer
 *
 * return 0 for success or negative value for failure
 */
int platform_init(int argc, char *argv[], void **platform);

/**
 * platform_create_rpmsg_vdev - create rpmsg vdev
 *
 * It will create rpmsg virtio device, and returns the rpmsg virtio
 * device pointer.
 *
 * @platform: pointer to the private data
 * @vdev_index: index of the virtio device, there can more than one vdev
 *              on the platform.
 * @role: virtio master or virtio slave of the vdev
 * @rst_cb: virtio device reset callback
 * @ns_bind_cb: rpmsg name service bind callback
 *
 * return pointer to the rpmsg virtio device
 */
struct rpmsg_device *
platform_create_rpmsg_vdev(void *platform, unsigned int vdev_index,
			   unsigned int role,
			   void (*rst_cb)(struct virtio_device *vdev),
			   rpmsg_ns_bind_cb ns_bind_cb);

/**
 * platform_poll - platform poll function
 *
 * @platform: pointer to the platform
 *
 * return negative value for errors, otherwise 0.
 */
int platform_poll(void *platform);

/**
 * platform_release_rpmsg_vdev - release rpmsg virtio device
 *
 * @rpdev: pointer to the rpmsg device
 */
void platform_release_rpmsg_vdev(struct rpmsg_device *rpdev);

/**
 * platform_cleanup - clean up the platform resource
 *
 * @platform: pointer to the platform
 */
void platform_cleanup(void *platform);

#if defined __cplusplus
}
#endif

#endif /* PLATFORM_INFO_H_ */

