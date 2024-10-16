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

#include "asc_security_core/utils/string_utils.h"
#include "asc_security_core/utils/iconv.h"
#include "asc_security_core/utils/string_utils.h"

char *code2string(code2string_t *list, int code)
{
    for (; list->code != -1 && list->code!=code; list++);

    return list->string;
}

int string2code(code2string_t *list, char *string, uint32_t len)
{
    for (; list->code != -1 && str_ncmp(list->string, str_len(list->string), string, len); list++);

    return list->code;
}