/*
 * This file is part of INAV Project.
 *
 * INAV Project is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV Project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV Project.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#define DEFAULT_FEATURES            (FEATURE_OSD | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TELEMETRY)

#define TARGET_BOARD_IDENTIFIER     "JBF7"
#define USBD_PRODUCT_STRING         "RDQJBF7"

#define LED0                        PC4

#define USE_BEEPER
#define BEEPER                      PC15
#define BEEPER_INVERTED

// ************************** SPI ******************

#define USE_SPI

#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN                PA5
#define SPI1_MISO_PIN               PA6
#define SPI1_MOSI_PIN               PA7

#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN                PB13
#define SPI2_MISO_PIN               PB14
#define SPI2_MOSI_PIN               PB15

#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN                PB3
#define SPI3_MISO_PIN   	        PB4
#define SPI3_MOSI_PIN   	        PB5

// *************** Gyro & ACC **********************

#define USE_IMU_MPU6500
#define MPU6500_SPI_BUS             BUS_SPI1
#define MPU6500_CS_PIN              PC3
#define IMU_MPU6500_ALIGN           CW90_DEG

// *************** I2C/Baro/Mag *********************
#define USE_I2C
#define USE_I2C_DEVICE_2
#define I2C2_SCL                    PB10    // SCL pad TX3
#define I2C2_SDA                    PB11    // SDA pad RX3

#define USE_BARO
#define USE_BARO_ALL
#define BARO_I2C_BUS                BUS_I2C2

#define USE_MAG
#define USE_MAG_ALL

// *************** SD Card **************************
#define USE_SDCARD
#define USE_SDCARD_SPI
#define SDCARD_SPI_BUS              BUS_SPI3
#define SDCARD_CS_PIN               PA4
#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

// *************** OSD *****************************

#define USE_MAX7456
#define MAX7456_SPI_BUS             BUS_SPI2
#define MAX7456_CS_PIN              PB12

//******* FLASH ********
#define USE_FLASHFS
#define USE_FLASH_M25P16
#define M25P16_SPI_BUS              BUS_SPI3
#define M25P16_CS_PIN               PB9

#define USE_FLASH_W25M
#define W25M_SPI_BUS                BUS_SPI3
#define W25M_CS_PIN                 PB9

#define USE_FLASH_W25M02G
#define W25M02G_SPI_BUS             BUS_SPI3
#define W25M02G_CS_PIN              PB9

#define USE_FLASH_W25M512
#define W25M512_SPI_BUS             BUS_SPI3
#define W25M512_CS_PIN              PB9

#define USE_FLASH_W25N01G
#define W25N01G_SPI_BUS             BUS_SPI3
#define W25N01G_CS_PIN              PB9

// *************** PINIO ***************************
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN                  PC13
#define PINIO2_PIN                  PC14
#define PINIO3_PIN                  PB8
#define PINIO1_FLAGS                PINIO_FLAGS_INVERTED

// *************** UART *****************************
#define USB_IO
#define USE_VCP

#define USE_UART1
#define UART1_RX_PIN                PA10
#define UART1_TX_PIN                PA9

#define USE_UART2   
#define UART2_RX_PIN                PA3
#define UART2_TX_PIN                PA2

#define USE_UART3   
#define UART3_RX_PIN                PB11
#define UART3_TX_PIN                PB10

#define USE_UART4   
#define UART4_RX_PIN                PC11
#define UART4_TX_PIN                PC10

#define USE_UART5   
#define UART5_RX_PIN                PD2
#define UART5_TX_PIN                PC12

#define SERIAL_PORT_COUNT           6

#define DEFAULT_RX_TYPE             RX_TYPE_SERIAL
#define SERIALRX_PROVIDER           SERIALRX_CRSF

// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE                ADC1
#define ADC_CHANNEL_1_PIN           PC1
#define ADC_CHANNEL_2_PIN           PC2
#define ADC_CHANNEL_3_PIN           PC0 

#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2
#define RSSI_ADC_CHANNEL            ADC_CHN_3

#define VOLTAGE_METER_SCALE         110
#define CURRENT_METER_SCALE         100

#define USE_LED_STRIP
#define WS2811_PIN                  PA1

#define TARGET_IO_PORTA             0xffff
#define TARGET_IO_PORTB             0xffff
#define TARGET_IO_PORTC             0xffff
#define TARGET_IO_PORTD             0xffff
#define TARGET_IO_PORTE             0xffff
#define TARGET_IO_PORTF             0xffff

#define MAX_PWM_OUTPUT_PORTS        8
#define TARGET_MOTOR_COUNT          4           
#define USE_DSHOT
#define USE_ESC_SENSOR
#define USE_SERIAL_4WAY_BLHELI_INTERFACE
