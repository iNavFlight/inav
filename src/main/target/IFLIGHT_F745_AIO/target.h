/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#define TARGET_BOARD_IDENTIFIER "IFRC"

#define USBD_PRODUCT_STRING "IFLIGHT_F745_AIO"

#define LED0       PC13
#define BEEPER     PD15
#define BEEPER_INVERTED

#define MPU6000_CS_PIN        PA4
#define MPU6000_SPI_BUS       BUS_SPI1

#define USE_IMU_MPU6000
#define IMU_MPU6000_ALIGN CW0_DEG

// MPU6000 interrupts
#define USE_MPU_DATA_READY_SIGNAL
#define MPU_INT_EXTI PD0
#define USE_EXTI

#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C1
#define USE_BARO_DPS310

#define USE_VCP
#define USE_USB_DETECT
#define USB_DETECT_PIN   PC4

#define USE_UART1
#define UART1_RX_PIN PA10
#define UART1_TX_PIN PA9

#define USE_UART2
#define UART2_RX_PIN PA3
#define UART2_TX_PIN PA2

//#define USE_UART3
//#define UART3_RX_PIN PB11
//#define UART3_TX_PIN PB10
 
#define USE_UART4
#define UART4_RX_PIN PA1
#define UART4_TX_PIN PA0

//#define USE_UART6
//#define UART6_RX_PIN PC7
//#define UART6_TX_PIN PC6

#define USE_UART7
#define UART7_RX_PIN PE7
#define UART7_TX_PIN PE8

//#define USE_UART8
//#define UART8_RX_PIN PE0
//#define UART8_TX_PIN PC4

//#define USE_SOFTSERIAL1
//#define USE_SOFTSERIAL2

#define SERIAL_PORT_COUNT 5 //VCP, USART1, USART2, USART3, UART4, UART5, USART6, USART7, USART8, SOFTSERIAL x 2

#define USE_ESCSERIAL

#define USE_SPI
#define USE_SPI_DEVICE_1
//#define USE_SPI_DEVICE_2
#define USE_SPI_DEVICE_3
#define USE_SPI_DEVICE_4

#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

//unused
//#define SPI2_SCK_PIN            PB13 //interferes with i2c2 and uart3
//#define SPI2_MISO_PIN           PB14
//#define SPI2_MOSI_PIN           PB15

#define SPI3_SCK_PIN            PC10
#define SPI3_MISO_PIN           PC11
#define SPI3_MOSI_PIN           PC12

#define SPI4_SCK_PIN            PE2
#define SPI4_MISO_PIN           PE5
#define SPI4_MOSI_PIN           PE6

#define USE_MAX7456
#define MAX7456_SPI_BUS    BUS_SPI4
#define MAX7456_CS_PIN      PE4
//#define MAX7456_SPI_CLK         (SPI_CLOCK_STANDARD) // 10MHz
//#define MAX7456_RESTORE_CLK     (SPI_CLOCK_FAST)

#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT
#define USE_FLASHFS
#define USE_FLASH_M25P16
#define M25P16_CS_PIN            PA15
#define M25P16_SPI_BUS      BUS_SPI3

#define USE_I2C
#define USE_I2C_DEVICE_1 
#define I2C1_SCL               PB8
#define I2C1_SDA               PB9

#define USE_I2C_DEVICE_2  // External matches UART_3
#define I2C2_SCL               PB10
#define I2C2_SDA               PB11

#define USE_ADC      
#define ADC_CHANNEL_1_PIN               PC3
#define ADC_CHANNEL_2_PIN               PC2
#define ADC_CHANNEL_3_PIN               PC5

#define VBAT_ADC_CHANNEL                ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL       ADC_CHN_2
#define RSSI_ADC_CHANNEL                ADC_CHN_3

#define USE_LED_STRIP
#define WS2811_PIN                      PD12

#define DEFAULT_RX_TYPE                 RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS

#define USE_SERIAL_4WAY_BLHELI_INTERFACE
#define MAX_PWM_OUTPUT_PORTS            5
#define USE_DSHOT
#define USE_ESC_SENSOR

#define TARGET_IO_PORTA 0xffff
#define TARGET_IO_PORTB 0xffff
#define TARGET_IO_PORTC 0xffff
#define TARGET_IO_PORTD 0xffff
#define TARGET_IO_PORTE 0xffff

#define USED_TIMERS  ( TIM_N(1) | TIM_N(2) | TIM_N(3) | TIM_N(4) | TIM_N(8) | TIM_N(9) )