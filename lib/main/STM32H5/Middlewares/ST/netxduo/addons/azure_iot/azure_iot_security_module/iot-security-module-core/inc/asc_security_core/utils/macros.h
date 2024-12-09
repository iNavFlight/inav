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

#ifndef __MACROS_H__
#define __MACROS_H__
#include <asc_config.h>

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#endif

#ifdef __clang__
#define ATTRIBUTE_FORMAT(fmt_index, args_index) __attribute__((__format__ (__printf__, fmt_index, args_index)))
#elif __GNUC__
#define ATTRIBUTE_FORMAT(fmt_index, args_index) __attribute__((format(printf, fmt_index, args_index)))
#else
#define ATTRIBUTE_FORMAT(fmt_index, args_index)
#endif

#ifdef __clang__
#define ATTRIBUTE_NONNUL(_index, ...) __attribute__((nonnull (_index, ##__VA_ARGS__)))
#elif __GNUC__
#define ATTRIBUTE_NONNUL(_index, ...) __attribute__((nonnull(_index, ##__VA_ARGS__)))
#else
#define ATTRIBUTE_NONNUL(_index, ...)
#endif

#endif /* __MACROS_H__ */
