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


#define TARGET_BOARD_IDENTIFIER "CLRAIR"

#define USBD_PRODUCT_STRING "CLRACINGF4AIR"
#ifdef OPBL
#define USBD_SERIALNUMBER_STRING "0x8020000"
#endif
#define LED0                    PB5
#define BEEPER                PB4
#define BEEPER_INVERTED

#define INVERTER_PIN_UART1      PC0 // PC0 used as inverter select GPIO
// MPU-6000 GRYO
#define MPU6000_CS_PIN          PA4
#define MPU6000_SPI_BUS         BUS_SPI1
#define USE_GYRO
#define USE_GYRO_MPU6000
#define GYRO_MPU6000_ALIGN      CW0_DEG
#define USE_ACC
#define USE_ACC_MPU6000
#define ACC_MPU6000_ALIGN       CW0_DEG

//MPU-9250
#define MPU9250_CS_PIN          PA4
#define MPU9250_SPI_BUS         BUS_SPI1
#define USE_GYRO
#define USE_GYRO_MPU9250
#define GYRO_MPU9250_ALIGN      CW0_DEG
#define USE_ACC
#define USE_ACC_MPU9250
#define ACC_MPU9250_ALIGN       CW0_DEG
#define USE_MAG
#define USE_MAG_MPU9250

// MPU6 interrupts
#define USE_EXTI
#define MPU_INT_EXTI            PC4
#define USE_MPU_DATA_READY_SIGNAL

#define USE_BARO
#define USE_BARO_BMP280
#define USE_BARO_SPI_BMP280
#define BMP280_SPI_BUS          BUS_SPI3
#define BMP280_CS_PIN           PB3

#define USE_OSD
#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI3
#define MAX7456_CS_PIN          PA15

#define USE_VCP

#define USE_UART1
#define UART1_RX_PIN            PA10
#define UART1_TX_PIN            PA9
#define UART1_AHB1_PERIPHERALS  RCC_AHB1Periph_DMA2

#define USE_UART3
#define UART3_RX_PIN            PB11
#define UART3_TX_PIN            PB10

 #define USE_UART4
 #define UART4_RX_PIN            PA1
 #define UART4_TX_PIN            PA0

#define USE_UART6
#define UART6_RX_PIN            PC7
#define UART6_TX_PIN            PC6

#define SERIAL_PORT_COUNT       5 //VCP, USART1, USART3,USART4, USART6,

#define USE_ESCSERIAL
#define ESCSERIAL_TIMER_TX_HARDWARE 0 // PWM 1

#define USE_SPI
#define USE_SPI_DEVICE_1

#define USE_SPI_DEVICE_3
#define SPI3_NSS_PIN            PA15
#define SPI3_SCK_PIN            PC10
#define SPI3_MISO_PIN          PC11
#define SPI3_MOSI_PIN          PC12

#define USE_ADC
#define ADC_CHANNEL_1_PIN               PC1
#define ADC_CHANNEL_2_PIN               PC2
#define ADC_CHANNEL_3_PIN               PA0
#define CURRENT_METER_ADC_CHANNEL       ADC_CHN_1
#define VBAT_ADC_CHANNEL                ADC_CHN_2
#define RSSI_ADC_CHANNEL                ADC_CHN_3

#define USE_ESC_SENSOR
#define DEFAULT_RX_FEATURE      FEATURE_RX_SERIAL
#define DEFAULT_FEATURES         (FEATURE_CURRENT_METER | FEATURE_TELEMETRY| FEATURE_VBAT | FEATURE_OSD )

#define SPEKTRUM_BIND_PIN       UART1_RX_PIN
#define USE_SERIAL_4WAY_BLHELI_INTERFACE
// Number of available PWM outputs
#define MAX_PWM_OUTPUT_PORTS    6
#define TARGET_MOTOR_COUNT      6

#define TARGET_IO_PORTA (0xffff & ~(BIT(14)|BIT(13)))
#define TARGET_IO_PORTB (0xffff & ~(BIT(2)))
#define TARGET_IO_PORTC (0xffff & ~(BIT(15)|BIT(14)|BIT(13)))
#define TARGET_IO_PORTD BIT(2)

#define USABLE_TIMER_CHANNEL_COUNT 8
#define USED_TIMERS  ( TIM_N(1) | TIM_N(2) | TIM_N(3) | TIM_N(4) | TIM_N(5) | TIM_N(9) | TIM_N(11) )
