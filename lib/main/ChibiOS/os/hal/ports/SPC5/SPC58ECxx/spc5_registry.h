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
 * @file    SPC58ECxx/spc5_registry.h
 * @brief   SPC58ECxx capabilities registry.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef SPC5_REGISTRY_H
#define SPC5_REGISTRY_H

/*===========================================================================*/
/* Platform capabilities.                                                    */
/*===========================================================================*/

/**
 * @name    SPC58ECxx capabilities
 * @{
 */
/* LINFlex attributes.*/
#define SPC5_HAS_LINFLEX0                   TRUE
#define SPC5_LINFLEX0_PCTL                  92
#define SPC5_LINFLEX0_RXI_HANDLER           vector376
#define SPC5_LINFLEX0_TXI_HANDLER           vector377
#define SPC5_LINFLEX0_ERR_HANDLER           vector378
#define SPC5_LINFLEX0_RXI_NUMBER            376
#define SPC5_LINFLEX0_TXI_NUMBER            377
#define SPC5_LINFLEX0_ERR_NUMBER            378
#define SPC5_LINFLEX0_CLK                   SPC5_AC12_DC2_CLK

#define SPC5_HAS_LINFLEX1                   TRUE
#define SPC5_LINFLEX1_PCTL                  220
#define SPC5_LINFLEX1_RXI_HANDLER           vector380
#define SPC5_LINFLEX1_TXI_HANDLER           vector381
#define SPC5_LINFLEX1_ERR_HANDLER           vector382
#define SPC5_LINFLEX1_RXI_NUMBER            380
#define SPC5_LINFLEX1_TXI_NUMBER            381
#define SPC5_LINFLEX1_ERR_NUMBER            382
#define SPC5_LINFLEX1_CLK                   SPC5_AC12_DC2_CLK

#define SPC5_HAS_LINFLEX2                   TRUE
#define SPC5_LINFLEX2_PCTL                  91
#define SPC5_LINFLEX2_RXI_HANDLER           vector384
#define SPC5_LINFLEX2_TXI_HANDLER           vector385
#define SPC5_LINFLEX2_ERR_HANDLER           vector386
#define SPC5_LINFLEX2_RXI_NUMBER            384
#define SPC5_LINFLEX2_TXI_NUMBER            385
#define SPC5_LINFLEX2_ERR_NUMBER            386
#define SPC5_LINFLEX2_CLK                   SPC5_AC12_DC2_CLK

#define SPC5_HAS_LINFLEX3                   FALSE
#define SPC5_HAS_LINFLEX4                   FALSE
#define SPC5_HAS_LINFLEX5                   FALSE
#define SPC5_HAS_LINFLEX6                   FALSE
#define SPC5_HAS_LINFLEX7                   FALSE
#define SPC5_HAS_LINFLEX8                   FALSE
#define SPC5_HAS_LINFLEX9                   FALSE

/* SIUL2 attributes.*/
#define SPC5_HAS_SIUL2                      TRUE
#define SPC5_SIUL2_PCTL                     15
#define SPC5_SIUL2_NUM_PORTS                13
#define SPC5_SIUL2_NUM_MUXES                420
/** @} */

#endif /* SPC5_REGISTRY_H */

/** @} */
