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

#define DEF_TIM_DMAMAP__D(dma, stream, channel)         DMA_TAG(dma, stream, channel)
#define DEF_TIM_DMAMAP__NONE                            DMA_NONE

// Define TIM device/DMA/MUX 
#define DEF_TIM(tim, ch, pin, usage, flags, dmavar)     {               \
     tim,                                                               \
     IO_TAG(pin),                                                       \
     DEF_TIM_CHNL_ ## ch,                                               \
     DEF_TIM_OUTPUT(ch) | flags,                                        \
     IOCFG_AF_PP,                                                       \
     DEF_TIM_AF(TCH_## tim ## _ ## ch, pin),                            \
     usage,                                                             \
     DEF_TIM_DMAMAP(dmavar, tim ## _ ## ch),                            \
     DEF_TIM_DMA_REQUEST(tim ## _ ## ch)                                \
  }

// AF mappings
#define DEF_TIM_AF(timch, pin)        CONCAT(DEF_TIM_AF__, DEF_TIM_AF__ ## pin ## __ ## timch)
// MUX mappings
#define DEF_TIM_AF__D(af_n, tim_n)    GPIO_MUX_ ## af_n

#define DEF_TIM_DMA_REQUEST(timch) \
    CONCAT(DEF_TIM_DMA_REQ__, DEF_TIM_TCH2BTCH(timch))


/* add the DMA mappings here */
// D(DMAx, Stream, Channel)
// at32f43x has DMAMUX that allow arbitrary assignment of peripherals to streams.
#define DEF_TIM_DMA_FULL \
    D(1, 1, 0), D(1, 2, 0), D(1, 3, 0), D(1, 4, 0), D(1, 5, 0), D(1, 6, 0), D(1, 7, 0), \
    D(2, 1, 0), D(2, 2, 0), D(2, 3, 0), D(2, 4, 0), D(2, 5, 0), D(2, 6, 0), D(2, 7, 0)

#define DEF_TIM_DMA__BTCH_TMR1_CH1    DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR1_CH2    DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR1_CH3    DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR1_CH4    DEF_TIM_DMA_FULL

#define DEF_TIM_DMA__BTCH_TMR2_CH1    DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR2_CH2    DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR2_CH3    DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR2_CH4    DEF_TIM_DMA_FULL

#define DEF_TIM_DMA__BTCH_TMR3_CH1    DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR3_CH2    DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR3_CH3    DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR3_CH4    DEF_TIM_DMA_FULL

#define DEF_TIM_DMA__BTCH_TMR4_CH1    DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR4_CH2    DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR4_CH3    DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR4_CH4    DEF_TIM_DMA_FULL

#define DEF_TIM_DMA__BTCH_TMR5_CH1    DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR5_CH2    DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR5_CH3    DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR5_CH4    DEF_TIM_DMA_FULL

#define DEF_TIM_DMA__BTCH_TMR8_CH1    DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR8_CH2    DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR8_CH3    DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR8_CH4    DEF_TIM_DMA_FULL

#define DEF_TIM_DMA__BTCH_TMR15_CH1   DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR15_CH2   NONE

#define DEF_TIM_DMA__BTCH_TMR16_CH1   DEF_TIM_DMA_FULL

#define DEF_TIM_DMA__BTCH_TMR17_CH1   DEF_TIM_DMA_FULL

#define DEF_TIM_DMA__BTCH_TMR20_CH1   DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR20_CH2   DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR20_CH3   DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR20_CH4   DEF_TIM_DMA_FULL

#define DEF_TIM_DMA__BTCH_TMR9_CH1    NONE
#define DEF_TIM_DMA__BTCH_TMR9_CH2    NONE

#define DEF_TIM_DMA__BTCH_TMR10_CH1   NONE
#define DEF_TIM_DMA__BTCH_TMR11_CH1   NONE

#define DEF_TIM_DMA__BTCH_TMR12_CH1   NONE
#define DEF_TIM_DMA__BTCH_TMR12_CH2   NONE
#define DEF_TIM_DMA__BTCH_TMR13_CH1   NONE
#define DEF_TIM_DMA__BTCH_TMR14_CH1   NONE

// TIM_UP table
#define DEF_TIM_DMA__BTCH_TMR1_UP     DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR2_UP     DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR3_UP     DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR4_UP     DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR5_UP     DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR6_UP     DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR7_UP     DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR8_UP     DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR9_UP     DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR10_UP    DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR11_UP    DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR12_UP    DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR13_UP    DEF_TIM_DMA_FULL
#define DEF_TIM_DMA__BTCH_TMR14_UP    DEF_TIM_DMA_FULL

#define DMA_REQUEST_NONE 255

#define DEF_TIM_DMA_REQ__BTCH_TMR1_CH1    DMAMUX_DMAREQ_ID_TMR1_CH1
#define DEF_TIM_DMA_REQ__BTCH_TMR1_CH2    DMAMUX_DMAREQ_ID_TMR1_CH2
#define DEF_TIM_DMA_REQ__BTCH_TMR1_CH3    DMAMUX_DMAREQ_ID_TMR1_CH3
#define DEF_TIM_DMA_REQ__BTCH_TMR1_CH4    DMAMUX_DMAREQ_ID_TMR1_CH4

#define DEF_TIM_DMA_REQ__BTCH_TMR2_CH1    DMAMUX_DMAREQ_ID_TMR2_CH1
#define DEF_TIM_DMA_REQ__BTCH_TMR2_CH2    DMAMUX_DMAREQ_ID_TMR2_CH2
#define DEF_TIM_DMA_REQ__BTCH_TMR2_CH3    DMAMUX_DMAREQ_ID_TMR2_CH3
#define DEF_TIM_DMA_REQ__BTCH_TMR2_CH4    DMAMUX_DMAREQ_ID_TMR2_CH4

#define DEF_TIM_DMA_REQ__BTCH_TMR3_CH1    DMAMUX_DMAREQ_ID_TMR3_CH1
#define DEF_TIM_DMA_REQ__BTCH_TMR3_CH2    DMAMUX_DMAREQ_ID_TMR3_CH2
#define DEF_TIM_DMA_REQ__BTCH_TMR3_CH3    DMAMUX_DMAREQ_ID_TMR3_CH3
#define DEF_TIM_DMA_REQ__BTCH_TMR3_CH4    DMAMUX_DMAREQ_ID_TMR3_CH4

#define DEF_TIM_DMA_REQ__BTCH_TMR4_CH1    DMAMUX_DMAREQ_ID_TMR4_CH1
#define DEF_TIM_DMA_REQ__BTCH_TMR4_CH2    DMAMUX_DMAREQ_ID_TMR4_CH2
#define DEF_TIM_DMA_REQ__BTCH_TMR4_CH3    DMAMUX_DMAREQ_ID_TMR4_CH3
#define DEF_TIM_DMA_REQ__BTCH_TMR4_CH4    DMAMUX_DMAREQ_ID_TMR4_CH4

#define DEF_TIM_DMA_REQ__BTCH_TMR5_CH1    DMAMUX_DMAREQ_ID_TMR5_CH1
#define DEF_TIM_DMA_REQ__BTCH_TMR5_CH2    DMAMUX_DMAREQ_ID_TMR5_CH2
#define DEF_TIM_DMA_REQ__BTCH_TMR5_CH3    DMAMUX_DMAREQ_ID_TMR5_CH3
#define DEF_TIM_DMA_REQ__BTCH_TMR5_CH4    DMAMUX_DMAREQ_ID_TMR5_CH4

#define DEF_TIM_DMA_REQ__BTCH_TMR8_CH1    DMAMUX_DMAREQ_ID_TMR8_CH1
#define DEF_TIM_DMA_REQ__BTCH_TMR8_CH2    DMAMUX_DMAREQ_ID_TMR8_CH2
#define DEF_TIM_DMA_REQ__BTCH_TMR8_CH3    DMAMUX_DMAREQ_ID_TMR8_CH3
#define DEF_TIM_DMA_REQ__BTCH_TMR8_CH4    DMAMUX_DMAREQ_ID_TMR8_CH4

#define DEF_TIM_DMA_REQ__BTCH_TMR20_CH1   DMAMUX_DMAREQ_ID_TMR20_CH1
#define DEF_TIM_DMA_REQ__BTCH_TMR20_CH2   DMAMUX_DMAREQ_ID_TMR20_CH2
#define DEF_TIM_DMA_REQ__BTCH_TMR20_CH3   DMAMUX_DMAREQ_ID_TMR20_CH3
#define DEF_TIM_DMA_REQ__BTCH_TMR20_CH4   DMAMUX_DMAREQ_ID_TMR20_CH4

// TIM_UP request table
#define DEF_TIM_DMA_REQ__BTCH_TMR1_UP     DMAMUX_DMAREQ_ID_TMR1_OVERFLOW
#define DEF_TIM_DMA_REQ__BTCH_TMR2_UP     DMAMUX_DMAREQ_ID_TMR2_OVERFLOW
#define DEF_TIM_DMA_REQ__BTCH_TMR3_UP     DMAMUX_DMAREQ_ID_TMR3_OVERFLOW
#define DEF_TIM_DMA_REQ__BTCH_TMR4_UP     DMAMUX_DMAREQ_ID_TMR4_OVERFLOW
#define DEF_TIM_DMA_REQ__BTCH_TMR5_UP     DMAMUX_DMAREQ_ID_TMR5_OVERFLOW
#define DEF_TIM_DMA_REQ__BTCH_TMR8_UP     DMAMUX_DMAREQ_ID_TMR8_OVERFLOW
#define DEF_TIM_DMA_REQ__BTCH_TMR20_UP    DMAMUX_DMAREQ_ID_TMR20_OVERFLOW


// AF table 
//GPIOA
#define DEF_TIM_AF__PA0__TCH_TMR2_CH1     D(1, 2)    //A MUX1 1
#define DEF_TIM_AF__PA1__TCH_TMR2_CH2     D(1, 2)    //A MUX1 2
#define DEF_TIM_AF__PA2__TCH_TMR2_CH3     D(1, 2)       
#define DEF_TIM_AF__PA3__TCH_TMR2_CH4     D(1, 2)
#define DEF_TIM_AF__PA5__TCH_TMR2_CH1     D(1, 2)
#define DEF_TIM_AF__PA7__TCH_TMR1_CH1N    D(1, 1)
#define DEF_TIM_AF__PA8__TCH_TMR1_CH1     D(1, 1)
#define DEF_TIM_AF__PA9__TCH_TMR1_CH2     D(1, 1)
#define DEF_TIM_AF__PA10__TCH_TMR1_CH3    D(1, 1)
#define DEF_TIM_AF__PA11__TCH_TMR1_CH4    D(1, 1)
#define DEF_TIM_AF__PA15__TCH_TMR2_CH1    D(1, 2)

#define DEF_TIM_AF__PA0__TCH_TMR5_CH1     D(2, 5)
#define DEF_TIM_AF__PA1__TCH_TMR5_CH2     D(2, 5)
#define DEF_TIM_AF__PA2__TCH_TMR5_CH3     D(2, 5)
#define DEF_TIM_AF__PA3__TCH_TMR5_CH4     D(2, 5)
#define DEF_TIM_AF__PA6__TCH_TMR3_CH1     D(2, 3)
#define DEF_TIM_AF__PA7__TCH_TMR3_CH2     D(2, 3)

#define DEF_TIM_AF__PA2__TCH_TMR9_CH1     D(3, 9)
#define DEF_TIM_AF__PA3__TCH_TMR9_CH2     D(3, 9)
#define DEF_TIM_AF__PA5__TCH_TMR8_CH1N    D(3, 8)
#define DEF_TIM_AF__PA7__TCH_TMR8_CH1N    D(3, 8)

#define DEF_TIM_AF__PA6__TCH_TMR13_CH1    D(9, 13)
#define DEF_TIM_AF__PA7__TCH_TMR14_CH1    D(9, 14)

//GPIOB
#define DEF_TIM_AF__PB0__TCH_TMR1_CH2N    D(1, 1)
#define DEF_TIM_AF__PB1__TCH_TMR1_CH3N    D(1, 1)
#define DEF_TIM_AF__PB2__TCH_TMR2_CH4     D(1, 2)
#define DEF_TIM_AF__PB3__TCH_TMR2_CH2     D(1, 2)
#define DEF_TIM_AF__PB8__TCH_TMR2_CH1     D(1, 2)
#define DEF_TIM_AF__PB9__TCH_TMR2_CH2     D(1, 2)
#define DEF_TIM_AF__PB10__TCH_TMR2_CH3    D(1, 2)
#define DEF_TIM_AF__PB11__TCH_TMR2_CH4    D(1, 2)
#define DEF_TIM_AF__PB13__TCH_TMR1_CH1N   D(1, 1)
#define DEF_TIM_AF__PB14__TCH_TMR1_CH2N   D(1, 1)
#define DEF_TIM_AF__PB15__TCH_TMR1_CH3N   D(1, 1)

#define DEF_TIM_AF__PB0__TCH_TMR3_CH3     D(2, 3)
#define DEF_TIM_AF__PB1__TCH_TMR3_CH4     D(2, 3)
#define DEF_TIM_AF__PB2__TCH_TMR20_CH1    D(2, 20)
#define DEF_TIM_AF__PB4__TCH_TMR3_CH1     D(2, 3)
#define DEF_TIM_AF__PB5__TCH_TMR3_CH2     D(2, 3)
#define DEF_TIM_AF__PB6__TCH_TMR4_CH1     D(2, 4)
#define DEF_TIM_AF__PB7__TCH_TMR4_CH2     D(2, 4)
#define DEF_TIM_AF__PB8__TCH_TMR4_CH3     D(2, 4)
#define DEF_TIM_AF__PB9__TCH_TMR4_CH4     D(2, 4)
#define DEF_TIM_AF__PB11__TCH_TMR5_CH4     D(2, 5)
#define DEF_TIM_AF__PB12__TCH_TMR5_CH1     D(2, 5)

#define DEF_TIM_AF__PB0__TCH_TMR8_CH2N    D(3, 8)
#define DEF_TIM_AF__PB1__TCH_TMR8_CH3N    D(3, 8)
#define DEF_TIM_AF__PB8__TCH_TMR10_CH1    D(3, 10)
#define DEF_TIM_AF__PB9__TCH_TMR11_CH1    D(3, 11)
#define DEF_TIM_AF__PB14__TCH_TMR8_CH2N   D(3, 8)
#define DEF_TIM_AF__PB15__TCH_TMR8_CH3N   D(3, 8)

#define DEF_TIM_AF__PB14__TCH_TMR12_CH1   D(9, 12)
#define DEF_TIM_AF__PB15__TCH_TMR12_CH2   D(9, 12)

//GPIOC
#define DEF_TIM_AF__PC2__TCH_TMR20_CH2    D(2, 20)
#define DEF_TIM_AF__PC6__TCH_TMR3_CH1     D(2, 3)
#define DEF_TIM_AF__PC7__TCH_TMR3_CH2     D(2, 3)
#define DEF_TIM_AF__PC8__TCH_TMR3_CH3     D(2, 3)
#define DEF_TIM_AF__PC9__TCH_TMR3_CH4     D(2, 3)
#define DEF_TIM_AF__PC10__TCH_TMR5_CH2    D(2, 5)
#define DEF_TIM_AF__PC11__TCH_TMR5_CH3    D(2, 5)

#define DEF_TIM_AF__PC4__TCH_TMR9_CH1     D(3, 9)
#define DEF_TIM_AF__PC5__TCH_TMR9_CH2     D(3, 9)
#define DEF_TIM_AF__PC6__TCH_TMR8_CH1     D(3, 8)
#define DEF_TIM_AF__PC7__TCH_TMR8_CH2     D(3, 8)
#define DEF_TIM_AF__PC8__TCH_TMR8_CH3     D(3, 8)
#define DEF_TIM_AF__PC9__TCH_TMR8_CH4     D(3, 8)
#define DEF_TIM_AF__PC12__TCH_TMR11_CH1   D(3, 11)

//GPIOD
#define DEF_TIM_AF__PD12__TCH_TMR4_CH1    D(2, 4)
#define DEF_TIM_AF__PD13__TCH_TMR4_CH2    D(2, 4)
#define DEF_TIM_AF__PD14__TCH_TMR4_CH3    D(2, 4)
#define DEF_TIM_AF__PD15__TCH_TMR4_CH4    D(2, 4)

//GPIOE
#define DEF_TIM_AF__PE8__TCH_TMR1_CH1N    D(1, 1)
#define DEF_TIM_AF__PE9__TCH_TMR1_CH1     D(1, 1)
#define DEF_TIM_AF__PE10__TCH_TMR1_CH2N   D(1, 1)
#define DEF_TIM_AF__PE11__TCH_TMR1_CH2    D(1, 1)
#define DEF_TIM_AF__PE12__TCH_TMR1_CH3N   D(1, 1)
#define DEF_TIM_AF__PE13__TCH_TMR1_CH3    D(1, 1)
#define DEF_TIM_AF__PE14__TCH_TMR1_CH4    D(1, 1)

#define DEF_TIM_AF__PE3__TCH_TMR3_CH1     D(3, 3)
#define DEF_TIM_AF__PE4__TCH_TMR3_CH2     D(3, 3)
#define DEF_TIM_AF__PE5__TCH_TMR3_CH3     D(3, 3)
#define DEF_TIM_AF__PE6__TCH_TMR3_CH4     D(3, 3)

#define DEF_TIM_AF__PE5__TCH_TMR9_CH1     D(3, 9)
#define DEF_TIM_AF__PE6__TCH_TMR9_CH2     D(3, 9)

#define DEF_TIM_AF__PE1__TCH_TMR20_CH4     D(6, 20)
#define DEF_TIM_AF__PE2__TCH_TMR20_CH1     D(6, 20)
#define DEF_TIM_AF__PE3__TCH_TMR20_CH2     D(6, 20)
#define DEF_TIM_AF__PE4__TCH_TMR20_CH1N     D(6, 20)
#define DEF_TIM_AF__PE5__TCH_TMR20_CH2N     D(6, 20)
#define DEF_TIM_AF__PE6__TCH_TMR20_CH3N     D(6, 20)

//GPIOF
#define DEF_TIM_AF__PF2__TCH_TMR20_CH3      D(2, 20)
#define DEF_TIM_AF__PF3__TCH_TMR20_CH4      D(2, 20)
#define DEF_TIM_AF__PF4__TCH_TMR20_CH1N      D(2, 20)
#define DEF_TIM_AF__PF5__TCH_TMR20_CH2N     D(2, 20)
#define DEF_TIM_AF__PF6__TCH_TMR20_CH4      D(2, 20)
#define DEF_TIM_AF__PF10__TCH_TMR5_CH4      D(2, 5)
#define DEF_TIM_AF__PF12__TCH_TMR20_CH1     D(2, 20)
#define DEF_TIM_AF__PF13__TCH_TMR20_CH2     D(2, 20)
#define DEF_TIM_AF__PF14__TCH_TMR20_CH3     D(2, 20)
#define DEF_TIM_AF__PF15__TCH_TMR20_CH4     D(2, 20)

#define DEF_TIM_AF__PF6__TCH_TMR10_CH1    D(3, 10)
#define DEF_TIM_AF__PF7__TCH_TMR11_CH1    D(3, 11)

#define DEF_TIM_AF__PF8__TCH_TMR13_CH1    D(9, 13)
#define DEF_TIM_AF__PF9__TCH_TMR14_CH1    D(9, 14)

// GPIOG
#define DEF_TIM_AF__PG0__TCH_TMR20_CH1N    D(2, 20)
#define DEF_TIM_AF__PG1__TCH_TMR20_CH2N    D(2, 20)
#define DEF_TIM_AF__PG2__TCH_TMR20_CH3N    D(2, 20)

//GPIOH
#define DEF_TIM_AF__PH2__TCH_TMR5_CH1    D(2, 5)
#define DEF_TIM_AF__PH3__TCH_TMR5_CH2    D(2, 5) 

