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

#ifndef BOARD_H
#define BOARD_H

/*
 * Setup for Texas Instruments TM4C1294 Launchpad Board.
 */

/*
 * Board identifier.
 */
#define BOARD_TI_TM4C1294_LAUNCHPAD
#define BOARD_NAME              "Texas Instruments TM4C1294 Launchpad"

/*
 * Ethernet PHY type.
 */
#define BOARD_PHY_ADDR                  0   /* 0 is internal PHY */
#define BOARD_PHY_ID                    0x2000A221  /* internal PHY ID */
/* uncomment when using RMII */
//#define BOARD_PHY_RMII

/*
 * MCU type and revision as defined in the TI header.
 */
#define PART_TM4C1294NCPDT
#define TARGET_IS_TM4C129_RA0

/*
 * Board oscillators-related settings.
 */
#define TIVA_XTAL_VALUE         25000000

/*
 * IO pins assignments.
 */
#define GPIOA_UART0_RX          0
#define GPIOA_UART0_TX          1
#define GPIOA_PIN2              2
#define GPIOA_PIN3              3
#define GPIOA_PIN4              4
#define GPIOA_PIN5              5
#define GPIOA_PIN6              6
#define GPIOA_PIN7              7

#define GPIOB_PIN0              0
#define GPIOB_PIN1              1
#define GPIOB_PIN2              2
#define GPIOB_PIN3              3
#define GPIOB_PIN4              4
#define GPIOB_PIN5              5
#define GPIOB_PIN6              6
#define GPIOB_PIN7              7

#define GPIOC_TCK_SWCLK         0
#define GPIOC_TMS_SWDIO         1
#define GPIOC_TDI               2
#define GPIOC_TDO_SWO           3
#define GPIOC_PIN4              4
#define GPIOC_PIN5              5
#define GPIOC_PIN6              6
#define GPIOC_PIN7              7

#define GPIOD_PIN0              0
#define GPIOD_PIN1              1
#define GPIOD_PIN2              2
#define GPIOD_PIN3              3
#define GPIOD_PIN4              4
#define GPIOD_PIN5              5
#define GPIOD_PIN6              6
#define GPIOD_PIN7              7

#define GPIOE_PIN0              0
#define GPIOE_PIN1              1
#define GPIOE_PIN2              2
#define GPIOE_PIN3              3
#define GPIOE_PIN4              4
#define GPIOE_PIN5              5
#define GPIOE_PIN6              6
#define GPIOE_PIN7              7

#define GPIOF_LED0              0
#define GPIOF_PIN1              1
#define GPIOF_PIN2              2
#define GPIOF_PIN3              3
#define GPIOF_LED1              4
#define GPIOF_PIN5              5
#define GPIOF_PIN6              6
#define GPIOF_PIN7              7

#define GPIOG_PIN0              0
#define GPIOG_PIN1              1
#define GPIOG_PIN2              2
#define GPIOG_PIN3              3
#define GPIOG_PIN4              4
#define GPIOG_PIN5              5
#define GPIOG_PIN6              6
#define GPIOG_PIN7              7

#define GPIOH_PIN0              0
#define GPIOH_PIN1              1
#define GPIOH_PIN2              2
#define GPIOH_PIN3              3
#define GPIOH_PIN4              4
#define GPIOH_PIN5              5
#define GPIOH_PIN6              6
#define GPIOH_PIN7              7

#define GPIOJ_SW1               0
#define GPIOJ_PIN1              1
#define GPIOJ_PIN2              2
#define GPIOJ_PIN3              3
#define GPIOJ_PIN4              4
#define GPIOJ_PIN5              5
#define GPIOJ_PIN6              6
#define GPIOJ_PIN7              7

#define GPIOK_PIN0              0
#define GPIOK_PIN1              1
#define GPIOK_PIN2              2
#define GPIOK_PIN3              3
#define GPIOK_PIN4              4
#define GPIOK_PIN5              5
#define GPIOK_PIN6              6
#define GPIOK_PIN7              7

#define GPIOL_PIN0              0
#define GPIOL_PIN1              1
#define GPIOL_PIN2              2
#define GPIOL_PIN3              3
#define GPIOL_PIN4              4
#define GPIOL_PIN5              5
#define GPIOL_PIN6              6
#define GPIOL_PIN7              7

#define GPIOM_PIN0              0
#define GPIOM_PIN1              1
#define GPIOM_PIN2              2
#define GPIOM_PIN3              3
#define GPIOM_PIN4              4
#define GPIOM_PIN5              5
#define GPIOM_PIN6              6
#define GPIOM_PIN7              7

#define GPION_LED2              0
#define GPION_LED3              1
#define GPION_PIN2              2
#define GPION_PIN3              3
#define GPION_PIN4              4
#define GPION_PIN5              5
#define GPION_PIN6              6
#define GPION_PIN7              7

#define GPIOP_PIN0              0
#define GPIOP_PIN1              1
#define GPIOP_PIN2              2
#define GPIOP_PIN3              3
#define GPIOP_PIN4              4
#define GPIOP_PIN5              5
#define GPIOP_PIN6              6
#define GPIOP_PIN7              7

#define GPIOQ_PIN0              0
#define GPIOQ_PIN1              1
#define GPIOQ_PIN2              2
#define GPIOQ_PIN3              3
#define GPIOQ_PIN4              4
#define GPIOQ_PIN5              5
#define GPIOQ_PIN6              6
#define GPIOQ_PIN7              7

/*
 * IO lines assignments.
 */
#define LINE_UART0_RX           PAL_LINE(GPIOA, 0U)
#define LINE_UART0_TX           PAL_LINE(GPIOA, 1U)

#define LINE_LED0               PAL_LINE(GPIOF, 0U)
#define LINE_LED1               PAL_LINE(GPIOF, 4U)

#define LINE_LED2               PAL_LINE(GPION, 0U)
#define LINE_LED3               PAL_LINE(GPION, 1U)

#define LINE_SW1                PAL_LINE(GPIOJ, 0U)

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 */
#define VAL_GPIOA_DATA          0b00000000
#define VAL_GPIOA_DIR           0b00000000
#define VAL_GPIOA_AFSEL         0b00000000
#define VAL_GPIOA_DR2R          0b11111111
#define VAL_GPIOA_DR4R          0b00000000
#define VAL_GPIOA_DR8R          0b00000000
#define VAL_GPIOA_ODR           0b00000000
#define VAL_GPIOA_PUR           0b00000000
#define VAL_GPIOA_PDR           0b00000000
#define VAL_GPIOA_SLR           0b00000000
#define VAL_GPIOA_DEN           0b11111111
#define VAL_GPIOA_AMSEL         0b0000
#define VAL_GPIOA_PCTL          0x00000000

#define VAL_GPIOB_DATA          0b00000000
#define VAL_GPIOB_DIR           0b00000000
#define VAL_GPIOB_AFSEL         0b00000000
#define VAL_GPIOB_DR2R          0b11111111
#define VAL_GPIOB_DR4R          0b00000000
#define VAL_GPIOB_DR8R          0b00000000
#define VAL_GPIOB_ODR           0b00000000
#define VAL_GPIOB_PUR           0b00000000
#define VAL_GPIOB_PDR           0b00000000
#define VAL_GPIOB_SLR           0b00000000
#define VAL_GPIOB_DEN           0b11111111
#define VAL_GPIOB_AMSEL         0b0000
#define VAL_GPIOB_PCTL          0x00000000

#define VAL_GPIOC_DATA          0b00000000
#define VAL_GPIOC_DIR           0b00001000
#define VAL_GPIOC_AFSEL         0b00001111
#define VAL_GPIOC_DR2R          0b11111111
#define VAL_GPIOC_DR4R          0b00000000
#define VAL_GPIOC_DR8R          0b00000000
#define VAL_GPIOC_ODR           0b00000000
#define VAL_GPIOC_PUR           0b00001111
#define VAL_GPIOC_PDR           0b00000000
#define VAL_GPIOC_SLR           0b00000000
#define VAL_GPIOC_DEN           0b11111111
#define VAL_GPIOC_AMSEL         0b0000
#define VAL_GPIOC_PCTL          0x00001111

#define VAL_GPIOD_DATA          0b00000000
#define VAL_GPIOD_DIR           0b00000000
#define VAL_GPIOD_AFSEL         0b00000000
#define VAL_GPIOD_DR2R          0b11111111
#define VAL_GPIOD_DR4R          0b00000000
#define VAL_GPIOD_DR8R          0b00000000
#define VAL_GPIOD_ODR           0b00000000
#define VAL_GPIOD_PUR           0b00000000
#define VAL_GPIOD_PDR           0b00000000
#define VAL_GPIOD_SLR           0b00000000
#define VAL_GPIOD_DEN           0b11111111
#define VAL_GPIOD_AMSEL         0b0000
#define VAL_GPIOD_PCTL          0x00000000

#define VAL_GPIOE_DATA          0b00000000
#define VAL_GPIOE_DIR           0b00000000
#define VAL_GPIOE_AFSEL         0b00000000
#define VAL_GPIOE_DR2R          0b11111111
#define VAL_GPIOE_DR4R          0b00000000
#define VAL_GPIOE_DR8R          0b00000000
#define VAL_GPIOE_ODR           0b00000000
#define VAL_GPIOE_PUR           0b00000000
#define VAL_GPIOE_PDR           0b00000000
#define VAL_GPIOE_SLR           0b00000000
#define VAL_GPIOE_DEN           0b11111111
#define VAL_GPIOE_AMSEL         0b0000
#define VAL_GPIOE_PCTL          0x00000000

#define VAL_GPIOF_DATA          0b00000000
#define VAL_GPIOF_DIR           0b00000000
#define VAL_GPIOF_AFSEL         0b00000000
#define VAL_GPIOF_DR2R          0b11111111
#define VAL_GPIOF_DR4R          0b00000000
#define VAL_GPIOF_DR8R          0b00000000
#define VAL_GPIOF_ODR           0b00000000
#define VAL_GPIOF_PUR           0b00000000
#define VAL_GPIOF_PDR           0b00000000
#define VAL_GPIOF_SLR           0b00000000
#define VAL_GPIOF_DEN           0b11111111
#define VAL_GPIOF_AMSEL         0b0000
#define VAL_GPIOF_PCTL          0x00000000

#define VAL_GPIOG_DATA          0b00000000
#define VAL_GPIOG_DIR           0b00000000
#define VAL_GPIOG_AFSEL         0b00000000
#define VAL_GPIOG_DR2R          0b11111111
#define VAL_GPIOG_DR4R          0b00000000
#define VAL_GPIOG_DR8R          0b00000000
#define VAL_GPIOG_ODR           0b00000000
#define VAL_GPIOG_PUR           0b00000000
#define VAL_GPIOG_PDR           0b00000000
#define VAL_GPIOG_SLR           0b00000000
#define VAL_GPIOG_DEN           0b11111111
#define VAL_GPIOG_AMSEL         0b0000
#define VAL_GPIOG_PCTL          0x00000000

#define VAL_GPIOH_DATA          0b00000000
#define VAL_GPIOH_DIR           0b00000000
#define VAL_GPIOH_AFSEL         0b00000000
#define VAL_GPIOH_DR2R          0b11111111
#define VAL_GPIOH_DR4R          0b00000000
#define VAL_GPIOH_DR8R          0b00000000
#define VAL_GPIOH_ODR           0b00000000
#define VAL_GPIOH_PUR           0b00000000
#define VAL_GPIOH_PDR           0b00000000
#define VAL_GPIOH_SLR           0b00000000
#define VAL_GPIOH_DEN           0b11111111
#define VAL_GPIOH_AMSEL         0b0000
#define VAL_GPIOH_PCTL          0x00000000

#define VAL_GPIOJ_DATA          0b00000000
#define VAL_GPIOJ_DIR           0b00000000
#define VAL_GPIOJ_AFSEL         0b00000000
#define VAL_GPIOJ_DR2R          0b11111111
#define VAL_GPIOJ_DR4R          0b00000000
#define VAL_GPIOJ_DR8R          0b00000000
#define VAL_GPIOJ_ODR           0b00000000
#define VAL_GPIOJ_PUR           0b00000001
#define VAL_GPIOJ_PDR           0b00000000
#define VAL_GPIOJ_SLR           0b00000000
#define VAL_GPIOJ_DEN           0b11111111
#define VAL_GPIOJ_AMSEL         0b0000
#define VAL_GPIOJ_PCTL          0x00000000

#define VAL_GPIOK_DATA          0b00000000
#define VAL_GPIOK_DIR           0b00000000
#define VAL_GPIOK_AFSEL         0b00000000
#define VAL_GPIOK_DR2R          0b11111111
#define VAL_GPIOK_DR4R          0b00000000
#define VAL_GPIOK_DR8R          0b00000000
#define VAL_GPIOK_ODR           0b00000000
#define VAL_GPIOK_PUR           0b00000000
#define VAL_GPIOK_PDR           0b00000000
#define VAL_GPIOK_SLR           0b00000000
#define VAL_GPIOK_DEN           0b11111111
#define VAL_GPIOK_AMSEL         0b0000
#define VAL_GPIOK_PCTL          0x00000000

#define VAL_GPIOL_DATA          0b00000000
#define VAL_GPIOL_DIR           0b00000000
#define VAL_GPIOL_AFSEL         0b00000000
#define VAL_GPIOL_DR2R          0b11111111
#define VAL_GPIOL_DR4R          0b00000000
#define VAL_GPIOL_DR8R          0b00000000
#define VAL_GPIOL_ODR           0b00000000
#define VAL_GPIOL_PUR           0b00000000
#define VAL_GPIOL_PDR           0b00000000
#define VAL_GPIOL_SLR           0b00000000
#define VAL_GPIOL_DEN           0b11111111
#define VAL_GPIOL_AMSEL         0b0000
#define VAL_GPIOL_PCTL          0x00000000

#define VAL_GPIOM_DATA          0b00000000
#define VAL_GPIOM_DIR           0b00000000
#define VAL_GPIOM_AFSEL         0b00000000
#define VAL_GPIOM_DR2R          0b11111111
#define VAL_GPIOM_DR4R          0b00000000
#define VAL_GPIOM_DR8R          0b00000000
#define VAL_GPIOM_ODR           0b00000000
#define VAL_GPIOM_PUR           0b00000000
#define VAL_GPIOM_PDR           0b00000000
#define VAL_GPIOM_SLR           0b00000000
#define VAL_GPIOM_DEN           0b11111111
#define VAL_GPIOM_AMSEL         0b0000
#define VAL_GPIOM_PCTL          0x00000000

#define VAL_GPION_DATA          0b00000000
#define VAL_GPION_DIR           0b00000000
#define VAL_GPION_AFSEL         0b00000000
#define VAL_GPION_DR2R          0b11111111
#define VAL_GPION_DR4R          0b00000000
#define VAL_GPION_DR8R          0b00000000
#define VAL_GPION_ODR           0b00000000
#define VAL_GPION_PUR           0b00000000
#define VAL_GPION_PDR           0b00000000
#define VAL_GPION_SLR           0b00000000
#define VAL_GPION_DEN           0b11111111
#define VAL_GPION_AMSEL         0b0000
#define VAL_GPION_PCTL          0x00000000

#define VAL_GPIOP_DATA          0b00000000
#define VAL_GPIOP_DIR           0b00000000
#define VAL_GPIOP_AFSEL         0b00000000
#define VAL_GPIOP_DR2R          0b11111111
#define VAL_GPIOP_DR4R          0b00000000
#define VAL_GPIOP_DR8R          0b00000000
#define VAL_GPIOP_ODR           0b00000000
#define VAL_GPIOP_PUR           0b00000000
#define VAL_GPIOP_PDR           0b00000000
#define VAL_GPIOP_SLR           0b00000000
#define VAL_GPIOP_DEN           0b11111111
#define VAL_GPIOP_AMSEL         0b0000
#define VAL_GPIOP_PCTL          0x00000000

#define VAL_GPIOQ_DATA          0b00000000
#define VAL_GPIOQ_DIR           0b00000000
#define VAL_GPIOQ_AFSEL         0b00000000
#define VAL_GPIOQ_DR2R          0b11111111
#define VAL_GPIOQ_DR4R          0b00000000
#define VAL_GPIOQ_DR8R          0b00000000
#define VAL_GPIOQ_ODR           0b00000000
#define VAL_GPIOQ_PUR           0b00000000
#define VAL_GPIOQ_PDR           0b00000000
#define VAL_GPIOQ_SLR           0b00000000
#define VAL_GPIOQ_DEN           0b11111111
#define VAL_GPIOQ_AMSEL         0b0000
#define VAL_GPIOQ_PCTL          0x00000000

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* BOARD_H */
