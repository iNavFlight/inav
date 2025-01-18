/*
 * Copyright (c) 2017, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

 /*****************************************************************************
  * shmem_demo.c
  * This demo demonstrates the use of shared mem. between the APU and RPU.
  * This demo does so via the following steps:
  *
  *  1. Open the shared memory device.
  *  2. Clear the demo control TX/RX available values in shared memory.
  *  3. APU set demo control in shared memory to notify RPU demo has started
  *  4. APU will write message to the shared memory.
  *  5. APU will increase TX avail values in the shared memory to notify RPU
  *     there is a message ready to read.
  *  6. APU will poll the RX avail value in th shared memory to see if RPU
  *     has echoed back the message into the shared memory.
  *  7. When APU knows there is new RX message available, it will read the
  *     RX message from the shared memory.
  *  8. APU will verify the message to see if it matches the one it has sent.
  *  9. Close the shared memory device.
  *
  * Here is the Shared memory structure of this demo:
  * |0    | 4Bytes | DEMO control status shows if demo starts or not |
  * |0x04 | 4Bytes | number of APU to RPU buffers available to RPU |
  * |0x08 | 4Bytes | number of APU to RPU buffers consumed by RPU |
  * |0x0c | 4Bytes | number of RPU to APU buffers available to APU |
  * |0x10 | 4Bytes | number of RPU to APU buffers consumed by APU |
  * |0x14 | 1KBytes | APU to RPU buffer |
  * ... ...
  * |0x800 | 1KBytes | RPU to APU buffer |
  */

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <metal/errno.h>
#include <string.h>
#include <metal/io.h>
#include <metal/alloc.h>
#include <metal/device.h>
#include "common.h"

/* Shared memory offsets */
#define SHM_DEMO_CNTRL_OFFSET      0x0
#define SHM_TX_AVAIL_OFFSET        0x04
#define SHM_RX_AVAIL_OFFSET        0x0C
#define SHM_TX_BUFFER_OFFSET       0x14
#define SHM_RX_BUFFER_OFFSET       0x800

#define SHM_BUFFER_SIZE          0x400

#define DEMO_STATUS_IDLE         0x0
#define DEMO_STATUS_START        0x1 /* Status value to indicate demo start */

#define TEST_MSG "Hello World - libmetal shared memory demo"

struct msg_hdr_s {
	uint32_t index;
	uint32_t len;
};

/**
 * @brief shmem_task() - Show use of shared memory with Libmetal.
 *        Write message to RPU. RPU will then read and echo
 *        back. Confirm if echoed message is identical.
 *        If messages differ, report error.
 *
 *        Steps:
 *        1. Clear demo control and TX/RX avaiable values
 *
 * @param[in] shm_io - metal i/o region of the shared memory
 * @return - return 0 on success, otherwise return error number indicating
 *           type of error
 */
int shmem_echo(struct metal_io_region *shm_io)
{
	void *tx_data = NULL;
	void *rx_data = NULL;
	unsigned int tx_count = 0;
	unsigned int rx_count = 0;
	struct msg_hdr_s *msg_hdr;
	unsigned int data_len;
	int ret;

	LPRINTF("Setting up shared memory demo.\n");
	/* clear demo status value */
	metal_io_write32(shm_io, SHM_DEMO_CNTRL_OFFSET, 0);
	/* Clear TX/RX avail */
	metal_io_write32(shm_io, SHM_TX_AVAIL_OFFSET, 0);
	metal_io_write32(shm_io, SHM_RX_AVAIL_OFFSET, 0);

	LPRINTF("Starting shared memory demo.\n");
	/* Notify the remote the demo starts */
	metal_io_write32(shm_io, SHM_DEMO_CNTRL_OFFSET, DEMO_STATUS_START);

	/* preparing data to send */
	data_len = sizeof(struct msg_hdr_s) + strlen(TEST_MSG) + 1;
	tx_data = metal_allocate_memory(data_len);
	if (!tx_data) {
		LPERROR("Failed to allocate memory.\n");
		ret = -1;
		goto out;
	}
	msg_hdr = (struct msg_hdr_s *)tx_data;
	msg_hdr->index = tx_count;
	msg_hdr->len = strlen(TEST_MSG) + 1;
	sprintf(tx_data + sizeof(*msg_hdr), TEST_MSG);
	LPRINTF("Sending message: %s\n",
		(char *)(tx_data + sizeof(*msg_hdr)));

	/* write data to the shared memory*/
	ret = metal_io_block_write(shm_io, SHM_TX_BUFFER_OFFSET,
		tx_data, data_len);
	if (ret < 0){
		LPERROR("Unable to metal_io_block_write()\n");
		goto out;
	}
	/* Increase number of buffers available to notify the remote */
	tx_count++;
	metal_io_write32(shm_io, SHM_TX_AVAIL_OFFSET, tx_count);

	/* wait for remote to echo back the data */
	while (metal_io_read32(shm_io, SHM_RX_AVAIL_OFFSET) == rx_count);
	rx_count++;
	/* New RX data is available, allocate buffer to received data */
	rx_data = metal_allocate_memory(data_len);
	if (!rx_data) {
		LPERROR("Failed to allocate memory\n");
		ret = -1;
		goto out;
	}
	/* read data from the shared memory*/
	metal_io_block_read(shm_io, SHM_RX_BUFFER_OFFSET,
		 rx_data, data_len);
	if (ret < 0){
		LPERROR("Unable to metal_io_block_read()\n");
		goto out;
	}
	/* verify the received data */
	ret = memcmp(tx_data, rx_data, data_len);
	if (ret) {
		LPERROR("Received data verification failed.\n");
		LPRINTF("Expected:");
		dump_buffer(tx_data, data_len);
		LPRINTF("Actual:");
		dump_buffer(rx_data, data_len);
	} else {
		LPRINTF("Message Received: %s\n",
			(char *)(rx_data + sizeof(*msg_hdr)));
	}
	/* Notify the remote the demo has finished. */
	metal_io_write32(shm_io, SHM_DEMO_CNTRL_OFFSET, DEMO_STATUS_IDLE);

out:
	if (tx_data)
		metal_free_memory(tx_data);
	if (rx_data)
		metal_free_memory(rx_data);
	LPRINTF("Shared memory demo: %s.\n", ret ? "Failed": "Passed" );
	return ret;
}

int shmem_demo()
{
	struct metal_device *device = NULL;
	struct metal_io_region *io = NULL;
	int ret = 0;

	print_demo("shared memory");

	/* Open the shared memory device */
	ret = metal_device_open(BUS_NAME, SHM_DEV_NAME, &device);
	if (ret) {
		LPERROR("Failed to open device %s.\n", SHM_DEV_NAME);
		return ret;
	}

	/* get shared memory device IO region */
	io = metal_device_io_region(device, 0);
	if (!io) {
		LPERROR("Failed to get io region for %s.\n", device->name);
		ret = -ENODEV;
		goto out;
	}

	/* Run the demo */
	ret = shmem_echo(io);

out:
	if (device)
		metal_device_close(device);
	return ret;
}
