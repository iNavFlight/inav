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

//this is Part of INAV Sourcecode, more exactly target.h for MAMBAF722, modified by LTwin8 to fit HIFIONRCF7 AIO and HIFIONRCF7 PRO

//Used resources are the INAV-Code and the Betaflight unified target file https://raw.githubusercontent.com/betaflight/unified-targets/master/configs/default/JHEF-JHEF7DUAL.config

#pragma once

#define USE_TARGET_CONFIG

#define TARGET_BOARD_IDENTIFIER         "JHEF"
#define USBD_PRODUCT_STRING             "JHEF7DUAL"

// ******** Board LEDs  **********************
#define LED0                            PA15                            //taken from unified target
//#define LED1                            PC14                          //No 2nd LED defined in unified target

// ******* Beeper ***********
#define BEEPER                          PC15                             //taken from unified target
#define BEEPER_INVERTED


// ******* GYRO and ACC ********
#define USE_EXTI
#define GYRO_INT_EXTI                   PC3                             //taken from unified target, additional Interrupt available on C03
#define USE_MPU_DATA_READY_SIGNAL

#define MPU6000_CS_PIN                  SPI1_NSS_PIN                    //B02 & A04, taken from unified target
#define MPU6000_SPI_BUS                 BUS_SPI1

#define USE_IMU_MPU6000
#define IMU_MPU6000_ALIGN               CW90_DEG                        //taken from unified target

// *************** Baro **************************
#define USE_I2C
#define USE_I2C_DEVICE_1
#define USE_I2C_PULLUP
#define I2C1_SCL                        PB6        // SCL pad TX3       //taken from unified target
#define I2C1_SDA                        PB7        // SDA pad RX3       //taken from unified target
#define DEFAULT_I2C_BUS                 BUS_I2C1

#define USE_BARO
#define BARO_I2C_BUS                    DEFAULT_I2C_BUS

#define USE_BARO_BMP280
#define USE_BARO_BMP085
#define USE_BARO_MS5611


// *************** PINIO **************************
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO2_PIN                  PB9 // VTX power switcher
#define PINIO1_PIN                  PC14 // 2xCamera switcher


//*********** Magnetometer / Compass *************
//#define USE_MAG
//#define MAG_I2C_BUS                     DEFAULT_I2C_BUS               //taken from unified target, no mag onboard

//#define USE_MAG_HMC5883
//#define USE_MAG_QMC5883
//#define USE_MAG_LIS3MDL

// ******* SERIAL ********
#define USE_VCP
#define USE_UART1
#define USE_UART2
#define USE_UART3
#define USE_UART4
#define USE_UART5
#define USE_UART6
//#define USE_SOFTSERIAL1
//SoftSerial not needed, 6 full UARTs available, for example: GPS, ESC-telemetry, S.Port, VTX (TBS, IRC or DJI), Bluetooth, 1 free UART left, 2 free UARTS left if using only one uart for RC&telemetry (F.Port or CRSF for example)

#define UART1_TX_PIN                    PA9                             //taken from unified target
#define UART1_RX_PIN                    PA10                            //taken from unified target

#define UART2_TX_PIN                    PA2                             //taken from unified target
#define UART2_RX_PIN                    PA3                             //taken from unified target

#define UART3_TX_PIN                    PB10                            //taken from unified target
#define UART3_RX_PIN                    PB11                            //taken from unified target

#define UART4_TX_PIN                    PA0                             //taken from unified target
#define UART4_RX_PIN                    PA1                             //taken from unified target

#define UART5_TX_PIN                    PC12                            //taken from unified target
#define UART5_RX_PIN                    PD2                             //taken from unified target

#define UART6_TX_PIN                    PC6                             //taken from unified target
#define UART6_RX_PIN                    PC7                             //taken from unified target
/*
#define SOFTSERIAL_1_TX_PIN             PA2                             //dont need this, we have 6 UARTs in HW
#define SOFTSERIAL_1_RX_PIN             PA2
*/
#define SERIAL_PORT_COUNT               6

// ******* SPI ********
#define USE_SPI

#define USE_SPI_DEVICE_1                                                //taken from unified target //MPU6000
#define SPI1_NSS_PIN                    PA4                             //taken from unified target //Gyro CS 2
#define SPI1_SCK_PIN                    PA5                             //taken from unified target
#define SPI1_MISO_PIN                   PA6                             //taken from unified target
#define SPI1_MOSI_PIN                   PA7                             //taken from unified target

#define USE_SPI_DEVICE_2                                                //taken from unified target //OSD Chip
#define SPI2_NSS_PIN                    PB12                            //taken from unified target
#define SPI2_SCK_PIN                    PB13                            //taken from unified target
#define SPI2_MISO_PIN                   PB14                            //taken from unified target
#define SPI2_MOSI_PIN                   PB15                            //taken from unified target

#define USE_SPI_DEVICE_3                                                //taken from unified target // BlackBox Flash
#define SPI3_NSS_PIN                    PC13                            //taken from unified target
#define SPI3_SCK_PIN                    PC10                            //taken from unified target
#define SPI3_MISO_PIN                   PC11                            //taken from unified target
#define SPI3_MOSI_PIN                   PB5                             //taken from unified target

// ******* ADC ********
#define USE_ADC
#define ADC_CHANNEL_1_PIN               PC2                             //taken from unified target
#define ADC_CHANNEL_2_PIN               PC0                             //taken from unified target
#define ADC_CHANNEL_3_PIN               PC1                             //taken from unified target

#define VBAT_ADC_CHANNEL                ADC_CHN_1
#define RSSI_ADC_CHANNEL                ADC_CHN_2
#define CURRENT_METER_ADC_CHANNEL       ADC_CHN_3

#define VBAT_SCALE_DEFAULT              1100

// ******* OSD ********
#define USE_OSD                                                         //matches unified target
#define USE_MAX7456
#define MAX7456_SPI_BUS                 BUS_SPI2
#define MAX7456_CS_PIN                  SPI2_NSS_PIN

//******* FLASH ********
#define USE_FLASHFS                                                     //matches unified target, exact chip type not known yet, but seems right, the 30.5mm HIFIONRC has 16MB BB
#define USE_FLASH_M25P16
#define M25P16_CS_PIN                   SPI3_NSS_PIN
#define M25P16_SPI_BUS                  BUS_SPI3

#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT                  //yes, no SD Card

//************ LEDSTRIP *****************
#define USE_LED_STRIP
#define WS2811_PIN                      PA8                             //taken from unified target

// ******* FEATURES ********
#define DEFAULT_RX_FEATURE              FEATURE_RX_SERIAL
#define SERIALRX_UART                   SERIAL_PORT_USART2              //taken from Product Page
#define SERIALRX_PROVIDER               SERIALRX_SBUS

#define DEFAULT_FEATURES                (FEATURE_OSD | FEATURE_TELEMETRY) //no SoftSerial

#define TARGET_IO_PORTA                 0xffff                          //not sure
#define TARGET_IO_PORTB                 0xffff                          //not sure
#define TARGET_IO_PORTC                 0xffff                          //not sure
#define TARGET_IO_PORTD                 (BIT(2))                        //not sure

#define MAX_PWM_OUTPUT_PORTS            8                               //not sure, would guess 4, because its aiming towards quadcopters
#define TARGET_MOTOR_COUNT              4                               //taken from unified target, Motors 1,2,3,4 are located on PB0, PB1, PB4, PB3

// ESC-related features
#define USE_DSHOT
#define USE_SERIALSHOT
#define USE_ESC_SENSOR
#define USE_SERIAL_4WAY_BLHELI_INTERFACE
