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

// qUark Mini Wing v4

#define TARGET_BOARD_IDENTIFIER "QMW4"

#define USBD_PRODUCT_STRING  "QMWv4"

#define CUSTOM_CLOCK_TREE
#define PLL_M 8
#define PLL_N 168
#define PLL_P 2
#define PLL_Q 7

#define LED0                    PA15 // RED
#define LED1                    PD2  // GREEN
#define LED2                    PC15 // BLUE

#define BEEPER                  PC9
#define BEEPER_INVERTED

// ********** Gyro, ACC and Compass ****************
#define USE_SPI
#define USE_SPI_DEVICE_2

#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN	        PB14
#define SPI2_MOSI_PIN	        PB15

#define BMI088_GYRO_CS_PIN      PC4
#define BMI088_ACC_CS_PIN       PC5
#define BMI088_SPI_BUS          BUS_SPI2

#define USE_IMU_BMI088
#define IMU_BMI088_ALIGN        CW0_DEG

//#define LIS3MDL_SPI_BUS         BUS_SPI2
//#define USE_MAG_LIS3MDL

#define BMP280_CS_PIN           PA4
#define BMP280_SPI_BUS          BUS_SPI2
#define USE_BARO_BMP280

// *************** SD Card **************************
#define USE_SDCARD
#define USE_SDCARD_SPI
#define SDCARD_SPI_BUS          BUS_SPI3
#define SDCARD_CS_PIN           PB12

#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            PC10
#define SPI3_MISO_PIN   	    PC11
#define SPI3_MOSI_PIN   	    PC12

#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

// *************** OSD *****************************
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN   	    PA6
#define SPI1_MOSI_PIN   	    PA7

#define USE_OSD
#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI1
#define MAX7456_CS_PIN          PB1

// *************** UART *****************************
#define USE_VCP
#define VBUS_SENSING_PIN        PA9
#define VBUS_SENSING_ENABLED

#define USE_UART1
#define UART1_RX_PIN            PB7
#define UART1_TX_PIN            PB6

#define USE_UART2
#define UART2_RX_PIN            PA3
#define UART2_TX_PIN            PA2

#define USE_UART6
#define UART6_RX_PIN            PC7
#define UART6_TX_PIN            PC6

#define USE_SOFTSERIAL1
#define SOFTSERIAL_1_RX_PIN     PB3 // RCIN
#define SOFTSERIAL_1_TX_PIN     PB3 // RCIN

#define SERIAL_PORT_COUNT       5

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART6

// *************** I2C ****************************
#define USE_I2C
#define USE_I2C_DEVICE_2
#define I2C1_SCL                PB10
#define I2C1_SDA                PB11

#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C2
#define USE_BARO_MS5611
#define USE_BARO_BMP085
#define USE_BARO_DPS310

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C2
#define USE_MAG_AK8963
#define USE_MAG_AK8975
#define USE_MAG_HMC5883
#define USE_MAG_QMC5883
#define USE_MAG_IST8310
#define USE_MAG_IST8308
#define USE_MAG_MAG3110
#define USE_MAG_LIS3MDL

#define TEMPERATURE_I2C_BUS     BUS_I2C2

#define USE_RANGEFINDER
#define USE_RANGEFINDER_MSP
#define USE_RANGEFINDER_HCSR04_I2C
#define RANGEFINDER_I2C_BUS     BUS_I2C2

#define PITOT_I2C_BUS           BUS_I2C2

// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE                ADC1
#define ADC1_DMA_STREAM             DMA2_Stream0
#define ADC_CHANNEL_1_PIN           PC0
#define ADC_CHANNEL_2_PIN           PC2
#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2

#define DEFAULT_FEATURES        (FEATURE_OSD | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TELEMETRY )
#define VBAT_SCALE_DEFAULT      597
#define CURRENT_METER_SCALE     179

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         (BIT(2))

#define MAX_PWM_OUTPUT_PORTS    5
