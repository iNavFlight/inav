/*
    Copyright (C) 2016 Stephane D'Alu

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
 * See: https://www.microbit.co.uk/device/pins
 *      https://lancaster-university.github.io/microbit-docs/ubit/display/
 */

/* Board identifier. */
#define BOARD_MICROBIT
#define BOARD_NAME              "micro:bit"

/* Board oscillators-related settings. */
#define NRF51_XTAL_VALUE        16000000
#define NRF51_LFCLK_SOURCE      0  /* RC oscillator */

/*
 * IO pins assignments.
 */
#define IOPORT1_P0              3U
#define IOPORT1_P1              2U
#define IOPORT1_P2              1U
#define IOPORT1_P3              4U
#define IOPORT1_P4              5U
#define IOPORT1_P5             17U
#define IOPORT1_P6             12U
#define IOPORT1_P7             11U
#define IOPORT1_P8             18U
#define IOPORT1_P9             10U
#define IOPORT1_P10             6U
#define IOPORT1_P11            26U
#define IOPORT1_P12            20U
#define IOPORT1_P13            23U
#define IOPORT1_P14            22U
#define IOPORT1_P15            21U
#define IOPORT1_P16            16U
#define IOPORT1_P19             0U
#define IOPORT1_P20            30U
#define IOPORT1_BTN_A          17U
#define IOPORT1_BTN_B          26U
#define IOPORT1_BTN_RST        19U  
#define IOPORT1_LED_COL_1       4U
#define IOPORT1_LED_COL_2       5U
#define IOPORT1_LED_COL_3       6U
#define IOPORT1_LED_COL_4       7U
#define IOPORT1_LED_COL_5       8U
#define IOPORT1_LED_COL_6       9U
#define IOPORT1_LED_COL_7      10U
#define IOPORT1_LED_COL_8      11U
#define IOPORT1_LED_COL_9      12U
#define IOPORT1_LED_ROW_1      13U
#define IOPORT1_LED_ROW_2      14U
#define IOPORT1_LED_ROW_3      15U
#define IOPORT1_PAD_0             IOPORT1_P0
#define IOPORT1_PAD_1             IOPORT1_P1
#define IOPORT1_PAD_2             IOPORT1_P2
#define IOPORT1_SPI_MOSI       21U
#define IOPORT1_SPI_MISO       22U
#define IOPORT1_SPI_SCK        23U
#define IOPORT1_I2C_SCL         0U
#define IOPORT1_I2C_SDA        30U
#define IOPORT1_UART_TX        24U
#define IOPORT1_UART_RX        25U
#define IOPORT1_ACC_INT1       28U
#define IOPORT1_ACC_INT2       27U
#define IOPORT1_MAG_INT1       29U


/*
 * IO lines assignments.
 */
#define LINE_P0                PAL_LINE(IOPORT1, IOPORT1_P0)
#define LINE_P1                PAL_LINE(IOPORT1, IOPORT1_P1)
#define LINE_P2                PAL_LINE(IOPORT1, IOPORT1_P2)
#define LINE_P3                PAL_LINE(IOPORT1, IOPORT1_P3)
#define LINE_P4                PAL_LINE(IOPORT1, IOPORT1_P4)
#define LINE_P5                PAL_LINE(IOPORT1, IOPORT1_P5)
#define LINE_P6                PAL_LINE(IOPORT1, IOPORT1_P6)
#define LINE_P7                PAL_LINE(IOPORT1, IOPORT1_P7)
#define LINE_P8                PAL_LINE(IOPORT1, IOPORT1_P8)
#define LINE_P9                PAL_LINE(IOPORT1, IOPORT1_P9)
#define LINE_P10               PAL_LINE(IOPORT1, IOPORT1_P10)
#define LINE_P11               PAL_LINE(IOPORT1, IOPORT1_P11)
#define LINE_P12               PAL_LINE(IOPORT1, IOPORT1_P12)
#define LINE_P13               PAL_LINE(IOPORT1, IOPORT1_P13)
#define LINE_P14               PAL_LINE(IOPORT1, IOPORT1_P14)
#define LINE_P15               PAL_LINE(IOPORT1, IOPORT1_P15)
#define LINE_P16               PAL_LINE(IOPORT1, IOPORT1_P16)
#define LINE_P19               PAL_LINE(IOPORT1, IOPORT1_P19)
#define LINE_P20               PAL_LINE(IOPORT1, IOPORT1_P20)
#define LINE_BTN_A             PAL_LINE(IOPORT1, IOPORT1_BTN_A)
#define LINE_BTN_B             PAL_LINE(IOPORT1, IOPORT1_BTN_B)
#define LINE_BTN_RST           PAL_LINE(IOPORT1, IOPORT1_BTN_RST)
#define LINE_LED_COL_1         PAL_LINE(IOPORT1, IOPORT1_LED_COL_1)
#define LINE_LED_COL_2         PAL_LINE(IOPORT1, IOPORT1_LED_COL_2)
#define LINE_LED_COL_3         PAL_LINE(IOPORT1, IOPORT1_LED_COL_3)
#define LINE_LED_COL_4         PAL_LINE(IOPORT1, IOPORT1_LED_COL_4)
#define LINE_LED_COL_5         PAL_LINE(IOPORT1, IOPORT1_LED_COL_5)
#define LINE_LED_COL_6         PAL_LINE(IOPORT1, IOPORT1_LED_COL_6)
#define LINE_LED_COL_7         PAL_LINE(IOPORT1, IOPORT1_LED_COL_7)
#define LINE_LED_COL_8         PAL_LINE(IOPORT1, IOPORT1_LED_COL_8)
#define LINE_LED_COL_9         PAL_LINE(IOPORT1, IOPORT1_LED_COL_9)
#define LINE_LED_ROW_1         PAL_LINE(IOPORT1, IOPORT1_LED_ROW_1)
#define LINE_LED_ROW_2         PAL_LINE(IOPORT1, IOPORT1_LED_ROW_2)
#define LINE_LED_ROW_3         PAL_LINE(IOPORT1, IOPORT1_LED_ROW_3)
#define LINE_PAD_0             PAL_LINE(IOPORT1, IOPORT1_PAD_0)
#define LINE_PAD_1             PAL_LINE(IOPORT1, IOPORT1_PAD_1)
#define LINE_PAD_2             PAL_LINE(IOPORT1, IOPORT1_PAD_2)
#define LINE_SPI_MOSI          PAL_LINE(IOPORT1, IOPORT1_SPI_MOSI)
#define LINE_SPI_MISO          PAL_LINE(IOPORT1, IOPORT1_SPI_MISO)
#define LINE_SPI_SCK           PAL_LINE(IOPORT1, IOPORT1_SPI_SCK)
#define LINE_I2C_SCL           PAL_LINE(IOPORT1, IOPORT1_I2C_SCL)
#define LINE_I2C_SDA           PAL_LINE(IOPORT1, IOPORT1_I2C_SDA)
#define LINE_UART_TX           PAL_LINE(IOPORT1, IOPORT1_UART_TX)
#define LINE_UART_RX           PAL_LINE(IOPORT1, IOPORT1_UART_RX)
#define LINE_ACC_INT1          PAL_LINE(IOPORT1, IOPORT1_ACC_INT1)
#define LINE_ACC_INT2          PAL_LINE(IOPORT1, IOPORT1_ACC_INT2)
#define LINE_MAG_INT1          PAL_LINE(IOPORT1, IOPORT1_MAG_INT1)


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
