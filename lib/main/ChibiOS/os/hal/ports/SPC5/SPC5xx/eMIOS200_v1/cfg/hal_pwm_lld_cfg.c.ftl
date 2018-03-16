[#ftl]
[@pp.dropOutputFile /]
[@pp.changeOutputFile name="hal_pwm_lld_cfg.c" /]
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
 * @file    hal_pwm_lld_cfg.c
 * @brief   PWM Driver configuration code.
 *
 * @addtogroup PWM
 * @{
 */

#include "hal.h"
#include "hal_pwm_lld_cfg.h"

#if HAL_USE_PWM || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

[#list conf.instance.emios200_settings.pwm_configurations.configs.pwm_configuration_settings as settings]
  [#assign name = settings.symbolic_name.value[0]?trim /]
  [#assign frequency = settings.clock_frequency.value[0]?trim /]
  [#assign period = settings.period.value[0]?trim /]
  [#assign mode = settings.alignment_mode.value[0]?trim /]
  [#assign period_cb = settings.period_callback.value[0]?string?trim /]
  [#if period_cb == ""]
    [#assign period_cb = "NULL" /]
  [/#if]
  [#assign ch0_output_mode = settings.channel_0_settings.output_mode.value[0]?trim /]
  [#assign ch0_cb = settings.channel_0_settings.channel_callback.value[0]?string?trim /]
  [#if ch0_cb == ""]
    [#assign ch0_cb = "NULL" /]
  [/#if]

/**
 * @brief   Structure defining the PWM configuration "${name}".
 */
const PWMConfig pwm_config_${name} = {
  ${frequency},
  ${period},
  ${period_cb},
  {
    {
      PWM_OUTPUT_${ch0_output_mode},
      ${ch0_cb}
    }
  },
  PWM_${mode}
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

#endif /* HAL_USE_PWM */

/** @} */
