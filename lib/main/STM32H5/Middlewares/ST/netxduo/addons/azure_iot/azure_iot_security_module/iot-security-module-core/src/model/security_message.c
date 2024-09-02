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
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <asc_config.h>

#include "asc_security_core/model/security_message.h"


void security_message_clear(security_message_t *security_message_ptr)
{
    if (security_message_ptr == NULL) {
        return;
    }

    security_message_ptr->data = NULL;
    security_message_ptr->size = 0;
}


bool security_message_is_empty(security_message_t *security_message_ptr)
{
    if (security_message_ptr == NULL)
    {
        return true;
    }

    return security_message_ptr->data == NULL || security_message_ptr->size == 0;
}