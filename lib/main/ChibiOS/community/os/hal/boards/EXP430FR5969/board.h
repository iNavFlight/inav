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
 * Setup for the EXP430FR5969 LaunchPad board
 */

/*
 * Board identifier.
 */
#define BOARD_EXP430FR5959
#define BOARD_NAME "MSP430FR5969 LaunchPad"

/*
 * IO lines assignments.
 */
#define LINE_LED_G                                PAL_LINE(IOPORT1, 0U)
#define LINE_LED_R                                PAL_LINE(IOPORT2, 14U)
#define LINE_SW_S1                                PAL_LINE(IOPORT2, 13U)
#define LINE_SW_S2                                PAL_LINE(IOPORT1, 1U)

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 * Please refer to the MSP430X Family Users Guide for details.
 */
/*
 * Port A setup:
 * 
 * P1.0 - Green LED               (output low)
 * P1.1 - Switch S2               (input pullup)
 * P1.2 - BoosterPack BP19        (input pullup)
 * P1.3 - BoosterPack BP11        (input pullup)
 * P1.4 - BoosterPack BP12        (input pullup)
 * P1.5 - BoosterPack BP13        (input pullup)
 * P1.6 - BoosterPack BP15        (input pullup)
 * P1.7 - BoosterPack BP14        (input pullup)
 * P2.0 - Application UART TX     (alternate 2)
 * P2.1 - Application UART RX     (alternate 2)
 * P2.2 - BoosterPack BP7         (input pullup)
 * P2.3 - N/C                     (input pullup)
 * P2.4 - BoosterPack BP6         (input pullup)
 * P2.5 - BoosterPack BP4         (input pullup)
 * P2.6 - BoosterPack BP3         (input pullup)
 * P2.7 - N/C                     (input pullup)
 */
#define VAL_IOPORT1_OUT   0xFCFE
#define VAL_IOPORT1_DIR   0x0001
#define VAL_IOPORT1_REN   0xFCFE
#define VAL_IOPORT1_SEL0  0x0000
#define VAL_IOPORT1_SEL1  0x0300

/*
 * Port B setup:
 * 
 * P3.0 - BoosterPack BP18        (input pullup)
 * P3.1 - N/C                     (input pullup)
 * P3.2 - N/C                     (input pullup)
 * P3.3 - N/C                     (input pullup)
 * P3.4 - BoosterPack BP8         (input pullup)
 * P3.5 - BoosterPack BP9         (input pullup)
 * P3.6 - BoosterPack BP10        (input pullup)
 * P3.7 - N/C                     (input pullup)
 * P4.0 - Application UART CTS    (input pullup)
 * P4.1 - Application UART RTS    (output high)
 * P4.2 - BoosterPack BP2         (input pullup)
 * P4.3 - BoosterPack BP5         (input pullup)
 * P4.4 - N/C                     (input pullup)
 * P4.5 - Switch S1               (input pullup)
 * P4.6 - Red LED                 (output low)
 * P4.7 - N/C                     (input pullup)
 */
#define VAL_IOPORT2_OUT   0xBFFF
#define VAL_IOPORT2_DIR   0x4200
#define VAL_IOPORT2_REN   0xBDFF
#define VAL_IOPORT2_SEL0  0x0000
#define VAL_IOPORT2_SEL1  0x0000

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
