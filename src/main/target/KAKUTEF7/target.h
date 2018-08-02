/*
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#define TARGET_BOARD_IDENTIFIER "KTF7"
#define USBD_PRODUCT_STRING "KakuteF7"

// Use target-specific hardware descriptors (don't use common_hardware.h)
#define USE_TARGET_HARDWARE_DESCRIPTORS

#define LED0   PA2

#define BEEPER   PD15
#define BEEPER_INVERTED

#define USE_ACC
#define USE_GYRO

// ICM-20689
#define USE_ACC_MPU6500
#define USE_GYRO_MPU6500
#define GYRO_MPU6500_ALIGN       CW270_DEG
#define ACC_MPU6500_ALIGN        CW270_DEG

#define USE_EXTI
#define USE_MPU_DATA_READY_SIGNAL
#define MPU6500_EXTI_PIN            PE1

#define MPU6500_CS_PIN           SPI4_NSS_PIN
#define MPU6500_SPI_BUS          BUS_SPI4

#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB6        // SCL pad
#define I2C1_SDA                PB7        // SDA pad

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_HMC5883
#define MAG_HMC5883_ALIGN       CW270_DEG
#define USE_MAG_QMC5883
#define USE_MAG_MAG3110
#define USE_MAG_IST8310

#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C1
#define USE_BARO_MS5611
#define USE_BARO_BMP280


#define USE_VCP
#define VBUS_SENSING_PIN PA8

#define USE_UART1
#define UART1_RX_PIN     PA10
#define UART1_TX_PIN     PA9

#define USE_UART2
#define UART2_TX_PIN     PD5 
#define UART2_RX_PIN     PD6

#define USE_UART3
#define UART3_RX_PIN     PB11
#define UART3_TX_PIN     PB10

#define USE_UART4
#define UART4_TX_PIN     PA0
#define UART4_RX_PIN     PA1

#define USE_UART6
#define UART6_RX_PIN     PC7
#define UART6_TX_PIN     PC6

#define USE_UART7
#define UART7_RX_PIN     PE7
#define UART7_TX_PIN     NONE

#define SERIAL_PORT_COUNT 7 //VCP,UART1,UART2,UART3,UAER4,UART6,UART7

#define USE_SPI
#define USE_SPI_DEVICE_1     //SD
#define USE_SPI_DEVICE_2     //OSD
#define USE_SPI_DEVICE_4     //IMU

#define SPI1_NSS_PIN        PA4
#define SPI1_SCK_PIN        PA5
#define SPI1_MISO_PIN       PA6
#define SPI1_MOSI_PIN       PA7

#define SPI2_NSS_PIN        PB12
#define SPI2_SCK_PIN        PB13
#define SPI2_MISO_PIN       PB14
#define SPI2_MOSI_PIN       PB15

#define SPI4_NSS_PIN        PE4
#define SPI4_SCK_PIN        PE2
#define SPI4_MISO_PIN       PE5
#define SPI4_MOSI_PIN       PE6


#define USE_OSD
#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI2
#define MAX7456_CS_PIN          SPI2_NSS_PIN

#define USE_SDCARD
#define SDCARD_DETECT_INVERTED
#define SDCARD_DETECT_PIN                   PD8
#define SDCARD_DETECT_EXTI_LINE             EXTI_Line8
#define SDCARD_DETECT_EXTI_PIN_SOURCE       EXTI_PinSource8
#define SDCARD_DETECT_EXTI_PORT_SOURCE      EXTI_PortSourceGPIOD
#define SDCARD_DETECT_EXTI_IRQn             EXTI3_IRQn

#define SDCARD_SPI_INSTANCE                 SPI1
#define SDCARD_SPI_CS_PIN                   SPI1_NSS_PIN

#define SDCARD_DMA_CHANNEL_TX               DMA2_Stream5
#define SDCARD_DMA_CHANNEL_TX_COMPLETE_FLAG DMA_FLAG_TCIF1_5
#define SDCARD_DMA_CLK                      RCC_AHB1Periph_DMA2
#define SDCARD_DMA_CHANNEL                  DMA_CHANNEL_3

#define SENSORS_SET (SENSOR_ACC | SENSOR_BARO)

#define USE_ADC
#define ADC_CHANNEL_1_PIN               PC2
#define ADC_CHANNEL_2_PIN               PC3
#define ADC_CHANNEL_3_PIN               PC5
#define CURRENT_METER_ADC_CHANNEL       ADC_CHN_1
#define VBAT_ADC_CHANNEL                ADC_CHN_2
#define RSSI_ADC_CHANNEL                ADC_CHN_3

 #define USE_LED_STRIP

//Following configuration needs to be reviewed, when LED is enabled, VCP stops to work
//Until someone with deeper knowledge od DMA fixes it, LED are disabled in target
#define WS2811_PIN                      PD12
#define WS2811_TIMER                    TIM4
#define WS2811_TIMER_CHANNEL            TIM_CHANNEL_1
#define WS2811_DMA_HANDLER_IDENTIFER    DMA1_ST0_HANDLER
#define WS2811_DMA_STREAM               DMA1_Stream0
#define WS2811_DMA_IT                   DMA_IT_TCIF0
#define WS2811_DMA_CHANNEL              DMA_CHANNEL_2
#define WS2811_TIMER_GPIO_AF            GPIO_AF2_TIM4

#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

#define DEFAULT_FEATURES        (FEATURE_TX_PROF_SEL | FEATURE_OSD)
#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART6

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

// Number of available PWM outputs
#define MAX_PWM_OUTPUT_PORTS    6
#define TARGET_MOTOR_COUNT      6

#define TARGET_IO_PORTA 0xffff
#define TARGET_IO_PORTB 0xffff
#define TARGET_IO_PORTC 0xffff
#define TARGET_IO_PORTD 0xffff
#define TARGET_IO_PORTE 0xffff

#define USABLE_TIMER_CHANNEL_COUNT 7

#define USED_TIMERS  ( TIM_N(1) | TIM_N(5) | TIM_N(3) | TIM_N(4) | TIM_N(8) )

