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

#define TARGET_BOARD_IDENTIFIER "F7B5"

#define USBD_PRODUCT_STRING  "VANTAC_RF007"

#define LED0                    PC14
#define LED1                    PC15

#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define USE_IMU_ICM42605 // is actually ICM42688P
#define IMU_ICM42605_ALIGN     CW90_DEG
#define ICM42605_CS_PIN        PA4
#define ICM42605_EXTI_PIN      PB2
#define ICM42605_SPI_BUS       BUS_SPI1

// *************** I2C /Baro/Mag *********************
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9

//#define USE_I2C_DEVICE_1 // clashes with UART1
//#define I2C1_SCL                PB6
//#define I2C1_SDA                PB7

#if defined(VANTAC_RF007) || defined(VANTAC_RF007_9SERVOS)
#define USE_I2C_DEVICE_2 // clashes with UART3
#define I2C2_SCL                PB10
#define I2C2_SDA                PB11
#define DEFAULT_I2C BUS_I2C2
#else
#define DEFAULT_I2C BUS_I2C1
#endif

#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C1
#define USE_BARO_SPL06
#define SPL06_I2C_ADDR 119

#define TEMPERATURE_I2C_BUS     DEFAULT_I2C

#define PITOT_I2C_BUS           DEFAULT_I2C

#define USE_RANGEFINDER
#define RANGEFINDER_I2C_BUS     DEFAULT_I2C

// *************** SPI2 Blackbox *******************
#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15

#define USE_FLASHFS
#define USE_FLASH_W25N01G
#define W25N01G_SPI_BUS          BUS_SPI2
#define W25N01G_CS_PIN           PB12
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

// *************** UART *****************************
#define USE_VCP

#ifdef VANTAC_RF007
#define USE_UART1 // clashes with I2C1
#define UART1_TX_PIN            PB6
#define UART1_RX_PIN            PB7 // pin labelled "SBUS"
#endif

//#define USE_UART2 // clashes with 2 servo outputs
//#define UART2_TX_PIN            PA2 // pin labelled as "RPM"
//#define UART2_RX_PIN            PA3 // pin labelled as "TLM"

#ifdef VANTAC_RF007_NOI2C
#define USE_UART3
// port labelled "C"
#define UART3_TX_PIN            PB10
#define UART3_RX_PIN            PB11
#endif

#define USE_UART4
// port labelled "A"
#define UART4_TX_PIN            PA0
#define UART4_RX_PIN            PA1

#define USE_UART5
// port internal receiver
#define UART5_TX_PIN            PC12
#define UART5_RX_PIN            PD2

#if defined(VANTAC_RF007)
#define SERIAL_PORT_COUNT       4
#elif defined(VANTAC_RF007_9SERVOS)
#define SERIAL_PORT_COUNT       3
#elif defined(VANTAC_RF007_NOI2C)
#define SERIAL_PORT_COUNT       4
#endif

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_FBUS
#define SERIALRX_UART           SERIAL_PORT_USART5

#define SENSORS_SET (SENSOR_ACC|SENSOR_BARO)

// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE                ADC1
#define ADC1_DMA_STREAM             DMA2_Stream0

#define ADC_CHANNEL_1_PIN           PC2
#define ADC_CHANNEL_2_PIN           PC1
#define ADC_CHANNEL_3_PIN           PC0

#define VBAT_ADC_CHANNEL            ADC_CHN_3 // port labelled "EXT-V"
//BEC ADC is ADC_CHN_2
//BUS ADC is ADC_CHN_1

#define VBAT_SCALE_DEFAULT          2464

#define DEFAULT_FEATURES                (FEATURE_TELEMETRY | FEATURE_VBAT | FEATURE_TX_PROF_SEL)

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA 0xffff
#define TARGET_IO_PORTB 0xffff
#define TARGET_IO_PORTC 0xffff
#define TARGET_IO_PORTD 0xffff

#if defined(VANTAC_RF007)
#define MAX_PWM_OUTPUT_PORTS        7
#elif defined(VANTAC_RF007_9SERVOS) || defined(VANTAC_RF007_NOI2C)
#define MAX_PWM_OUTPUT_PORTS        9
#endif

#define USE_DSHOT
#define USE_SERIALSHOT
#define USE_ESC_SENSOR
#define USE_SMARTPORT_MASTER // no internal current sensor, enable SMARTPORT_MASTER so external ones can be used
