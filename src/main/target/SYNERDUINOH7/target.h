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
 *
 ***Base of the Matek H743 Modded** 
 */



#pragma once

#define TARGET_BOARD_IDENTIFIER "SYNH743"
#define USBD_PRODUCT_STRING     "SYNERDUIN0H7"


#define LED0                    PE3
#define LED1                    PE4

#define BEEPER                  PA15
#define BEEPER_INVERTED
#define BEEPER_PWM_FREQUENCY    2500

// *************** SPI1 Flash BLACKBOX  ******************
//#define USE_SPI_DEVICE_1
//#define SPI1_SCK_PIN            PB3
//#define SPI1_MISO_PIN           PB4
//#define SPI1_MOSI_PIN           PD7

//#define USE_BLACKBOX
//#define USE_FLASHFS

//#define USE_FLASH_W25N01G
//#define W25N01G_SPI_BUS         BUS_SPI1
//#define W25N01G_CS_PIN          PD6 

//#define USE_FLASH_W25Q64
//#define W25Q64_SPI_BUS          BUS_SPI1
//#define W25Q64_CS_PIN           PD6

//#define USE_FLASH_M25P16
//#define M25P16_SPI_BUS          BUS_SPI1
//#define M25P16_CS_PIN           PD6 


// *************** SPI1 Flash BLACKBOX  *********Might Affect UART2********
//#define USE_SPI_DEVICE_1
//#define SPI1_SCK_PIN            PB3
//#define SPI1_MISO_PIN           PB4
//#define SPI1_MOSI_PIN           PD7

//#define USE_BLACKBOX
//#define USE_FLASHFS

//#define USE_FLASH_W25N01G
//#define W25N01G_SPI_BUS         BUS_SPI1
//#define W25N01G_CS_PIN          PD6 

//#define USE_FLASH_W25Q64
//#define W25Q64_SPI_BUS          BUS_SPI1
//#define W25Q64_CS_PIN           PD6

//#define USE_FLASH_M25P16
//#define M25P16_SPI_BUS          BUS_SPI1
//#define M25P16_CS_PIN           PD6 

//#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT
// *************** IMU generic ***********************
//#define USE_DUAL_GYRO
// *************** SPI2 IMU1 ******************************
//#define USE_SPI_DEVICE_2
//#define SPI1_SCK_PIN            PB13
//#define SPI1_MISO_PIN           PB14
//#define SPI1_MOSI_PIN           PB15

//#define USE_IMU_ICM42605

//#define IMU_ICM42605_ALIGN      CW90_DEG_FLIP
//#define ICM42605_SPI_BUS        BUS_SPI1
//#define ICM42605_CS_PIN         PB15

//#define USE_IMU_ICM20689

//#define IMU_ICM20689_ALIGN      CW90_DEG_FLIP
//#define ICM20689_SPI_BUS        BUS_SPI1
//#define ICM20689_CS_PIN         PB15

// *************** SPI1 IMU0 ******************************
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI2_SCK_PIN            PA5 // PB13 
#define SPI2_MISO_PIN           PA6 // PB14
#define SPI2_MOSI_PIN           PA7 // PB15

#define USE_IMU_MPU6000

#define IMU_MPU6000_ALIGN       CW90_DEG
#define MPU6000_SPI_BUS         BUS_SPI1 // BUS_SPI2
#define MPU6000_CS_PIN          PA4 // PB12

#define USE_IMU_MPU6500

#define IMU_MPU6500_ALIGN       CW90_DEG
#define MPU6500_SPI_BUS         BUS_SPI1 // BUS_SPI2
#define MPU6500_CS_PIN          PA4 // PB12

#define USE_IMU_MPU9250

#define IMU_MPU9250_ALIGN       CW90_DEG
#define MPU9250_SPI_BUS          BUS_SPI1 // BUS_SPI2
#define MPU9250_CS_PIN          PA4 // PB12

#define USE_IMU_BMI160

#define IMU_BMI160_ALIGN       CW270_DEG
#define BMI160_SPI_BUS          BUS_SPI1 // BUS_SPI2
#define BMI160_CS_PIN          PA4 // PB12

#define USE_IMU_BMI270

#define IMU_BMI270_ALIGN       CW270_DEG
#define BMI270_SPI_BUS          BUS_SPI1 // BUS_SPI2
#define BMI270_CS_PIN          PA4 // PB12

// *************** SPI4 OSD **********Some readon the SPI OLED was there***********
//#define USE_SPI_DEVICE_4
//#define SPI4_SCK_PIN            PE12
//#define SPI4_MISO_PIN           PE13
//#define SPI4_MOSI_PIN           PE14

//#define USE_MAX7456
//#define MAX7456_SPI_BUS         BUS_SPI4
//#define MAX7456_CS_PIN          PE11

// *************** I2C /Baro/Mag *********************
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB6
#define I2C1_SDA                PB7

#define USE_I2C_DEVICE_2
#define I2C2_SCL                PB10
#define I2C2_SDA                PB11
//** BARO **//
#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C2
#define USE_BARO_BMP085
#define USE_BARO_BMP280
#define USE_BARO_MS5611
#define USE_BARO_DPS310
#define USE_BARO_SPL06
//** MAG **//
#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_ALL
//** TEMP **//
#define TEMPERATURE_I2C_BUS     BUS_I2C2
#define PITOT_I2C_BUS           BUS_I2C2
//** RANGE **//
#define USE_RANGEFINDER
#define RANGEFINDER_I2C_BUS     BUS_I2C1

// *************** UART *****************************
#define USE_VCP

#define USE_UART1
#define UART1_TX_PIN            PA9
#define UART1_RX_PIN            PA10

#define USE_UART2
#define UART2_TX_PIN            PD5
#define UART2_RX_PIN            PD6

#define USE_UART3
#define UART3_TX_PIN            PD8
#define UART3_RX_PIN            PD9

#define USE_UART4
#define UART4_TX_PIN            PB9
#define UART4_RX_PIN            PB8

#define USE_UART6
#define UART6_TX_PIN            PC6
#define UART6_RX_PIN            PC7

#define USE_UART7
#define UART7_TX_PIN            PE8
#define UART7_RX_PIN            PE7

#define USE_UART8
#define UART8_TX_PIN            PE1
#define UART8_RX_PIN            PE0

//#define USE_SOFTSERIAL1
//#define SOFTSERIAL_1_TX_PIN      PC6  //TX6 pad
//#define SOFTSERIAL_1_RX_PIN      PC6  //TX6 pad

#define SERIAL_PORT_COUNT       8

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_CRSF
#define SERIALRX_UART           SERIAL_PORT_USART6

// *************** SDIO SD BLACKBOX*******************
#define USE_SDCARD
#define USE_SDCARD_SDIO
#define SDCARD_SDIO_DEVICE      SDIODEV_1
#define SDCARD_SDIO_4BIT

#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE                ADC1

#define ADC_CHANNEL_1_PIN           PC0  //ADC123 VBAT1
#define ADC_CHANNEL_2_PIN           PC1  //ADC123 CURR1
#define ADC_CHANNEL_3_PIN           PC5  //ADC12  RSSI
#define ADC_CHANNEL_4_PIN           PC4  //ADC12  AirS
//#define ADC_CHANNEL_5_PIN           PA4  //ADC12  VB2
//#define ADC_CHANNEL_6_PIN           PA7  //ADC12  CU2

#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2
#define RSSI_ADC_CHANNEL            ADC_CHN_3
#define AIRSPEED_ADC_CHANNEL        ADC_CHN_4

// *************** PINIO ***************************
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN                  PD10  // VTX power switcher
#define PINIO2_PIN                  PD11  // 2xCamera switcher

// *************** LEDSTRIP ************************
#define USE_LED_STRIP
#define WS2811_PIN                  PA8

#define DEFAULT_FEATURES            (FEATURE_TELEMETRY | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TX_PROF_SEL | FEATURE_BLACKBOX)//FEATURE_OSD | 
#define CURRENT_METER_SCALE         250

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA 0xffff
#define TARGET_IO_PORTB 0xffff
#define TARGET_IO_PORTC 0xffff
#define TARGET_IO_PORTD 0xffff
#define TARGET_IO_PORTE 0xffff

#define MAX_PWM_OUTPUT_PORTS        14
#define USE_DSHOT
#define USE_ESC_SENSOR

