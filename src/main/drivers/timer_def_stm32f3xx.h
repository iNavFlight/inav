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

#define timerDMASafeType_t  uint16_t

#define DEF_TIM_DMAMAP__D(dma, channel)         DMA_TAG(dma, 0, channel)
#define DEF_TIM_DMAMAP__NONE                    DMA_NONE

#define DEF_TIM(tim, ch, pin, usage, flags)     { tim, IO_TAG(pin), DEF_TIM_CHNL_##ch, DEF_TIM_OUTPUT(ch) | flags, IOCFG_AF_PP, DEF_TIM_AF(TCH_## tim ## _ ## ch, pin), usage, DEF_TIM_DMAMAP(0, tim ## _ ## ch) }

// AF mappings
#define DEF_TIM_AF(timch, pin)                  CONCAT(DEF_TIM_AF__, DEF_TIM_AF__ ## pin ## __ ## timch)
#define DEF_TIM_AF__D(af_n)                     GPIO_AF_ ## af_n

/* add the DMA mappings here */
// D(dma_n, channel_n)

#define DEF_TIM_DMA__BTCH_TIM1_CH1    D(1, 2)
#define DEF_TIM_DMA__BTCH_TIM1_CH2    D(1, 3)
#define DEF_TIM_DMA__BTCH_TIM1_CH4    D(1, 4)
#define DEF_TIM_DMA__BTCH_TIM1_TRIG   D(1, 4)
#define DEF_TIM_DMA__BTCH_TIM1_COM    D(1, 4)
#define DEF_TIM_DMA__BTCH_TIM1_CH3    D(1, 6)

#define DEF_TIM_DMA__BTCH_TIM2_CH3    D(1, 1)
#define DEF_TIM_DMA__BTCH_TIM2_CH1    D(1, 5)
#define DEF_TIM_DMA__BTCH_TIM2_CH2    D(1, 7)
#define DEF_TIM_DMA__BTCH_TIM2_CH4    D(1, 7)

#define DEF_TIM_DMA__BTCH_TIM3_CH2    NONE
#define DEF_TIM_DMA__BTCH_TIM3_CH3    D(1, 2)
#define DEF_TIM_DMA__BTCH_TIM3_CH4    D(1, 3)
#define DEF_TIM_DMA__BTCH_TIM3_CH1    D(1, 6)
#define DEF_TIM_DMA__BTCH_TIM3_TRIG   D(1, 6)

#define DEF_TIM_DMA__BTCH_TIM4_CH1    D(1, 1)
#define DEF_TIM_DMA__BTCH_TIM4_CH2    D(1, 4)
#define DEF_TIM_DMA__BTCH_TIM4_CH3    D(1, 5)
#define DEF_TIM_DMA__BTCH_TIM4_CH4    NONE

#define DEF_TIM_DMA__BTCH_TIM15_CH1   D(1, 5)
#define DEF_TIM_DMA__BTCH_TIM15_CH2   NONE
#define DEF_TIM_DMA__BTCH_TIM15_UP    D(1, 5)
#define DEF_TIM_DMA__BTCH_TIM15_TRIG  D(1, 5)
#define DEF_TIM_DMA__BTCH_TIM15_COM   D(1, 5)

// #ifdef REMAP_TIM16_DMA
// #define DEF_TIM_DMA__BTCH_TIM16_CH1   D(1, 6)
// #else
#define DEF_TIM_DMA__BTCH_TIM16_CH1   D(1, 3)
// #endif

// #ifdef REMAP_TIM17_DMA
// #define DEF_TIM_DMA__BTCH_TIM17_CH1   D(1, 7)
// #else
#define DEF_TIM_DMA__BTCH_TIM17_CH1   D(1, 1)
// #endif

#define DEF_TIM_DMA__BTCH_TIM8_CH3    D(2, 1)
#define DEF_TIM_DMA__BTCH_TIM8_CH4    D(2, 2)
#define DEF_TIM_DMA__BTCH_TIM8_TRIG   D(2, 2)
#define DEF_TIM_DMA__BTCH_TIM8_COM    D(2, 2)
#define DEF_TIM_DMA__BTCH_TIM8_CH1    D(2, 3)
#define DEF_TIM_DMA__BTCH_TIM8_CH2    D(2, 5)

// AF table

#define DEF_TIM_AF__PA0__TCH_TIM2_CH1     D(1)
#define DEF_TIM_AF__PA1__TCH_TIM2_CH2     D(1)
#define DEF_TIM_AF__PA2__TCH_TIM2_CH3     D(1)
#define DEF_TIM_AF__PA3__TCH_TIM2_CH4     D(1)
#define DEF_TIM_AF__PA5__TCH_TIM2_CH1     D(1)
#define DEF_TIM_AF__PA6__TCH_TIM16_CH1    D(1)
#define DEF_TIM_AF__PA7__TCH_TIM17_CH1    D(1)
#define DEF_TIM_AF__PA12__TCH_TIM16_CH1   D(1)
#define DEF_TIM_AF__PA13__TCH_TIM16_CH1N  D(1)
#define DEF_TIM_AF__PA15__TCH_TIM2_CH1    D(1)

#define DEF_TIM_AF__PA4__TCH_TIM3_CH2     D(2)
#define DEF_TIM_AF__PA6__TCH_TIM3_CH1     D(2)
#define DEF_TIM_AF__PA7__TCH_TIM3_CH2     D(2)
#define DEF_TIM_AF__PA15__TCH_TIM8_CH1    D(2)

#define DEF_TIM_AF__PA7__TCH_TIM8_CH1N    D(4)

#define DEF_TIM_AF__PA14__TCH_TIM8_CH2    D(5)

#define DEF_TIM_AF__PA7__TCH_TIM1_CH1N    D(6)
#define DEF_TIM_AF__PA8__TCH_TIM1_CH1     D(6)
#define DEF_TIM_AF__PA9__TCH_TIM1_CH2     D(6)
#define DEF_TIM_AF__PA10__TCH_TIM1_CH3    D(6)
#define DEF_TIM_AF__PA11__TCH_TIM1_CH1N   D(6)
#define DEF_TIM_AF__PA12__TCH_TIM1_CH2N   D(6)

#define DEF_TIM_AF__PA1__TCH_TIM15_CH1N   D(9)
#define DEF_TIM_AF__PA2__TCH_TIM15_CH1    D(9)
#define DEF_TIM_AF__PA3__TCH_TIM15_CH2    D(9)

#define DEF_TIM_AF__PA9__TCH_TIM2_CH3     D(10)
#define DEF_TIM_AF__PA10__TCH_TIM2_CH4    D(10)
#define DEF_TIM_AF__PA11__TCH_TIM4_CH1    D(10)
#define DEF_TIM_AF__PA12__TCH_TIM4_CH2    D(10)
#define DEF_TIM_AF__PA13__TCH_TIM4_CH3    D(10)
#define DEF_TIM_AF__PA11__TCH_TIM1_CH4    D(11)

#define DEF_TIM_AF__PB3__TCH_TIM2_CH2     D(1)
#define DEF_TIM_AF__PB4__TCH_TIM16_CH1    D(1)
#define DEF_TIM_AF__PB6__TCH_TIM16_CH1N   D(1)
#define DEF_TIM_AF__PB7__TCH_TIM17_CH1N   D(1)
#define DEF_TIM_AF__PB8__TCH_TIM16_CH1    D(1)
#define DEF_TIM_AF__PB9__TCH_TIM17_CH1    D(1)
#define DEF_TIM_AF__PB10__TCH_TIM2_CH3    D(1)
#define DEF_TIM_AF__PB11__TCH_TIM2_CH4    D(1)
#define DEF_TIM_AF__PB14__TCH_TIM15_CH1   D(1)
#define DEF_TIM_AF__PB15__TCH_TIM15_CH2   D(1)

#define DEF_TIM_AF__PB0__TCH_TIM3_CH3     D(2)
#define DEF_TIM_AF__PB1__TCH_TIM3_CH4     D(2)
#define DEF_TIM_AF__PB4__TCH_TIM3_CH1     D(2)
#define DEF_TIM_AF__PB5__TCH_TIM3_CH2     D(2)
#define DEF_TIM_AF__PB6__TCH_TIM4_CH1     D(2)
#define DEF_TIM_AF__PB7__TCH_TIM4_CH2     D(2)
#define DEF_TIM_AF__PB8__TCH_TIM4_CH3     D(2)
#define DEF_TIM_AF__PB9__TCH_TIM4_CH4     D(2)
#define DEF_TIM_AF__PB15__TCH_TIM15_CH1N  D(2)

#define DEF_TIM_AF__PB5__TCH_TIM8_CH3N    D(3)

#define DEF_TIM_AF__PB0__TCH_TIM8_CH2N    D(4)
#define DEF_TIM_AF__PB1__TCH_TIM8_CH3N    D(4)
#define DEF_TIM_AF__PB3__TCH_TIM8_CH1N    D(4)
#define DEF_TIM_AF__PB4__TCH_TIM8_CH2N    D(4)
#define DEF_TIM_AF__PB15__TCH_TIM1_CH3N   D(4)

#define DEF_TIM_AF__PB6__TCH_TIM8_CH1     D(5)

#define DEF_TIM_AF__PB0__TCH_TIM1_CH2N    D(6)
#define DEF_TIM_AF__PB1__TCH_TIM1_CH3N    D(6)
#define DEF_TIM_AF__PB13__TCH_TIM1_CH1N   D(6)
#define DEF_TIM_AF__PB14__TCH_TIM1_CH2N   D(6)

#define DEF_TIM_AF__PB5__TCH_TIM17_CH1    D(10)
#define DEF_TIM_AF__PB7__TCH_TIM3_CH4     D(10)
#define DEF_TIM_AF__PB8__TCH_TIM8_CH2     D(10)
#define DEF_TIM_AF__PB9__TCH_TIM8_CH3     D(10)

#define DEF_TIM_AF__PC6__TCH_TIM3_CH1     D(2)
#define DEF_TIM_AF__PC7__TCH_TIM3_CH2     D(2)
#define DEF_TIM_AF__PC8__TCH_TIM3_CH3     D(2)
#define DEF_TIM_AF__PC9__TCH_TIM3_CH4     D(2)

#define DEF_TIM_AF__PC6__TCH_TIM8_CH1     D(4)
#define DEF_TIM_AF__PC7__TCH_TIM8_CH2     D(4)
#define DEF_TIM_AF__PC8__TCH_TIM8_CH3     D(4)
#define DEF_TIM_AF__PC9__TCH_TIM8_CH4     D(4)

#define DEF_TIM_AF__PC10__TCH_TIM8_CH1N   D(4)
#define DEF_TIM_AF__PC11__TCH_TIM8_CH2N   D(4)
#define DEF_TIM_AF__PC12__TCH_TIM8_CH3N   D(4)
#define DEF_TIM_AF__PC13__TCH_TIM8_CH1N   D(4)

#define DEF_TIM_AF__PD3__TCH_TIM2_CH1     D(2)
#define DEF_TIM_AF__PD4__TCH_TIM2_CH2     D(2)
#define DEF_TIM_AF__PD6__TCH_TIM2_CH4     D(2)
#define DEF_TIM_AF__PD7__TCH_TIM2_CH3     D(2)

#define DEF_TIM_AF__PD12__TCH_TIM4_CH1    D(2)
#define DEF_TIM_AF__PD13__TCH_TIM4_CH2    D(2)
#define DEF_TIM_AF__PD14__TCH_TIM4_CH3    D(2)
#define DEF_TIM_AF__PD15__TCH_TIM4_CH4    D(2)

#define DEF_TIM_AF__PD1__TCH_TIM8_CH4     D(4)

#define DEF_TIM_AF__PF9__TCH_TIM15_CH1    D(3)
#define DEF_TIM_AF__PF10__TCH_TIM15_CH2   D(3)
