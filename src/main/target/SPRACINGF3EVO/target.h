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

#if defined(SPRACINGAIRBIT)
#define TARGET_BOARD_IDENTIFIER "SPAB"
#else
#define TARGET_BOARD_IDENTIFIER "SPEV"
#endif

#define LED0                    PB8

#define BEEPER                  PC15
#define BEEPER_INVERTED

#define USE_EXTI
#define GYRO_INT_EXTI            PC13
#define USE_MPU_DATA_READY_SIGNAL
#define ENSURE_MPU_DATA_READY_IS_LOW

#define MPU6500_CS_PIN          PB9
#define MPU6500_SPI_BUS         BUS_SPI1

#define MPU9250_CS_PIN          PB9
#define MPU9250_SPI_BUS         BUS_SPI1

#define USE_GYRO
#define USE_GYRO_MPU6500
#define GYRO_MPU6500_ALIGN      CW180_DEG
#define USE_GYRO_MPU9250
#define GYRO_MPU9250_ALIGN      CW180_DEG

#define USE_ACC
#define USE_ACC_MPU6500
#define ACC_MPU6500_ALIGN       CW180_DEG
#define USE_ACC_MPU9250
#define ACC_MPU9250_ALIGN       CW180_DEG

#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C1

#ifdef SPRACINGAIRBIT
#define USE_BARO_BMP388
#else
#define USE_BARO_BMP280
#define USE_BARO_MS5611
#endif

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define MAG_MPU9250_ALIGN       CW270_DEG
#define USE_MAG_MPU9250
#define USE_MAG_HMC5883
#define USE_MAG_QMC5883
#define USE_MAG_IST8310
#define USE_MAG_IST8308
#define USE_MAG_MAG3110
#define USE_MAG_LIS3MDL

#define USE_VCP
#define USE_UART1
#define USE_UART2
#define USE_UART3

#define UART1_TX_PIN            PA9
#define UART1_RX_PIN            PA10

#define UART2_TX_PIN            PA14 // PA14 / SWCLK
#define UART2_RX_PIN            PA15

#define UART3_TX_PIN            PB10 // PB10 (AF7)
#define UART3_RX_PIN            PB11 // PB11 (AF7)

#if defined(SPRACINGF3AIRBIT)
    // no softserial
#elif defined(SPRACINGF3EVO_1SS)
    #define USE_SOFTSERIAL1
    #define SOFTSERIAL_1_RX_PIN     PB0
    #define SOFTSERIAL_1_TX_PIN     PB1
#else
    #define USE_SOFTSERIAL1
    #define SOFTSERIAL_1_RX_PIN     PA6
    #define SOFTSERIAL_1_TX_PIN     PA7

    #define USE_SOFTSERIAL2
    #define SOFTSERIAL_2_RX_PIN     PB0
    #define SOFTSERIAL_2_TX_PIN     PB1
#endif

#if defined(USE_SOFTSERIAL1) && defined (USE_SOFTSERIAL2)
    #define SERIAL_PORT_COUNT       6
#elif defined(USE_SOFTSERIAL1) || defined (USE_SOFTSERIAL2)
    #define SERIAL_PORT_COUNT       5
#else
    #define SERIAL_PORT_COUNT       4
#endif

#define USE_I2C
#define USE_I2C_DEVICE_1

#define USE_SPI
#define USE_SPI_DEVICE_1 // PB9,3,4,5 on AF5 SPI1 (MPU)
#define USE_SPI_DEVICE_2 // PB12,13,14,15 on AF5 SPI2 (SDCard)

#define SPI1_NSS_PIN            PB9
#define SPI1_SCK_PIN            PB3
#define SPI1_MISO_PIN           PB4
#define SPI1_MOSI_PIN           PB5

#define SPI2_NSS_PIN            PB12
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15

#define USE_SDCARD
#define USE_SDCARD_SPI
#define SDCARD_DETECT_INVERTED
#define SDCARD_DETECT_PIN       PC14
#define SDCARD_SPI_BUS          BUS_SPI2
#define SDCARD_CS_PIN           SPI2_NSS_PIN

#define USE_ADC
#define ADC_INSTANCE            ADC2
#define ADC_CHANNEL_1_PIN               PA4
#define ADC_CHANNEL_2_PIN               PA5
#define ADC_CHANNEL_3_PIN               PB2
#define VBAT_ADC_CHANNEL                ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL       ADC_CHN_2
#define RSSI_ADC_CHANNEL                ADC_CHN_3

#ifdef SPRACINGAIRBIT
// No RSSI                      (PB2 Used for BARO INT)
#undef ADC_CHANNEL_3_PIN
#undef RSSI_ADC_CHANNEL
#endif


#define USE_LED_STRIP
#define WS2811_PIN                      PA8

#define USE_RANGEFINDER
#define RANGEFINDER_I2C_BUS             BUS_I2C1
#define USE_RANGEFINDER_HCSR04
#define RANGEFINDER_HCSR04_TRIGGER_PIN       PB0
#define RANGEFINDER_HCSR04_ECHO_PIN          PB1
#define USE_RANGEFINDER_SRF10

#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

#ifdef SPRACINGAIRBIT
// SPRacingAIRBIT is designed specifically for SERIAL_RX SPEKTRUM1024 + LTM telemetry or RX via MSP.
#define USE_TELEMETRY
#define USE_TELEMETRY_LTM
#undef USE_TELEMETRY_FRSKY
#define USE_RX_MSP

#undef USE_RX_PPM
#undef USE_RX_PWM
#undef USE_SERIALRX_CRSF       // Team Black Sheep Crossfire protocol
#undef USE_SERIALRX_IBUS       // FlySky and Turnigy receivers
#undef USE_SERIALRX_SBUS       // Frsky and Futaba receivers
#undef USE_SERIALRX_SUMD       // Graupner Hott protocol

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#else
#define DEFAULT_RX_TYPE         RX_TYPE_PPM
#endif

#ifdef SPRACINGAIRBIT
#define USE_RANGEFINDER
#define USE_RANGEFINDER_MSP
#define USE_OPFLOW
#define USE_OPFLOW_MSP
#endif

#ifdef SPRACINGAIRBIT
#define DEFAULT_FEATURES        (FEATURE_TX_PROF_SEL | FEATURE_BLACKBOX | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TELEMETRY)
#else
#define DEFAULT_FEATURES        (FEATURE_TX_PROF_SEL | FEATURE_BLACKBOX | FEATURE_RSSI_ADC | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TELEMETRY)
#endif

#define USE_SPEKTRUM_BIND
#define BIND_PIN                PB11 // UART3

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

// Number of available PWM outputs
#define MAX_PWM_OUTPUT_PORTS    10

// IO - stm32f303cc in 48pin package
#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         (BIT(13)|BIT(14)|BIT(15))
#define TARGET_IO_PORTF         (BIT(0)|BIT(1)|BIT(4))
