/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2016 Xilinx, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**************************************************************************
 * FILE NAME
 *
 *       platform_info.c
 *
 * DESCRIPTION
 *
 *       This file implements APIs to get platform specific
 *       information for OpenAMP.
 *
 **************************************************************************/

#include <metal/alloc.h>
#include <metal/atomic.h>
#include <metal/io.h>
#include <metal/irq.h>
#include <metal/shmem.h>
#include <metal/utilities.h>
#include <openamp/remoteproc.h>
#include <openamp/rpmsg_virtio.h>
#include <poll.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include "rsc_table.h"

#define IPI_CHAN_NUMS 2
#define IPI_CHAN_SEND 0
#define IPI_CHAN_RECV 1
#define UNIX_PREFIX "unix:"
#define UNIXS_PREFIX "unixs:"

#define RSC_MEM_PA  0x0UL
#define SHARED_BUF_PA   0x10000UL
#define SHARED_BUF_SIZE 0x40000UL

#define _rproc_wait() metal_cpu_yield()

struct vring_ipi_info {
	/* Socket file path */
	const char *path;
	int fd;
	atomic_int sync;
};

struct remoteproc_priv {
	const char *shm_file;
	int shm_size;
	struct metal_io_region *shm_old_io;
	struct metal_io_region shm_new_io;
	struct remoteproc_mem shm;
	struct vring_ipi_info ipi;
};

static struct remoteproc_priv rproc_priv_table [] = {
	{
		.shm_file = "openamp.shm",
		.shm_size = 0x80000,
		.ipi = {
			.path = "unixs:/tmp/openamp.event.0",
		},
	},
	{
		.shm_file = "openamp.shm",
		.shm_size = 0x80000,
		.ipi = {
			.path = "unix:/tmp/openamp.event.0",
		},
	},
};

static struct remoteproc rproc_inst;

/* External functions */
extern int init_system(void);
extern void cleanup_system(void);

static int linux_proc_block_read(struct metal_io_region *io,
				 unsigned long offset,
				 void *restrict dst,
				 memory_order order,
				 int len)
{
	void *src = metal_io_virt(io, offset);

	(void)order;
	(void)memcpy(dst, src, len);
	return len;
}

static int linux_proc_block_write(struct metal_io_region *io,
				  unsigned long offset,
				  const void *restrict src,
				  memory_order order,
				  int len)
{
	void *dst = metal_io_virt(io, offset);

	(void)order;
	(void)memcpy(dst, src, len);
	return len;
}

static void linux_proc_block_set(struct metal_io_region *io,
				unsigned long offset,
				unsigned char value,
				memory_order order,
				int len)
{
	void *dst = metal_io_virt(io, offset);

	(void)order;
	(void)memset(dst, value, len);
	return;
}

static struct metal_io_ops linux_proc_io_ops = {
	.write = NULL,
	.read = NULL,
	.block_read = linux_proc_block_read,
	.block_write = linux_proc_block_write,
	.block_set = linux_proc_block_set,
	.close = NULL,
};

static int sk_unix_client(const char *descr)
{
	struct sockaddr_un addr;
	int fd;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);

	memset(&addr, 0, sizeof addr);
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, descr + strlen(UNIX_PREFIX),
		sizeof addr.sun_path);
	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) >= 0) {
		printf("connected to %s\n", descr + strlen(UNIX_PREFIX));
		return fd;
	}

	close(fd);
	return -1;
}

static int sk_unix_server(const char *descr)
{
	struct sockaddr_un addr;
	int fd, nfd;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);

	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, descr + strlen(UNIXS_PREFIX),
		sizeof addr.sun_path);
	unlink(addr.sun_path);
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		goto fail;
	}

	listen(fd, 5);
	printf("Waiting for connection on %s\n", addr.sun_path);
	nfd = accept(fd, NULL, NULL);
	close(fd);
	return nfd;
fail:
	close(fd);
	return -1;
}

static inline int is_sk_unix_server(const char *descr)
{
	if (memcmp(UNIXS_PREFIX, descr, strlen(UNIXS_PREFIX)))
		return 0;
	else
		return 1;
}

static int event_open(const char *descr)
{
	int fd = -1;
	int i;

	if (descr == NULL) {
		return fd;
	}

	if (!is_sk_unix_server(descr)) {
		/* UNIX client.  Retry to connect a few times to give the peer
		 *  a chance to setup.  */
		for (i = 0; i < 100 && fd == -1; i++) {
			fd = sk_unix_client(descr);
			if (fd == -1)
				usleep(i * 10 * 1000);
		}
	} else {
		/* UNIX server. */
		fd = sk_unix_server(descr);
	}
	printf("Open IPI: %s\n", descr);
	return fd;
}

static int linux_proc_irq_handler(int vect_id, void *data)
{
	char dummy_buf[32];
	struct vring_ipi_info *ipi = data;

	read(vect_id, dummy_buf, sizeof(dummy_buf));
	atomic_flag_clear(&ipi->sync);
	return 0;
}

static struct remoteproc *
linux_proc_init(struct remoteproc *rproc,
		struct remoteproc_ops *ops, void *arg)
{
	struct remoteproc_priv *prproc = arg;
	struct metal_io_region *io;
	struct remoteproc_mem *shm;
	struct vring_ipi_info *ipi;
	int ret;

	if (!rproc || !prproc)
		return NULL;
	rproc->priv = prproc;
	/* Create shared memory io */
	ret = metal_shmem_open(prproc->shm_file, prproc->shm_size, &io);
	if (ret) {
		printf("Failed to init rproc, failed to open shm %s.\n",
		       prproc->shm_file);
		return NULL;
	}
	prproc->shm_old_io = io;
	shm = &prproc->shm;
	shm->pa = 0;
	shm->da = 0;
	shm->size = prproc->shm_size;
	metal_io_init(&prproc->shm_new_io, io->virt, &shm->pa,
		      shm->size, -1, 0, &linux_proc_io_ops);
	shm->io = &prproc->shm_new_io;

	/* Open IPI */
	ipi = &prproc->ipi;
	if (!ipi->path) {
		fprintf(stderr,
			"ERROR: No IPI sock path specified.\n");
		goto err;
	}
	ipi->fd = event_open(ipi->path);
	if (ipi->fd < 0) {
		fprintf(stderr,
			"ERROR: Failed to open sock %s for IPI.\n",
			ipi->path);
		goto err;
	}
	metal_irq_register(ipi->fd, linux_proc_irq_handler, NULL, ipi);
	rproc->ops = ops;
	return rproc;

err:
	return NULL;
}


static void linux_proc_remove(struct remoteproc *rproc)
{
	struct remoteproc_priv *prproc;
	struct vring_ipi_info *ipi;
	struct metal_io_region *io;

	if (!rproc)
		return;
	prproc = rproc->priv;

	/* Close IPI */
	ipi = &prproc->ipi;
	if (ipi->fd >= 0) {
		metal_irq_unregister(ipi->fd, 0, NULL, ipi);
		close(ipi->fd);
	}

	/* Close shared memory */
	io = prproc->shm_old_io;
	if (io && io->ops.close) {
		io->ops.close(io);
		prproc->shm_old_io = NULL;
	}
}

static void *
linux_proc_mmap(struct remoteproc *rproc, metal_phys_addr_t *pa,
		metal_phys_addr_t *da, size_t size,
		unsigned int attribute, struct metal_io_region **io)
{
	struct remoteproc_mem *mem;
	struct remoteproc_priv *prproc;
	metal_phys_addr_t lpa, lda;
	void *va;

	(void)attribute;
	(void)size;
	lpa = *pa;
	lda = *da;

	if (lpa == METAL_BAD_PHYS && lda == METAL_BAD_PHYS)
		return NULL;
	if (lpa == METAL_BAD_PHYS)
		lpa = lda;
	if (lda == METAL_BAD_PHYS)
		lda = lpa;

	if (!rproc)
		return NULL;
	prproc = rproc->priv;
	mem = &prproc->shm;
	va = metal_io_phys_to_virt(mem->io, lpa);
	if (va) {
		if (io)
			*io = mem->io;
		metal_list_add_tail(&rproc->mems, &mem->node);
	}
	return va;
}

static int linux_proc_notify(struct remoteproc *rproc, uint32_t id)
{
	struct remoteproc_priv *prproc;
	struct vring_ipi_info *ipi;
	char dummy = 1;

	(void)id;
	if (!rproc)
		return -1;
	prproc = rproc->priv;
	ipi = &prproc->ipi;
	send(ipi->fd, &dummy, 1, MSG_NOSIGNAL);
	return 0;
}

/* processor operations from r5 to a53. It defines
 * notification operation and remote processor managementi operations. */
static struct remoteproc_ops linux_proc_ops = {
	.init = linux_proc_init,
	.remove = linux_proc_remove,
	.mmap = linux_proc_mmap,
	.notify = linux_proc_notify,
	.start = NULL,
	.stop = NULL,
	.shutdown = NULL,
};

/* RPMsg virtio shared buffer pool */
static struct rpmsg_virtio_shm_pool shpool;

static int platform_slave_setup_resource_table(const char *shm_file,
					       int shm_size,
					       void *rsc_table, int rsc_size,
					       metal_phys_addr_t rsc_pa)
{
	struct metal_io_region *io;
	void *rsc_shm;
	int ret;

	ret = metal_shmem_open(shm_file, shm_size, &io);
	if (ret) {
		printf("Failed to init rproc, failed to open shm %s.\n",
		       shm_file);
		return -1;
	}
	rsc_shm = metal_io_virt(io, rsc_pa);
	memcpy(rsc_shm, rsc_table, rsc_size);
	io->ops.close(io);
	free(io);
	return 0;
}

static struct remoteproc *
platform_create_proc(int proc_index, int rsc_index)
{
	struct remoteproc_priv *prproc;
	void *rsc_table, *rsc_table_shm;
	int rsc_size;
	int ret;
	metal_phys_addr_t pa;

	(void)proc_index;
	rsc_table = get_resource_table(rsc_index, &rsc_size);

	prproc = &rproc_priv_table[proc_index];
	/* Setup resource table
	 * This step can be done out of the application.
	 * Assumes the unix server side setup resource table. */
	if (is_sk_unix_server(prproc->ipi.path)) {
		ret = platform_slave_setup_resource_table(prproc->shm_file,
							  prproc->shm_size,
							  rsc_table, rsc_size,
							  RSC_MEM_PA);
		if (ret) {
			printf("Failed to initialize resource table\n");
			return NULL;
		}
	}

	/* Initialize remoteproc instance */
	if (!remoteproc_init(&rproc_inst, &linux_proc_ops, prproc))
		return NULL;

	/* Mmap resource table */
	pa = RSC_MEM_PA;
	rsc_table_shm = remoteproc_mmap(&rproc_inst, &pa, NULL, rsc_size,
					0, &rproc_inst.rsc_io);

	/* parse resource table to remoteproc */
	ret = remoteproc_set_rsc_table(&rproc_inst, rsc_table_shm, rsc_size);
	if (ret) {
		printf("Failed to set resource table to remoteproc\r\n");
		remoteproc_remove(&rproc_inst);
		return NULL;
	}
	printf("Initialize remoteproc successfully.\r\n");
	return &rproc_inst;
}

int platform_init(int argc, char *argv[], void **platform)
{
	unsigned long proc_id = 0;
	unsigned long rsc_id = 0;
	struct remoteproc *rproc;

	if (!platform) {
		fprintf(stderr, "Failed to initialize platform, NULL pointer"
			"to store platform data.\n");
		return -EINVAL;
	}
	/* Initialize HW system components */
	init_system();

	if (argc >= 2) {
		proc_id = strtoul(argv[1], NULL, 0);
	}

	if (argc >= 3) {
		rsc_id = strtoul(argv[2], NULL, 0);
	}

	rproc = platform_create_proc(proc_id, rsc_id);
	if (!rproc) {
		fprintf(stderr, "Failed to create remoteproc device.\n");
		return -EINVAL;
	}
	*platform = rproc;
	return 0;
}

struct  rpmsg_device *
platform_create_rpmsg_vdev(void *platform, unsigned int vdev_index,
			   unsigned int role,
			   void (*rst_cb)(struct virtio_device *vdev),
			   rpmsg_ns_bind_cb ns_bind_cb)
{
	struct remoteproc *rproc = platform;
	struct rpmsg_virtio_device *rpmsg_vdev;
	struct virtio_device *vdev;
	void *shbuf;
	struct metal_io_region *shbuf_io;
	int ret;

	/* Setup resource table */
	rpmsg_vdev = metal_allocate_memory(sizeof(*rpmsg_vdev));
	if (!rpmsg_vdev)
		return NULL;
	shbuf_io = remoteproc_get_io_with_pa(rproc, SHARED_BUF_PA);
	if (!shbuf_io)
		return NULL;
	shbuf = metal_io_phys_to_virt(shbuf_io, SHARED_BUF_PA);

	printf("creating remoteproc virtio\r\n");
	/* TODO: can we have a wrapper for the following two functions? */
	vdev = remoteproc_create_virtio(rproc, vdev_index, role, rst_cb);
	if (!vdev) {
		printf("failed remoteproc_create_virtio\r\n");
		goto err1;
	}

	printf("initializing rpmsg shared buffer pool\r\n");
	/* Only RPMsg virtio master needs to initialize the shared buffers pool */
	rpmsg_virtio_init_shm_pool(&shpool, shbuf, SHARED_BUF_SIZE);

	printf("initializing rpmsg vdev\r\n");
	/* RPMsg virtio slave can set shared buffers pool argument to NULL */
	ret =  rpmsg_init_vdev(rpmsg_vdev, vdev, ns_bind_cb,
			       shbuf_io,
			       &shpool);
	if (ret) {
		printf("failed rpmsg_init_vdev\r\n");
		goto err2;
	}
	return rpmsg_virtio_get_rpmsg_device(rpmsg_vdev);
err2:
	remoteproc_remove_virtio(rproc, vdev);
err1:
	metal_free_memory(rpmsg_vdev);
	return NULL;
}

int platform_poll(void *priv)
{
	struct remoteproc *rproc = priv;
	struct remoteproc_priv *prproc;
	struct vring_ipi_info *ipi;
	unsigned int flags;

	prproc = rproc->priv;
	ipi = &prproc->ipi;
	while(1) {
		flags = metal_irq_save_disable();
		if (!(atomic_flag_test_and_set(&ipi->sync))) {
			metal_irq_restore_enable(flags);
			remoteproc_get_notification(rproc, RSC_NOTIFY_ID_ANY);
			break;
		}
		_rproc_wait();
		metal_irq_restore_enable(flags);
	}
	return 0;
}

void platform_release_rpmsg_vdev(struct rpmsg_device *rpdev)
{
	(void)rpdev;
}

void platform_cleanup(void *platform)
{
	struct remoteproc *rproc = platform;

	if (rproc)
		remoteproc_remove(rproc);
	cleanup_system();
}
