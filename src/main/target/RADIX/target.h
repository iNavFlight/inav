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

#include <stdbool.h>

#define TARGET_BOARD_IDENTIFIER "RADIX"

#define FLASH_PAGE_SIZE            (0x4000)     // 16kB for settings
#define CONFIG_START_FLASH_ADDRESS (0x08004000) // 2nd 16kB sector

#define USBD_PRODUCT_STRING     "BrainFPV RADIX"

#define TARGET_CONFIG

#define LED0_PIN                PA4
#define LED1_PIN                NONE
#define LED0_INVERTED

#define BEEPER                  NONE
#define USE_LED_STRIP

#define USE_ESCSERIAL
#define ESCSERIAL_TIMER_TX_HARDWARE 0 // PWM 1

#define USE_BRAINFPV_FPGA
#define BRAINFPVFPGA_SPI_INSTANCE SPI3
#define BRAINFPVFPGA_SPI_DIVISOR  16
#define BRAINFPVFPGA_CS_PIN       PC15
#define BRAINFPVFPGA_CDONE_PIN    PB12
#define BRAINFPVFPGA_CRESET_PIN   PB1
#define BRAINFPVFPGA_CLOCK_PIN    PA8
#define BRAINFPVFPGA_RESET_PIN    PC4

#define IDLE_COUNTS_PER_SEC_AT_NO_LOAD (16564455)

#define BRAINFPV
#define USE_CMS
#define USE_MAX7456
#define USE_OSD
#define OSD_CALLS_CMS
#define VIDEO_BITS_PER_PIXEL 2
#define INCLUDE_VIDEO_QUADSPI
#define VIDEO_QSPI_CLOCK_PIN PB2
#define VIDEO_QSPI_IO0_PIN   PC9
#define VIDEO_QSPI_IO1_PIN   PC10
#define VIDEO_VSYNC          PB5
#define VIDEO_HSYNC          PC2
#define BRAINFPV_OSD_SYNC_TH_DEFAULT 30
#define BRAINFPV_OSD_SYNC_TH_MIN 5
#define BRAINFPV_OSD_SYNC_TH_MAX 60


#define USE_VTX_CONTROL
#define VTX_SMARTAUDIO
#define CAMERA_CONTROL_PIN    PB9

#define USE_BRAINFPV_SPECTROGRAPH

#define USE_EXTI
#define USE_GYRO
#define USE_ACC

#define USE_ACCGYRO_BMI160
#define USE_GYRO_BMI160
#define USE_ACC_BMI160
#define GYRO_BMI160_ALIGN    CW0_DEG
#define ACC_BMI160_ALIGN     CW0_DEG
#define BMI160_SPI_BUS       BUS_SPI3
#define BUS_SPEED_BMI160     BUS_SPEED_STANDARD
#define BMI160_CS_PIN        PB4
#define BMI160_INT_EXTI      PC13

#define USE_BARO
#define USE_BARO_BMP280
#define USE_BARO_SPI_BMP280
//#define BMP280_SPI_DIVISOR   16
#define BMP280_SPI_BUS       BUS_SPI3
#define BMP280_CS_PIN        PB8

#define USABLE_TIMER_CHANNEL_COUNT 8

// Number of available PWM outputs
#define MAX_PWM_OUTPUT_PORTS    6
#define TARGET_MOTOR_COUNT      6

#define USE_VCP
#define VBUS_SENSING_PIN        PA9
#define VBUS_SENSING_ENABLED

#define USE_UART
#define USE_UART1
#undef USE_UART1_TX_DMA
#define USE_UART1_TX_NODMA
#define UART1_RX_PIN            PB7
#define UART1_TX_PIN            PB6

#define USE_UART3
#define UART3_RX_PIN            PC5
#define UART3_TX_PIN            PB10

#define USE_UART4
#define UART4_RX_PIN            PA1
#define UART4_TX_PIN            PA0

#define USE_UART6
#undef USE_UART6_TX_DMA
#define UART6_RX_PIN            PC7
#define UART6_TX_PIN            PC6

#define SERIAL_PORT_COUNT       5 //VCP, USART1,  USART3, USART4, USART6

#define USE_SPI

#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            PB3
#define SPI3_MISO_PIN           PC11
#define SPI3_MOSI_PIN           PC12

#define BOARD_HAS_VOLTAGE_DIVIDER
#define USE_ADC
// XXX should use this
//#define ADC_VOLTAGE_REFERENCE_MV 3245
#define ADC_CHANNEL_1_PIN               PC1
#define ADC_CHANNEL_2_PIN               PC0
#define ADC_CHANNEL_3_PIN               PC3

#define VBAT_ADC_CHANNEL                ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL       ADC_CHN_2
#define RSSI_ADC_CHANNEL                ADC_CHN_3
#define VBAT_SCALE_DEFAULT            1200
#define CURRENT_METER_SCALE           200

#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT
#define USE_SDCARD
#define SDCARD_SPI_INSTANCE                 SPI1
#define SDCARD_DETECT_INVERTED
#define SDCARD_DETECT_PIN                   PB13
#define SDCARD_SPI_CS_PIN                   PB15
// SPI1 is on APB2, 90MHz
#define SDCARD_SPI_INITIALIZATION_CLOCK_DIVIDER 256 // 350kHz
#define SDCARD_SPI_FULL_SPEED_CLOCK_DIVIDER     4   // 22.5MHz
#define SDCARD_DMA_CHANNEL_TX                DMA2_Stream3
#define SDCARD_DMA_CHANNEL_TX_COMPLETE_FLAG  DMA_FLAG_TCIF3
#define SDCARD_DMA_CLK                       RCC_AHB1Periph_DMA2
#define SDCARD_DMA_CHANNEL                   DMA_Channel_3

#define DEFAULT_FEATURES        (FEATURE_OSD)
#define DEFAULT_RX_FEATURE      FEATURE_RX_PPM
#define SERIALRX_PROVIDER       SERIALRX_SBUS

#define SPEKTRUM_BIND
// PPM input
#define BIND_PIN                PB14

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         (BIT(2))

#define USE_DSHOT
#define USED_TIMERS             ( TIM_N(1) | TIM_N(2) | TIM_N(3) | TIM_N(5) | TIM_N(8) | TIM_N(12) )

bool brainfpv_settings_updated;
bool brainfpv_settings_updated_from_cms;
void brainFPVUpdateSettings(void);
