/* BLACKPILLSYN FC Shield
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

#define TARGET_BOARD_IDENTIFIER "BLPLSYDU"

#define USBD_PRODUCT_STRING  "BLACKPILLSYN"

#define LED0                    PC13
#define LED1                    NONE

#define BEEPER                  PC14
#define BEEPER_INVERTED

// *************** SPI1 Gyro & ACC **********************
#define USE_SPI
#define USE_SPI_DEVICE_1

#define SPI1_NSS_PIN            PA4

#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define USE_IMU_MPU6000
#define IMU_MPU6000_ALIGN       CW90_DEG
#define MPU6000_CS_PIN          PA4
#define MPU6000_SPI_BUS         BUS_SPI1

#define USE_IMU_MPU6500
#define IMU_MPU6500_ALIGN       CW90_DEG
#define MPU6500_CS_PIN          PA4
#define MPU6500_SPI_BUS         BUS_SPI1

#define USE_IMU_MPU9250
#define IMU_MPU9250_ALIGN       CW90_DEG
#define MPU9250_CS_PIN          PA4
#define MPU9250_SPI_BUS         BUS_SPI1

// **** BMI IMU ****// 

#define USE_IMU_BMI160
#define IMU_BMI160_ALIGN       CW90_DEG
#define BMI160_CS_PIN          PA4
#define BMI160_SPI_BUS         BUS_SPI1


#define USE_IMU_BMI270
#define IMU_BMI270_ALIGN       CW90_DEG
#define BMI270_CS_PIN          PA4
#define BMI270_SPI_BUS         BUS_SPI1


#define USE_EXTI
#define GYRO_INT_EXTI            PA1
#define USE_MPU_DATA_READY_SIGNAL

// *************** SPI2 OSD *****************************
#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15

#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI2
#define MAX7456_CS_PIN          PB12

// *************** UART *****************************
#define USE_VCP
#define VBUS_SENSING_PIN        PC15
#define VBUS_SENSING_ENABLED

#define USE_UART1
#define UART1_TX_PIN            PA9
#define UART1_RX_PIN            PA10

#define USE_UART2
#define UART2_TX_PIN            PA2
#define UART2_RX_PIN            PA3


#define USE_SOFTSERIAL1
#define SOFTSERIAL_1_TX_PIN     PB6 
#define SOFTSERIAL_1_RX_PIN     PB7
#define SERIAL_PORT_COUNT       4

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART2

// *************** I2C /Baro/Mag/Pitot ********************
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9

#define DEFAULT_I2C_BUS         BUS_I2C1

//*** as of INAV5.1.0 and INAV 6.0.0 Discontinued use of I2C 
//***sensors for GYRO and ACC

//#define USE_IMU_MPU6050
//#define IMU_MPU6050_ALIGN       CW90_DEG
//#define MPU6050_I2C_BUS         BUS_I2C1


//** BARO **//

#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C1
#define USE_BARO_BMP280
#define USE_BARO_MS5611
#define USE_BARO_BMP085
#define USE_BARO_DPS310
#define USE_BARO_SPL06
#define USE_BARO_BMP085

//** MAG **//

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_HMC5883
#define USE_MAG_QMC5883
#define USE_MAG_IST8310
#define USE_MAG_MAG3110
#define USE_MAG_LIS3MDL
#define USE_MAG_MPU9250




//** PITOT **//


#define PITOT_I2C_BUS           BUS_I2C1
#define TEMPERATURE_I2C_BUS     BUS_I2C1
#define BNO055_I2C_BUS          BUS_I2C1

// ********** Optiical Flow adn Lidar **************

#define USE_RANGEFINDER

#define USE_RANGEFINDER_MSP
#define USE_RANGEFINDER_BENEWAKE
#define USE_RANGEFINDER_VL53L0X
#define USE_RANGEFINDER_VL53L1X
#define USE_RANGEFINDER_US42
#define USE_RANGEFINDER_TOF10120_I2C


// Optic flow boards

#define USE_OPFLOW

#define USE_OPFLOW_MSP
#define USE_OPFLOW_CXOF



// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE                ADC1
#define ADC1_DMA_STREAM             DMA2_Stream0
#define ADC_CHANNEL_1_PIN           PA1
#define ADC_CHANNEL_2_PIN           PB1
#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2


// *************** LED2811 ************************

#define USE_LED_STRIP
#define WS2811_PIN                      PB5


// ***************  OTHERS *************************
#define DEFAULT_FEATURES        (FEATURE_TX_PROF_SEL | FEATURE_OSD | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TELEMETRY | FEATURE_SOFTSERIAL )

#define USE_SPEKTRUM_BIND
#define BIND_PIN                PA10 //  RX1

#define USE_DSHOT
#define USE_ESC_SENSOR

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         (BIT(2))

#define MAX_PWM_OUTPUT_PORTS       7
