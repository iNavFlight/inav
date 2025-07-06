/*
 * Copyright (c) 2017, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include <sys/types.h>
#include <metal/irq.h>
#include <metal/atomic.h>
#include <metal/cpu.h>
#include <stdio.h>

#define BUS_NAME        "platform"
#define IPI_DEV_NAME    "ff340000.ipi"
#define SHM_DEV_NAME    "3ed80000.shm"
#define TTC_DEV_NAME    "ff110000.timer"

/* Apply this snippet to the device tree in an overlay so that
 * Linux userspace can see and use TTC0:
 *   &TTC0 {
 *         compatible = "ttc0_libmetal_demo";
 *         status = "okay";
 * };
 */


/* IPI registers offset */
#define IPI_TRIG_OFFSET 0x0  /* IPI trigger reg offset */
#define IPI_OBS_OFFSET  0x4  /* IPI observation reg offset */
#define IPI_ISR_OFFSET  0x10 /* IPI interrupt status reg offset */
#define IPI_IMR_OFFSET  0x14 /* IPI interrupt mask reg offset */
#define IPI_IER_OFFSET  0x18 /* IPI interrupt enable reg offset */
#define IPI_IDR_OFFSET  0x1C /* IPI interrup disable reg offset */

#define IPI_MASK        0x100 /* IPI mask for kick from RPU. */

/* TTC counter offsets */
#define XTTCPS_CLK_CNTRL_OFFSET 0x0  /* TTC counter clock control reg offset */
#define XTTCPS_CNT_CNTRL_OFFSET 0xC  /* TTC counter control reg offset */
#define XTTCPS_CNT_VAL_OFFSET   0x18 /* TTC counter val reg offset */
#define XTTCPS_CNT_OFFSET(ID) ((ID) == 1 ? 0 : 1 << (ID)) /* TTC counter offset
							     ID is from 1 to 3 */

/* TTC counter control masks */
#define XTTCPS_CNT_CNTRL_RST_MASK  0x10U /* TTC counter control reset mask */
#define XTTCPS_CNT_CNTRL_DIS_MASK  0x01U /* TTC counter control disable mask */

#define LPRINTF(format, ...) \
  printf("CLIENT> " format, ##__VA_ARGS__)

#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

/**
 * @brief shmem_demo() - Show use of shared memory with Libmetal.
 *        For NUM_TIMES times, send message to RPU and notify RPU by writing to
 *        share mem that RPU is polling. Once detected, RPU will then similarly
 *        write message and notify APU and the APU will then verify the
 *        response. If the message does not match expected response, record
 *        error. Afterwards, report test result and clean up.
 *        Notes:
 *        * The RPU will repeatedly wait for shared mem. from APU until APU
 *          notifies remote by changing the KEEP_GOING value in shared memory.
 *
 * @return - return 0 on success, otherwise return error number indicating
 *           type of error
 */
int shmem_demo();

/**
 * @brief ipi_shmem_demo() - shared memory IPI demo
 *        This task will:
 *        * Get the timestamp and put it into the ping shared memory
 *        * Update the shared memory descriptor for the new available
 *          ping buffer.
 *        * Trigger IPI to notifty the remote.
 *        * Repeat the above steps until it sends out all the packages.
 *        * Monitor IPI interrupt, verify every received package.
 *        * After all the packages are received, it sends out shutdown
 *          message to the remote.
 *
 * @return - return 0 on success, otherwise return error number indicating
 *           type of error.
 */
int ipi_shmem_demo();

/**
 * @brief atomic_shmem_demo() - Shared memory atomic operation demo
 *        This task will:
 *        - Write to shared memory to notify the remote to start atomic add
 *          on the shared memory descriptor memory for 1000 times.
 *        - Start atomic add by 1 for 1000 times to first 32 bits of memory
 *          in the shared memory location at 3ed00000 which is
 *          pointed to by shm_io.
 *        - Wait for the remote to write to shared memory
 *        - Once it received the polling kick from the remote, it will check
 *          if the value stored in the shared memory for the atomic add is
 *          2000.
 *        - It will print if the atomic add test has passed or not.
 *
 * @param[in] channel- hold shared mem. device
 * @return - If setup failed, return the corresponding error number. Otherwise
 *           return 0 on success.
 */
int atomic_shmem_demo();

/**
 * @brief ipi_latency_demo() - Show performance of  IPI with Libmetal.
 *        For NUM_TIMES times, repeatedly send an IPI from APU and then detect
 *        this IPI from RPU and measure the latency. Similarly, measure the
 *        latency from RPU to APU. Each iteration, record this latency and
 *        after the loop has finished, report the total latency in nanseconds.
 *        Notes:
 *        * The RPU will repeatedly wait for IPI from APU until APU notifies
 *          remote by changing the KEEPGOING value in shared memory.
 *        * To further ensure the accuracy of the readings a different thread
 *          (i.e. the IRQ handler) will stop the timer measuring RPU to APU
 *          latency.
 *
 * @return - 0 on success, error code if failure.
 */
int ipi_latency_demo();

/**
 * @brief shmem_latency_demo_demo() - Show performance of shared memory
 *        For 8, 512, and 1024 bytes, measure latency from block write to block
 *        read on remote side in shared memory. For each size, find average
 *        latency by running NUM_TIMES times and reporting the average latency
 *        for both APU block write to RPU block read as well as RPU block write
 *        to APU block read.
 *
 * @return - 0 on success, error code if failure.
 */
int shmem_latency_demo();

/**
 * @brief shmem_throughput_demo_demo() - Show performance of shared memory
 *        Record average throughput for APU block read, write, RPU block read
 *        and write for sizes 1/2KB, 1KB and 2KB. For each size, run 1000 times
 *        each operation and record average.
 *
 * @return - 0 on success, error code if failure.
 */
int shmem_throughput_demo();

/**
 * @breif wait_for_notified() - Loop until notified bit in channel is set.
 *
 * @param[in] notified - pointer to the notified variable
 */
static inline void  wait_for_notified(atomic_int *notified)
{
	unsigned int flags;

	do {

		flags = metal_irq_save_disable();
		if (!atomic_flag_test_and_set(notified)) {
			metal_irq_restore_enable(flags);
			break;
		}
		metal_cpu_yield();
		metal_irq_restore_enable(flags);
	} while(1);
}

/**
 * @breif dump_buffer() - print hex value of each byte in the buffer
 *
 * @param[in] buf - pointer to the buffer
 * @param[in] len - len of the buffer
 */
static inline void dump_buffer(void *buf, unsigned int len)
{
	unsigned int i;
	unsigned char *tmp = (unsigned char *)buf;

	for (i = 0; i < len; i++) {
		printf(" %02x", *(tmp++));
		if (!(i % 20))
			printf("\n");
	}
	printf("\n");
}

/**
 * @brief print_demo() - print demo string
 *
 * @param[in] name - demo name
 */
static inline void print_demo(char *name)
{
	LPRINTF("****** libmetal demo: %s ******\n", name);
}

#endif /* __COMMON_H__ */
