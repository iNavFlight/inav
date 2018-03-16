[#ftl]
[@pp.dropOutputFile /]
[@pp.changeOutputFile name="hal_pwm_lld_cfg.h" /]
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
 * @file    hal_pwm_lld_cfg.h
 * @brief   PWM Driver configuration macros and structures.
 *
 * @addtogroup PWM
 * @{
 */

#ifndef _PWM_LLD_CFG_H_
#define _PWM_LLD_CFG_H_

#if HAL_USE_PWM || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

/* List of the PWMConfig structures defined in pwm_lld_cfg.c.*/
[#list conf.instance.emios_settings.pwm_configurations.configs.pwm_configuration_settings as settings]
extern const PWMConfig pwm_config_${settings.symbolic_name.value[0]?trim};
[/#list]

#ifdef __cplusplus
extern "C" {
#endif
  /* List of the callback functions referenced from the PWMConfig
     structures in pwm_lld_cfg.c.*/
[#assign period_callbacks = []]
[#assign channel_callbacks = []]
[#list conf.instance.emios_settings.pwm_configurations.configs.pwm_configuration_settings as settings]
  [#assign period_callback = settings.period_callback.value[0]?string?trim /]
  [#if period_callback != ""]
    [#if !period_callbacks?seq_contains(period_callback)]
      [#assign period_callbacks = period_callbacks + [period_callback]]
    [/#if]
  [/#if]
  [#assign ch0_callback = settings.channel_0_settings.channel_callback.value[0]?string?trim /]
  [#if ch0_callback != ""]
    [#if !channel_callbacks?seq_contains(ch0_callback)]
      [#assign channel_callbacks = channel_callbacks + [ch0_callback]]
    [/#if]
  [/#if]
  [#assign ch1_callback = settings.channel_1_settings.channel_callback.value[0]?string?trim /]
  [#if ch1_callback != ""]
    [#if !channel_callbacks?seq_contains(ch1_callback)]
      [#assign channel_callbacks = channel_callbacks + [ch1_callback]]
    [/#if]
  [/#if]
  [#assign ch2_callback = settings.channel_2_settings.channel_callback.value[0]?string?trim /]
  [#if ch2_callback != ""]
    [#if !channel_callbacks?seq_contains(ch2_callback)]
      [#assign channel_callbacks = channel_callbacks + [ch2_callback]]
    [/#if]
  [/#if]
  [#assign ch3_callback = settings.channel_3_settings.channel_callback.value[0]?string?trim /]
  [#if ch3_callback != ""]
    [#if !channel_callbacks?seq_contains(ch3_callback)]
      [#assign channel_callbacks = channel_callbacks + [ch3_callback]]
    [/#if]
  [/#if]
  [#assign ch4_callback = settings.channel_4_settings.channel_callback.value[0]?string?trim /]
  [#if ch4_callback != ""]
    [#if !channel_callbacks?seq_contains(ch4_callback)]
      [#assign channel_callbacks = channel_callbacks + [ch4_callback]]
    [/#if]
  [/#if]
  [#assign ch5_callback = settings.channel_5_settings.channel_callback.value[0]?string?trim /]
  [#if ch5_callback != ""]
    [#if !channel_callbacks?seq_contains(ch5_callback)]
      [#assign channel_callbacks = channel_callbacks + [ch5_callback]]
    [/#if]
  [/#if]
  [#assign ch6_callback = settings.channel_6_settings.channel_callback.value[0]?string?trim /]
  [#if ch6_callback != ""]
    [#if !channel_callbacks?seq_contains(ch6_callback)]
      [#assign channel_callbacks = channel_callbacks + [ch6_callback]]
    [/#if]
  [/#if]
[/#list]
[#list period_callbacks?sort as cb]
  void ${cb}(PWMDriver *pwmp);
[/#list]
[#list channel_callbacks?sort as cb]
  void ${cb}(PWMDriver *pwmp);
[/#list]
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_PWM */

#endif /* _PWM_LLD_CFG_H_ */

/** @} */
