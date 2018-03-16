[#ftl]
[@pp.dropOutputFile /]
[@pp.changeOutputFile name="hal_can_lld_cfg.h" /]
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
 * @file    hal_can_lld_cfg.h
 * @brief   CAN Driver configuration macros and structures.
 *
 * @addtogroup CAN
 * @{
 */

#ifndef _CAN_LLD_CFG_H_
#define _CAN_LLD_CFG_H_

#if HAL_USE_CAN || defined(__DOXYGEN__)

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

/* List of the CANConfig structures defined in can_lld_cfg.c.*/
[#list conf.instance.flexcan_settings.can_configurations.configs.can_configuration_settings as settings]
extern const CANConfig can_config_${settings.symbolic_name.value[0]?trim};
[/#list]

#ifdef __cplusplus
extern "C" {
#endif
  /* List of the callback functions referenced from the CANConfig
     structures in can_lld_cfg.c.*/
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_ICU */

#endif /* _ICU_LLD_CFG_H_ */

/** @} */
