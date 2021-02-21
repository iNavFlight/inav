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

#pragma once

// leor // #else
#define TARGET_BOARD_IDENTIFIER "M41R"
#define USBD_PRODUCT_STRING     "MATEKF411RX"
// leor // #endif

// leor // #define LED0_PIN                PC13
#define LED0                    PC13

// leor // #define USE_BEEPER
#define BEEPER                  PC15
#define BEEPER_INVERTED

#define USE_SPI

// leor // #define ENABLE_DSHOT_DMAR       DSHOT_DMAR_AUTO
// leor // #define DSHOT_BITBANG_DEFAULT   DSHOT_BITBANG_OFF

// *************** SPI1 Gyro & ACC **********************
#define USE_SPI_DEVICE_1

#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

// leor // #define GYRO_1_CS_PIN           PA4
#define MPU6000_CS_PIN          PA4
// leor // #define GYRO_1_SPI_INSTANCE     SPI1
#define MPU6000_SPI_BUS     BUS_SPI1

#define USE_EXTI
// leor // #define USE_GYRO_EXTI
// leor // #define GYRO_1_EXTI_PIN         PA1
#define GYRO_INT_EXTI            PA1
#define USE_MPU_DATA_READY_SIGNAL

// leor // #define USE_GYRO
// leor // #define USE_GYRO_SPI_MPU6000
#define USE_IMU_MPU6000
// leor // #if defined(CRAZYBEEF4FS) || defined(CRAZYBEEF4FR) || defined(CRAZYBEEF4DX)
// leor // #else
// leor // #define GYRO_1_ALIGN            CW180_DEG
#define IMU_MPU6000_ALIGN            CW180_DEG
// leor // #endif
// leor // #define USE_ACC
// leor // #define USE_ACC_SPI_MPU6000
// *************** SPI2 OSD *****************************
#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15

// leor // USE_OSD added
#define USE_OSD
#define USE_MAX7456
// leor // #define MAX7456_SPI_INSTANCE    SPI2
#define MAX7456_SPI_BUS         BUS_SPI2
// leor // #define MAX7456_SPI_CS_PIN      PB12
#define MAX7456_CS_PIN          PB12

// leor // #if defined(CRAZYBEEF4DX)

// leor // #else
// *************** SPI3 CC2500 ***************************
#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            PB3
#define SPI3_MISO_PIN           PB4
#define SPI3_MOSI_PIN           PB5
#define RX_NSS_PIN              PA15

#define USE_RX_SPI
#define RX_SPI_INSTANCE         SPI3

// leor // #define RX_SPI_BIND_PIN         PB2
#define BIND_PIN         PB2

// leor // #define DEFAULT_RX_FEATURE      FEATURE_RX_SPI
#define DEFAULT_RX_TYPE         RX_TYPE_SPI

// leor // #if defined(CRAZYBEEF4FS)

// leor // #elif defined(CRAZYBEEF4FR)

// leor // #else // MATEKF411RX
#define RX_CC2500_SPI_DISABLE_CHIP_DETECTION
#define RX_SPI_EXTI_PIN             PC14
#define RX_SPI_LED_PIN              PB9
#define RX_SPI_LED_INVERTED

#define USE_RX_CC2500_SPI_PA_LNA
#define RX_CC2500_SPI_TX_EN_PIN      PA8
#define RX_CC2500_SPI_LNA_EN_PIN     PA13

#define USE_RX_CC2500_SPI_DIVERSITY
#define RX_CC2500_SPI_ANT_SEL_PIN    PA14

#define USE_RX_FRSKY_SPI_D
#define USE_RX_FRSKY_SPI_X
#define USE_RX_SFHSS_SPI
#define USE_RX_REDPINE_SPI
#define RX_SPI_DEFAULT_PROTOCOL RX_SPI_FRSKY_X
#define USE_RX_FRSKY_SPI_TELEMETRY
// leor // #endif
// leor // #endif // CRAZYBEEF4DX

// *************** UART *****************************
#define USE_VCP

#define USE_UART1
#define UART1_TX_PIN            PA9
#define UART1_RX_PIN            PA10

#define USE_UART2
#define UART2_TX_PIN            PA2
#define UART2_RX_PIN            PA3

#define USE_SOFTSERIAL1
#define SOFTSERIAL_1_TX_PIN     PA0 // ST1 pad
#define SOFTSERIAL_1_RX_PIN     PA0
#define SERIAL_PORT_COUNT       4

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART1

// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE         ADC1  // Default added
// leor // #define ADC1_DMA_OPT            0  // DMA 2 Stream 0 Channel 0
#define ADC1_DMA_STREAM             DMA2_Stream0

// leor // #define VBAT_ADC_PIN            PB0
// leor // #define CURRENT_METER_ADC_PIN   PB1
#define ADC_CHANNEL_1_PIN           PB0
#define ADC_CHANNEL_2_PIN           PB1
#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2

// ***************  OTHERS *************************
// leor // #define USE_ESCSERIAL
#define USE_DSHOT
#define USE_ESC_SENSOR
#define USE_SERIALSHOT
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

// leor // #define DEFAULT_FEATURES                (FEATURE_OSD | FEATURE_TELEMETRY )
// leor // #define DEFAULT_VOLTAGE_METER_SOURCE    VOLTAGE_METER_ADC
// leor // #define DEFAULT_CURRENT_METER_SOURCE    CURRENT_METER_ADC
#define DEFAULT_FEATURES        (FEATURE_TX_PROF_SEL | FEATURE_OSD | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TELEMETRY )
#define CURRENT_METER_SCALE_DEFAULT 179

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         (BIT(2))

// leor // #define USABLE_TIMER_CHANNEL_COUNT 9
// leor // #define USED_TIMERS             ( TIM_N(1)|TIM_N(2)|TIM_N(4)|TIM_N(5)|TIM_N(9))
#define MAX_PWM_OUTPUT_PORTS       7