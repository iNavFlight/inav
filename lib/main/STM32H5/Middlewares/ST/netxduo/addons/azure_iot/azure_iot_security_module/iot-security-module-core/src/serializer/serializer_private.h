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

#ifndef SERIALIZER_PRIVATE_H
#define SERIALIZER_PRIVATE_H

#include <stdbool.h>
#include <stdint.h>

#include <asc_config.h>

#include "flatcc/flatcc_builder.h"

#include "asc_security_core/utils/collection/collection.h"
#include "asc_security_core/serializer.h"

struct serializer {
    /* This macro must be first in object */
    COLLECTION_INTERFACE(serializer_t);
    
    flatcc_builder_t builder;
    serializer_state_t state;
};

/**
 * @brief   A convenience method to begin a new event.
 *          Callable from states:   SERIALIZER_STATE_INITIALIZED, SERIALIZER_STATE_MESSAGE_PROCESSING
 * @note    To preserve buffer consistency, this call must be followed by adding a payload and then
 *          calling @b AzureIoTSecurity_Event_vec_push_end .
 * 
 * @param serializer            The serializer
 * @param timestamp             The event timestamp
 * @param collection_interval   The collection interval
 *
 * @return  ASC_RESULT_OK on success,
 *          ASC_RESULT_EXCEPTION otherwise
 */
asc_result_t serializer_event_start(serializer_t *serializer, unsigned long timestamp, unsigned long collection_interval);

#endif /* SERIALIZER_PRIVATE_H */
