/*
 * INAV Target: Cipherwing F4 (custom OmnibusF4 variant)
 * Author: CypherWing (Jeevesh Singh Vishwavijay)
 *
 * Notes:
 * - Unique board ID "CYF4"
 * - UARTs remapped per your layout
 * - 8-motor timer map
 */

#pragma once

/* -------- Board ID / USB name -------- */
#define TARGET_BOARD_IDENTIFIER "CYF4"
#define USBD_PRODUCT_STRING     "Cipherwing F4"

/* -------- LEDs / BEEPER -------- */
#define LED0                    PB5
#define BEEPER                  PB4
#define BEEPER_INVERTED

/* -------- I2C (external sensor bus) -------- */
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9
#define I2C_EXT_BUS             BUS_I2C1

/* -------- IMUs (share SPI1 CS) -------- */
#define USE_IMU_MPU6000
#define MPU6000_SPI_BUS         BUS_SPI1
#define MPU6000_CS_PIN          PA4
#define IMU_MPU6000_ALIGN       CW180_DEG

#define USE_IMU_ICM20689
#define ICM20689_SPI_BUS        MPU6000_SPI_BUS
#define ICM20689_CS_PIN         MPU6000_CS_PIN
#define IMU_ICM20689_ALIGN      IMU_MPU6000_ALIGN

#define USE_IMU_ICM42605
#define ICM42605_SPI_BUS        MPU6000_SPI_BUS
#define ICM42605_CS_PIN         MPU6000_CS_PIN
#define IMU_ICM42605_ALIGN      IMU_MPU6000_ALIGN

/* -------- MAG / BARO via I2C1 -------- */
#define USE_MAG
#define USE_MAG_ALL
#define MAG_I2C_BUS             I2C_EXT_BUS

#define USE_BARO
#define USE_BARO_BMP085
#define USE_BARO_BMP280
#define USE_BARO_MS5611
#define BARO_I2C_BUS            I2C_EXT_BUS

/* -------- USB / VBUS -------- */
#define USE_VCP
#define VBUS_SENSING_PIN        PC5
#define VBUS_SENSING_ENABLED

/* -------- UARTs --------
 *   UART1: RX=PA10, TX=PA9
 *   UART2: RX=PA3,  TX=PA2
 *   UART3: RX=PB11, TX=PB10
 *   UART4: RX=PA1,  TX=PA0   (⚠️ PA0 also used by ADC on classic OMNIBUS; see ADC section)
 *   UART5: RX=PD2,  TX=PC12
 *   UART6: RX=PC7,  TX=PC6
 */
#define USE_UART1
#define UART1_RX_PIN            PA10
#define UART1_TX_PIN            PA9

#define USE_UART2
#define UART2_RX_PIN            PA3
#define UART2_TX_PIN            PA2

#define USE_UART3
#define UART3_RX_PIN            PB11
#define UART3_TX_PIN            PB10

#define USE_UART4
#define UART4_RX_PIN            PA1
#define UART4_TX_PIN            PA0

#define USE_UART5
#define UART5_RX_PIN            PD2
#define UART5_TX_PIN            PC13

#define USE_UART6
#define UART6_RX_PIN            PC7
#define UART6_TX_PIN            PC6

/* VCP + UART1..6 */
#define SERIAL_PORT_COUNT       7
#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART1

/* -------- SPI buses -------- */
#define USE_SPI
#define USE_SPI_DEVICE_1
#define USE_SPI_DEVICE_3
#define SPI3_NSS_PIN            PB3
#define SPI3_SCK_PIN            PC10
#define SPI3_MISO_PIN           PC11
#define SPI3_MOSI_PIN           PC12

/* -------- OSD (MAX7456 on SPI3) -------- */
#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI3
#define MAX7456_CS_PIN          PA15

/* -------- Blackbox Flash (M25P16 on SPI3) -------- */
#define USE_FLASHFS
#define USE_FLASH_M25P16
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT
#define M25P16_SPI_BUS          BUS_SPI3
#define M25P16_CS_PIN           SPI3_NSS_PIN

/* -------- ADCs --------
 * Omnibus defaults:
 *  - PC2 = VBAT
 *  - PC1 = CURRENT
 *  - PA0 = RSSI analog
 *
 * ⚠️ If you intend to USE UART4 (TX on PA0), don't assign RSSI to PA0.
 *    Either disable RSSI_ADC or move UART4 to another port (not typical).
 */
#define USE_ADC

// Define analog-capable pins
#define ADC_CHANNEL_1_PIN               PC2   // VBAT
#define ADC_CHANNEL_2_PIN               PC1   // RSSI

// Map INAV analog channels
#define VBAT_ADC_CHANNEL                ADC_CHN_1
#define RSSI_ADC_CHANNEL                ADC_CHN_2

/* -------- LED Strip (WS2812) -------- */
#define USE_LED_STRIP
#define WS2811_PIN                      PB6   /* keeps PA1 free for UART4 RX */

/* -------- Misc / defaults -------- */
#define DISABLE_RX_PWM_FEATURE
#define USE_SERIAL_4WAY_BLHELI_INTERFACE
#define DEFAULT_FEATURES (FEATURE_BLACKBOX | FEATURE_VBAT | FEATURE_OSD)

/* -------- Motors / DSHOT -------- */
#undef  MAX_PWM_OUTPUT_PORTS
#undef  TARGET_MOTOR_COUNT
#define MAX_PWM_OUTPUT_PORTS    8
#define TARGET_MOTOR_COUNT      8
#define USE_DSHOT
#define USE_ESC_SENSOR

/* -------- IO masks -------- */
#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff

