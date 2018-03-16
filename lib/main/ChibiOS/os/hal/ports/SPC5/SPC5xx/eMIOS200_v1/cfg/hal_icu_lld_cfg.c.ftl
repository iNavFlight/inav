[#ftl]
[@pp.dropOutputFile /]
[@pp.changeOutputFile name="hal_icu_lld_cfg.c" /]
/*
    SPC5 HAL - Copyright (C) 2013 STMicroelectronics

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    hal_icu_lld_cfg.c
 * @brief   ICU Driver configuration code.
 *
 * @addtogroup ICU
 * @{
 */

#include "hal.h"
#include "hal_icu_lld_cfg.h"

#if HAL_USE_ICU || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

[#list conf.instance.emios200_settings.icu_configurations.configs.icu_configuration_settings as settings]
  [#assign name = settings.symbolic_name.value[0]?trim /]
  [#assign mode = settings.capture_mode.value[0]?trim /]
  [#assign frequency = settings.clock_frequency.value[0]?trim /]
  [#assign period_cb = settings.notifications.period_callback.value[0]?string?trim /]
  [#if period_cb == ""]
    [#assign period_cb = "NULL" /]
  [/#if]
  [#assign width_cb = settings.notifications.width_callback.value[0]?string?trim /]
  [#if width_cb == ""]
    [#assign width_cb = "NULL" /]
  [/#if]
  [#assign overflow_cb = settings.notifications.overflow_callback.value[0]?string?trim /]
  [#if overflow_cb == ""]
    [#assign overflow_cb = "NULL" /]
  [/#if]
/**
 * @brief   Structure defining the ICU configuration "${name}".
 */
const ICUConfig icu_config_${name} = {
  ICU_INPUT_${mode},
  ${frequency},
  ${width_cb},
  ${period_cb},
  ${overflow_cb}
};

[/#list]
/*===========================================================================*/
/* Driver local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

#endif /* HAL_USE_ICU */

/** @} */
