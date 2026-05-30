/*
 * RadioMaster Nexus (Original) - iNAV target
 *
 * Based on the NEXUSX target by functionpointer, adapted for the
 * original (discontinued) Nexus hardware.
 *
 * Key differences from Nexus-X/XR:
 *   - IMU EXTI on PA15 (X/XR uses PB8)
 *   - IMU alignment CW180 (Rotorflight uses CW90 with a different
 *     board orientation reference; iNAV uses the arrow on the case)
 *   - Flash is W25N01G 128MB (X/XR uses W25N02K 256MB)
 *   - No internal ELRS receiver (X/XR has RP4TD-M on UART5)
 *   - No PINIO1 receiver power gate
 *   - Voltage input 5-12.6V (X/XR supports 3.6-70V)
 *   - 5 servo/motor outputs vs 7-9 on X/XR
 *   - Baro on I2C1 (PB8/PB9), not I2C2
 *   - UART1 on PA9/PA10, not PB6/PB7
 *
 * Pin mapping derived from:
 *   - Rotorflight NEXUS_F7 target (authoritative)
 *   - groundflight project (joshperry/groundflight)
 *   - RadioMaster documentation
 */

#pragma once

#define TARGET_BOARD_IDENTIFIER "NEXS"
#define USBD_PRODUCT_STRING     "RadioMaster Nexus"

/* ---- LEDs ---- */
#define LED0                    PC14  // active low
#define LED1                    PC15  // active low

/* ---- Beeper ---- */
// No dedicated beeper pin on Nexus hardware

/* ---- SPI ---- */
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15

/* ---- IMU: ICM-42688-P ---- */
// iNAV uses ICM42605 driver which is register-compatible with ICM42688P
#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN      CW180_DEG
#define ICM42605_CS_PIN         PA4
#define ICM42605_SPI_BUS        BUS_SPI1
#define ICM42605_EXTI_PIN       PA15

/* ---- I2C ---- */
// I2C1: PB8/PB9 - internal, used for barometer
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9

// I2C2: PB10/PB11 - external via Port C connector (shared with UART3)
// Available for external sensors (magnetometer, rangefinder, etc.)
#define USE_I2C_DEVICE_2
#define I2C2_SCL                PB10
#define I2C2_SDA                PB11
#define I2C_DEVICE_2_SHARES_UART3

/* ---- Barometer: SPL06-001 ---- */
// Confirmed on I2C1 per Rotorflight NEXUS_F7 dump
#define USE_BARO
#define USE_BARO_SPL06
#define BARO_I2C_BUS            BUS_I2C1

/* ---- Magnetometer (external, optional via Port C / I2C2) ---- */
#define USE_MAG
#define USE_MAG_ALL
#define MAG_I2C_BUS             BUS_I2C2

/* ---- Flash: W25N01G (128MB) ---- */
#define USE_FLASHFS
#define USE_FLASH_W25N01G
#define W25N01G_SPI_BUS         BUS_SPI2
#define W25N01G_CS_PIN          PB12
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

/* ---- UARTs ---- */
// OG Nexus UART layout (confirmed from Rotorflight NEXUS_F7 dump):
//   UART1 [DSM port]  : PA9 (TX) / PA10 (RX)
//   UART2 [SBUS/FREQ] : PA2 (TX) / PA3 (RX) - shared with RPM/TLM pins
//   UART3 [Port C]    : PB11 (TX) / PB10 (RX) - shared with I2C2
//   UART4 [Port A]    : PA1 (TX) / PA0 (RX) - primary CRSF receiver
//   UART6 [Port B]    : PC7 (TX) / PC6 (RX)
//
// NOTE: No UART5 on OG Nexus (X/XR uses UART5 for internal ELRS)

#define USE_VCP
#define USE_USB_DETECT
#define USB_DETECT_PIN          NONE

#define USE_UART1
#define UART1_TX_PIN            PA9
#define UART1_RX_PIN            PA10

#define USE_UART2
#define UART2_TX_PIN            PA2
#define UART2_RX_PIN            PA3

#define USE_UART3
#define UART3_TX_PIN            PB11
#define UART3_RX_PIN            PB10

#define USE_UART4
#define USE_UART4_SWAP
#define UART4_TX_PIN            PA1
#define UART4_RX_PIN            PA0

#define USE_UART6
#define UART6_TX_PIN            PC7
#define UART6_RX_PIN            PC6

#define SERIAL_PORT_COUNT       6  // VCP + UART1-4 + UART6

/* ---- Default serial receiver ---- */
#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_CRSF
#define SERIALRX_UART           SERIAL_PORT_USART4


/* ---- ADC ---- */
// OG Nexus has no EXT-V input (unlike X/XR which has a dedicated
// high-voltage sense on PC0). Only two ADC channels:
//   ADC_BUS = PC2, divider 320 (Vin rail, 5-12.6V)
//   ADC_BEC = PC1, divider 160 (BEC 5V rail)
// Map Vin as VBAT since it's the primary power input
#define USE_ADC
#define ADC_INSTANCE            ADC1
#define ADC_CHANNEL_1_PIN       PC2   // Vin (input power rail)
#define VBAT_ADC_CHANNEL        ADC_CHN_1
#define ADC_CHANNEL_2_PIN       PC1   // BEC 5V rail
// VBAT scale: hardware-verified value (divider ratio ~320)
#define VBAT_SCALE_DEFAULT      320

/* ---- Sensors ---- */
#define SENSORS_SET             (SENSOR_ACC | SENSOR_BARO)

/* ---- PWM / Servo / Motor outputs ---- */
// OG Nexus outputs (from Rotorflight dump):
//   S1:   PB4  (TIM3_CH1)  - Servo header
//   S2:   PB5  (TIM3_CH2)  - Servo header
//   S3:   PB0  (TIM3_CH3)  - Servo header
//   S4:   PB3  (TIM2_CH2)  - Servo header (Tail)
//   M1:   PB6  (TIM4_CH1)  - ESC header (motor only, NOT UART1)
//
// Pin multiplexing when UARTs freed:
//   PA2 (TIM5_CH3) - shared with UART2 TX / FREQ input
//   PA3 (TIM9_CH2) - shared with UART2 RX / PPM input

#define MAX_PWM_OUTPUT_PORTS    7

/* ---- No internal receiver ---- */
// OG Nexus has no internal ELRS receiver.
// No UART5, no PINIO1 power gate.

/* ---- Platform ---- */
#define USE_SERIAL_4WAY_BLHELI_INTERFACE
#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff

#define DEFAULT_FEATURES        (FEATURE_TX_PROF_SEL | FEATURE_BLACKBOX)
