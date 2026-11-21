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

#define TARGET_BOARD_IDENTIFIER "M7LT"
#define USBD_PRODUCT_STRING     "MICOAIR743LITE"
#define VBUS_SENSING_PIN        PA8

#define LED0                    PE3
#define LED1                    PE4
#define LED2                    PE2

#define USE_LED_STRIP
#define WS2811_PIN              PD14
#define BEEPER                  PD15
#define BEEPER_INVERTED

// The Lite has one IMU and no SPI OSD chip.
#define USE_SPI
#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            PB3
#define SPI3_MISO_PIN           PB4
#define SPI3_MOSI_PIN           PD6
#define USE_IMU_ICM45686
#define USE_TARGET_IMU_HARDWARE_DESCRIPTORS
#define ICM45686_SPI_BUS        BUS_SPI3
#define ICM45686_CS_PIN         PD4
#define ICM45686_EXTI_PIN       PD5
#define IMU_ICM45686_ALIGN      CW90_DEG

#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9
#define USE_I2C_DEVICE_2
#define I2C2_SCL                PB10
#define I2C2_SDA                PB11

// SPA06 is polled over the internal bus; external sensors use I2C1.
#define USE_BARO
#define USE_BARO_DPS310
#define DPS310_I2C_BUS          BUS_I2C2
#define DPS310_I2C_ADDR         0x77
#define DEFAULT_I2C_BUS         BUS_I2C1
#define USE_MAG
#define USE_MAG_ALL
#define MAG_I2C_BUS             DEFAULT_I2C_BUS

#define USE_VCP
#define USE_UART1
#define UART1_TX_PIN            PA9
#define UART1_RX_PIN            PA10
#define USE_UART2
#define UART2_TX_PIN            PA2
#define UART2_RX_PIN            PA3
#define USE_UART3
#define UART3_TX_PIN            PD8
#define UART3_RX_PIN            PD9
#define USE_UART4
#define UART4_TX_PIN            PA0
#define UART4_RX_PIN            PA1
#define USE_UART5
#define UART5_TX_PIN            PB6
#define UART5_RX_PIN            PB5
#define USE_UART6
#define UART6_TX_PIN            PC6
#define UART6_RX_PIN            PC7
#define USE_UART7
#define UART7_RX_PIN            PE7  // ESC telemetry; TX is not connected.
#define USE_UART8
#define UART8_TX_PIN            PE1
#define UART8_RX_PIN            PE0
#define SERIAL_PORT_COUNT       9

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART6

#define USE_SDCARD
#define USE_SDCARD_SDIO
#define SDCARD_SDIO_DEVICE      SDIODEV_1
#define SDCARD_SDIO_4BIT
#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

#define USE_ADC
#define ADC_INSTANCE                ADC1
#define ADC_CHANNEL_1_PIN           PC0
#define ADC_CHANNEL_2_PIN           PC1
#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2
#define VBAT_SCALE_DEFAULT          2112
#define CURRENT_METER_SCALE         249  // 40.2 A/V expressed in 0.1 mV/A

#define USE_RANGEFINDER
#define USE_RANGEFINDER_MSP
#define USE_OPFLOW
#define USE_OPFLOW_MSP

// Leave the optional LED strip disabled so TIM4 is available for M11/M12.
#define DEFAULT_FEATURES        (FEATURE_VBAT | FEATURE_CURRENT_METER | FEATURE_OSD | FEATURE_TELEMETRY)
#define USE_DSHOT
#define USE_ESC_SENSOR
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff
#define TARGET_IO_PORTE         0xffff

#define MAX_PWM_OUTPUT_PORTS    14
