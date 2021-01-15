/*
    Copyright (C) 2014..2017 Marco Veeneman

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
 * @file    TM4C123x/tiva_isr.h
 * @brief   TM4C123x ISR remapper driver header.
 *
 * @addtogroup TM4C123x_ISR
 * @{
 */

#ifndef _TIVA_ISR_H_
#define _TIVA_ISR_H_

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    ISR names and numbers remapping
 * @{
 */

/* UDMA units.*/
#define TIVA_UDMA_SW_HANDLER                VectorF8
#define TIVA_UDMA_ERR_HANDLER               VectorFC

#define TIVA_UDMA_SW_NUMBER                 46
#define TIVA_UDMA_ERR_NUMBER                47

/* GPIO units.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1232C3PM) || defined(PART_TM4C1232D5PM)  \
  || defined(PART_TM4C1232E6PM) || defined(PART_TM4C1232H6PM) || defined(PART_TM4C1236D5PM)  \
  || defined(PART_TM4C1236E6PM) || defined(PART_TM4C1236H6PM) || defined(PART_TM4C123AE6PM)  \
  || defined(PART_TM4C123AH6PM) || defined(PART_TM4C123FE6PM) || defined(PART_TM4C123FH6PM)
#define TIVA_GPIOA_HANDLER                  Vector40
#define TIVA_GPIOB_HANDLER                  Vector44
#define TIVA_GPIOC_HANDLER                  Vector48
#define TIVA_GPIOD_HANDLER                  Vector4C
#define TIVA_GPIOE_HANDLER                  Vector50
#define TIVA_GPIOF_HANDLER                  VectorB8
#define TIVA_GPIOG_HANDLER                  VectorBC

#define TIVA_GPIOA_NUMBER                   0
#define TIVA_GPIOB_NUMBER                   1
#define TIVA_GPIOC_NUMBER                   2
#define TIVA_GPIOD_NUMBER                   3
#define TIVA_GPIOE_NUMBER                   4
#define TIVA_GPIOF_NUMBER                   30
#define TIVA_GPIOG_NUMBER                   31
#endif
#if defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM) || defined(PART_TM4C1231E6PM)   \
  || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM)  \
  || defined(PART_TM4C1233E6PM) || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1237D5PM)  \
  || defined(PART_TM4C1237E6PM) || defined(PART_TM4C1237H6PM) || defined(PART_TM4C123BE6PM)  \
  || defined(PART_TM4C123BH6PM) || defined(PART_TM4C123GE6PM) || defined(PART_TM4C123GH6PM)
#define TIVA_GPIOA_HANDLER                  Vector40
#define TIVA_GPIOB_HANDLER                  Vector44
#define TIVA_GPIOC_HANDLER                  Vector48
#define TIVA_GPIOD_HANDLER                  Vector4C
#define TIVA_GPIOE_HANDLER                  Vector50
#define TIVA_GPIOF_HANDLER                  VectorB8

#define TIVA_GPIOA_NUMBER                   0
#define TIVA_GPIOB_NUMBER                   1
#define TIVA_GPIOC_NUMBER                   2
#define TIVA_GPIOD_NUMBER                   3
#define TIVA_GPIOE_NUMBER                   4
#define TIVA_GPIOF_NUMBER                   30
#endif
#if defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PZ) || defined(PART_TM4C1231H6PZ)   \
  || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PZ) || defined(PART_TM4C1233H6PZ)  \
  || defined(PART_TM4C1237D5PZ) || defined(PART_TM4C1237E6PZ) || defined(PART_TM4C1237H6PZ)  \
  || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PZ) || defined(PART_TM4C123GE6PZ)  \
  || defined(PART_TM4C123GH6PZ)
#define TIVA_GPIOA_HANDLER                  Vector40
#define TIVA_GPIOB_HANDLER                  Vector44
#define TIVA_GPIOC_HANDLER                  Vector48
#define TIVA_GPIOD_HANDLER                  Vector4C
#define TIVA_GPIOE_HANDLER                  Vector50
#define TIVA_GPIOF_HANDLER                  VectorB8
#define TIVA_GPIOG_HANDLER                  VectorBC
#define TIVA_GPIOH_HANDLER                  VectorC0
#define TIVA_GPIOJ_HANDLER                  Vector118
#define TIVA_GPIOK_HANDLER                  Vector11C
#define TIVA_GPIOL_HANDLER                  Vector120

#define TIVA_GPIOA_NUMBER                   0
#define TIVA_GPIOB_NUMBER                   1
#define TIVA_GPIOC_NUMBER                   2
#define TIVA_GPIOD_NUMBER                   3
#define TIVA_GPIOE_NUMBER                   4
#define TIVA_GPIOF_NUMBER                   30
#define TIVA_GPIOG_NUMBER                   31
#define TIVA_GPIOH_NUMBER                   32
#define TIVA_GPIOJ_NUMBER                   54
#define TIVA_GPIOK_NUMBER                   55
#define TIVA_GPIOL_NUMBER                   56
#endif
#if defined(PART_TM4C1231H6PGE) || defined(PART_TM4C1233H6PGE) || defined(PART_TM4C1237H6PGE)\
  || defined(PART_TM4C123BH6PGE) || defined(PART_TM4C123GH6PGE)
#define TIVA_GPIOA_HANDLER                  Vector40
#define TIVA_GPIOB_HANDLER                  Vector44
#define TIVA_GPIOC_HANDLER                  Vector48
#define TIVA_GPIOD_HANDLER                  Vector4C
#define TIVA_GPIOE_HANDLER                  Vector50
#define TIVA_GPIOF_HANDLER                  VectorB8
#define TIVA_GPIOG_HANDLER                  VectorBC
#define TIVA_GPIOH_HANDLER                  VectorC0
#define TIVA_GPIOJ_HANDLER                  Vector118
#define TIVA_GPIOK_HANDLER                  Vector11C
#define TIVA_GPIOL_HANDLER                  Vector120
#define TIVA_GPIOM_HANDLER                  Vector1FC
#define TIVA_GPION_HANDLER                  Vector200
#define TIVA_GPIOP0_HANDLER                 Vector210
#define TIVA_GPIOP1_HANDLER                 Vector214
#define TIVA_GPIOP2_HANDLER                 Vector218
#define TIVA_GPIOP3_HANDLER                 Vector21C
#define TIVA_GPIOP4_HANDLER                 Vector220
#define TIVA_GPIOP5_HANDLER                 Vector224
#define TIVA_GPIOP6_HANDLER                 Vector228
#define TIVA_GPIOP7_HANDLER                 Vector22C

#define TIVA_GPIOA_NUMBER                   0
#define TIVA_GPIOB_NUMBER                   1
#define TIVA_GPIOC_NUMBER                   2
#define TIVA_GPIOD_NUMBER                   3
#define TIVA_GPIOE_NUMBER                   4
#define TIVA_GPIOF_NUMBER                   30
#define TIVA_GPIOG_NUMBER                   31
#define TIVA_GPIOH_NUMBER                   32
#define TIVA_GPIOJ_NUMBER                   54
#define TIVA_GPIOK_NUMBER                   55
#define TIVA_GPIOL_NUMBER                   56
#define TIVA_GPIOM_NUMBER                   111
#define TIVA_GPION_NUMBER                   112
#define TIVA_GPIOP0_NUMBER                  116
#define TIVA_GPIOP1_NUMBER                  117
#define TIVA_GPIOP2_NUMBER                  118
#define TIVA_GPIOP3_NUMBER                  119
#define TIVA_GPIOP4_NUMBER                  120
#define TIVA_GPIOP5_NUMBER                  121
#define TIVA_GPIOP6_NUMBER                  122
#define TIVA_GPIOP7_NUMBER                  123
#endif
#if defined(PART_TM4C123BH6ZRB) || defined(PART_TM4C123GH6ZRB) || defined(PART_TM4C123GH5ZXR)
#define TIVA_GPIOA_HANDLER                  Vector40
#define TIVA_GPIOB_HANDLER                  Vector44
#define TIVA_GPIOC_HANDLER                  Vector48
#define TIVA_GPIOD_HANDLER                  Vector4C
#define TIVA_GPIOE_HANDLER                  Vector50
#define TIVA_GPIOF_HANDLER                  VectorB8
#define TIVA_GPIOG_HANDLER                  VectorBC
#define TIVA_GPIOH_HANDLER                  VectorC0
#define TIVA_GPIOJ_HANDLER                  Vector118
#define TIVA_GPIOK_HANDLER                  Vector11C
#define TIVA_GPIOL_HANDLER                  Vector120
#define TIVA_GPIOM_HANDLER                  Vector1FC
#define TIVA_GPION_HANDLER                  Vector200
#define TIVA_GPIOP0_HANDLER                 Vector210
#define TIVA_GPIOP1_HANDLER                 Vector214
#define TIVA_GPIOP2_HANDLER                 Vector218
#define TIVA_GPIOP3_HANDLER                 Vector21C
#define TIVA_GPIOP4_HANDLER                 Vector220
#define TIVA_GPIOP5_HANDLER                 Vector224
#define TIVA_GPIOP6_HANDLER                 Vector228
#define TIVA_GPIOP7_HANDLER                 Vector22C
#define TIVA_GPIOQ0_HANDLER                 Vector230
#define TIVA_GPIOQ1_HANDLER                 Vector234
#define TIVA_GPIOQ2_HANDLER                 Vector238
#define TIVA_GPIOQ3_HANDLER                 Vector23C
#define TIVA_GPIOQ4_HANDLER                 Vector240
#define TIVA_GPIOQ5_HANDLER                 Vector244
#define TIVA_GPIOQ6_HANDLER                 Vector248
#define TIVA_GPIOQ7_HANDLER                 Vector24C

#define TIVA_GPIOA_NUMBER                   0
#define TIVA_GPIOB_NUMBER                   1
#define TIVA_GPIOC_NUMBER                   2
#define TIVA_GPIOD_NUMBER                   3
#define TIVA_GPIOE_NUMBER                   4
#define TIVA_GPIOF_NUMBER                   30
#define TIVA_GPIOG_NUMBER                   31
#define TIVA_GPIOH_NUMBER                   32
#define TIVA_GPIOJ_NUMBER                   54
#define TIVA_GPIOK_NUMBER                   55
#define TIVA_GPIOL_NUMBER                   56
#define TIVA_GPIOM_NUMBER                   111
#define TIVA_GPION_NUMBER                   112
#define TIVA_GPIOP0_NUMBER                  116
#define TIVA_GPIOP1_NUMBER                  117
#define TIVA_GPIOP2_NUMBER                  118
#define TIVA_GPIOP3_NUMBER                  119
#define TIVA_GPIOP4_NUMBER                  120
#define TIVA_GPIOP5_NUMBER                  121
#define TIVA_GPIOP6_NUMBER                  122
#define TIVA_GPIOP7_NUMBER                  123
#define TIVA_GPIOQ0_NUMBER                  124
#define TIVA_GPIOQ1_NUMBER                  125
#define TIVA_GPIOQ2_NUMBER                  126
#define TIVA_GPIOQ3_NUMBER                  127
#define TIVA_GPIOQ4_NUMBER                  128
#define TIVA_GPIOQ5_NUMBER                  129
#define TIVA_GPIOQ6_NUMBER                  130
#define TIVA_GPIOQ7_NUMBER                  131
#endif

/* GPTM units.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM)  \
  || defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PM) || defined(PART_TM4C1231E6PZ)  \
  || defined(PART_TM4C1231H6PGE) || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1231H6PZ) \
  || defined(PART_TM4C1232C3PM) || defined(PART_TM4C1232D5PM) || defined(PART_TM4C1232E6PM)  \
  || defined(PART_TM4C1232H6PM) || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM)  \
  || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PM) || defined(PART_TM4C1233E6PZ)  \
  || defined(PART_TM4C1233H6PGE) || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1233H6PZ) \
  || defined(PART_TM4C1236D5PM) || defined(PART_TM4C1236E6PM) || defined(PART_TM4C1236H6PM)  \
  || defined(PART_TM4C1237D5PM) || defined(PART_TM4C1237D5PZ) || defined(PART_TM4C1237E6PM)  \
  || defined(PART_TM4C1237E6PZ) || defined(PART_TM4C1237H6PGE) || defined(PART_TM4C1237H6PM) \
  || defined(PART_TM4C1237H6PZ) || defined(PART_TM4C123AE6PM) || defined(PART_TM4C123AH6PM)  \
  || defined(PART_TM4C123BE6PM) || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PGE) \
  || defined(PART_TM4C123BH6PM) || defined(PART_TM4C123BH6PZ) || defined(PART_TM4C123BH6ZRB) \
  || defined(PART_TM4C123FE6PM) || defined(PART_TM4C123FH6PM) || defined(PART_TM4C123GE6PM)  \
  || defined(PART_TM4C123GE6PZ) || defined(PART_TM4C123GH6PGE) || defined(PART_TM4C123GH6PM) \
  || defined(PART_TM4C123GH6PZ) || defined(PART_TM4C123GH6ZRB) || defined(PART_TM4C123GH5ZXR)
#define TIVA_GPT0A_HANDLER                   Vector8C
#define TIVA_GPT0B_HANDLER                   Vector90
#define TIVA_GPT1A_HANDLER                   Vector94
#define TIVA_GPT1B_HANDLER                   Vector98
#define TIVA_GPT2A_HANDLER                   Vector9C
#define TIVA_GPT2B_HANDLER                   VectorA0
#define TIVA_GPT3A_HANDLER                   VectorCC
#define TIVA_GPT3B_HANDLER                   VectorD0
#define TIVA_GPT4A_HANDLER                   Vector158
#define TIVA_GPT4B_HANDLER                   Vector15C
#define TIVA_GPT5A_HANDLER                   Vector1B0
#define TIVA_GPT5B_HANDLER                   Vector1B4

#define TIVA_GPT0A_NUMBER                    19
#define TIVA_GPT0B_NUMBER                    20
#define TIVA_GPT1A_NUMBER                    21
#define TIVA_GPT1B_NUMBER                    22
#define TIVA_GPT2A_NUMBER                    23
#define TIVA_GPT2B_NUMBER                    24
#define TIVA_GPT3A_NUMBER                    35
#define TIVA_GPT3B_NUMBER                    36
#define TIVA_GPT4A_NUMBER                    70
#define TIVA_GPT4B_NUMBER                    71
#define TIVA_GPT5A_NUMBER                    92
#define TIVA_GPT5B_NUMBER                    93

#define TIVA_WGPT0A_HANDLER                  Vector1B8
#define TIVA_WGPT0B_HANDLER                  Vector1BC
#define TIVA_WGPT1A_HANDLER                  Vector1C0
#define TIVA_WGPT1B_HANDLER                  Vector1C4
#define TIVA_WGPT2A_HANDLER                  Vector1C8
#define TIVA_WGPT2B_HANDLER                  Vector1CC
#define TIVA_WGPT3A_HANDLER                  Vector1D0
#define TIVA_WGPT3B_HANDLER                  Vector1D4
#define TIVA_WGPT4A_HANDLER                  Vector1D8
#define TIVA_WGPT4B_HANDLER                  Vector1DC
#define TIVA_WGPT5A_HANDLER                  Vector1E0
#define TIVA_WGPT5B_HANDLER                  Vector1E4

#define TIVA_WGPT0A_NUMBER                   94
#define TIVA_WGPT0B_NUMBER                   95
#define TIVA_WGPT1A_NUMBER                   96
#define TIVA_WGPT1B_NUMBER                   97
#define TIVA_WGPT2A_NUMBER                   98
#define TIVA_WGPT2B_NUMBER                   99
#define TIVA_WGPT3A_NUMBER                   100
#define TIVA_WGPT3B_NUMBER                   101
#define TIVA_WGPT4A_NUMBER                   102
#define TIVA_WGPT4B_NUMBER                   103
#define TIVA_WGPT5A_NUMBER                   104
#define TIVA_WGPT5B_NUMBER                   105
#endif

/* WDT units.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM)  \
  || defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PM) || defined(PART_TM4C1231E6PZ)  \
  || defined(PART_TM4C1231H6PGE) || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1231H6PZ) \
  || defined(PART_TM4C1232C3PM) || defined(PART_TM4C1232D5PM) || defined(PART_TM4C1232E6PM)  \
  || defined(PART_TM4C1232H6PM) || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM)  \
  || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PM) || defined(PART_TM4C1233E6PZ)  \
  || defined(PART_TM4C1233H6PGE) || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1233H6PZ) \
  || defined(PART_TM4C1236D5PM) || defined(PART_TM4C1236E6PM) || defined(PART_TM4C1236H6PM)  \
  || defined(PART_TM4C1237D5PM) || defined(PART_TM4C1237D5PZ) || defined(PART_TM4C1237E6PM)  \
  || defined(PART_TM4C1237E6PZ) || defined(PART_TM4C1237H6PGE) || defined(PART_TM4C1237H6PM) \
  || defined(PART_TM4C1237H6PZ) || defined(PART_TM4C123AE6PM) || defined(PART_TM4C123AH6PM)  \
  || defined(PART_TM4C123BE6PM) || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PGE) \
  || defined(PART_TM4C123BH6PM) || defined(PART_TM4C123BH6PZ) || defined(PART_TM4C123BH6ZRB) \
  || defined(PART_TM4C123FE6PM) || defined(PART_TM4C123FH6PM) || defined(PART_TM4C123GE6PM)  \
  || defined(PART_TM4C123GE6PZ) || defined(PART_TM4C123GH6PGE) || defined(PART_TM4C123GH6PM) \
  || defined(PART_TM4C123GH6PZ) || defined(PART_TM4C123GH6ZRB) || defined(PART_TM4C123GH5ZXR)
#define TIVA_WDT_HANDLER                    Vector88

#define TIVA_WDT_NUMBER                     18
#endif

/* ADC units.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM)  \
  || defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PM) || defined(PART_TM4C1231E6PZ)  \
  || defined(PART_TM4C1231H6PGE) || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1231H6PZ) \
  || defined(PART_TM4C1232C3PM) || defined(PART_TM4C1232D5PM) || defined(PART_TM4C1232E6PM)  \
  || defined(PART_TM4C1232H6PM) || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM)  \
  || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PM) || defined(PART_TM4C1233E6PZ)  \
  || defined(PART_TM4C1233H6PGE) || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1233H6PZ) \
  || defined(PART_TM4C1236D5PM) || defined(PART_TM4C1236E6PM) || defined(PART_TM4C1236H6PM)  \
  || defined(PART_TM4C1237D5PM) || defined(PART_TM4C1237D5PZ) || defined(PART_TM4C1237E6PM)  \
  || defined(PART_TM4C1237E6PZ) || defined(PART_TM4C1237H6PGE) || defined(PART_TM4C1237H6PM) \
  || defined(PART_TM4C1237H6PZ) || defined(PART_TM4C123AE6PM) || defined(PART_TM4C123AH6PM)  \
  || defined(PART_TM4C123BE6PM) || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PGE) \
  || defined(PART_TM4C123BH6PM) || defined(PART_TM4C123BH6PZ) || defined(PART_TM4C123BH6ZRB) \
  || defined(PART_TM4C123FE6PM) || defined(PART_TM4C123FH6PM) || defined(PART_TM4C123GE6PM)  \
  || defined(PART_TM4C123GE6PZ) || defined(PART_TM4C123GH6PGE) || defined(PART_TM4C123GH6PM) \
  || defined(PART_TM4C123GH6PZ) || defined(PART_TM4C123GH6ZRB) || defined(PART_TM4C123GH5ZXR)
#define TIVA_ADC0_SEQ0_HANDLER              Vector78
#define TIVA_ADC0_SEQ1_HANDLER              Vector7C
#define TIVA_ADC0_SEQ2_HANDLER              Vector80
#define TIVA_ADC0_SEQ3_HANDLER              Vector84
#define TIVA_ADC1_SEQ0_HANDLER              Vector100
#define TIVA_ADC1_SEQ1_HANDLER              Vector104
#define TIVA_ADC1_SEQ2_HANDLER              Vector108
#define TIVA_ADC1_SEQ3_HANDLER              Vector10C

#define TIVA_ADC0_SEQ0_NUMBER               14
#define TIVA_ADC0_SEQ1_NUMBER               15
#define TIVA_ADC0_SEQ2_NUMBER               16
#define TIVA_ADC0_SEQ3_NUMBER               17
#define TIVA_ADC1_SEQ0_NUMBER               48
#define TIVA_ADC1_SEQ1_NUMBER               49
#define TIVA_ADC1_SEQ2_NUMBER               50
#define TIVA_ADC1_SEQ3_NUMBER               51
#endif

/* UART units.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM)  \
  || defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PM) || defined(PART_TM4C1231E6PZ)  \
  || defined(PART_TM4C1231H6PGE) || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1231H6PZ) \
  || defined(PART_TM4C1232C3PM) || defined(PART_TM4C1232D5PM) || defined(PART_TM4C1232E6PM)  \
  || defined(PART_TM4C1232H6PM) || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM)  \
  || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PM) || defined(PART_TM4C1233E6PZ)  \
  || defined(PART_TM4C1233H6PGE) || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1233H6PZ) \
  || defined(PART_TM4C1236D5PM) || defined(PART_TM4C1236E6PM) || defined(PART_TM4C1236H6PM)  \
  || defined(PART_TM4C1237D5PM) || defined(PART_TM4C1237D5PZ) || defined(PART_TM4C1237E6PM)  \
  || defined(PART_TM4C1237E6PZ) || defined(PART_TM4C1237H6PGE) || defined(PART_TM4C1237H6PM) \
  || defined(PART_TM4C1237H6PZ) || defined(PART_TM4C123AE6PM) || defined(PART_TM4C123AH6PM)  \
  || defined(PART_TM4C123BE6PM) || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PGE) \
  || defined(PART_TM4C123BH6PM) || defined(PART_TM4C123BH6PZ) || defined(PART_TM4C123BH6ZRB) \
  || defined(PART_TM4C123FE6PM) || defined(PART_TM4C123FH6PM) || defined(PART_TM4C123GE6PM)  \
  || defined(PART_TM4C123GE6PZ) || defined(PART_TM4C123GH6PGE) || defined(PART_TM4C123GH6PM) \
  || defined(PART_TM4C123GH6PZ) || defined(PART_TM4C123GH6ZRB) || defined(PART_TM4C123GH5ZXR)
#define TIVA_UART0_HANDLER                  Vector54
#define TIVA_UART1_HANDLER                  Vector58
#define TIVA_UART2_HANDLER                  VectorC4
#define TIVA_UART3_HANDLER                  Vector12C
#define TIVA_UART4_HANDLER                  Vector130
#define TIVA_UART5_HANDLER                  Vector134
#define TIVA_UART6_HANDLER                  Vector138
#define TIVA_UART7_HANDLER                  Vector13C

#define TIVA_UART0_NUMBER                   5
#define TIVA_UART1_NUMBER                   6
#define TIVA_UART2_NUMBER                   33
#define TIVA_UART3_NUMBER                   59
#define TIVA_UART4_NUMBER                   60
#define TIVA_UART5_NUMBER                   61
#define TIVA_UART6_NUMBER                   62
#define TIVA_UART7_NUMBER                   63
#endif

/* SPI units.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM)  \
  || defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PM) || defined(PART_TM4C1231E6PZ)  \
  || defined(PART_TM4C1231H6PGE) || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1231H6PZ) \
  || defined(PART_TM4C1232C3PM) || defined(PART_TM4C1232D5PM) || defined(PART_TM4C1232E6PM)  \
  || defined(PART_TM4C1232H6PM) || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM)  \
  || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PM) || defined(PART_TM4C1233E6PZ)  \
  || defined(PART_TM4C1233H6PGE) || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1233H6PZ) \
  || defined(PART_TM4C1236D5PM) || defined(PART_TM4C1236E6PM) || defined(PART_TM4C1236H6PM)  \
  || defined(PART_TM4C1237D5PM) || defined(PART_TM4C1237D5PZ) || defined(PART_TM4C1237E6PM)  \
  || defined(PART_TM4C1237E6PZ) || defined(PART_TM4C1237H6PGE) || defined(PART_TM4C1237H6PM) \
  || defined(PART_TM4C1237H6PZ) || defined(PART_TM4C123AE6PM) || defined(PART_TM4C123AH6PM)  \
  || defined(PART_TM4C123BE6PM) || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PGE) \
  || defined(PART_TM4C123BH6PM) || defined(PART_TM4C123BH6PZ) || defined(PART_TM4C123BH6ZRB) \
  || defined(PART_TM4C123FE6PM) || defined(PART_TM4C123FH6PM) || defined(PART_TM4C123GE6PM)  \
  || defined(PART_TM4C123GE6PZ) || defined(PART_TM4C123GH6PGE) || defined(PART_TM4C123GH6PM) \
  || defined(PART_TM4C123GH6PZ) || defined(PART_TM4C123GH6ZRB) || defined(PART_TM4C123GH5ZXR)
#define TIVA_SSI0_HANDLER                   Vector5C
#define TIVA_SSI1_HANDLER                   VectorC8
#define TIVA_SSI2_HANDLER                   Vector124
#define TIVA_SSI3_HANDLER                   Vector128

#define TIVA_SSI0_NUMBER                    7
#define TIVA_SSI1_NUMBER                    34
#define TIVA_SSI2_NUMBER                    57
#define TIVA_SSI3_NUMBER                    58
#endif

/* I2C units.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PZ)  \
  || defined(PART_TM4C1231H6PGE) || defined(PART_TM4C1231H6PZ) || defined(PART_TM4C1232C3PM) \
  || defined(PART_TM4C1232D5PM) || defined(PART_TM4C1232E6PM) || defined(PART_TM4C1232H6PM)  \
  || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PZ) || defined(PART_TM4C1233H6PGE) \
  || defined(PART_TM4C1233H6PZ) || defined(PART_TM4C1236D5PM) || defined(PART_TM4C1236E6PM)  \
  || defined(PART_TM4C1236H6PM) || defined(PART_TM4C1237D5PZ) || defined(PART_TM4C1237E6PZ)  \
  || defined(PART_TM4C1237H6PGE) || defined(PART_TM4C1237H6PZ) || defined(PART_TM4C123AE6PM) \
  || defined(PART_TM4C123AH6PM) || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PGE) \
  || defined(PART_TM4C123BH6PZ) || defined(PART_TM4C123BH6ZRB) || defined(PART_TM4C123FE6PM) \
  || defined(PART_TM4C123FH6PM) || defined(PART_TM4C123GE6PZ) || defined(PART_TM4C123GH6PGE) \
  || defined(PART_TM4C123GH6PZ) || defined(PART_TM4C123GH6ZRB) || defined(PART_TM4C123GH5ZXR)
#define TIVA_I2C0_HANDLER                   Vector60
#define TIVA_I2C1_HANDLER                   VectorD4
#define TIVA_I2C2_HANDLER                   Vector150
#define TIVA_I2C3_HANDLER                   Vector154
#define TIVA_I2C4_HANDLER                   Vector1F4
#define TIVA_I2C5_HANDLER                   Vector1F8

#define TIVA_I2C0_NUMBER                    8
#define TIVA_I2C1_NUMBER                    37
#define TIVA_I2C2_NUMBER                    68
#define TIVA_I2C3_NUMBER                    69
#define TIVA_I2C4_NUMBER                    109
#define TIVA_I2C5_NUMBER                    110
#endif
#if defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM) || defined(PART_TM4C1231E6PM)   \
  || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM)  \
  || defined(PART_TM4C1233E6PM) || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1237D5PM)  \
  || defined(PART_TM4C1237E6PM) || defined(PART_TM4C1237H6PM) || defined(PART_TM4C123BE6PM)  \
  || defined(PART_TM4C123BH6PM) || defined(PART_TM4C123GE6PM) || defined(PART_TM4C123GH6PM)
#define TIVA_I2C0_HANDLER                   Vector60
#define TIVA_I2C1_HANDLER                   VectorD4
#define TIVA_I2C2_HANDLER                   Vector150
#define TIVA_I2C3_HANDLER                   Vector154

#define TIVA_I2C0_NUMBER                    8
#define TIVA_I2C1_NUMBER                    37
#define TIVA_I2C2_NUMBER                    68
#define TIVA_I2C3_NUMBER                    69
#endif

/* CAN units.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM)  \
  || defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PM) || defined(PART_TM4C1231E6PZ)  \
  || defined(PART_TM4C1231H6PGE) || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1231H6PZ) \
  || defined(PART_TM4C1232C3PM) || defined(PART_TM4C1232D5PM) || defined(PART_TM4C1232E6PM)  \
  || defined(PART_TM4C1232H6PM) || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM)  \
  || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PM) || defined(PART_TM4C1233E6PZ)  \
  || defined(PART_TM4C1233H6PGE) || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1233H6PZ) \
  || defined(PART_TM4C1236D5PM) || defined(PART_TM4C1236E6PM) || defined(PART_TM4C1236H6PM)  \
  || defined(PART_TM4C1237D5PM) || defined(PART_TM4C1237D5PZ) || defined(PART_TM4C1237E6PM)  \
  || defined(PART_TM4C1237E6PZ) || defined(PART_TM4C1237H6PGE) || defined(PART_TM4C1237H6PM) \
  || defined(PART_TM4C1237H6PZ)
#define TIVA_CAN0_HANDLER                   VectorDC

#define TIVA_CAN0_NUMBER                    39
#endif
#if defined(PART_TM4C123AE6PM) || defined(PART_TM4C123AH6PM) || defined(PART_TM4C123BE6PM)   \
  || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PGE) || defined(PART_TM4C123BH6PM) \
  || defined(PART_TM4C123BH6PZ) || defined(PART_TM4C123BH6ZRB) || defined(PART_TM4C123FE6PM) \
  || defined(PART_TM4C123FH6PM) || defined(PART_TM4C123GE6PM) || defined(PART_TM4C123GE6PZ)  \
  || defined(PART_TM4C123GH6PGE) || defined(PART_TM4C123GH6PM) || defined(PART_TM4C123GH6PZ) \
  || defined(PART_TM4C123GH6ZRB) || defined(PART_TM4C123GH5ZXR)
#define TIVA_CAN0_HANDLER                   VectorDC
#define TIVA_CAN1_HANDLER                   VectorE0

#define TIVA_CAN0_NUMBER                    39
#define TIVA_CAN1_NUMBER                    40
#endif

/* USB units.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM)  \
  || defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PM) || defined(PART_TM4C1231E6PZ)  \
  || defined(PART_TM4C1231H6PGE) || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1231H6PZ) \
  || defined(PART_TM4C123AE6PM) || defined(PART_TM4C123AH6PM) || defined(PART_TM4C123BE6PM)  \
  || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PGE) || defined(PART_TM4C123BH6PM) \
  || defined(PART_TM4C123BH6PZ) || defined(PART_TM4C123BH6ZRB)
/* No interrupt handler and number.*/
#endif
#if defined(PART_TM4C1232C3PM) || defined(PART_TM4C1232D5PM) || defined(PART_TM4C1232E6PM)   \
  || defined(PART_TM4C1232H6PM) || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM)  \
  || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PM) || defined(PART_TM4C1233E6PZ)  \
  || defined(PART_TM4C1233H6PGE) || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1233H6PZ) \
  || defined(PART_TM4C1236D5PM) || defined(PART_TM4C1236E6PM) || defined(PART_TM4C1236H6PM)  \
  || defined(PART_TM4C1237D5PM) || defined(PART_TM4C1237D5PZ) || defined(PART_TM4C1237E6PM)  \
  || defined(PART_TM4C1237E6PZ) || defined(PART_TM4C1237H6PGE) || defined(PART_TM4C1237H6PM) \
  || defined(PART_TM4C1237H6PZ) || defined(PART_TM4C123FE6PM) || defined(PART_TM4C123FH6PM)  \
  || defined(PART_TM4C123GE6PM) || defined(PART_TM4C123GE6PZ) || defined(PART_TM4C123GH6PGE) \
  || defined(PART_TM4C123GH6PM) || defined(PART_TM4C123GH6PZ) || defined(PART_TM4C123GH6ZRB) \
  || defined(PART_TM4C123GH5ZXR)
#define TIVA_USB0_HANDLER                   VectorF0

#define TIVA_USB0_NUMBER                    44
#endif

/* AC units.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM)  \
  || defined(PART_TM4C1231E6PM) || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1232C3PM)  \
  || defined(PART_TM4C1232D5PM) || defined(PART_TM4C1232E6PM) || defined(PART_TM4C1232H6PM)  \
  || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM) || defined(PART_TM4C1233E6PM)  \
  || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1236D5PM) || defined(PART_TM4C1236E6PM)  \
  || defined(PART_TM4C1236H6PM) || defined(PART_TM4C1237D5PM) || defined(PART_TM4C1237E6PM)  \
  || defined(PART_TM4C1237H6PM) || defined(PART_TM4C123AE6PM) || defined(PART_TM4C123AH6PM)  \
  || defined(PART_TM4C123BE6PM) || defined(PART_TM4C123BH6PM) || defined(PART_TM4C123FE6PM)  \
  || defined(PART_TM4C123FH6PM) || defined(PART_TM4C123GE6PM) || defined(PART_TM4C123GH6PM)
#define TIVA_AC0_HANDLER                    VectorA4
#define TIVA_AC1_HANDLER                    VectorA8

#define TIVA_AC0_NUMBER                     25
#define TIVA_AC1_NUMBER                     26
#endif
#if defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PZ) || defined(PART_TM4C1231H6PGE)  \
  || defined(PART_TM4C1231H6PZ) || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PZ)  \
  || defined(PART_TM4C1233H6PGE) || defined(PART_TM4C1233H6PZ) || defined(PART_TM4C1237D5PZ) \
  || defined(PART_TM4C1237E6PZ) || defined(PART_TM4C1237H6PGE) || defined(PART_TM4C1237H6PZ) \
  || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PGE) || defined(PART_TM4C123BH6PZ) \
  || defined(PART_TM4C123BH6ZRB) || defined(PART_TM4C123GE6PZ) || defined(PART_TM4C123GH6PGE)\
  || defined(PART_TM4C123GH6PZ) || defined(PART_TM4C123GH6ZRB) || defined(PART_TM4C123GH5ZXR)
#define TIVA_AC0_HANDLER                    VectorA4
#define TIVA_AC1_HANDLER                    VectorA8
#define TIVA_AC2_HANDLER                    VectorAC

#define TIVA_AC0_NUMBER                     25
#define TIVA_AC1_NUMBER                     26
#define TIVA_AC2_NUMBER                     27
#endif

/* PWM units.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM)  \
  || defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PM) || defined(PART_TM4C1231E6PZ)  \
  || defined(PART_TM4C1231H6PGE) || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1231H6PZ) \
  || defined(PART_TM4C1232C3PM) || defined(PART_TM4C1232D5PM) || defined(PART_TM4C1232E6PM)  \
  || defined(PART_TM4C1232H6PM) || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM)  \
  || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PM) || defined(PART_TM4C1233E6PZ)  \
  || defined(PART_TM4C1233H6PGE) || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1233H6PZ) \
  || defined(PART_TM4C1236D5PM) || defined(PART_TM4C1236E6PM) || defined(PART_TM4C1236H6PM)  \
  || defined(PART_TM4C1237D5PM) || defined(PART_TM4C1237D5PZ) || defined(PART_TM4C1237E6PM)  \
  || defined(PART_TM4C1237E6PZ) || defined(PART_TM4C1237H6PGE) || defined(PART_TM4C1237H6PM) \
  || defined(PART_TM4C1237H6PZ)
/* No interrupt handler and number.*/
#endif
#if defined(PART_TM4C123AE6PM) || defined(PART_TM4C123AH6PM) || defined(PART_TM4C123BE6PM)   \
  || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PGE) || defined(PART_TM4C123BH6PM) \
  || defined(PART_TM4C123BH6PZ) || defined(PART_TM4C123BH6ZRB) || defined(PART_TM4C123FE6PM) \
  || defined(PART_TM4C123FH6PM) || defined(PART_TM4C123GE6PM) || defined(PART_TM4C123GE6PZ)  \
  || defined(PART_TM4C123GH6PGE) || defined(PART_TM4C123GH6PM) || defined(PART_TM4C123GH6PZ) \
  || defined(PART_TM4C123GH6ZRB) || defined(PART_TM4C123GH5ZXR)
#define TIVA_PWM0FAULT_HANDLER              Vector64
#define TIVA_PWM0GEN0_HANDLER               Vector68
#define TIVA_PWM0GEN1_HANDLER               Vector6C
#define TIVA_PWM0GEN2_HANDLER               Vector70
#define TIVA_PWM0GEN3_HANDLER               VectorF4
#define TIVA_PWM1FAULT_HANDLER              Vector268
#define TIVA_PWM1GEN0_HANDLER               Vector258
#define TIVA_PWM1GEN1_HANDLER               Vector25C
#define TIVA_PWM1GEN2_HANDLER               Vector260
#define TIVA_PWM1GEN3_HANDLER               Vector264

#define TIVA_PWM0FAULT_NUMBER               9
#define TIVA_PWM0GEN0_NUMBER                10
#define TIVA_PWM0GEN1_NUMBER                11
#define TIVA_PWM0GEN2_NUMBER                12
#define TIVA_PWM0GEN3_NUMBER                45
#define TIVA_PWM1FAULT_NUMBER               138
#define TIVA_PWM1GEN0_NUMBER                134
#define TIVA_PWM1GEN1_NUMBER                135
#define TIVA_PWM1GEN2_NUMBER                136
#define TIVA_PWM1GEN3_NUMBER                137
#endif

/* QEI units.*/
#if defined(PART_TM4C1230C3PM) || defined(PART_TM4C1230D5PM) || defined(PART_TM4C1230E6PM)   \
  || defined(PART_TM4C1230H6PM) || defined(PART_TM4C1231C3PM) || defined(PART_TM4C1231D5PM)  \
  || defined(PART_TM4C1231D5PZ) || defined(PART_TM4C1231E6PM) || defined(PART_TM4C1231E6PZ)  \
  || defined(PART_TM4C1231H6PGE) || defined(PART_TM4C1231H6PM) || defined(PART_TM4C1231H6PZ) \
  || defined(PART_TM4C1232C3PM) || defined(PART_TM4C1232D5PM) || defined(PART_TM4C1232E6PM)  \
  || defined(PART_TM4C1232H6PM) || defined(PART_TM4C1233C3PM) || defined(PART_TM4C1233D5PM)  \
  || defined(PART_TM4C1233D5PZ) || defined(PART_TM4C1233E6PM) || defined(PART_TM4C1233E6PZ)  \
  || defined(PART_TM4C1233H6PGE) || defined(PART_TM4C1233H6PM) || defined(PART_TM4C1233H6PZ) \
  || defined(PART_TM4C1236D5PM) || defined(PART_TM4C1236E6PM) || defined(PART_TM4C1236H6PM)  \
  || defined(PART_TM4C1237D5PM) || defined(PART_TM4C1237D5PZ) || defined(PART_TM4C1237E6PM)  \
  || defined(PART_TM4C1237E6PZ) || defined(PART_TM4C1237H6PGE) || defined(PART_TM4C1237H6PM) \
  || defined(PART_TM4C1237H6PZ) || defined(PART_TM4C123AE6PM) || defined(PART_TM4C123AH6PM)
/* No interrupt handler and number.*/
#endif
#if defined(PART_TM4C123BE6PM) || defined(PART_TM4C123BE6PZ) || defined(PART_TM4C123BH6PGE)  \
  || defined(PART_TM4C123BH6PM) || defined(PART_TM4C123BH6PZ) || defined(PART_TM4C123BH6ZRB) \
  || defined(PART_TM4C123FE6PM) || defined(PART_TM4C123FH6PM) || defined(PART_TM4C123GE6PM)  \
  || defined(PART_TM4C123GE6PZ) || defined(PART_TM4C123GH6PGE) || defined(PART_TM4C123GH6PM) \
  || defined(PART_TM4C123GH6PZ) || defined(PART_TM4C123GH6ZRB) || defined(PART_TM4C123GH5ZXR)
#define TIVA_QEI0_HANLDER                   Vector74
#define TIVA_QEI1_HANLDER                   VectorD8

#define TIVA_QEI0_NUMBER                    13
#define TIVA_QEI1_NUMBER                    38
#endif

/**
 * @}
 */

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

#endif /* _TIVA_ISR_H_ */

/**
 * @}
 */
