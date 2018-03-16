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

[#list conf.instance.flexpwm_settings.pwm0_configurations.configs.pwm_configuration_settings as settings]
  [#assign name = settings.symbolic_name.value[0]?trim /]
  [#assign frequency = settings.clock_frequency.value[0]?trim /]
  [#assign period = settings.period.value[0]?trim /]
  [#assign mode = settings.alignment_mode.value[0]?trim /]
  [#assign period_cb = settings.period_callback.value[0]?string?trim /]
  [#assign sync_pwm0 = conf.instance.flexpwm_settings.synchronized_flexpwm0.value[0]?upper_case /]
  [#if period_cb == ""]
    [#assign period_cb = "NULL" /]
  [/#if]
  [#assign ch0_output_mode = settings.channel_0_settings.output_mode.value[0]?trim /]
  [#assign ch0_complementary_output_mode = settings.channel_0_settings.complementary_output_mode.value[0]?trim /]
  [#assign ch0_cb = settings.channel_0_settings.channel_callback.value[0]?string?trim /]
  [#if ch0_cb == ""]
    [#assign ch0_cb = "NULL" /]
  [/#if]
  [#assign ch1_output_mode = settings.channel_1_settings.output_mode.value[0]?trim /]
  [#assign ch1_complementary_output_mode = settings.channel_1_settings.complementary_output_mode.value[0]?trim /]
  [#assign ch1_cb = settings.channel_1_settings.channel_callback.value[0]?string?trim /]
  [#if ch1_cb == ""]
    [#assign ch1_cb = "NULL" /]
  [/#if]
  [#if sync_pwm0 == "TRUE"]
    [#assign ch2_output_mode = settings.channel_2_settings.output_mode.value[0]?trim /]
    [#assign ch2_complementary_output_mode = settings.channel_2_settings.complementary_output_mode.value[0]?trim /]
    [#assign ch2_cb = settings.channel_2_settings.channel_callback.value[0]?string?trim /]
    [#if ch2_cb == ""]
      [#assign ch2_cb = "NULL" /]
    [/#if]
    [#assign ch3_output_mode = settings.channel_3_settings.output_mode.value[0]?trim /]
    [#assign ch3_complementary_output_mode = settings.channel_3_settings.complementary_output_mode.value[0]?trim /]
    [#assign ch3_cb = settings.channel_3_settings.channel_callback.value[0]?string?trim /]
    [#if ch3_cb == ""]
      [#assign ch3_cb = "NULL" /]
    [/#if]
    [#assign ch4_output_mode = settings.channel_4_settings.output_mode.value[0]?trim /]
    [#assign ch4_complementary_output_mode = settings.channel_4_settings.complementary_output_mode.value[0]?trim /]
    [#assign ch4_cb = settings.channel_4_settings.channel_callback.value[0]?string?trim /]
    [#if ch4_cb == ""]
      [#assign ch4_cb = "NULL" /]
    [/#if]
    [#assign ch5_output_mode = settings.channel_5_settings.output_mode.value[0]?trim /]
    [#assign ch5_complementary_output_mode = settings.channel_5_settings.complementary_output_mode.value[0]?trim /]
    [#assign ch5_cb = settings.channel_5_settings.channel_callback.value[0]?string?trim /]
    [#if ch5_cb == ""]
      [#assign ch5_cb = "NULL" /]
    [/#if]
    [#assign ch6_output_mode = settings.channel_6_settings.output_mode.value[0]?trim /]
    [#assign ch6_complementary_output_mode = settings.channel_6_settings.complementary_output_mode.value[0]?trim /]
    [#assign ch6_cb = settings.channel_6_settings.channel_callback.value[0]?string?trim /]
    [#if ch6_cb == ""]
      [#assign ch6_cb = "NULL" /]
    [/#if]
    [#assign ch7_output_mode = settings.channel_7_settings.output_mode.value[0]?trim /]
    [#assign ch7_complementary_output_mode = settings.channel_7_settings.complementary_output_mode.value[0]?trim /]
    [#assign ch7_cb = settings.channel_7_settings.channel_callback.value[0]?string?trim /]
    [#if ch7_cb == ""]
      [#assign ch7_cb = "NULL" /]
    [/#if]
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
      PWM_OUTPUT_${ch0_output_mode} | PWM_COMPLEMENTARY_OUTPUT_${ch0_complementary_output_mode},
      ${ch0_cb}
    },
    {
      PWM_OUTPUT_${ch1_output_mode} | PWM_COMPLEMENTARY_OUTPUT_${ch1_complementary_output_mode},
      ${ch1_cb}
	[#if sync_pwm0 == "TRUE"]
    },
	{
      PWM_OUTPUT_${ch2_output_mode} | PWM_COMPLEMENTARY_OUTPUT_${ch2_complementary_output_mode},
      ${ch2_cb}
    },
	{
      PWM_OUTPUT_${ch3_output_mode} | PWM_COMPLEMENTARY_OUTPUT_${ch3_complementary_output_mode},
      ${ch3_cb}
    },
	{
      PWM_OUTPUT_${ch4_output_mode} | PWM_COMPLEMENTARY_OUTPUT_${ch4_complementary_output_mode},
      ${ch4_cb}
    },
	{
      PWM_OUTPUT_${ch5_output_mode} | PWM_COMPLEMENTARY_OUTPUT_${ch5_complementary_output_mode},
      ${ch5_cb}
    },
	{
      PWM_OUTPUT_${ch6_output_mode} | PWM_COMPLEMENTARY_OUTPUT_${ch6_complementary_output_mode},
      ${ch6_cb}
    },
	{
      PWM_OUTPUT_${ch7_output_mode} | PWM_COMPLEMENTARY_OUTPUT_${ch7_complementary_output_mode},
      ${ch7_cb}
	[/#if]
	}
  },
  PWM_${mode}
};

[/#list]
[#list conf.instance.flexpwm_settings.pwm1_configurations.configs.pwm_configuration_settings as settings]
  [#assign name = settings.symbolic_name.value[0]?trim /]
  [#assign frequency = settings.clock_frequency.value[0]?trim /]
  [#assign period = settings.period.value[0]?trim /]
  [#assign mode = settings.alignment_mode.value[0]?trim /]
  [#assign period_cb = settings.period_callback.value[0]?string?trim /]
  [#assign sync_pwm1 = conf.instance.flexpwm_settings.synchronized_flexpwm1.value[0]?upper_case /]
  [#if period_cb == ""]
    [#assign period_cb = "NULL" /]
  [/#if]
  [#assign ch0_output_mode = settings.channel_0_settings.output_mode.value[0]?trim /]
  [#assign ch0_complementary_output_mode = settings.channel_0_settings.complementary_output_mode.value[0]?trim /]
  [#assign ch0_cb = settings.channel_0_settings.channel_callback.value[0]?string?trim /]
  [#if ch0_cb == ""]
    [#assign ch0_cb = "NULL" /]
  [/#if]
  [#assign ch1_output_mode = settings.channel_1_settings.output_mode.value[0]?trim /]
  [#assign ch1_complementary_output_mode = settings.channel_1_settings.complementary_output_mode.value[0]?trim /]
  [#assign ch1_cb = settings.channel_1_settings.channel_callback.value[0]?string?trim /]
  [#if ch1_cb == ""]
    [#assign ch1_cb = "NULL" /]
  [/#if]
  [#if sync_pwm1 == "TRUE"]
    [#assign ch2_output_mode = settings.channel_2_settings.output_mode.value[0]?trim /]
    [#assign ch2_complementary_output_mode = settings.channel_2_settings.complementary_output_mode.value[0]?trim /]
    [#assign ch2_cb = settings.channel_2_settings.channel_callback.value[0]?string?trim /]
    [#if ch2_cb == ""]
      [#assign ch2_cb = "NULL" /]
    [/#if]
    [#assign ch3_output_mode = settings.channel_3_settings.output_mode.value[0]?trim /]
    [#assign ch3_complementary_output_mode = settings.channel_3_settings.complementary_output_mode.value[0]?trim /]
    [#assign ch3_cb = settings.channel_3_settings.channel_callback.value[0]?string?trim /]
    [#if ch3_cb == ""]
      [#assign ch3_cb = "NULL" /]
    [/#if]
    [#assign ch4_output_mode = settings.channel_4_settings.output_mode.value[0]?trim /]
    [#assign ch4_complementary_output_mode = settings.channel_4_settings.complementary_output_mode.value[0]?trim /]
    [#assign ch4_cb = settings.channel_4_settings.channel_callback.value[0]?string?trim /]
    [#if ch4_cb == ""]
      [#assign ch4_cb = "NULL" /]
    [/#if]
    [#assign ch5_output_mode = settings.channel_5_settings.output_mode.value[0]?trim /]
    [#assign ch5_complementary_output_mode = settings.channel_5_settings.complementary_output_mode.value[0]?trim /]
    [#assign ch5_cb = settings.channel_5_settings.channel_callback.value[0]?string?trim /]
    [#if ch5_cb == ""]
      [#assign ch5_cb = "NULL" /]
    [/#if]
    [#assign ch6_output_mode = settings.channel_6_settings.output_mode.value[0]?trim /]
    [#assign ch6_complementary_output_mode = settings.channel_6_settings.complementary_output_mode.value[0]?trim /]
    [#assign ch6_cb = settings.channel_6_settings.channel_callback.value[0]?string?trim /]
    [#if ch6_cb == ""]
      [#assign ch6_cb = "NULL" /]
    [/#if]
    [#assign ch7_output_mode = settings.channel_7_settings.output_mode.value[0]?trim /]
    [#assign ch7_complementary_output_mode = settings.channel_7_settings.complementary_output_mode.value[0]?trim /]
    [#assign ch7_cb = settings.channel_7_settings.channel_callback.value[0]?string?trim /]
    [#if ch7_cb == ""]
      [#assign ch7_cb = "NULL" /]
    [/#if]
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
      PWM_OUTPUT_${ch0_output_mode} | PWM_COMPLEMENTARY_OUTPUT_${ch0_complementary_output_mode},
      ${ch0_cb}
    },
    {
      PWM_OUTPUT_${ch1_output_mode} | PWM_COMPLEMENTARY_OUTPUT_${ch1_complementary_output_mode},
      ${ch1_cb}
	[#if sync_pwm1 == "TRUE"]
    },
	{
      PWM_OUTPUT_${ch2_output_mode} | PWM_COMPLEMENTARY_OUTPUT_${ch2_complementary_output_mode},
      ${ch2_cb}
    },
	{
      PWM_OUTPUT_${ch3_output_mode} | PWM_COMPLEMENTARY_OUTPUT_${ch3_complementary_output_mode},
      ${ch3_cb}
    },
	{
      PWM_OUTPUT_${ch4_output_mode} | PWM_COMPLEMENTARY_OUTPUT_${ch4_complementary_output_mode},
      ${ch4_cb}
    },
	{
      PWM_OUTPUT_${ch5_output_mode} | PWM_COMPLEMENTARY_OUTPUT_${ch5_complementary_output_mode},
      ${ch5_cb}
    },
	{
      PWM_OUTPUT_${ch6_output_mode} | PWM_COMPLEMENTARY_OUTPUT_${ch6_complementary_output_mode},
      ${ch6_cb}
    },
	{
      PWM_OUTPUT_${ch7_output_mode} | PWM_COMPLEMENTARY_OUTPUT_${ch7_complementary_output_mode},
      ${ch7_cb}
	[/#if]
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
