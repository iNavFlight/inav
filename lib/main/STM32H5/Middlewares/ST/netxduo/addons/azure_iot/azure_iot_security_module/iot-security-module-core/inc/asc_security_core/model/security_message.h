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

#ifndef SECURITY_MESSAGE_H
#define SECURITY_MESSAGE_H

#include <stdbool.h>
#include <stdint.h>

#include <asc_config.h>

#include "asc_security_core/asc_result.h"

typedef struct {
    uint8_t *data;
    size_t size;
} security_message_t;


/**
 * @brief Clear Security Message
 *
 * @param security_message_ptr  security message ptr
 */
void security_message_clear(security_message_t *security_message_ptr);


/**
 * @brief Predicate is empty Security Message
 *
 * @param security_message_ptr  security message ptr
 *
 * @return true iff security message is empty
 */
bool security_message_is_empty(security_message_t *security_message_ptr);

#endif /* SECURITY_MESSAGE_H */