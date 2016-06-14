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

#define TARGET_BOARD_IDENTIFIER "REVO"

#define USBD_PRODUCT_STRING "Revolution"
#ifdef OPBL
#define USBD_SERIALNUMBER_STRING "0x8020000"
#endif

//#define LED0
#define LED0_GPIO               GPIOB
#define LED0_PIN                Pin_5
#define LED0_PERIPHERAL         RCC_APB2Periph_GPIOB

//#define LED1
#define LED1_GPIO               GPIOB
#define LED1_PIN                Pin_4
#define LED1_PERIPHERAL         RCC_APB2Periph_GPIOB

//#define BEEPER
#define BEEP_GPIO               GPIOB
#define BEEP_PIN                Pin_4
#define BEEP_PERIPHERAL         RCC_APB2Periph_GPIOA

//#define INVERTER
//#define INVERTER_GPIO           GPIOC // PC0 used as inverter select GPIO
#define INVERTER_PIN            Pin_0
#define INVERTER_USART          USART1
#define INVERTER_PERIPHERAL     RCC_APB2Periph_GPIOC

#define MPU6000_CS_GPIO         GPIOA
#define MPU6000_CS_PIN          GPIO_Pin_4
#define MPU6000_SPI_INSTANCE    SPI1

#define ACC
#define USE_ACC_SPI_MPU6000
#define GYRO_MPU6000_ALIGN      CW270_DEG

#define GYRO
#define USE_GYRO_SPI_MPU6000
#define ACC_MPU6000_ALIGN       CW270_DEG

// MPU6000 interrupts
//!!#define USE_MPU_DATA_READY_SIGNAL
//!!#define EXTI_CALLBACK_HANDLER_COUNT 2 // MPU data ready (mag disabled)
//!!#define MPU_INT_EXTI PC4
//!!#define USE_EXTI

#define MAG
#define USE_MAG_AK8975
#define USE_MAG_HMC5883
#define USE_MAG_MAG3110
#define MAG_HMC5883_ALIGN       CW90_DEG

//#define USE_MAG_NAZA
//#define MAG_NAZA_ALIGN CW180_DEG_FLIP

#define BARO
#define USE_BARO_MS5611
#define USE_BARO_BMP085

//#define PITOT
//#define USE_PITOT_MS4525
//#define MS4525_BUS I2C_DEVICE_EXT

#define M25P16_SPI_INSTANCE     SPI3
#define M25P16_CS_GPIO          GPIOB
#define M25P16_CS_PIN           GPIO_Pin_3

#define USE_FLASHFS
#define USE_FLASH_M25P16

#define USABLE_TIMER_CHANNEL_COUNT 12

#define USE_VCP
#define VBUS_SENSING_GPIO       GPIOC
#define VBUS_SENSING_PIN        GPIO_Pin_5

#define USE_USART1
//#define USE_USART3
#define USE_USART6

#define SERIAL_PORT_COUNT 4 //VCP, USART1, USART3, USART6

#define USE_SPI

#define USE_SPI_DEVICE_1

#define USE_SPI_DEVICE_3
#define SPI3_NSS_GPIO           GPIOB
#define SPI3_NSS_PIN            GPIO_Pin_3
#define SPI3_SCK_GPIO           GPIOC
#define SPI3_SCK_PIN            GPIO_Pin_10
#define SPI3_MISO_GPIO          GPIOC
#define SPI3_MISO_PIN           GPIO_Pin_11
#define SPI3_MOSI_GPIO          GPIOC
#define SPI3_MOSI_PIN           GPIO_Pin_12

#define USE_I2C
#define I2C_DEVICE (I2CDEV_1)

#define USE_ADC
#define CURRENT_METER_ADC_GPIO      GPIOC
#define CURRENT_METER_ADC_GPIO_PIN  GPIO_Pin_1
#define CURRENT_METER_ADC_CHANNEL   ADC_Channel_11

#define VBAT_ADC_GPIO               GPIOC
#define VBAT_ADC_GPIO_PIN           GPIO_Pin_2
#define VBAT_ADC_CHANNEL            ADC_Channel_12

//#define RSSI_ADC_GPIO               GPIOA
#define RSSI_ADC_GPIO_PIN           GPIO_Pin_0
#define RSSI_ADC_CHANNEL            ADC_Channel_0

#define SENSORS_SET (SENSOR_ACC)

//#define LED_STRIP
//#define LED_STRIP_TIMER TIM5

#define DEFAULT_RX_FEATURE FEATURE_RX_PPM
#define DEFAULT_FEATURES (FEATURE_BLACKBOX | FEATURE_ONESHOT125 | FEATURE_RX_SERIAL)

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA 0xffff
#define TARGET_IO_PORTB 0xffff
#define TARGET_IO_PORTC 0xffff
#define TARGET_IO_PORTD 0xffff

#define USED_TIMERS  ( TIM_N(2) | TIM_N(3) | TIM_N(5) | TIM_N(12) | TIM_N(8) | TIM_N(9))

//#define TIMER_APB1_PERIPHERALS (RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM5 | RCC_APB1Periph_TIM12 | RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC)
//#define TIMER_APB2_PERIPHERALS (RCC_APB2Periph_TIM8 | RCC_APB2Periph_TIM9)

