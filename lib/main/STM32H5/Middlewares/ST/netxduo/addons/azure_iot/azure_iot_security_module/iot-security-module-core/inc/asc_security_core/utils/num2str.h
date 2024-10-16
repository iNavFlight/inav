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

#ifndef _NUM2STR_H_
#define _NUM2STR_H_
#include <asc_config.h>

#define __STR_AUX(x) #x
#define NUM_2_STR(x) __STR_AUX(x)
#define NUM_2_STR_SIZE(x) (sizeof(NUM_2_STR(x)))

#define ULLONG64_MAX_STR NUM_2_STR(18446744073709551615)
#define ULLONG64_MAX_STR_SIZE (sizeof(ULLONG64_MAX_STR))

#define LLONG64_MAX_STR NUM_2_STR(9223372036854775807)
#define LLONG64_MAX_STR_SIZE (sizeof(LLONG64_MAX_STR))

#define ULONG32_MAX_STR NUM_2_STR(4294967295)
#define ULONG32_MAX_STR_SIZE (sizeof(ULONG32_MAX_STR))

#define LONG32_MAX_STR NUM_2_STR(2147483647)
#define LONG32_MAX_STR_SIZE (sizeof(LONG32_MAX_STR))

#define UINT16_MAX_STR NUM_2_STR(65535)
#define UINT16_MAX_STR_SIZE (sizeof(UINT16_MAX_STR))

#define INT16_MAX_STR NUM_2_STR(32767)
#define INT16_MAX_STR_SIZE (sizeof(INT16_MAX_STR))

#endif /* _NUM2STR_H_ */