/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once


#define TARGET_BOARD_IDENTIFIER "NOX1"
#define USBD_PRODUCT_STRING  "NoxF4V1"

#define LED0                    PA4

#define BEEPER                  PC13
#define BEEPER_INVERTED

#define I2C2_SCL                NONE
#define I2C2_SDA                NONE

// *************** SPI **********************
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PB3
#define SPI1_MISO_PIN   	    PB4
#define SPI1_MOSI_PIN   	    PB5

#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN   		PB14
#define SPI2_MOSI_PIN   		PB15


// *************** SPI Gyro & ACC **********************
#define MPU6000_CS_PIN          PB12
#define MPU6000_SPI_BUS         BUS_SPI2

#define USE_EXTI
#define GYRO_INT_EXTI           PA8
#define USE_MPU_DATA_READY_SIGNAL

#define USE_GYRO
#define USE_GYRO_MPU6000

#define USE_ACC
#define USE_ACC_MPU6000

// *************** SPI BARO *****************************
#define USE_BARO
#define USE_BARO_BMP280
#define USE_BARO_SPI_BMP280
#define BMP280_SPI_BUS          BUS_SPI2
#define BMP280_CS_PIN           PA9

// *************** SPI OSD *****************************
#define USE_OSD
#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI2
#define MAX7456_CS_PIN          PA10
//#define MAX7456_SPI_CLK         (SPI_CLOCK_STANDARD*2)
//#define MAX7456_RESTORE_CLK     (SPI_CLOCK_FAST)

// *************** SPI FLASH **************************
#define USE_FLASHFS
#define USE_FLASH_M25P16
#define M25P16_CS_PIN           PA15
#define M25P16_SPI_BUS          BUS_SPI1
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

// *************** UART *****************************
#define USE_UART_INVERTER

#define USE_VCP

#define USE_UART1
#define UART1_TX_PIN            PB6
#define UART1_RX_PIN            PB7

#define USE_UART2
#define UART2_TX_PIN            PA2
#define UART2_RX_PIN            PA3

// #define INVERTER_PIN_UART2      PC14
#define INVERTER_PIN_UART2_RX   PC14 // PC14 used as inverter select GPIO

#define USE_SOFTSERIAL1
#define SOFTSERIAL_1_TX_PIN      PA2 // Workaround for softserial not initializing with only RX
#define SOFTSERIAL_1_RX_PIN      PA2 // Backdoor timer on UART2_TX, used for ESC telemetry

#define SERIAL_PORT_COUNT       4 //VCP, USART1, USART2, SOFTSERIAL1

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART2

// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE                         ADC1
#define ADC_CHANNEL_1_PIN               NONE
#define ADC_CHANNEL_2_PIN               PA5
#define ADC_CHANNEL_3_PIN               NONE

#define CURRENT_METER_ADC_CHANNEL        ADC_CHN_1
#define VBAT_ADC_CHANNEL                 ADC_CHN_2
#define RSSI_ADC_CHANNEL                   ADC_CHN_3
/*
#define ADC_INSTANCE                ADC1
#define ADC1_DMA_STREAM             DMA2_Stream0
#define ADC_CHANNEL_1_PIN           PA5
#define ADC_CHANNEL_2_PIN           NONE
//#define ADC_CHANNEL_3_PIN           PA0
#define VBAT_ADC_CHANNEL            ADC_CHN_1
//#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2
//#define RSSI_ADC_CHANNEL            ADC_CHN_3
*/
// *************** LED2812 ************************
#define USE_LED_STRIP
#define WS2811_PIN                      PA0
#define WS2811_DMA_HANDLER_IDENTIFER    DMA2_ST1_HANDLER
#define WS2811_DMA_STREAM               DMA2_Stream1
#define WS2811_DMA_CHANNEL              DMA_Channel_6

// ***************  OTHERS *************************
#define DEFAULT_FEATURES        (FEATURE_OSD | FEATURE_SOFTSERIAL | FEATURE_VBAT)


// #define USE_SPEKTRUM_BIND
// #define BIND_PIN                PA10 //  RX1

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA (0xffff & ~(BIT(14)|BIT(13)))
#define TARGET_IO_PORTB (0xffff & ~(BIT(2)|BIT(11)))
#define TARGET_IO_PORTC (BIT(13)|BIT(14)|BIT(15))

#define MAX_PWM_OUTPUT_PORTS       4
