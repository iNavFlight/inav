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

#define TARGET_BOARD_IDENTIFIER "F405AIO"       
#define USBD_PRODUCT_STRING  "AocodaRCF405AIO"  

#define LED0                 PC13

#define USE_BEEPER
#define BEEPER               PB8
#define BEEPER_INVERTED

/*** UART ***/
#define USB_IO
#define USE_VCP

#define USE_UART1
#define UART1_RX_PIN         PA10
#define UART1_TX_PIN         PA9

#define USE_UART2
#define UART2_RX_PIN         PA3
#define UART2_TX_PIN         PA2

#define USE_UART3
#define UART3_RX_PIN         PC11
#define UART3_TX_PIN         PC10

#define USE_UART4
#define UART4_RX_PIN         PA1
#define UART4_TX_PIN         PA0

#define USE_UART5
#define UART5_RX_PIN         PC12
#define UART5_TX_PIN         PD2

#define SERIAL_PORT_COUNT    6               

/*** Gyro & Acc ***/
#define USE_TARGET_IMU_HARDWARE_DESCRIPTORS

#define USE_SPI
#define USE_SPI_DEVICE_1

#define SPI1_SCK_PIN         PA5
#define SPI1_MISO_PIN        PA6
#define SPI1_MOSI_PIN        PA7

#define USE_IMU_MPU6500
#define MPU6500_CS_PIN           PA4
#define MPU6500_SPI_BUS          BUS_SPI1
#define MPU6500_EXTI_PIN         PC5
#define IMU_MPU6500_ALIGN        CW90_DEG

#define USE_IMU_MPU6000
#define MPU6000_CS_PIN           PA4
#define MPU6000_SPI_BUS          BUS_SPI1
#define MPU6000_EXTI_PIN         PC5
#define IMU_MPU6000_ALIGN        CW90_DEG

#define USE_IMU_BMI270
#define BMI270_CS_PIN           PA4
#define BMI270_SPI_BUS          BUS_SPI1
#define BMI270_EXTI_PIN         PC5
#define IMU_BMI270_ALIGN        CW90_DEG

/*** I2C (Baro & Mag) ***/
#define USE_I2C

#define USE_I2C_DEVICE_1
#define I2C1_SCL             PB6
#define I2C1_SDA             PB7

// Baro
#define USE_BARO
#define USE_BARO_BMP280
#define USE_BARO_MS5611
#define USE_BARO_DPS310
#define USE_BARO_SPL06
#define BARO_I2C_BUS         BUS_I2C1	

// Mag 
#define USE_MAG
#define MAG_I2C_BUS          BUS_I2C1
#define USE_MAG_ALL

/*** Onboard Flash ***/    
#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN         PB3
#define SPI3_MISO_PIN   	 PB5
#define SPI3_MOSI_PIN   	 PB4

#define M25P16_SPI_BUS       BUS_SPI3
#define M25P16_CS_PIN        PC0

#define USE_FLASHFS
#define USE_FLASH_M25P16
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

/*** OSD ***/
#define USE_OSD
#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN         PB13
#define SPI2_MISO_PIN        PB14
#define SPI2_MOSI_PIN        PB15

#define USE_MAX7456
#define MAX7456_SPI_BUS      BUS_SPI2
#define MAX7456_CS_PIN       PA13

/*** ADC ***/
#define USE_ADC
#define ADC_CHANNEL_1_PIN    PC2
#define ADC_CHANNEL_2_PIN    PC3
#define ADC_CHANNEL_3_PIN    PC1

#define VBAT_ADC_CHANNEL          ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL ADC_CHN_3
#define RSSI_ADC_CHANNEL          ADC_CHN_2

/*** LED ***/
#define USE_LED_STRIP
#define WS2811_PIN PB1

/*** Default settings ***/
#define SERIALRX_UART        SERIAL_PORT_USART2
#define DEFAULT_RX_TYPE      RX_TYPE_SERIAL
#define SERIALRX_PROVIDER    SERIALRX_CRSF
#define CURRENT_METER_SCALE  250

/*** Optical Flow & Lidar ***/
#define USE_RANGEFINDER
#define USE_RANGEFINDER_MSP
#define USE_OPFLOW
#define USE_OPFLOW_MSP

/*** Timer/PWM output ***/
#define USE_SERIAL_4WAY_BLHELI_INTERFACE
#define MAX_PWM_OUTPUT_PORTS 8
#define USE_DSHOT
#define USE_ESC_SENSOR

/*** Misc ***/
#define DEFAULT_FEATURES     (FEATURE_TX_PROF_SEL | FEATURE_CURRENT_METER | FEATURE_TELEMETRY | FEATURE_VBAT | FEATURE_OSD | FEATURE_BLACKBOX)

#define TARGET_IO_PORTA      0xffff
#define TARGET_IO_PORTB      0xffff
#define TARGET_IO_PORTC      0xffff
#define TARGET_IO_PORTD      (BIT(2))
