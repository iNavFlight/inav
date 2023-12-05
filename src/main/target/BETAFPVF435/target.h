/*
 * This file is part of INAV.
 *
 * INAV are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * INAV are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */
//https://www.arterychip.com/download/DS/DS_AT32F435_437_V2.02-EN.pdf
#pragma once

#define TARGET_BOARD_IDENTIFIER "BHER"
#define USBD_PRODUCT_STRING     "BETAFPVF435"

#define LED0                PB5

#define USE_BEEPER
#define BEEPER              PB4
#define BEEPER_INVERTED

//#define ENABLE_DSHOT_DMAR       DSHOT_DMAR_AUTO
//#define DSHOT_BITBANG_DEFAULT   DSHOT_BITBANG_AUTO
#define ENABLE_DSHOT

// *************** Gyro & ACC **********************
#define USE_SPI
#define USE_SPI_DEVICE_1

#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define SPI1_NSS_PIN           PA4
//#define GYRO_1_SPI_INSTANCE     SPI1

//#define USE_EXTI
//#define USE_GYRO_EXTI
//#define GYRO_1_EXTI_PIN         PC4
//#define USE_MPU_DATA_READY_SIGNAL
//#define ENSURE_MPU_DATA_READY_IS_LOW

// MPU6000
#define USE_IMU_MPU6000
#define IMU_MPU6000_ALIGN       CW270_DEG 
#define MPU6000_SPI_BUS         BUS_SPI1
#define MPU6000_CS_PIN          SPI1_NSS_PIN

// ICM42605/ICM42688P
#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN      CW270_DEG
#define ICM42605_SPI_BUS        BUS_SPI1
#define ICM42605_CS_PIN         SPI1_NSS_PIN

//#define USE_ACC
#define USE_IMU_BMI270
#define IMU_BMI270_ALIGN      CW270_DEG
#define BMI270_SPI_BUS        BUS_SPI1
#define BMI270_CS_PIN         SPI1_NSS_PIN

// *************** Baro **************************
#define USE_BARO
#define USE_BARO_BMP280
#define SPI3_NSS_PIN    PB3
#define BMP280_SPI_BUS  BUS_SPI3
#define BMP280_CS_PIN  SPI3_NSS_PIN
#define USE_BARO_DPS310
#define DPS310_SPI_BUS  BUS_SPI3
#define DPS310_CS_PIN   SPI3_NSS_PIN

// *************** BLACKBOX **************************
#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15

#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT
#define USE_FLASHFS
#define FLASH_CS_PIN            PB12

#define USE_FLASH_M25P16
#define M25P16_SPI_BUS  BUS_SPI2
#define M25P16_CS_PIN   FLASH_CS_PIN
#define USE_FLASH_W25N01G          // 1Gb NAND flash support
#define W25N01G_SPI_BUS BUS_SPI2
#define W25N01G_CS_PIN   FLASH_CS_PIN
#define USE_FLASH_W25M             // Stacked die support
#define W25M_SPI_BUS    BUS_SPI2
#define W25M_CS_PIN   FLASH_CS_PIN
#define USE_FLASH_W25M512          // 512Kb (256Kb x 2 stacked) NOR flash support
#define W25M512_SPI_BUS BUS_SPI2
#define W25M512_CS_PIN   FLASH_CS_PIN
#define USE_FLASH_W25M02G          // 2Gb (1Gb x 2 stacked) NAND flash support
#define W25M02G_SPI_BUS BUS_SPI2
#define W25M02G_CS_PIN FLASH_CS_PIN

// *************** OSD *****************************
#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            PC10
#define SPI3_MISO_PIN           PC11
#define SPI3_MOSI_PIN           PC12

#if 0
#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI3
#define MAX7456_CS_PIN          PA15
#endif

// I2C
#define USE_I2C
#define USE_I2C_DEVICE_2
#define I2C2_SCL                PB10        // SCL pad
#define I2C2_SDA                PB11        // SDA pad
#define USE_I2C_PULLUP

#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C2
#define USE_BARO_BMP280
#define USE_BARO_DPS310

// *************** UART *****************************
#define USE_VCP
#define USE_USB_DETECT
//#define USB_DETECT_PIN          PC5


#define USE_UART1
#define UART1_RX_PIN            PA10
#define UART1_TX_PIN            PA9

#define USE_UART3
#define UART3_RX_PIN            PB11
#define UART3_TX_PIN            PB10
#define USE_UART_INVERTER
#define INVERTER_PIN_UART3_RX   PC9
#define INVERTER_PIN_USART3_RX   PC9

#define USE_UART4
#define UART4_RX_PIN            PA1
#define UART4_TX_PIN            PA0

#define USE_UART5
#define UART5_RX_PIN            PD2
#define UART5_TX_PIN            PD2

#define USE_UART6
#define UART6_RX_PIN            PC7
#define UART6_TX_PIN            PC6

#define SERIAL_PORT_COUNT       6

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_CRSF
#define SERIALRX_UART           SERIAL_PORT_USART3

// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE            ADC1
#define ADC1_DMA_STREAM             DMA2_CHANNEL5
#define ADC_CHANNEL1_PIN        PC2
#define ADC_CHANNEL2_PIN        PC1
#define ADC_CHANNEL3_PIN        PC0
#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2
#define RSSI_ADC_CHANNEL        ADC_CHN_3

#define USE_LED_STRIP
#define WS2811_PIN              PB6

#define DEFAULT_FEATURES        (FEATURE_TX_PROF_SEL | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TELEMETRY | FEATURE_OSD | FEATURE_LED_STRIP )
#define DEFAULT_VOLTAGE_METER_SOURCE    VOLTAGE_METER_ADC
#define DEFAULT_CURRENT_METER_SOURCE    CURRENT_METER_ADC

#define VBAT_SCALE_DEFAULT          110
#define CURRENT_METER_SCALE_DEFAULT 800

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff
#define TARGET_IO_PORTE         BIT(2)
#define TARGET_IO_PORTH         (BIT(1)|BIT(2)|BIT(3))


#define MAX_PWM_OUTPUT_PORTS    8

#define USE_SERIAL_4WAY_BLHELI_INTERFACE
#define USE_DSHOT
#define USE_ESC_SENSOR
#define USE_ESCSERIAL
#define USE_RPM_FILTER
