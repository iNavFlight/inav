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

#define TARGET_BOARD_IDENTIFIER "AXHP"
#define USBD_PRODUCT_STRING     "AXISFLYINGH743PRO"

#define USE_TARGET_CONFIG

// *************** LED, BEEPER ***************
#define LED0                    PD3
#define BEEPER                  PC13
#define BEEPER_INVERTED

// *************** Gyro / Acc - dual ICM42688P (registered as DEVHW_ICM42605,
// whose WHO_AM_I switch also matches ICM42688P) ***************
#define USE_DUAL_GYRO
#define USE_TARGET_IMU_HARDWARE_DESCRIPTORS     // Don't use common busdev descriptors for IMU
#define USE_IMU_ICM42605

// SPI1: Gyro 1
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define GYRO_1_SPI_BUS           BUS_SPI1
#define GYRO_1_CS_PIN            PC4
#define GYRO_1_EXTI_PIN          PA4
#define GYRO_1_ALIGN             CW0_DEG

// SPI4: Gyro 2
#define USE_SPI_DEVICE_4
#define SPI4_SCK_PIN             PE12
#define SPI4_MISO_PIN            PE13
#define SPI4_MOSI_PIN            PE14

#define GYRO_2_SPI_BUS           BUS_SPI4
#define GYRO_2_CS_PIN            PE15
#define GYRO_2_EXTI_PIN          PE11
#define GYRO_2_ALIGN             CW90_DEG

// Motor/servo DMA note: SERVO4_PIN (PD15/TIM4_CH4) has no per-channel DMAMUX
// request on this MCU; USE_DSHOT_DMAR repoints TIM4_CH4 at the TIM4_UP burst
// request so the channel keeps a valid DMA path. This makes all DSHOT-capable
// outputs use one shared burst DMA stream per timer
#define USE_DSHOT_DMAR

// *************** SPI2: OSD *****************
#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN             PB13
#define SPI2_MISO_PIN            PB14
#define SPI2_MOSI_PIN            PB15

#define USE_MAX7456
#define MAX7456_SPI_BUS          BUS_SPI2
#define MAX7456_CS_PIN           PB12

// *************** SPI3: Flash ***************
#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN             PB3
#define SPI3_MISO_PIN            PB4
#define SPI3_MOSI_PIN            PB5

#define USE_FLASHFS
#define USE_FLASH_M25P16
#define M25P16_SPI_BUS           BUS_SPI3
#define M25P16_CS_PIN            PD7
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

// *************** I2C1: Mag (external) *********************
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                 PB8
#define I2C1_SDA                 PB9

#define USE_MAG
#define MAG_I2C_BUS              BUS_I2C1
#define USE_MAG_ALL

// *************** I2C2: Baro *********************
#define USE_I2C_DEVICE_2
#define I2C2_SCL                 PB10
#define I2C2_SDA                 PB11

#define USE_BARO
#define BARO_I2C_BUS             BUS_I2C2
#define USE_BARO_DPS310

// *************** Serial ports *****************************
#define USE_VCP

#define USE_UART1
#define UART1_TX_PIN             PB6
#define UART1_RX_PIN             PB7

#define USE_UART2
#define UART2_TX_PIN             PD5
#define UART2_RX_PIN             PD6

#define USE_UART3
#define UART3_TX_PIN             PD8
#define UART3_RX_PIN             PD9

#define USE_UART4
#define UART4_TX_PIN             PC10
#define UART4_RX_PIN             PC11

// RX only - hard-wired to ESC telemetry, see config.c
#define USE_UART5
#define UART5_TX_PIN             PC12  // Unconnected placeholder: H7 UART5 driver's .af_tx lacks an #ifdef UART5_TX_PIN guard
#define UART5_RX_PIN             PD2

#define USE_UART7
#define UART7_TX_PIN             PE8
#define UART7_RX_PIN             PE7

#define USE_UART8
#define UART8_TX_PIN             PE1
#define UART8_RX_PIN             PE0

#define SERIAL_PORT_COUNT        8

#define DEFAULT_FEATURES         (FEATURE_OSD | FEATURE_TELEMETRY | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TX_PROF_SEL | FEATURE_BLACKBOX)
#define DEFAULT_RX_TYPE          RX_TYPE_SERIAL
#define SERIALRX_PROVIDER        SERIALRX_CRSF
#define SERIALRX_UART            SERIAL_PORT_USART2

#define GPS_UART                 SERIAL_PORT_USART8

// Default MSP DisplayPort (HD OSD) port
#define MSP_UART                 SERIAL_PORT_USART1

#define USE_ESC_SENSOR

// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE                ADC1
#define ADC_CHANNEL_1_PIN           PC0
#define ADC_CHANNEL_2_PIN           PC1
#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2

#define DEFAULT_VOLTAGE_METER_SOURCE   VOLTAGE_METER_ADC
#define DEFAULT_CURRENT_METER_SOURCE   CURRENT_METER_ADC

// *************** LED2812 ************************
#define USE_LED_STRIP
#define WS2811_PIN               PE5

// *************** PINIO ***************************
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN               PE2   // VTX PWR box
#define PINIO2_PIN               PA8   // CAM 1,2 box

// *************** Blackbox ************************
#define DEFAULT_BLACKBOX_DEVICE        BLACKBOX_DEVICE_FLASH

// ***************  OTHERS *************************
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff
#define TARGET_IO_PORTE         0xffff

#define USE_DSHOT
#define MAX_PWM_OUTPUT_PORTS     12
