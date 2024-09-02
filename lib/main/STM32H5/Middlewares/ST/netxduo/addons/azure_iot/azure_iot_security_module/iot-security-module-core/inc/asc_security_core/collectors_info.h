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

#ifndef _COLLECTORS_INFO_H_
#define _COLLECTORS_INFO_H_

#include <stdint.h>

#include <asc_config.h>

#define COLLECTORS_INFO_SIZE COLLECTORS_COUNT

typedef struct {
    unsigned long interval;
} collector_info_t;

typedef intptr_t collectors_info_t;
/**
 * @brief                   Initialize collectors info module
 *
 * @return                  Collectors info data struct handler
 */
collectors_info_t *collectors_info_init(void);

/**
 * @brief                   Deinitialize collectors info module
 *
 * @param collectors_info   collectors_info_t *
 *
 * @return                  None
 */
void collectors_info_deinit(collectors_info_t *collectors_info);

/**
 * @brief                   Get collectors info
 *
 * @param collectors_info   collectors_info_t *
 * 
 * @param size              [out] size of @c collector_info_t array
 *
 * @return                  @c collector_info_t pointer to collector info array
 */
collector_info_t *collectors_info_get(collectors_info_t *collectors_info, uint32_t *size);

#endif /* _COLLECTORS_INFO_H_ */
