/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#define timerDMASafeType_t  uint32_t

// Mappings for STDLIB defines
// #define DEF_TIM_CHNL_CH1    TIM_CHANNEL_1
// #define DEF_TIM_CHNL_CH1N   TIM_CHANNEL_1
// #define DEF_TIM_CHNL_CH2    TIM_CHANNEL_2
// #define DEF_TIM_CHNL_CH2N   TIM_CHANNEL_2
// #define DEF_TIM_CHNL_CH3    TIM_CHANNEL_3
// #define DEF_TIM_CHNL_CH3N   TIM_CHANNEL_3
// #define DEF_TIM_CHNL_CH4    TIM_CHANNEL_4
// #define DEF_TIM_CHNL_CH4N   TIM_CHANNEL_4

#define DEF_TIM_DMAMAP__D(dma, stream, channel)         DMA_TAG(dma, stream, channel)
#define DEF_TIM_DMAMAP__NONE                            DMA_NONE

#define DEF_TIM(tim, ch, pin, usage, flags, dmavar)     { tim, IO_TAG(pin), DEF_TIM_CHNL_ ## ch, DEF_TIM_OUTPUT(ch) | flags, IOCFG_AF_PP, DEF_TIM_AF(TCH_## tim ## _ ## ch, pin), usage, DEF_TIM_DMAMAP(dmavar, tim ## _ ## ch) }

// AF mappings
#define DEF_TIM_AF(timch, pin)        CONCAT(DEF_TIM_AF__, DEF_TIM_AF__ ## pin ## __ ## timch)
#define DEF_TIM_AF__D(af_n, tim_n)    GPIO_AF ## af_n ## _TIM ## tim_n

/* F7 Stream Mappings */
// D(DMAx, Stream, Channel)
#define DEF_TIM_DMA__BTCH_TIM1_CH1    D(2, 6, 0),D(2, 1, 6),D(2, 3, 6)
#define DEF_TIM_DMA__BTCH_TIM1_CH2    D(2, 6, 0),D(2, 2, 6)
#define DEF_TIM_DMA__BTCH_TIM1_CH3    D(2, 6, 0),D(2, 6, 6)
#define DEF_TIM_DMA__BTCH_TIM1_CH4    D(2, 4, 6)

#define DEF_TIM_DMA__BTCH_TIM2_CH1    D(1, 5, 3)
#define DEF_TIM_DMA__BTCH_TIM2_CH2    D(1, 6, 3)
#define DEF_TIM_DMA__BTCH_TIM2_CH3    D(1, 1, 3)
#define DEF_TIM_DMA__BTCH_TIM2_CH4    D(1, 7, 3),D(1, 6, 3)

#define DEF_TIM_DMA__BTCH_TIM3_CH1    D(1, 4, 5)
#define DEF_TIM_DMA__BTCH_TIM3_CH2    D(1, 5, 5)
#define DEF_TIM_DMA__BTCH_TIM3_CH3    D(1, 7, 5)
#define DEF_TIM_DMA__BTCH_TIM3_CH4    D(1, 2, 5)

#define DEF_TIM_DMA__BTCH_TIM4_CH1    D(1, 0, 2)
#define DEF_TIM_DMA__BTCH_TIM4_CH2    D(1, 3, 2)
#define DEF_TIM_DMA__BTCH_TIM4_CH3    D(1, 7, 2)

#define DEF_TIM_DMA__BTCH_TIM5_CH1    D(1, 2, 6)
#define DEF_TIM_DMA__BTCH_TIM5_CH2    D(1, 4, 6)
#define DEF_TIM_DMA__BTCH_TIM5_CH3    D(1, 0, 6)
#define DEF_TIM_DMA__BTCH_TIM5_CH4    D(1, 1, 6),D(1, 3, 6)

#define DEF_TIM_DMA__BTCH_TIM8_CH1    D(2, 2, 7),D(2, 2, 0)
#define DEF_TIM_DMA__BTCH_TIM8_CH2    D(2, 3, 7),D(2, 2, 0)
#define DEF_TIM_DMA__BTCH_TIM8_CH3    D(2, 4, 7),D(2, 2, 0)
#define DEF_TIM_DMA__BTCH_TIM8_CH4    D(2, 7, 7)

#define DEF_TIM_DMA__BTCH_TIM4_CH4    NONE

#define DEF_TIM_DMA__BTCH_TIM9_CH1    NONE
#define DEF_TIM_DMA__BTCH_TIM9_CH2    NONE

#define DEF_TIM_DMA__BTCH_TIM10_CH1   NONE

#define DEF_TIM_DMA__BTCH_TIM11_CH1   NONE

#define DEF_TIM_DMA__BTCH_TIM12_CH1   NONE
#define DEF_TIM_DMA__BTCH_TIM12_CH2   NONE

#define DEF_TIM_DMA__BTCH_TIM13_CH1   NONE

#define DEF_TIM_DMA__BTCH_TIM14_CH1   NONE

// TIM_UP table
#define DEF_TIM_DMA__BTCH_TIM1_UP     D(2, 5, 6)
#define DEF_TIM_DMA__BTCH_TIM2_UP     D(1, 7, 3)
#define DEF_TIM_DMA__BTCH_TIM3_UP     D(1, 2, 5)
#define DEF_TIM_DMA__BTCH_TIM4_UP     D(1, 6, 2)
#define DEF_TIM_DMA__BTCH_TIM5_UP     D(1, 0, 6)
#define DEF_TIM_DMA__BTCH_TIM6_UP     D(1, 1, 7)
#define DEF_TIM_DMA__BTCH_TIM7_UP     D(1, 4, 1)
#define DEF_TIM_DMA__BTCH_TIM8_UP     D(2, 1, 7)
#define DEF_TIM_DMA__BTCH_TIM9_UP     NONE
#define DEF_TIM_DMA__BTCH_TIM10_UP    NONE
#define DEF_TIM_DMA__BTCH_TIM11_UP    NONE
#define DEF_TIM_DMA__BTCH_TIM12_UP    NONE
#define DEF_TIM_DMA__BTCH_TIM13_UP    NONE
#define DEF_TIM_DMA__BTCH_TIM14_UP    NONE

// AF table

//PORTA
#define DEF_TIM_AF__PA0__TCH_TIM2_CH1     D(1, 2)
#define DEF_TIM_AF__PA1__TCH_TIM2_CH2     D(1, 2)
#define DEF_TIM_AF__PA2__TCH_TIM2_CH3     D(1, 2)
#define DEF_TIM_AF__PA3__TCH_TIM2_CH4     D(1, 2)
#define DEF_TIM_AF__PA5__TCH_TIM2_CH1     D(1, 2)
#define DEF_TIM_AF__PA7__TCH_TIM1_CH1N    D(1, 1)
#define DEF_TIM_AF__PA8__TCH_TIM1_CH1     D(1, 1)
#define DEF_TIM_AF__PA9__TCH_TIM1_CH2     D(1, 1)
#define DEF_TIM_AF__PA10__TCH_TIM1_CH3    D(1, 1)
#define DEF_TIM_AF__PA11__TCH_TIM1_CH1N   D(1, 1)
#define DEF_TIM_AF__PA15__TCH_TIM2_CH1    D(1, 2)

#define DEF_TIM_AF__PA0__TCH_TIM5_CH1     D(2, 5)
#define DEF_TIM_AF__PA1__TCH_TIM5_CH2     D(2, 5)
#define DEF_TIM_AF__PA2__TCH_TIM5_CH3     D(2, 5)
#define DEF_TIM_AF__PA3__TCH_TIM5_CH4     D(2, 5)
#define DEF_TIM_AF__PA6__TCH_TIM3_CH1     D(2, 3)
#define DEF_TIM_AF__PA7__TCH_TIM3_CH2     D(2, 3)

#define DEF_TIM_AF__PA2__TCH_TIM9_CH1     D(3, 9)
#define DEF_TIM_AF__PA3__TCH_TIM9_CH2     D(3, 9)
#define DEF_TIM_AF__PA5__TCH_TIM8_CH1N    D(3, 8)
#define DEF_TIM_AF__PA7__TCH_TIM8_CH1N    D(3, 8)

#define DEF_TIM_AF__PA6__TCH_TIM13_CH1    D(9, 13)
#define DEF_TIM_AF__PA7__TCH_TIM14_CH1    D(9, 14)

//PORTB
#define DEF_TIM_AF__PB0__TCH_TIM1_CH2N    D(1, 1)
#define DEF_TIM_AF__PB1__TCH_TIM1_CH3N    D(1, 1)
#define DEF_TIM_AF__PB3__TCH_TIM2_CH2     D(1, 2)
#define DEF_TIM_AF__PB10__TCH_TIM2_CH3    D(1, 2)
#define DEF_TIM_AF__PB11__TCH_TIM2_CH4    D(1, 2)
#define DEF_TIM_AF__PB13__TCH_TIM1_CH1N   D(1, 1)
#define DEF_TIM_AF__PB14__TCH_TIM1_CH2N   D(1, 1)
#define DEF_TIM_AF__PB15__TCH_TIM1_CH3N   D(1, 1)

#define DEF_TIM_AF__PB0__TCH_TIM3_CH3     D(2, 3)
#define DEF_TIM_AF__PB1__TCH_TIM3_CH4     D(2, 3)
#define DEF_TIM_AF__PB4__TCH_TIM3_CH1     D(2, 3)
#define DEF_TIM_AF__PB5__TCH_TIM3_CH2     D(2, 3)
#define DEF_TIM_AF__PB6__TCH_TIM4_CH1     D(2, 4)
#define DEF_TIM_AF__PB7__TCH_TIM4_CH2     D(2, 4)
#define DEF_TIM_AF__PB8__TCH_TIM4_CH3     D(2, 4)
#define DEF_TIM_AF__PB9__TCH_TIM4_CH4     D(2, 4)

#define DEF_TIM_AF__PB0__TCH_TIM8_CH2N    D(3, 8)
#define DEF_TIM_AF__PB1__TCH_TIM8_CH3N    D(3, 8)
#define DEF_TIM_AF__PB8__TCH_TIM10_CH1    D(3, 10)
#define DEF_TIM_AF__PB9__TCH_TIM11_CH1    D(3, 11)
#define DEF_TIM_AF__PB14__TCH_TIM8_CH2N   D(3, 8)
#define DEF_TIM_AF__PB15__TCH_TIM8_CH3N   D(3, 8)

#define DEF_TIM_AF__PB14__TCH_TIM12_CH1   D(9, 12)
#define DEF_TIM_AF__PB15__TCH_TIM12_CH2   D(9, 12)

//PORTC
#define DEF_TIM_AF__PC6__TCH_TIM3_CH1     D(2, 3)
#define DEF_TIM_AF__PC7__TCH_TIM3_CH2     D(2, 3)
#define DEF_TIM_AF__PC8__TCH_TIM3_CH3     D(2, 3)
#define DEF_TIM_AF__PC9__TCH_TIM3_CH4     D(2, 3)

#define DEF_TIM_AF__PC6__TCH_TIM8_CH1     D(3, 8)
#define DEF_TIM_AF__PC7__TCH_TIM8_CH2     D(3, 8)
#define DEF_TIM_AF__PC8__TCH_TIM8_CH3     D(3, 8)
#define DEF_TIM_AF__PC9__TCH_TIM8_CH4     D(3, 8)

//PORTD
#define DEF_TIM_AF__PD12__TCH_TIM4_CH1    D(2, 4)
#define DEF_TIM_AF__PD13__TCH_TIM4_CH2    D(2, 4)
#define DEF_TIM_AF__PD14__TCH_TIM4_CH3    D(2, 4)
#define DEF_TIM_AF__PD15__TCH_TIM4_CH4    D(2, 4)

//PORTE
#define DEF_TIM_AF__PE8__TCH_TIM1_CH1N    D(1, 1)
#define DEF_TIM_AF__PE9__TCH_TIM1_CH1     D(1, 1)
#define DEF_TIM_AF__PE10__TCH_TIM1_CH2N   D(1, 1)
#define DEF_TIM_AF__PE11__TCH_TIM1_CH2    D(1, 1)
#define DEF_TIM_AF__PE12__TCH_TIM1_CH3N   D(1, 1)
#define DEF_TIM_AF__PE13__TCH_TIM1_CH3    D(1, 1)
#define DEF_TIM_AF__PE14__TCH_TIM1_CH4    D(1, 1)

#define DEF_TIM_AF__PE5__TCH_TIM9_CH1     D(3, 9)
#define DEF_TIM_AF__PE6__TCH_TIM9_CH2     D(3, 9)

//PORTF
#define DEF_TIM_AF__PF6__TCH_TIM10_CH1    D(3, 10)
#define DEF_TIM_AF__PF7__TCH_TIM11_CH1    D(3, 11)

//PORTH
#define DEF_TIM_AF__PH10__TCH_TIM5_CH1    D(2, 5)
#define DEF_TIM_AF__PH11__TCH_TIM5_CH2    D(2, 5)
#define DEF_TIM_AF__PH12__TCH_TIM5_CH3    D(2, 5)

#define DEF_TIM_AF__PH13__TCH_TIM8_CH1N   D(3, 8)
#define DEF_TIM_AF__PH14__TCH_TIM8_CH2N   D(3, 8)
#define DEF_TIM_AF__PH15__TCH_TIM8_CH3N   D(3, 8)

#define DEF_TIM_AF__PH6__TCH_TIM12_CH1    D(9, 12)
#define DEF_TIM_AF__PH9__TCH_TIM12_CH2    D(9, 12)

//PORTI
#define DEF_TIM_AF__PI0__TCH_TIM5_CH4     D(2, 5)

#define DEF_TIM_AF__PI2__TCH_TIM8_CH4     D(3, 8)
#define DEF_TIM_AF__PI5__TCH_TIM8_CH1     D(3, 8)
#define DEF_TIM_AF__PI6__TCH_TIM8_CH2     D(3, 8)
#define DEF_TIM_AF__PI7__TCH_TIM8_CH3     D(3, 8)
