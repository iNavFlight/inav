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

#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include <asc_config.h>

#define LOG_LEVEL_NOTSET 0
#define LOG_LEVEL_FATAL 1
#define LOG_LEVEL_ERROR 2
#define LOG_LEVEL_WARN 3
#define LOG_LEVEL_INFO 4
#define LOG_LEVEL_DEBUG 5

#ifndef ASC_LOG_LEVEL
#define ASC_LOG_LEVEL LOG_LEVEL_NOTSET
#endif

#if ASC_LOG_LEVEL == LOG_LEVEL_NOTSET
    #define log_debug(...)
    #define log_info(...)
    #define log_warn(...)
    #define log_error(...)
    #define log_fatal(...)
    #define logger_set_system_log_level(_l)
    #define logger_set_timestamp(_s)
    #define logger_get_system_log_level() (LOG_LEVEL_NOTSET)
#else
    #include "asc_security_core/utils/macros.h"
    #include "asc_security_core/component_id.h"

    #define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

    bool logger_log(component_id_t id, unsigned int level, const char *filename, const char *func, int line, const char *fmt, ...) ATTRIBUTE_FORMAT(6, 7);
/**
 * @brief Set system log level - highest priority.
 *
 * @param set   Requested log level, if (-1) - reset to default 'ASC_LOG_LEVEL'
 *
 * @return true on seccess, otherwise false.
 */
    bool logger_set_system_log_level(int set);
    void logger_set_timestamp(bool set);
    int logger_get_system_log_level(void);

    // define log by severity according to ASC_LOG_LEVEL
    #if ASC_LOG_LEVEL < LOG_LEVEL_DEBUG
        #define log_debug(...)
    #else
        #define log_debug(...)     logger_log(components_manager_get_self_id(), LOG_LEVEL_DEBUG, __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)
    #endif
    #if ASC_LOG_LEVEL < LOG_LEVEL_INFO
        #define log_info(...)
    #else
        #define log_info(...)      logger_log(components_manager_get_self_id(), LOG_LEVEL_INFO, __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)
    #endif
    #if ASC_LOG_LEVEL < LOG_LEVEL_WARN
        #define log_warn(...)
    #else
        #define log_warn(...)      logger_log(components_manager_get_self_id(), LOG_LEVEL_WARN, __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)
    #endif
    #if ASC_LOG_LEVEL < LOG_LEVEL_ERROR
        #define log_error(...)
    #else
        #define log_error(...)     logger_log(components_manager_get_self_id(), LOG_LEVEL_ERROR, __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)
    #endif
    #if ASC_LOG_LEVEL < LOG_LEVEL_FATAL
        #define log_fatal(...)
    #else
        #define log_fatal(...)     logger_log(components_manager_get_self_id(), LOG_LEVEL_FATAL, __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)
    #endif
#endif

#endif //LOGGER_H
