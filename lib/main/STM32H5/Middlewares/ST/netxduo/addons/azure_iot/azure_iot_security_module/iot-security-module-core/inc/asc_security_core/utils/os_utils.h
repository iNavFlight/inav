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

#ifndef OS_UTILS_H
#define OS_UTILS_H
#include <asc_config.h>

/**
 * @brief Unique hardware identifier for security module
 *
 * @return security module id
 */
const char *os_utils_get_security_module_id(void);

#endif /* OS_UTILS_H */
