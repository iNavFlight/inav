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

#define TARGET_BOARD_IDENTIFIER "M7V3"

#define USBD_PRODUCT_STRING     "MICOAIR743V3"
#define VBUS_SENSING_PIN        PA8

// LEDs
#define LED0                    PE3
#define LED1                    PE4
#define LED2                    PE2

#define USE_LED_STRIP
#define WS2811_PIN              PD14

// Beeper
#define BEEPER                  PD15
#define BEEPER_INVERTED
// GPIO beeper: TIM4 is shared with M7/M8 and the LED strip.

// SPI1: AT7456E OSD
#define USE_SPI
#define USE_SPI_DEVICE_1

#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI1
#define MAX7456_CS_PIN          PB12

// Select one complete IMU: gyro_to_use=0 for BMI088, 1 for ICM45686.
#define USE_DUAL_GYRO
#define USE_TARGET_IMU_HARDWARE_DESCRIPTORS

// SPI2: BMI088 IMU
#define USE_SPI_DEVICE_2

#define SPI2_SCK_PIN            PD3
#define SPI2_MISO_PIN           PC2
#define SPI2_MOSI_PIN           PC3

#define USE_IMU_BMI088
#define IMU_BMI088_ALIGN        CW270_DEG
#define BMI088_SPI_BUS          BUS_SPI2
#define BMI088_GYRO_CS_PIN      PD5
#define BMI088_GYRO_EXTI_PIN    PC15
#define BMI088_ACC_CS_PIN       PD4
#define BMI088_ACC_EXTI_PIN     PC14

// SPI3: ICM45686 IMU
#define USE_SPI_DEVICE_3

#define SPI3_SCK_PIN            PC10
#define SPI3_MISO_PIN           PC11
#define SPI3_MOSI_PIN           PC12

#define USE_IMU_ICM45686
#define IMU_ICM45686_ALIGN      CW90_DEG
#define ICM45686_SPI_BUS        BUS_SPI3
#define ICM45686_CS_PIN         PA15
#define ICM45686_EXTI_PIN       PD2

// UARTs
#define USE_VCP

#define USE_UART1
#define UART1_RX_PIN            PA10
#define UART1_TX_PIN            PA9

#define USE_UART2
#define UART2_RX_PIN            PA3
#define UART2_TX_PIN            PA2

#define USE_UART3
#define UART3_RX_PIN            PD9
#define UART3_TX_PIN            PD8

#define USE_UART4
#define UART4_RX_PIN            PA1
#define UART4_TX_PIN            PA0

#define USE_UART6
#define UART6_RX_PIN            PC7
#define UART6_TX_PIN            PC6

#define USE_UART7
#define UART7_RX_PIN            PE7  // ESC telemetry; TX is not exposed.

#define USE_UART8
#define UART8_RX_PIN            PE0
#define UART8_TX_PIN            PE1

#define SERIAL_PORT_COUNT       8

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART6

#define USE_DRONECAN
#define CAN1_RX                 PB8
#define CAN1_TX                 PB9

// I2C1: external connector; I2C2: onboard SPA06 and IST8310
#define USE_I2C
#define USE_I2C_DEVICE_1
#define USE_I2C_DEVICE_2

#define I2C1_SCL                PB6
#define I2C1_SDA                PB7
#define I2C2_SCL                PB10
#define I2C2_SDA                PB11

// SPA06-003 uses the DPS310-compatible 0x11 device path and 21-byte coefficients.
// The I2C address is fixed at decimal 119 (0x77).
#define USE_BARO
#define USE_BARO_DPS310
#define DPS310_I2C_BUS          BUS_I2C2
#define DPS310_I2C_ADDR         0x77

#define USE_MAG
#ifdef MICOAIR743V3_EXTMAG
// External compass connector; probe all supported compass drivers on I2C1.
#define USE_MAG_ALL
#define MAG_I2C_BUS             BUS_I2C1
#else
// Onboard IST8310 at 0x0E; the driver probes both supported addresses.
#define USE_MAG_IST8310
#define MAG_I2C_BUS             BUS_I2C2
#endif

// MSP optical-flow and rangefinder support
#define USE_RANGEFINDER
#define USE_RANGEFINDER_MSP
#define USE_OPFLOW
#define USE_OPFLOW_MSP

// SDMMC2: PB14/PB15/PB3/PB4, clock PD6, command PD7
#define USE_SDCARD
#define USE_SDCARD_SDIO
#define SDCARD_SDIO_DEVICE      SDIODEV_2
#define SDCARD_SDIO_4BIT
#define SDCARD_SDIO2_CK_ALT
#define SDCARD_SDIO2_CMD_ALT
#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

// ADC
#define USE_ADC
#define ADC_INSTANCE                ADC1
#define ADC_CHANNEL_1_PIN           PC0
#define ADC_CHANNEL_2_PIN           PC1
#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2
#define VBAT_SCALE_DEFAULT          2112
// INAV uses 0.1 mV/A: 10000 / 40.2 A/V, rounded to the nearest integer.
#define CURRENT_METER_SCALE         249

// VTX power enable
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN              PE15

// Enabling LED_STRIP reserves TIM4 and makes M7/M8 unavailable.
#define DEFAULT_FEATURES        (FEATURE_VBAT | FEATURE_CURRENT_METER | FEATURE_OSD | FEATURE_TELEMETRY)

#define USE_DSHOT
#define USE_ESC_SENSOR
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff
#define TARGET_IO_PORTE         0xffff

// M1-M10; the PWM driver counts the WS2811 pad separately.
#define MAX_PWM_OUTPUT_PORTS    10
