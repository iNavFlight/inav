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

#ifndef VERSION_H
#define VERSION_H
#include <asc_config.h>

#define SECURITY_MODULE_VERSION ((SECURITY_MODULE_VERSION_MAJOR << 24) |\
                                 (SECURITY_MODULE_VERSION_MINOR << 16) |\
                                  SECURITY_MODULE_VERSION_PATCH)

#endif /* VERSION_H */
