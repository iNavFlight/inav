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

#ifndef _MTI_H_
#define _MTI_H_
#include <asc_config.h>
/**
 * @brief Create Message Type Indicator in format "X.Y-E-S-T",
 * where "Major.Minor-Encoding-Source-Type". All fields are numeric
 */

#define MTI_NAME "mti"
#define SCHEMA_MAJOR_VER "2"
#define SCHEMA_MINOR_VER "3"

#define JSON_ENCODING   "0"
#define FLAT_BUFFERS    "1"

#define RAW_EVENTS_TYPE "0"

#define SCHEMA_ENCODING FLAT_BUFFERS

#define MTI_CORE SCHEMA_MAJOR_VER "." SCHEMA_MINOR_VER "-" SCHEMA_ENCODING "-"

#define MTI_TYPE "-" RAW_EVENTS_TYPE

#define MTI(source) MTI_CORE source MTI_TYPE

// Existing sources
#define MTI_SOURCE_AZURERTOS "0"
#define MTI_SOURCE_LINUX "1"

#endif /* _MTI_H_ */