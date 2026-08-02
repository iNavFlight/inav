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

/* 
*#define SPI1_SCK_PIN           PA5
*#define SPI1_MISO_PIN   	    PA6
*#define SPI1_MOSI_PIN   	    PA7
*#define BMI160_CS_PIN          PA4
*/ 
/**See Datasheet of the WA STM32F405XG**/
/**Base of the RIZA but as Dual IMU  and 8PWM**/

#pragma once

#define TARGET_BOARD_IDENTIFIER "SYND_F405A"
#define USBD_PRODUCT_STRING  "SYNERDUINOSTM_F405A"

#define LED0                    PB2  //Blue
#define LED1                    PA13  //Green
// *************** BLEEP **********************
#define BEEPER                  PB9
#define BEEPER_INVERTED
#define BEEPER_PWM_FREQUENCY    2500

// *************** IMU generic ***********************
#define USE_DUAL_GYRO

// *************** SPI1 Gyro & ACC ICM MPU **********************
#define USE_SPI
#define USE_SPI_DEVICE_1

#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN   	    PA6
#define SPI1_MOSI_PIN   	    PA7

// **** MPU IMU ****//
#define USE_IMU_MPU6000
#define IMU_MPU6000_ALIGN       CW90_DEG
#define MPU6000_CS_PIN           PA4
#define MPU6000_SPI_BUS          BUS_SPI1

#define USE_IMU_MPU9250
#define IMU_MPU9250_ALIGN        CW90_DEG
#define MPU9250_CS_PIN           PA4
#define MPU9250_SPI_BUS          BUS_SPI1

#define USE_IMU_MPU6500
#define IMU_MPU6500_ALIGN        CW90_DEG
#define MPU6500_CS_PIN           PA4
#define MPU6500_SPI_BUS          BUS_SPI1

// **** ICM IMU ****// 
#define USE_IMU_ICM20689
#define IMU_ICM20689_ALIGN        CW90_DEG
#define ICM20689_CS_PIN           PA4
#define ICM20689_SPI_BUS          BUS_SPI1

#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN        CW90_DEG
#define ICM42605_CS_PIN           PA4
#define ICM42605_SPI_BUS          BUS_SPI1


// *************** SPI2 Gyro & ACC  BMI BNO **********************
#define USE_SPI
#define USE_SPI_DEVICE_2

#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN   	    PC2
#define SPI2_MOSI_PIN   	    PC3

// **** BMI IMU ****// 
#define USE_IMU_BMI088
#define IMU_BMI088_ALIGN       CW270_DEG
#define BMI160_CS_PIN          PB12
#define BMI160_SPI_BUS         BUS_SPI2

#define USE_IMU_BMI160
#define IMU_BMI160_ALIGN       CW270_DEG
#define BMI160_CS_PIN          PB12
#define BMI160_SPI_BUS         BUS_SPI2

#define USE_IMU_BMI270
#define IMU_BMI270_ALIGN       CW270_DEG
#define BMI270_CS_PIN          PB12
#define BMI270_SPI_BUS         BUS_SPI2

// **** BNO IMU ****// 
#define USE_IMU_BNO055
#define IMU_BNO055_ALIGN       CW90_DEG
#define BNO055_CS_PIN          PB12
#define BNO055_SPI_BUS         BUS_SPI2

// **** SPI OSD ****// 
//#define USE_MAX7456
//#define MAX7456_SPI_BUS         BUS_SPI2
//#define MAX7456_CS_PIN          PC1

// *************** SPI3 Flash/SD Card  ****************uncomment to use SPI3
//#define USE_SPI_DEVICE_3
//#define SPI3_SCK_PIN            PC10
//#define SPI3_MISO_PIN   	    PC11
//#define SPI3_MOSI_PIN   	    PC12
// ***************  SD BLACKBOX*******************
#define USE_SDCARD
// *************** SPI SD *******************uncomment to use SPI3
//#define USE_SDCARD_SPI
//#define SDCARD_SPI_BUS          BUS_SPI3
//#define SDCARD_CS_PIN           PA15

// *************** SDIO SD *******************uncomment to use SDIO
#define USE_SDCARD_SDIO
#define SDCARD_SDIO_DEVICE      SDIODEV_1
#define SDCARD_SDIO_4BIT
#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT
//#else
//#   define USE_FLASHFS
//#   define USE_FLASH_M25P16
//#   define M25P16_SPI_BUS          BUS_SPI3
//#   define M25P16_CS_PIN           PC13
//#   define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT
//#endif

// *************** I2C /Baro/Mag/Pitot ********************

#define USE_I2C
#define USE_I2C_DEVICE_2
#define I2C2_SCL                PB10
#define I2C2_SDA                PB11

//** BARO **//

#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C2
#define USE_BARO_BMP280
#define USE_BARO_MS5611
#define USE_BARO_BMP085
#define USE_BARO_DPS310
#define USE_BARO_SPL06

//** MAG **//

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C2
#define USE_MAG_HMC5883
#define USE_MAG_QMC5883
#define USE_MAG_QMC5883P
#define USE_MAG_IST8310
#define USE_MAG_MAG3110
#define USE_MAG_LIS3MDL
#define USE_MAG_MPU9250

//#define USE_MAG_ALL


// *************** I2C PITOT*****************************
#define PITOT_I2C_BUS           BUS_I2C2
// *************** I2C TEMP*****************************
#define TEMPERATURE_I2C_BUS     BUS_I2C2
// *************** I2C RANGE*****************************
#define USE_RANGEFINDER
#define USE_RANGEFINDER_MSP
#define RANGEFINDER_I2C_BUS     BUS_I2C2


// *************** UART *****************************
#define USE_VCP

//UART1_RX_PIN                  PA10 // TIM1_CH3
//UART1_TX_PIN                  PA9 // TIM1_CH2

#define USE_UART1
#define UART1_TX_PIN            PB6
#define UART1_RX_PIN            PB7

#define USE_UART2
#define UART2_TX_PIN            PA2
#define UART2_RX_PIN            PA3

//#define USE_UART3
//#define UART3_TX_PIN            PC10
//#define UART3_RX_PIN            PC11

#define USE_UART4
#define UART4_TX_PIN            PA0
#define UART4_RX_PIN            PA1

//#define USE_UART5
//#define UART5_TX_PIN            PC12
//#define UART5_RX_PIN            PD2

#define USE_UART6
#define UART6_TX_PIN            PC6 // TIM8_CH1
#define UART6_RX_PIN            PC7 // TIM8_CH2

//#define USE_SOFTSERIAL1
//#define SOFTSERIAL_1_TX_PIN      PC6
//#define SOFTSERIAL_1_RX_PIN      PC7

//#define USE_SOFTSERIAL2
//#define SOFTSERIAL_2_TX_PIN      PA11
//#define SOFTSERIAL_2_RX_PIN      PA8

#define SERIAL_PORT_COUNT       5

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_CRSF
#define SERIALRX_UART           SERIAL_PORT_USART2


// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE                ADC1
#define ADC1_DMA_STREAM             DMA2_Stream4
#define ADC_CHANNEL_1_PIN           PC4
#define ADC_CHANNEL_2_PIN           PC5
#define ADC_CHANNEL_3_PIN           PB0
#define ADC_CHANNEL_4_PIN           PC0
#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2
#define RSSI_ADC_CHANNEL            ADC_CHN_3
#define AIRSPEED_ADC_CHANNEL        ADC_CHN_4

// *************** PINIO ***************************
//#define USE_PINIO
//#define USE_PINIOBOX
//#define PINIO1_PIN                  PA4
//#define PINIO2_PIN                  PA5

// *************** LEDSTRIP ************************
#define USE_LED_STRIP
#define WS2811_PIN                  PB1

// ***************  OTHERS *************************
#define DEFAULT_FEATURES        (FEATURE_TX_PROF_SEL | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TELEMETRY | FEATURE_SOFTSERIAL ) //FEATURE_OSD | 
#define VBAT_SCALE_DEFAULT      2100
#define CURRENT_METER_SCALE     423 

//#define USE_SPEKTRUM_BIND
//#define BIND_PIN                PA3 //  RX2

#define USE_DSHOT
#define USE_ESC_SENSOR
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         (BIT(2))

#define MAX_PWM_OUTPUT_PORTS    10

#define USE_SERIAL_4WAY_BLHELI_INTERFACE
#define USE_DSHOT
#define USE_DSHOT_DMAR
#define USE_ESC_SENSOR
