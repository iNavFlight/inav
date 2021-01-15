/*
    ChibiOS - Copyright (C) 2016 Andrew Wygle aka awygle

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

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * Setup for the EXP430FR6989 LaunchPad board
 */

/* NOTE: LCD segment pins configured as unused - controlled by LCD driver if
 * present
 */

/*
 * Board identifier.
 */
#define BOARD_EXP430FR6989
#define BOARD_NAME "MSP430FR6989 LaunchPad"

/*
 * IO lines assignments.
 */
#define LINE_LED_R                                PAL_LINE(IOPORT1, 0U)
#define LINE_LED_G                                PAL_LINE(IOPORT5, 7U)
#define LINE_SW_S1                                PAL_LINE(IOPORT1, 1U)
#define LINE_SW_S2                                PAL_LINE(IOPORT1, 2U)

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 * Please refer to the MSP430X Family Users Guide for details.
 */
/*
 * Port A setup:
 * 
 * P1.0 - Red LED                 (output low)
 * P1.1 - Switch S1               (input pullup falling-edge interrupt)
 * P1.2 - Switch S2               (input pullup falling-edge interrupt)
 * P1.3 - BoosterPack BP34        (input pullup)
 * P1.4 - BoosterPack BP7         (input pullup)
 * P1.5 - BoosterPack BP18        (input pullup)
 * P1.6 - BoosterPack BP15        (input pullup)
 * P1.7 - BoosterPack BP14        (input pullup)
 * P2.0 - BoosterPack BP8         (input pullup)
 * P2.1 - BoosterPack BP19        (input pullup)
 * P2.2 - BoosterPack BP35        (input pullup)
 * P2.3 - BoosterPack BP31        (input pullup)
 * P2.4 - BoosterPack BP12        (input pullup)
 * P2.5 - BoosterPack BP13        (input pullup)
 * P2.6 - BoosterPack BP39        (input pullup)
 * P2.7 - BoosterPack BP40        (input pullup)
 */
#define VAL_IOPORT1_OUT   0xFFFE
#define VAL_IOPORT1_DIR   0x0001
#define VAL_IOPORT1_REN   0xFFFE
#define VAL_IOPORT1_SEL0  0x0000
#define VAL_IOPORT1_SEL1  0x0000

/*
 * Port B setup:
 * 
 * P3.0 - BoosterPack BP33        (input pullup)
 * P3.1 - BoosterPack BP32        (input pullup)
 * P3.2 - BoosterPack BP5         (input pullup)
 * P3.3 - BoosterPack BP38        (input pullup)
 * P3.4 - Application UART TX     (alternate 1)
 * P3.5 - Application UART RX     (alternate 1)
 * P3.6 - BoosterPack BP37        (input pullup)
 * P3.7 - BoosterPack BP36        (input pullup)
 * P4.0 - BoosterPack BP10        (input pullup)
 * P4.1 - BoosterPack BP9         (input pullup)
 * P4.2 - BoosterPack BP4         (input pullup)
 * P4.3 - BoosterPack BP3         (input pullup)
 * P4.4 - LCD S8                  (input pullup)
 * P4.5 - LCD S7                  (input pullup)
 * P4.6 - LCD S6                  (input pullup)
 * P4.7 - BoosterPack BP11        (input pullup)
 */
#define VAL_IOPORT2_OUT   0xFFCF
#define VAL_IOPORT2_DIR   0x0000
#define VAL_IOPORT2_REN   0xFFCF
#define VAL_IOPORT2_SEL0  0x0030
#define VAL_IOPORT2_SEL1  0x0000

/*
 * Port C setup:
 * 
 * P5.0 - LCD S38                 (input pullup)
 * P5.1 - LCD S37                 (input pullup)
 * P5.2 - LCD S36                 (input pullup)
 * P5.3 - LCD S35                 (input pullup)
 * P5.4 - LCD S12                 (input pullup)
 * P5.5 - LCD S11                 (input pullup)
 * P5.6 - LCD S10                 (input pullup)
 * P5.7 - LCD S9                  (input pullup)
 * P6.0 - LCD R23                 (input pullup)
 * P6.1 - LCD R13                 (input pullup)
 * P6.2 - LCD R03                 (input pullup)
 * P6.3 - LCD COM0                (input pullup)
 * P6.4 - LCD COM1                (input pullup)
 * P6.5 - LCD COM2                (input pullup)
 * P6.6 - LCD COM3                (input pullup)
 * P6.7 - LCD S31                 (input pullup)
 */
#define VAL_IOPORT3_OUT   0xFFFF
#define VAL_IOPORT3_DIR   0x0000
#define VAL_IOPORT3_REN   0xFFFF
#define VAL_IOPORT3_SEL0  0x0000
#define VAL_IOPORT3_SEL1  0x0000

/*
 * Port D setup:
 * 
 * P7.0 - LCD S17                 (input pullup)
 * P7.1 - LCD S16                 (input pullup)
 * P7.2 - LCD S15                 (input pullup)
 * P7.3 - LCD S14                 (input pullup)
 * P7.4 - LCD S13                 (input pullup)
 * P7.5 - LCD S30                 (input pullup)
 * P7.6 - LCD S29                 (input pullup)
 * P7.7 - LCD S27                 (input pullup)
 * P8.0 - LCD S21                 (input pullup)
 * P8.1 - LCD S20                 (input pullup)
 * P8.2 - LCD S19                 (input pullup)
 * P8.3 - LCD S18                 (input pullup)
 * P8.4 - BoosterPack BP23        (input pullup)
 * P8.5 - BoosterPack BP24        (input pullup)
 * P8.6 - BoosterPack BP25        (input pullup)
 * P8.7 - BoosterPack BP26        (input pullup)
 */
#define VAL_IOPORT4_OUT   0xFFFF
#define VAL_IOPORT4_DIR   0x0000
#define VAL_IOPORT4_REN   0xFFFF
#define VAL_IOPORT4_SEL0  0x0000
#define VAL_IOPORT4_SEL1  0x0000

/*
 * Port E setup:
 * 
 * P9.0 - BoosterPack BP27        (input pullup)
 * P9.1 - BoosterPack BP28        (input pullup)
 * P9.2 - BoosterPack BP2         (input pullup)
 * P9.3 - BoosterPack BP6         (input pullup)
 * P9.4 - BoosterPack BP17        (input pullup)
 * P9.5 - BoosterPack BP29        (input pullup)
 * P9.6 - BoosterPack BP30        (input pullup)
 * P9.7 - Green LED               (output low)
 * P10.0 - LCD S4                 (input pullup)
 * P10.1 - LCD S28                (input pullup)
 * P10.2 - LCD S39                (input pullup)
 * P10.3 - N/C Internally         (input pullup)
 * P10.4 - N/C Internally         (input pullup)
 * P10.5 - N/C Internally         (input pullup)
 * P10.6 - N/C Internally         (input pullup)
 * P10.7 - N/C Internally         (input pullup)
 */
#define VAL_IOPORT5_OUT   0xFF7F
#define VAL_IOPORT5_DIR   0x0080
#define VAL_IOPORT5_REN   0xFF7F
#define VAL_IOPORT5_SEL0  0x0000
#define VAL_IOPORT5_SEL1  0x0000

/*
 * Port J setup:
 * 
 * PJ.0 - TDO                     (input pullup)
 * PJ.1 - TDI                     (input pullup)
 * PJ.2 - TMS                     (input pullup)
 * PJ.3 - TCK                     (input pullup)
 * PJ.4 - LFXIN                   (alternate 1)
 * PJ.5 - LFXOUT                  (alternate 1)
 * PJ.6 - HFXIN (N/C)             (input pullup)
 * PJ.7 - HFXOUT (N/C)            (input pullup)
 */
#define VAL_IOPORT0_OUT   0x00FF
#define VAL_IOPORT0_DIR   0x0000
#define VAL_IOPORT0_REN   0x00CF
#define VAL_IOPORT0_SEL0  0x0030
#define VAL_IOPORT0_SEL1  0x0000

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _BOARD_H_ */
