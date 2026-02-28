/*
 * Copyright (c) 2017, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 /*****************************************************************************
  * shmem_demod.c
  * This demo demonstrates the use of shared mem. between the APU and RPU.
  * This demo does so via the following steps:
  *
  *  1. Get the shared memory device I/O region.
  *  2. Clear the demo control value in shared memory.
  *  3. Check the demo control value in the shared memory to wait for APU
  *     to start the demo.
  *  4. Once the demo control value indicates the demo starts, it polls on
  *     RX available value to see if there is new RX message available.
  *  5. If there is a new RX message available, it reads the message from
  *     the shared memory
  *  6. It echos back the message to the shared memory
  *  7. It increases the TX available value in the shared memory to notify
  *     the other end there is a message available to read.
  *  8. Check if the demo control value and the RX available values to see
  *     if demo finishes and if there is new RX data available.
  *
  * Here is the Shared memory structure of this demo:
  * |0      | 4Bytes | DEMO control status shows if demo starts or not |
  * |0x04 | 4Bytes | number of APU to RPU buffers available to RPU |
  * |0x08 | 4Bytes | number of APU to RPU buffers consumed by RPU |
  * |0x0c | 4Bytes | number of RPU to APU buffers available to APU |
  * |0x10 | 4Bytes | number of RPU to APU buffers consumed by APU |
  * |0x14 | 1KBytes | APU to RPU buffer |
  * ... ...
  * |0x800 | 1KBytes | RPU to APU buffer |
  */

#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <metal/sys.h>
#include <metal/device.h>
#include <metal/io.h>
#include <metal/alloc.h>
#include "common.h"

/* Shared memory offsets */
#define SHM_DEMO_CNTRL_OFFSET       0x0
#define SHM_RX_AVAIL_OFFSET        0x04
#define SHM_RX_USED_OFFSET         0x08
#define SHM_TX_AVAIL_OFFSET        0x0C
#define SHM_TX_USED_OFFSET         0x10
#define SHM_RX_BUFFER_OFFSET       0x14
#define SHM_TX_BUFFER_OFFSET       0x800

#define SHM_BUFFER_SIZE          0x400

#define DEMO_STATUS_IDLE         0x0
#define DEMO_STATUS_START        0x1 /* Status value to indicate demo start */

struct msg_hdr_s {
	uint32_t index;
	uint32_t len;
};

/**
 * @brief shmem_echod() - Show use of shared memory with libmetal.
 *        Wait for message from APU. Once received, read and echo it back.
 *
 * @param[in] shm_io - metal i/o region of the shared memory
 * @return - return 0 on success, otherwise return error number indicating
 *           type of error
 */
static int shmem_echod(struct metal_io_region *shm_io)
{
	void *data = NULL;
	struct msg_hdr_s *msg_hdr;
	unsigned int rx_count = 0;
	unsigned int len;
	int ret = 0;

	/* clear demo status value */
	metal_io_write32(shm_io, SHM_DEMO_CNTRL_OFFSET, 0);

	/* allocate memory for receiving data */
	data = metal_allocate_memory(SHM_BUFFER_SIZE);
	if (!data) {
		LPERROR("Failed to allocate memory.\r\n");
		return -1;
	}

	LPRINTF("Wait for shared memory demo to start.\r\n");
	while (metal_io_read32(shm_io, SHM_DEMO_CNTRL_OFFSET) !=
		DEMO_STATUS_START);

	LPRINTF("Demo has started.\r\n");
	/* wait for message is available */
	while(metal_io_read32(shm_io, SHM_DEMO_CNTRL_OFFSET) ==
		DEMO_STATUS_START) {
		if (metal_io_read32(shm_io, SHM_RX_AVAIL_OFFSET)
			== rx_count)
			continue;
		/* Message is available, read the message header */
		ret = metal_io_block_read(shm_io, SHM_RX_BUFFER_OFFSET,
					data, sizeof(struct msg_hdr_s));
		if (ret < 0){
			LPERROR("Unable to metal_io_block_read()\n");
			return ret;
		}
		msg_hdr = (struct msg_hdr_s *)data;
		/* Get the length of the data, if the data length is
		 * too large, truncate it. */
		len = msg_hdr->len;
		if (msg_hdr->len >
			(SHM_BUFFER_SIZE - sizeof(*msg_hdr))) {
			LPERROR("Input message is too long %u.\n",
				msg_hdr->len);
			len = SHM_BUFFER_SIZE - sizeof(*msg_hdr);
		}
		/* Read the message data */
		ret = metal_io_block_read(shm_io,
				SHM_RX_BUFFER_OFFSET + sizeof(*msg_hdr),
				data + sizeof(*msg_hdr), len);

		rx_count++;
		ret = metal_io_block_write(shm_io,
			SHM_TX_BUFFER_OFFSET,
			(void*)data, sizeof(*msg_hdr) + len);
		if (ret < 0){
			LPERROR("Unable to metal_io_block_write()\n");
			return ret;
		}

		/* increase TX available value to notify the other end
		 * there is data ready to read. */
		metal_io_write32(shm_io, SHM_TX_AVAIL_OFFSET, rx_count);
	}

	metal_free_memory(data);
	LPRINTF("Shared memory test finished\r\n");
	return 0;
}

int shmem_demod()
{
	struct metal_io_region *io = NULL;
	int ret = 0;

	print_demo("shared memory");

	/* Get shared memory device IO region */
	if (!shm_dev) {
		ret = -ENODEV;
		goto out;
	}
	io = metal_device_io_region(shm_dev, 0);
	if (!io) {
		LPERROR("Failed to map io region for %s.\n", shm_dev->name);
		ret = -ENODEV;
		goto out;
	}

	ret = shmem_echod(io);

out:
	return ret;

}
