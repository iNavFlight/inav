#pragma once

#define TARGET_IO_PORTA  0x030F
#define TARGET_IO_PORTB  0x0FCF
#define TARGET_IO_PORTC  0x13C2
#define TARGET_IO_PORTD  0x0004

#define TARGET_BOARD_IDENTIFIER "AKC4"  // Changed from AIK4
#define USBD_PRODUCT_STRING     "make"

// IMU Configuration - ICM42688 instead of MPU6000/6500
#define USE_IMU_ICM42688
#define IMU_ICM42688_ALIGN      CW0_DEG
#define ICM42688_CS_PIN         SPI1_NSS_PIN  // PA4
#define ICM42688_SPI_BUS        BUS_SPI1

// Barometer - DPS368 instead of BMP280
#define USE_BARO
#define USE_BARO_DPS368
#define BARO_I2C_BUS            DEFAULT_I2C_BUS

// Magnetometer - IST8310
#define USE_MAG
#define USE_MAG_IST8310
#define MAG_I2C_BUS             DEFAULT_I2C_BUS

// Serial ports - Updated based on schematic
#define SERIAL_PORT_COUNT       7   // VCP, UART1, UART2, UART3, UART4, UART6

// UART assignments
#define USE_UART1
#define UART1_RX_PIN            PB7   // RX1
#define UART1_TX_PIN            PB6   // TX1

#define USE_UART2
#define UART2_TX_PIN            PA2   // TX2
#define UART2_RX_PIN            PA3   // RX2

#define USE_UART3
#define UART3_RX_PIN            PB11  // RX3
#define UART3_TX_PIN            PB10  // TX3

#define USE_UART4
#define UART4_TX_PIN            PA0   // TX4
#define UART4_RX_PIN            PA1  // RX4 (from schematic)

#define USE_UART5
#define UART5_TX_PIN            PC12   // TX5
#define UART5_RX_PIN            PD2   // RX5

#define USE_UART6
#define UART6_TX_PIN            PC6   // TX6
#define UART6_RX_PIN            PC7   // RX6

// Remove softserial since you have enough hardware UARTs
#undef USE_SOFTSERIAL1
#undef USE_SOFTSERIAL2

// I2C configuration
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL PB8
#define I2C1_SDA PB9

// Flash configuration
#define USE_FLASHFS
#define USE_FLASH_W25Q128
#define W25Q128_CS_PIN           SPI2_NSS_PIN  // PB12
#define W25Q128_SPI_BUS          BUS_SPI2

// LED Strip
#define USE_LED_STRIP
#define WS2811_PIN                      PB3   // Confirmed

// ADC configuration
#define USE_ADC
#define ADC_CHANNEL_2_PIN               PC1   // VBAT

#define VBAT_ADC_CHANNEL                ADC_CHN_2
// Default features
#define DEFAULT_FEATURES        (FEATURE_OSD  | FEATURE_VBAT | FEATURE_BLACKBOX)