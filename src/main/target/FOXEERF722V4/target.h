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

#define TARGET_BOARD_IDENTIFIER "FX74"
#define USBD_PRODUCT_STRING     "FOXEER F722 V4"

/*** Indicators ***/
#define LED0                    PC15
#define BEEPER                  PA4
#define BEEPER_INVERTED

/*** IMU sensors ***/
#define USE_EXTI

#define USE_MPU_DATA_READY_SIGNAL

#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN      CW270_DEG
#define ICM42605_SPI_BUS        BUS_SPI1
#define ICM42605_CS_PIN         PB2
#define ICM42605_EXTI_PIN       PC4

/*** SPI/I2C bus ***/
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15

#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            PC10
#define SPI3_MISO_PIN           PC11 
#define SPI3_MOSI_PIN           PB5

#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9

/*** Onboard flash ***/
#define USE_FLASHFS
#define USE_FLASH_M25P16
#define M25P16_CS_PIN           PB12
#define M25P16_SPI_BUS          BUS_SPI2

/*** OSD ***/
#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI3
#define MAX7456_CS_PIN          PC3

/*** Serial ports ***/
#define USE_VCP

#define USE_UART1
#define UART1_TX_PIN            PB6
#define UART1_RX_PIN            PB7



#define USE_UART3
#define UART3_TX_PIN            PB10
#define UART3_RX_PIN            PB11

#define USE_UART4
#define UART4_TX_PIN            PA0
#define UART4_RX_PIN            PA1

#define USE_UART5   
#define UART5_TX_PIN            PC12  
#define UART5_RX_PIN            PD2  

#ifdef FOXEERF722V4_X8
//X8 variant without UART2 and UART6
#define SERIAL_PORT_COUNT       5
#else
//Standard variant
#define SERIAL_PORT_COUNT       7

#define USE_UART2
#define UART2_TX_PIN            PA2
#define UART2_RX_PIN            PA3

#define USE_UART6
#define UART6_TX_PIN            PC6 
#define UART6_RX_PIN            PC7

#endif


/*** BARO & MAG ***/
#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C1
#define USE_BARO_BMP280
#define USE_BARO_MS5611
#define USE_BARO_DPS310
#define USE_BARO_SPL06

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_HMC5883
#define USE_MAG_QMC5883
#define USE_MAG_IST8310
#define USE_MAG_MAG3110
#define USE_MAG_LIS3MDL

#define BNO055_I2C_BUS          BUS_I2C1

/*** ADC ***/
#define USE_ADC
#define ADC_CHANNEL_1_PIN               PC0
#define ADC_CHANNEL_2_PIN               PC2
#define ADC_CHANNEL_3_PIN               PA0

#define VBAT_ADC_CHANNEL                ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL       ADC_CHN_2
#define RSSI_ADC_CHANNEL                ADC_CHN_3

/*** LED STRIP ***/
#define USE_LED_STRIP
#define WS2811_PIN                      PA15
 
/*** Default settings ***/
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT
#define SERIALRX_UART                   SERIAL_PORT_USART1
#define DEFAULT_RX_TYPE                 RX_TYPE_SERIAL
#define SERIALRX_PROVIDER               SERIALRX_SBUS

/*** Timer/PWM output ***/
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#ifdef FOXEERF722V4_X8
#define MAX_PWM_OUTPUT_PORTS            8
#else
#define MAX_PWM_OUTPUT_PORTS            4
#endif

#define USE_DSHOT
#define USE_ESC_SENSOR

/*** Used pins ***/
#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         (BIT(2))

