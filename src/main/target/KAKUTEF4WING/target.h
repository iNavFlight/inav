/*
 * This file is part of INAV.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#pragma once

#define TARGET_BOARD_IDENTIFIER "F4WI"
#define USBD_PRODUCT_STRING "KakuteF4Wing"

#define USE_TARGET_CONFIG

// ******* Status LED*****************
#define LED0                    PC5

// ******* Internal IMU ICM42688******
#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN       CW270_DEG
#define ICM42605_SPI_BUS         BUS_SPI1
#define ICM42605_CS_PIN          PA4
#define ICM42605_EXTI_PIN        PB12

// *************** I2C ****************
#define USE_I2C     
#define USE_I2C_DEVICE_2
#define I2C2_SCL                PB10     
#define I2C2_SDA                PB11      

// ********** External MAG On I2C2******
#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C2
#define USE_MAG_ALL

// ********** External Devices On I2C2******
#define TEMPERATURE_I2C_BUS     BUS_I2C2
#define PITOT_I2C_BUS           BUS_I2C2
#define USE_RANGEFINDER
#define RANGEFINDER_I2C_BUS     BUS_I2C2

// ********** Internal BARO On I2C2*********
#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C2
#define USE_BARO_BMP280
#define USE_BARO_DPS310
#define USE_BARO_SPL06

// *************** AT7456 OSD ***************
#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI2
#define MAX7456_CS_PIN          PC15

// *************** SPI FLASH BLACKBOX*********
#define USE_FLASHFS
#define USE_FLASH_M25P16
#define M25P16_CS_PIN           PC14
#define M25P16_SPI_BUS          BUS_SPI3
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

// *************** USB VCP ********************
#define USB_IO
#define USE_VCP
#define VBUS_SENSING_PIN        PA10
#define VBUS_SENSING_ENABLED

// *************** UART ********************
#define USE_UART_INVERTER
//UART1
#define USE_UART1
#define UART1_RX_PIN            PB7
#define UART1_TX_PIN            PB6

//UART2
#define USE_UART2
#define UART2_RX_PIN            PA3
#define UART2_TX_PIN            PA2

//The 4V5 pads close to UART3 are powered by both BEC and USB
//Config UART3 to serialRX,  So Receiver is powered when USB Plug-IN.
//UART3: SerialRX by Default
#define USE_UART3
#define UART3_RX_PIN            PC11
#define UART3_TX_PIN            PC10
#define INVERTER_PIN_UART3_RX   PC13

//UART5
#define USE_UART5
#define UART5_RX_PIN            PD2
#define UART5_TX_PIN            PC12

//UART6
#define USE_UART6
#define UART6_RX_PIN            PC7
#define UART6_TX_PIN            PC6

// #   define USE_SOFTSERIAL1
# define SERIAL_PORT_COUNT 6

// *************** SPI ********************
#define USE_SPI
// SPI1: Connected to ICM gyro
#define USE_SPI_DEVICE_1
#define SPI1_NSS_PIN            PA4
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

// SPI2: Connected to OSD
#define USE_SPI_DEVICE_2
#define SPI2_NSS_PIN            PC15
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PC2
#define SPI2_MOSI_PIN           PC3

// SPI3: Connected to flash memory
#define USE_SPI_DEVICE_3
#define SPI3_NSS_PIN            PC14
#define SPI3_SCK_PIN            PB3
#define SPI3_MISO_PIN           PB4
#define SPI3_MOSI_PIN           PB5

// *************** Battery Voltage Sense***********
#define USE_ADC
#define ADC_INSTANCE                ADC1
#define ADC1_DMA_STREAM             DMA2_Stream0
#define ADC_CHANNEL_1_PIN           PC0
#define ADC_CHANNEL_2_PIN           PC1
#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2
#define CURRENT_METER_SCALE         250     // Current_Meter 1V=40A
#define VBAT_SCALE_DEFAULT          1100    // VBAT_ADC 1V=11V

// *************** LED_STRIP **********************
#define USE_LED_STRIP
#define WS2811_PIN                      PA1
#define WS2811_DMA_HANDLER_IDENTIFER    DMA1_ST6_HANDLER
#define WS2811_DMA_STREAM               DMA1_Stream6
#define WS2811_DMA_CHANNEL              DMA_Channel_3

// *************** PINIO ***************************
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN                  PB14    // USER1
#define PINIO2_PIN                  PB15   // USER2
      
#define DEFAULT_FEATURES     (FEATURE_OSD | FEATURE_TELEMETRY | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TX_PROF_SEL | FEATURE_BLACKBOX | FEATURE_GPS)

// ***********Set rx type and procotol***************
#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART3

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA 0xffff
#define TARGET_IO_PORTB 0xffff
#define TARGET_IO_PORTC 0xffff
#define TARGET_IO_PORTD        (BIT(2))

#define USE_DSHOT
#define USE_ESC_SENSOR

#define MAX_PWM_OUTPUT_PORTS       6


