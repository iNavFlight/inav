[#ftl]
[@pp.dropOutputFile /]
[@pp.changeOutputFile name="hal_can_lld_cfg.c" /]
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
 * @file    hal_can_lld_cfg.c
 * @brief   CAN Driver configuration code.
 *
 * @addtogroup CAN
 * @{
 */

#include "hal.h"
#include "hal_can_lld_cfg.h"

#if HAL_USE_CAN || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

[#list conf.instance.flexcan_settings.can_configurations.configs.can_configuration_settings as settings]
  [#assign name = settings.symbolic_name.value[0]?trim /]
  [#assign warnings = settings.flexcan_enable_warnings.value[0]?upper_case /]
  [#assign loopback = settings.flexcan_enable_loopback.value[0]?upper_case /]
  [#assign propseg = settings.timings.propseg.value[0]?string?trim /]
  [#assign pseg1 = settings.timings.pseg1.value[0]?string?trim /]
  [#assign pseg2 = settings.timings.pseg2.value[0]?string?trim /]
  [#assign presdiv = settings.timings.presdiv.value[0]?string?trim /]
  [#if warnings == "TRUE"]
    [#assign warnings = "CAN_MCR_WRN_EN" /]
  [/#if]
  [#if warnings == "FALSE"]
    [#assign warnings = "0" /]
  [/#if]
  [#if loopback == "TRUE"]
    [#assign loopback = "CAN_CTRL_LPB | " /]
  [/#if]
  [#if loopback == "FALSE"]
    [#assign loopback = "" /]
  [/#if]

/**
 * @brief   Structure defining the CAN configuration "${name}".
 */
const CANConfig can_config_${name} = {

  ${warnings},
  ${loopback}CAN_CTRL_PROPSEG(${propseg}) | CAN_CTRL_PSEG2(${pseg2}) |
  CAN_CTRL_PSEG1(${pseg1}) | CAN_CTRL_PRESDIV(${presdiv}),
#if SPC5_CAN_USE_FILTERS
  {
	[#list settings.filters.filter as filters]
  	[#assign IDf = filters.id_mode.value[0]?string?trim/]
  	[#assign maskf = filters.id_mask.value[0]?string?trim/]
  	[#if IDf == "STANDARD"]
    	[#assign IDf = "0" /]
  	[/#if]
  	[#if IDf == "EXTENDED"]
  		[#assign IDf = "1" /]
 	[/#if]
  	[#if maskf == ""]
    	[#assign maskf = "0" /]
  	[/#if]	 
   {${IDf}, ${maskf}},
  [/#list]
  }
#endif
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
