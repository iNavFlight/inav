[#ftl]
[@pp.dropOutputFile /]
[@pp.changeOutputFile name="hal_icu_lld_cfg.h" /]
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
 * @file    hal_icu_lld_cfg.h
 * @brief   ICU Driver configuration macros and structures.
 *
 * @addtogroup ICU
 * @{
 */

#ifndef _ICU_LLD_CFG_H_
#define _ICU_LLD_CFG_H_

#if HAL_USE_ICU || defined(__DOXYGEN__)

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

/* List of the ICUConfig structures defined in icu_lld_cfg.c.*/
[#list conf.instance.emios200_settings.icu_configurations.configs.icu_configuration_settings as settings]
extern const ICUConfig icu_config_${settings.symbolic_name.value[0]?trim};
[/#list]

#ifdef __cplusplus
extern "C" {
#endif
  /* List of the callback functions referenced from the ICUConfig
     structures in icu_lld_cfg.c.*/
[#assign period_callbacks = []]
[#assign width_callbacks = []]
[#assign overflow_callbacks = []]
[#list conf.instance.emios200_settings.icu_configurations.configs.icu_configuration_settings as settings]
  [#assign period_callback = settings.notifications.period_callback.value[0]?string?trim /]
  [#if period_callback != ""]
    [#if !period_callbacks?seq_contains(period_callback)]
      [#assign period_callbacks = period_callbacks + [period_callback]]
    [/#if]
  [/#if]
  [#assign width_callback = settings.notifications.width_callback.value[0]?string?trim /]
  [#if width_callback != ""]
    [#if !width_callbacks?seq_contains(width_callback)]
      [#assign width_callbacks = width_callbacks + [width_callback]]
    [/#if]
  [/#if]
  [#assign overflow_callback = settings.notifications.overflow_callback.value[0]?string?trim /]
  [#if overflow_callback != ""]
    [#if !overflow_callbacks?seq_contains(overflow_callback)]
      [#assign overflow_callbacks = overflow_callbacks + [overflow_callback]]
    [/#if]
  [/#if]
[/#list]
[#list period_callbacks?sort as cb]
  void ${cb}(ICUDriver *icup);
[/#list]
[#list width_callbacks?sort as cb]
  void ${cb}(ICUDriver *icup);
[/#list]
[#list overflow_callbacks?sort as cb]
  void ${cb}(ICUDriver *icup);
[/#list]
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_ICU */

#endif /* _ICU_LLD_CFG_H_ */

/** @} */
