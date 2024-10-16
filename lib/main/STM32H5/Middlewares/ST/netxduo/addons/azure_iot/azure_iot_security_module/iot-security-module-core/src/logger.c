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
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include <asc_config.h>
#include "asc_security_core/components_manager.h"
// #define ASC_TIME_H_SUPPORT

#if ASC_LOG_LEVEL != LOG_LEVEL_NOTSET

#ifdef ASC_TIME_H_SUPPORT
#include <time.h>
#endif

#include "asc_security_core/logger.h"
#include "asc_security_core/utils/itime.h"
#include "asc_security_core/utils/string_utils.h"
#ifdef ASC_COMPONENT_CONFIGURATION
#include "asc_security_core/configuration.h"
#endif
#include "asc_security_core/utils/iconv.h"
#include "asc_security_core/utils/string_utils.h"
#include "asc_security_core/logger.h"

static unsigned int _system_level = ASC_LOG_LEVEL;
#ifdef ASC_LOG_TIMESTAMP_DEFAULT
static bool _timestamp = true;
#else
static bool _timestamp = false;
#endif

static code2string_t _log_levels[] = {
    {LOG_LEVEL_NOTSET, "NOSET"},
    {LOG_LEVEL_FATAL, "FATAL"},
    {LOG_LEVEL_ERROR, "ERROR"},
    {LOG_LEVEL_WARN, "WARN"},
    {LOG_LEVEL_INFO, "INFO"},
    {LOG_LEVEL_DEBUG, "DEBUG"},
    {-1, NULL}
};

static asc_result_t _cm_init(component_id_t id)
{
    return ASC_RESULT_OK;
}

#ifdef ASC_COMPONENT_CONFIGURATION
static bool _conf_validate_level(conf_t *conf)
{
    if (conf->value.type != CONF_TYPE_STRING) {
        log_error("Invalid configuration for component=[%.*s]: invalid type for key=[%.*s]",
            conf->component.length, conf->component.string, conf->key.length, conf->key.string);
        return false;
    }

    if (string2code(_log_levels, conf->value.value.string.string, conf->value.value.string.length) == -1) {
        log_error("Invalid configuration for component=[%.*s]: invalid type for key=[%.*s]",
            conf->component.length, conf->component.string, conf->key.length, conf->key.string);
        return false;
    }

    return true;
}

static asc_result_t _conf_validate_or_apply(linked_list_t *conf_list, conf_origin_t origin, bool validate_only)
{
    conf_t *conf;
    bool all_pass = true;

    linked_list_foreach(conf_list, conf)
    {
        char *token = NULL, *rest = NULL;
        size_t token_len = 0, rest_len = 0;
        component_id_t id;
        int code;

        log_debug("Validating [%.*s]: key=[%.*s]",
            conf->component.length, conf->component.string, conf->key.length, conf->key.string);

        if (!str_ncmp(conf->key.string, conf->key.length, "Level", str_len("Level"))) {
            if (!_conf_validate_level(conf)) {
                all_pass = false;
                continue;
            }

            if (validate_only) {
                continue;
            }

            code = string2code(_log_levels, conf->value.value.string.string, conf->value.value.string.length);
            logger_set_system_log_level(code);
            continue;
        }

        if (origin == CONF_ORIGIN_TWIN) {
            all_pass = false;
            log_error("Component=[%.*s] key=[%.*s] can't be configured via device twin",
                conf->component.length, conf->component.string, conf->key.length, conf->key.string);
            continue;
        }

        /* Validate key in format: <Component Name>_Level */
        if (str_split(conf->key.string, &token, &token_len, &rest, &rest_len, "_") != ASC_RESULT_OK) {
            log_error("Invalid configuration for component=[%.*s]: key=[%.*s]",
                conf->component.length, conf->component.string, conf->key.length, conf->key.string);
            all_pass = false;
            continue;
        }

        id = components_manager_get_id_by_name(token, token_len);
        if (id == 0) {
            log_error("Invalid component=[%.*s]", (uint32_t)token_len, token);
            all_pass = false;
            continue;
        }

        if (str_ncmp("Level", str_len("Level"), rest, rest_len)) {
            log_error("Invalid key=[%.*s]", (uint32_t)rest_len, rest);
            all_pass = false;
            continue;
        }

        if (!_conf_validate_level(conf)) {
                all_pass = false;
                continue;
            }

        if (validate_only) {
            continue;
        }

        if (!all_pass) {
            /* Shouldn't happen (it should fail on validation step) */
            log_error("Can't apply new log configuration - validation failed");
            continue;
        }

        code = string2code(_log_levels, conf->value.value.string.string, conf->value.value.string.length);
        components_manager_set_log_level(id, code);
    }

    return all_pass ? ASC_RESULT_OK : ASC_RESULT_BAD_ARGUMENT;
}

static asc_result_t _conf_validate(linked_list_t *conf_list, conf_origin_t origin)
{
    return _conf_validate_or_apply(conf_list, origin, true);
}

static asc_result_t _conf_apply(linked_list_t *conf_list, conf_origin_t origin)
{
    return _conf_validate_or_apply(conf_list, origin, false);
}
#endif

static asc_result_t _cm_deinit(component_id_t id)
{
    return ASC_RESULT_OK;
}

static asc_result_t _cm_subscribe(component_id_t id)
{
#ifdef ASC_COMPONENT_CONFIGURATION
    return configuration_component_register(components_manager_get_name(id), _conf_validate, _conf_apply);
#else
    return ASC_RESULT_OK;
#endif
}

static asc_result_t _cm_unsubscribe(component_id_t id)
{
#ifdef ASC_COMPONENT_CONFIGURATION
    return configuration_component_unregister(components_manager_get_name(id));
#else
    return ASC_RESULT_OK;
#endif
}

static component_ops_t _ops = {
    .init = _cm_init,
    .deinit = _cm_deinit,
    .subscribe = _cm_subscribe,
    .unsubscribe = _cm_unsubscribe,
};

COMPONENTS_FACTORY_DEFINITION(Logger, &_ops)

bool logger_set_system_log_level(int set)
{
    unsigned int level = (set < 0) ? ASC_LOG_LEVEL : (unsigned int)set;

    if (level > ASC_LOG_LEVEL) {
        log_error("Requested log level=[%u] is above than compiled=[%u]", level, ASC_LOG_LEVEL);
        return false;
    }
    _system_level = level;
    return true;
}

int logger_get_system_log_level(void)
{
    return (int)_system_level;
}

void logger_set_timestamp(bool set)
{
    _timestamp = set;
}

bool logger_log(component_id_t id, unsigned int level, const char *filename, const char *func, int line, const char *fmt, ...)
{
#define MDC_FORMAT "%s [%s/%s:%d] "
#define MDC_TS_FORMAT "%s %lu - [%s/%s:%d] "
#ifdef ASC_TIME_H_SUPPORT
    #define MDC_TS_TIME_H_FORMAT "%s %02d:%02d:%02d [%s/%s:%d] "
#endif
    const char *level_str = NULL;

    if (_system_level < level
#ifdef ASC_COMPONENT_CONFIGURATION
        || components_manager_get_log_level(id) < level
#endif
    )
    {
        return false;
    }

    level_str = code2string(_log_levels, (int)level);
    if (level_str == NULL) {
            level_str = "UNDEF";
    }

    if (_timestamp) {
        unsigned long rawtime = itime_time(NULL);
#ifdef ASC_TIME_H_SUPPORT
        struct tm *ptm = localtime((time_t *)&rawtime);
        if (ptm == NULL) {
            printf(MDC_TS_FORMAT, level_str, rawtime, filename, func, line);
        } else {
            printf(MDC_TS_TIME_H_FORMAT, level_str, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, filename, func, line);
        }
#else
        printf(MDC_TS_FORMAT, level_str, rawtime, filename, func, line);
#endif
    } else {
        printf(MDC_FORMAT, level_str, filename, func, line);
    }

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");
    return true;
}
#else
COMPONENTS_FACTORY_DEFINITION(Logger, NULL)
#endif
