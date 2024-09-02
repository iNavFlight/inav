/*******************************************************************************/
/*                                                                             */
/* Copyright (c) Microsoft Corporation. All rights reserved.                   */
/*                                                                             */
/* This software is licensed under the Microsoft Software License              */
/* Terms for Microsoft Azure Defender for IoT. Full text of the license can be */
/* found in the LICENSE file at https://aka.ms/AzureDefenderForIoT_EULA        */
/* and in the root directory of this software.                                 */
/*                                                                             */
/*******************************************************************************/
#include <string.h>

#include <asc_config.h>

#include "asc_security_core/logger.h"

#include "asc_security_core/serializer/custom_builder_allocator.h"

#define VS_SIZE 64
#define DS_SIZE 512
#define VB_SIZE 256
#define PL_SIZE 32
#define FS_SIZE 288
#define HT_SIZE 256
#define VD_SIZE 256

static uint8_t vs_buffer[VS_SIZE] = { 0 };
static uint8_t ds_buffer[DS_SIZE] = { 0 };
static uint8_t vb_buffer[VB_SIZE] = { 0 };
static uint8_t pl_buffer[PL_SIZE] = { 0 };
static uint8_t fs_buffer[FS_SIZE] = { 0 };
static uint8_t ht_buffer[HT_SIZE] = { 0 };
static uint8_t vd_buffer[VD_SIZE] = { 0 };

static flatcc_iovec_t buffers[FLATCC_BUILDER_ALLOC_BUFFER_COUNT] = {
    { .iov_base = vs_buffer, .iov_len = VS_SIZE },
    { .iov_base = ds_buffer, .iov_len = DS_SIZE },
    { .iov_base = vb_buffer, .iov_len = VB_SIZE },
    { .iov_base = pl_buffer, .iov_len = PL_SIZE },
    { .iov_base = fs_buffer, .iov_len = FS_SIZE },
    { .iov_base = ht_buffer, .iov_len = HT_SIZE },
    { .iov_base = vd_buffer, .iov_len = VD_SIZE },
    { .iov_base = NULL, .iov_len = 0 },
};

int serializer_custom_allocator(void *alloc_context, flatcc_iovec_t *b, size_t request, int zero_fill, int alloc_type)
{
    (void)alloc_context;

    if (request == 0) {
        if (b->iov_base) {
            b->iov_base = NULL;
            b->iov_len = 0;
        }
        return 0;
    }

    if (alloc_type >= FLATCC_BUILDER_ALLOC_BUFFER_COUNT) {
        log_error("Unexpected alloc_type=[%d]", alloc_type);
    }

    if (request > buffers[alloc_type].iov_len) {
        log_error("Allocation request with unexpected size=[%lu], alloc_type=[%d]", (unsigned long int)request, alloc_type);
        return -1;
    }

    *b = buffers[alloc_type];
    return 0;
}

void serializer_custom_allocator_reset(void)
{
    for (int i = 0; i < FLATCC_BUILDER_ALLOC_BUFFER_COUNT; ++i) {
        if (buffers[i].iov_base != NULL) {
            memset(buffers[i].iov_base, 0, buffers[i].iov_len);
        }
    }
}
