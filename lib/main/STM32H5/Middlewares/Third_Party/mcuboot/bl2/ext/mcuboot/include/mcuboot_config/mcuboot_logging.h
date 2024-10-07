/*
 * Copyright (c) 2017 Linaro Limited
 * Copyright (c) 2019 Arm Limited.
 * Copyright (c) 2023 STMicroelectronics.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __MCUBOOT_LOGGING_H__
#define __MCUBOOT_LOGGING_H__

#include "bootutil/ignore.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MCUBOOT_LOG_LEVEL_OFF       0
#define MCUBOOT_LOG_LEVEL_ERROR     1
#define MCUBOOT_LOG_LEVEL_WARNING   2
#define MCUBOOT_LOG_LEVEL_INFO      3
#define MCUBOOT_LOG_LEVEL_DEBUG     4

/*
 * The compiled log level determines the maximum level that can be
 * printed. Messages at or below this level can be printed.
 */
#ifndef MCUBOOT_LOG_LEVEL
#define MCUBOOT_LOG_LEVEL           MCUBOOT_LOG_LEVEL_INFO
#endif

#define MCUBOOT_LOG_MODULE_DECLARE(domain)      /* Ignore */
#define MCUBOOT_LOG_MODULE_REGISTER(domain)     /* Ignore */

#if MCUBOOT_LOG_LEVEL >= MCUBOOT_LOG_LEVEL_ERROR
#define MCUBOOT_LOG_ERR(_fmt, ...)                  \
    printf("[ERR] " _fmt "\r\n", ##__VA_ARGS__)
#else
#define MCUBOOT_LOG_ERR(...) IGNORE(__VA_ARGS__)
#endif

#if MCUBOOT_LOG_LEVEL >= MCUBOOT_LOG_LEVEL_WARNING
#define MCUBOOT_LOG_WRN(_fmt, ...)                  \
    printf("[WRN] " _fmt "\r\n", ##__VA_ARGS__)
#else
#define MCUBOOT_LOG_WRN(...) IGNORE(__VA_ARGS__)
#endif

#if MCUBOOT_LOG_LEVEL >= MCUBOOT_LOG_LEVEL_INFO
#define MCUBOOT_LOG_INF(_fmt, ...)                  \
    printf("[INF] " _fmt "\r\n", ##__VA_ARGS__)
#else
#define MCUBOOT_LOG_INF(...) IGNORE(__VA_ARGS__)
#endif

#if MCUBOOT_LOG_LEVEL >= MCUBOOT_LOG_LEVEL_DEBUG
#define MCUBOOT_LOG_DBG(_fmt, ...)                  \
    printf("[DBG] " _fmt "\r\n", ##__VA_ARGS__)
#else
#define MCUBOOT_LOG_DBG(...) IGNORE(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MCUBOOT_LOGGING_H__ */
