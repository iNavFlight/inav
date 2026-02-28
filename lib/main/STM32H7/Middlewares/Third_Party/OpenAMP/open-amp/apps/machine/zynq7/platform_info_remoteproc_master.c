/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
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

#include <string.h>
#include <openamp/hil.h>
#include <openamp/firmware.h>

/* Reference implementation that show cases platform_get_cpu_info and 
 platform_get_for_firmware API implementation for Bare metal environment */

extern struct hil_platform_ops zynq_a9_proc_ops;

/* IPC Device parameters */
#define SHM_ADDR                          (void *)0x08008000
#define SHM_SIZE                          0x00200000
#define VRING0_IPI_VECT                   6
#define VRING1_IPI_VECT                   3
#define MASTER_CPU_ID                     0
#define REMOTE_CPU_ID                     1

/**
 * This array provdes defnition of CPU nodes for master and remote
 * context. It contains two nodes beacuse the same file is intended
 * to use with both master and remote configurations. On zynq platform
 * only one node defintion is required for master/remote as there
 * are only two cores present in the platform.
 *
 * Only platform specific info is populated here. Rest of information
 * is obtained during resource table parsing.The platform specific
 * information includes;
 *
 * -CPU ID
 * -Shared Memory
 * -Interrupts
 * -Channel info.
 *
 * Although the channel info is not platform specific information
 * but it is conveneient to keep it in HIL so that user can easily
 * provide it without modifying the generic part.
 *
 * It is good idea to define hil_proc structure with platform
 * specific fields populated as this can be easily copied to hil_proc
 * structure passed as parameter in platform_get_processor_info. The
 * other option is to populate the required structures individually
 * and copy them one by one to hil_proc structure in platform_get_processor_info
 * function. The first option is adopted here.
 *
 *
 * 1) First node in the array is intended for the remote contexts and it
 *    defines Master CPU ID, shared memory, interrupts info, number of channels
 *    and there names. This node defines only one channel
 *   "rpmsg-openamp-demo-channel".
 *
 * 2)Second node is required by the master and it defines remote CPU ID,
 *   shared memory and interrupts info. In general no channel info is required by the
 *   Master node, however in baremetal master and linux remote case the linux
 *   rpmsg bus driver behaves as master so the rpmsg driver on linux side still needs
 *   channel info. This information is not required by the masters for baremetal
 *   remotes. 
 *
 */
struct hil_proc proc_table []=
{
    {
        /* CPU ID of remote */
        REMOTE_CPU_ID,

        /* Shared memory info - Last field is not used currently */
        {
            SHM_ADDR, SHM_SIZE, 0x00
        },

        /* VirtIO device info */
        {
            0, 0, 0,
            {
                {
                    /* Provide vring interrupts info here. Other fields are obtained
                     * from the rsc table so leave them empty.
                     */
                    NULL, NULL, 0, 0,
                    {
                        VRING0_IPI_VECT,0x1006,1
                    }
                },
                {
                    NULL, NULL, 0, 0,
                    {
                        VRING1_IPI_VECT,0x1006,1
                    }
                }
            }
        },

        /* Number of RPMSG channels */
        1,

        /* RPMSG channel info - Only channel name is expected currently */
        {
            {"rpmsg-openamp-demo-channel"}
        },

        /* HIL platform ops table. */
        &zynq_a9_proc_ops,

        /* Next three fields are for future use only */
        0,
        0,
        NULL
    }
};

/* Start and end addresses of firmware image for remotes. These are defined in the
 * object files that are obtained by converting the remote ELF Image into object
 * files. These symbols are not used for remotes.
 */
extern unsigned char _binary_firmware1_start;
extern unsigned char _binary_firmware1_end;

extern unsigned char _binary_firmware2_start;
extern unsigned char _binary_firmware2_end;

#define FIRMWARE1_START  (void *)&_binary_firmware1_start
#define FIRMWARE1_END    (void *)&_binary_firmware1_end

#define FIRMWARE2_START  (void *)&_binary_firmware2_start
#define FIRMWARE2_END    (void *)&_binary_firmware2_end

/* Init firmware table */

const struct firmware_info fw_table[] =
{
	{"firmware1",
	 (unsigned int)FIRMWARE1_START,
	 (unsigned int)FIRMWARE1_END},
	{"firmware2",
	 (unsigned int)FIRMWARE2_START,
	 (unsigned int)FIRMWARE2_END}
};

int fw_table_size = sizeof(fw_table)/sizeof(struct firmware_info);
