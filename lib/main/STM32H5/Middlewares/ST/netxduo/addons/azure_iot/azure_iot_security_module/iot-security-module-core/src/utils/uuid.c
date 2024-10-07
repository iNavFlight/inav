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
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <asc_config.h>

#include "asc_security_core/utils/irand.h"
#include "asc_security_core/utils/uuid.h"

static bool _initialized = false;

static uint64_t _uuid[2];

int uuid_generate(uint8_t *buf_out)
{
    if (!_initialized)
    {
        _uuid[0] = (uint64_t)irand_int() << 32 | irand_int();
        _uuid[1] = (uint64_t)irand_int() << 32 | irand_int();

        _initialized = true;
    }

    union {
        uint8_t b[16];
        uint64_t word[2];
    } result;

    result.word[0] = ++_uuid[0];
    result.word[1] = ++_uuid[1];

    memmove(buf_out, result.b, 16);

    return 0;
}