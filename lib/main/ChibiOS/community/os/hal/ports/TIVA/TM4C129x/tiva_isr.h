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
 * @file    TM4C129x/tiva_isr.h
 * @brief   TM4C129x ISR remapper driver header.
 *
 * @addtogroup TM4C129x_ISR
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

/* GPIO units.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1292NCPDT) || defined(PART_TM4C1294KCPDT)\
  || defined(PART_TM4C1294NCPDT) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129DNCPDT)\
  || defined(PART_TM4C129EKCPDT) || defined(PART_TM4C129ENCPDT)
#define TIVA_GPIOA_HANDLER                  Vector40
#define TIVA_GPIOB_HANDLER                  Vector44
#define TIVA_GPIOC_HANDLER                  Vector48
#define TIVA_GPIOD_HANDLER                  Vector4C
#define TIVA_GPIOE_HANDLER                  Vector50
#define TIVA_GPIOF_HANDLER                  VectorB8
#define TIVA_GPIOG_HANDLER                  VectorBC
#define TIVA_GPIOH_HANDLER                  VectorC0
#define TIVA_GPIOJ_HANDLER                  Vector10C
#define TIVA_GPIOK_HANDLER                  Vector110
#define TIVA_GPIOL_HANDLER                  Vector114
#define TIVA_GPIOM_HANDLER                  Vector160
#define TIVA_GPION_HANDLER                  Vector164
#define TIVA_GPIOP0_HANDLER                 Vector170
#define TIVA_GPIOP1_HANDLER                 Vector174
#define TIVA_GPIOP2_HANDLER                 Vector178
#define TIVA_GPIOP3_HANDLER                 Vector17C
#define TIVA_GPIOP4_HANDLER                 Vector180
#define TIVA_GPIOP5_HANDLER                 Vector184
#define TIVA_GPIOP6_HANDLER                 Vector188
#define TIVA_GPIOP7_HANDLER                 Vector18C
#define TIVA_GPIOQ0_HANDLER                 Vector190
#define TIVA_GPIOQ1_HANDLER                 Vector194
#define TIVA_GPIOQ2_HANDLER                 Vector198
#define TIVA_GPIOQ3_HANDLER                 Vector19C
#define TIVA_GPIOQ4_HANDLER                 Vector1A0
#define TIVA_GPIOQ5_HANDLER                 Vector1A4
#define TIVA_GPIOQ6_HANDLER                 Vector1A8
#define TIVA_GPIOQ7_HANDLER                 Vector1AC

#define TIVA_GPIOA_NUMBER                   0
#define TIVA_GPIOB_NUMBER                   1
#define TIVA_GPIOC_NUMBER                   2
#define TIVA_GPIOD_NUMBER                   3
#define TIVA_GPIOE_NUMBER                   4
#define TIVA_GPIOF_NUMBER                   30
#define TIVA_GPIOG_NUMBER                   31
#define TIVA_GPIOH_NUMBER                   32
#define TIVA_GPIOJ_NUMBER                   51
#define TIVA_GPIOK_NUMBER                   52
#define TIVA_GPIOL_NUMBER                   53
#define TIVA_GPIOM_NUMBER                   72
#define TIVA_GPION_NUMBER                   73
#define TIVA_GPIOP0_NUMBER                  76
#define TIVA_GPIOP1_NUMBER                  77
#define TIVA_GPIOP2_NUMBER                  78
#define TIVA_GPIOP3_NUMBER                  79
#define TIVA_GPIOP4_NUMBER                  80
#define TIVA_GPIOP5_NUMBER                  81
#define TIVA_GPIOP6_NUMBER                  82
#define TIVA_GPIOP7_NUMBER                  83
#define TIVA_GPIOQ0_NUMBER                  84
#define TIVA_GPIOQ1_NUMBER                  85
#define TIVA_GPIOQ2_NUMBER                  86
#define TIVA_GPIOQ3_NUMBER                  87
#define TIVA_GPIOQ4_NUMBER                  88
#define TIVA_GPIOQ5_NUMBER                  89
#define TIVA_GPIOQ6_NUMBER                  90
#define TIVA_GPIOQ7_NUMBER                  91
#endif
#if defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294NCZAD)\
  || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD) || defined(PART_TM4C1299NCZAD)\
  || defined(PART_TM4C129CNCZAD) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129ENCZAD)\
  || defined(PART_TM4C129LNCZAD) || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_GPIOA_HANDLER                  Vector40
#define TIVA_GPIOB_HANDLER                  Vector44
#define TIVA_GPIOC_HANDLER                  Vector48
#define TIVA_GPIOD_HANDLER                  Vector4C
#define TIVA_GPIOE_HANDLER                  Vector50
#define TIVA_GPIOF_HANDLER                  VectorB8
#define TIVA_GPIOG_HANDLER                  VectorBC
#define TIVA_GPIOH_HANDLER                  VectorC0
#define TIVA_GPIOJ_HANDLER                  Vector10C
#define TIVA_GPIOK_HANDLER                  Vector110
#define TIVA_GPIOL_HANDLER                  Vector114
#define TIVA_GPIOM_HANDLER                  Vector160
#define TIVA_GPION_HANDLER                  Vector164
#define TIVA_GPIOP0_HANDLER                 Vector170
#define TIVA_GPIOP1_HANDLER                 Vector174
#define TIVA_GPIOP2_HANDLER                 Vector178
#define TIVA_GPIOP3_HANDLER                 Vector17C
#define TIVA_GPIOP4_HANDLER                 Vector180
#define TIVA_GPIOP5_HANDLER                 Vector184
#define TIVA_GPIOP6_HANDLER                 Vector188
#define TIVA_GPIOP7_HANDLER                 Vector18C
#define TIVA_GPIOQ0_HANDLER                 Vector190
#define TIVA_GPIOQ1_HANDLER                 Vector194
#define TIVA_GPIOQ2_HANDLER                 Vector198
#define TIVA_GPIOQ3_HANDLER                 Vector19C
#define TIVA_GPIOQ4_HANDLER                 Vector1A0
#define TIVA_GPIOQ5_HANDLER                 Vector1A4
#define TIVA_GPIOQ6_HANDLER                 Vector1A8
#define TIVA_GPIOQ7_HANDLER                 Vector1AC
#define TIVA_GPIOR_HANDLER                  Vector1B0
#define TIVA_GPIOS_HANDLER                  Vector1B4
#define TIVA_GPIOT_HANDLER                  Vector1FC

#define TIVA_GPIOA_NUMBER                   0
#define TIVA_GPIOB_NUMBER                   1
#define TIVA_GPIOC_NUMBER                   2
#define TIVA_GPIOD_NUMBER                   3
#define TIVA_GPIOE_NUMBER                   4
#define TIVA_GPIOF_NUMBER                   30
#define TIVA_GPIOG_NUMBER                   31
#define TIVA_GPIOH_NUMBER                   32
#define TIVA_GPIOJ_NUMBER                   51
#define TIVA_GPIOK_NUMBER                   52
#define TIVA_GPIOL_NUMBER                   53
#define TIVA_GPIOM_NUMBER                   72
#define TIVA_GPION_NUMBER                   73
#define TIVA_GPIOP0_NUMBER                  76
#define TIVA_GPIOP1_NUMBER                  77
#define TIVA_GPIOP2_NUMBER                  78
#define TIVA_GPIOP3_NUMBER                  79
#define TIVA_GPIOP4_NUMBER                  80
#define TIVA_GPIOP5_NUMBER                  81
#define TIVA_GPIOP6_NUMBER                  82
#define TIVA_GPIOP7_NUMBER                  83
#define TIVA_GPIOQ0_NUMBER                  84
#define TIVA_GPIOQ1_NUMBER                  85
#define TIVA_GPIOQ2_NUMBER                  86
#define TIVA_GPIOQ3_NUMBER                  87
#define TIVA_GPIOQ4_NUMBER                  88
#define TIVA_GPIOQ5_NUMBER                  89
#define TIVA_GPIOQ6_NUMBER                  90
#define TIVA_GPIOQ7_NUMBER                  91
#define TIVA_GPIOR_NUMBER                   92
#define TIVA_GPIOS_NUMBER                   93
#define TIVA_GPIOT_NUMBER                   111
#endif

/* EPI units.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_EPI0_HANDLER                   Vector108

#define TIVA_EPI0_NUMBER                    50
#endif

/* CRC units.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
/* CRC has no interrupts.*/
#endif

/* AES Accelerator units.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD)
/* no interrupts.*/
#endif
#if defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD) || defined(PART_TM4C129DNCPDT) \
  || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT) || defined(PART_TM4C129ENCPDT)\
  || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD) || defined(PART_TM4C129XKCZAD)\
  || defined(PART_TM4C129XNCZAD)
#define TIVA_AES_HANDLER                    Vector1BC

#define TIVA_AES_NUMBER                     95
#endif

/* DES Accelerator units.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD)
/* no interrupts.*/
#endif
#if defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD) || defined(PART_TM4C129DNCPDT)\
  || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT) || defined(PART_TM4C129ENCPDT)\
  || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD) || defined(PART_TM4C129XKCZAD)\
  || defined(PART_TM4C129XNCZAD)
#define TIVA_DES_HANDLER                    Vector1C0

#define TIVA_DES_NUMBER                     51
#endif

/* SHA/MD5 Accelerator units.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD)
/* no interrupts.*/
#endif
#if defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD) || defined(PART_TM4C129DNCPDT)\
  || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT) || defined(PART_TM4C129ENCPDT)\
  || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD) || defined(PART_TM4C129XKCZAD)\
  || defined(PART_TM4C129XNCZAD)
#define TIVA_SHA_MD5_HANDLER                Vector1B8

#define TIVA_SHA_MD5_NUMBER                 94
#endif

/* GPT units.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_GPT0A_HANDLER                   Vector8C
#define TIVA_GPT0B_HANDLER                   Vector90
#define TIVA_GPT1A_HANDLER                   Vector94
#define TIVA_GPT1B_HANDLER                   Vector98
#define TIVA_GPT2A_HANDLER                   Vector9C
#define TIVA_GPT2B_HANDLER                   VectorA0
#define TIVA_GPT3A_HANDLER                   VectorCC
#define TIVA_GPT3B_HANDLER                   VectorD0
#define TIVA_GPT4A_HANDLER                   Vector13C
#define TIVA_GPT4B_HANDLER                   Vector140
#define TIVA_GPT5A_HANDLER                   Vector144
#define TIVA_GPT5B_HANDLER                   Vector148
#define TIVA_GPT6A_HANDLER                   Vector1C8
#define TIVA_GPT6B_HANDLER                   Vector1CC
#define TIVA_GPT7A_HANDLER                   Vector1D0
#define TIVA_GPT7B_HANDLER                   Vector1D4

#define TIVA_GPT0A_NUMBER                    19
#define TIVA_GPT0B_NUMBER                    20
#define TIVA_GPT1A_NUMBER                    21
#define TIVA_GPT1B_NUMBER                    22
#define TIVA_GPT2A_NUMBER                    23
#define TIVA_GPT2B_NUMBER                    24
#define TIVA_GPT3A_NUMBER                    35
#define TIVA_GPT3B_NUMBER                    36
#define TIVA_GPT4A_NUMBER                    63
#define TIVA_GPT4B_NUMBER                    64
#define TIVA_GPT5A_NUMBER                    65
#define TIVA_GPT5B_NUMBER                    66
#define TIVA_GPT6A_NUMBER                    98
#define TIVA_GPT6B_NUMBER                    99
#define TIVA_GPT7A_NUMBER                    100
#define TIVA_GPT7B_NUMBER                    101
#endif

/* WDT units.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_WDT_HANDLER                    Vector88

#define TIVA_WDT_NUMBER                     18
#endif

/* ADC units.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_ADC0_SEQ0_HANDLER              Vector78
#define TIVA_ADC0_SEQ1_HANDLER              Vector7C
#define TIVA_ADC0_SEQ2_HANDLER              Vector80
#define TIVA_ADC0_SEQ3_HANDLER              Vector84
#define TIVA_ADC1_SEQ0_HANDLER              VectorF8
#define TIVA_ADC1_SEQ1_HANDLER              VectorFC
#define TIVA_ADC1_SEQ2_HANDLER              Vector100
#define TIVA_ADC1_SEQ3_HANDLER              Vector104

#define TIVA_ADC0_SEQ0_NUMBER               14
#define TIVA_ADC0_SEQ1_NUMBER               15
#define TIVA_ADC0_SEQ2_NUMBER               16
#define TIVA_ADC0_SEQ3_NUMBER               17
#define TIVA_ADC1_SEQ0_NUMBER               46
#define TIVA_ADC1_SEQ1_NUMBER               47
#define TIVA_ADC1_SEQ2_NUMBER               48
#define TIVA_ADC1_SEQ3_NUMBER               49
#endif

/* UART units.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_UART0_HANDLER                  Vector54
#define TIVA_UART1_HANDLER                  Vector58
#define TIVA_UART2_HANDLER                  VectorC4
#define TIVA_UART3_HANDLER                  Vector120
#define TIVA_UART4_HANDLER                  Vector124
#define TIVA_UART5_HANDLER                  Vector128
#define TIVA_UART6_HANDLER                  Vector12C
#define TIVA_UART7_HANDLER                  Vector130

#define TIVA_UART0_NUMBER                   5
#define TIVA_UART1_NUMBER                   6
#define TIVA_UART2_NUMBER                   33
#define TIVA_UART3_NUMBER                   56
#define TIVA_UART4_NUMBER                   57
#define TIVA_UART5_NUMBER                   58
#define TIVA_UART6_NUMBER                   59
#define TIVA_UART7_NUMBER                   60
#endif

/* QSSI units.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_QSSI0_HANDLER                  Vector5C
#define TIVA_QSSI1_HANDLER                  VectorC8
#define TIVA_QSSI2_HANDLER                  Vector118
#define TIVA_QSSI3_HANDLER                  Vector11C

#define TIVA_QSSI0_NUMBER                   7
#define TIVA_QSSI1_NUMBER                   34
#define TIVA_QSSI2_NUMBER                   54
#define TIVA_QSSI3_NUMBER                   55
#endif

/* I2C units.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_I2C0_HANDLER                   Vector60
#define TIVA_I2C1_HANDLER                   VectorD4
#define TIVA_I2C2_HANDLER                   Vector134
#define TIVA_I2C3_HANDLER                   Vector138
#define TIVA_I2C4_HANDLER                   Vector158
#define TIVA_I2C5_HANDLER                   Vector15C
#define TIVA_I2C6_HANDLER                   Vector1D8
#define TIVA_I2C7_HANDLER                   Vector1DC
#define TIVA_I2C8_HANDLER                   Vector1F4
#define TIVA_I2C9_HANDLER                   Vector1F8

#define TIVA_I2C0_NUMBER                    8
#define TIVA_I2C1_NUMBER                    37
#define TIVA_I2C2_NUMBER                    61
#define TIVA_I2C3_NUMBER                    62
#define TIVA_I2C4_NUMBER                    70
#define TIVA_I2C5_NUMBER                    71
#define TIVA_I2C6_NUMBER                    102
#define TIVA_I2C7_NUMBER                    103
#define TIVA_I2C8_NUMBER                    109
#define TIVA_I2C9_NUMBER                    110
#endif

/* 1-Wire Master units.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)
#define TIVA_HAS_1WIRE                      FALSE
#endif
#if defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_1WIRE_HANDLER                  Vector1E4

#define TIVA_1WIRE_NUMBER                   105
#endif

/* CAN units.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_CAN0_HANDLER                   VectorD8
#define TIVA_CAN1_HANDLER                   VectorDC

#define TIVA_CAN0_NUMBER                    38
#define TIVA_CAN1_NUMBER                    39
#endif

/* Ethernet MAC units.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1297NCZAD)\
  || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)
/* no interrupts.*/
#endif
#if defined(PART_TM4C1292NCPDT) || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT)\
  || defined(PART_TM4C1294NCPDT) || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD)\
  || defined(PART_TM4C129EKCPDT) || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD)\
  || defined(PART_TM4C129LNCZAD) || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_MAC_HANDLER                    VectorE0

#define TIVA_MAC_NUMBER                     40
#endif

/* Ethernet PHY units.*/
#if defined(PART_TM4C1290NCPDT)|| defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT) \
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C129CNCPDT)\
  || defined(PART_TM4C129CNCZAD) || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD)
/* no interrupts.*/
#endif
#if  defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT) || defined(PART_TM4C1294NCZAD)\
  || defined(PART_TM4C1299KCZAD) || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
/* no interrupts.*/
#endif

/* USB units.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_USB0_HANDLER                   VectorE8

#define TIVA_USB0_NUMBER                    42
#endif

/* LCD units.*/
#if  defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD) || defined(PART_TM4C129DNCZAD)\
  || defined(PART_TM4C129LNCZAD) || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_LCD_HANDLER                    Vector1C4

#define TIVA_LCD_NUMBER                     97
#endif
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT)\
  || defined(PART_TM4C129CNCZAD) || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD)
/* no interrupts.*/
#endif

/* AC units.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_AC0_HANDLER                    VectorA4
#define TIVA_AC1_HANDLER                    VectorA8
#define TIVA_AC2_HANDLER                    VectorAC

#define TIVA_AC0_NUMBER                     25
#define TIVA_AC1_NUMBER                     26
#define TIVA_AC2_NUMBER                     27
#endif

/* PWM units.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_PWM0FAULT_HANDLER              Vector64
#define TIVA_PWM0GEN0_HANDLER               Vector68
#define TIVA_PWM0GEN1_HANDLER               Vector6C
#define TIVA_PWM0GEN2_HANDLER               Vector70
#define TIVA_PWM0GEN3_HANDLER               VectorEC

#define TIVA_PWM0FAULT_NUMBER               9
#define TIVA_PWM0GEN0_NUMBER                10
#define TIVA_PWM0GEN1_NUMBER                11
#define TIVA_PWM0GEN2_NUMBER                12
#define TIVA_PWM0GEN3_NUMBER                43
#endif

/* QEI units.*/
#if defined(PART_TM4C1290NCPDT) || defined(PART_TM4C1290NCZAD) || defined(PART_TM4C1292NCPDT)\
  || defined(PART_TM4C1292NCZAD) || defined(PART_TM4C1294KCPDT) || defined(PART_TM4C1294NCPDT)\
  || defined(PART_TM4C1294NCZAD) || defined(PART_TM4C1297NCZAD) || defined(PART_TM4C1299KCZAD)\
  || defined(PART_TM4C1299NCZAD) || defined(PART_TM4C129CNCPDT) || defined(PART_TM4C129CNCZAD)\
  || defined(PART_TM4C129DNCPDT) || defined(PART_TM4C129DNCZAD) || defined(PART_TM4C129EKCPDT)\
  || defined(PART_TM4C129ENCPDT) || defined(PART_TM4C129ENCZAD) || defined(PART_TM4C129LNCZAD)\
  || defined(PART_TM4C129XKCZAD) || defined(PART_TM4C129XNCZAD)
#define TIVA_QEI0_HANLDER                   Vector74

#define TIVA_QEI0_NUMBER                    13
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
