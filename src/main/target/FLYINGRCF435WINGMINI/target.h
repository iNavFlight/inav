/*
 * FlyingRC AT32F435 Mini fixed-wing target.
 * MCU: AT32F435CGU7 / AT32F435CMU7, QFN48, 8 MHz HEXT.
 */

#pragma once

#define TARGET_BOARD_IDENTIFIER "FAM1"

#if defined(AT32F435CMU7)
#define USBD_PRODUCT_STRING "FlyingRC ATF435 Mini CMU7"
#else
#define USBD_PRODUCT_STRING "FlyingRC ATF435 Mini CGU7"
#endif

/* Indicators */
#define LED0                    PB4 // Blue
#define LED1                    PB5 // Green
#define LED0_INVERTED
#define LED1_INVERTED

#define BEEPER                  PH2
#define BEEPER_INVERTED

#define USE_LED_STRIP
#define WS2811_PIN              PA8

/* SPI1: IMU and MAX7456 share the bus, with independent chip selects. */
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define USE_IMU_BMI270
#define IMU_BMI270_ALIGN        CW0_DEG_FLIP
#define BMI270_SPI_BUS          BUS_SPI1
#define BMI270_CS_PIN           PA4
#define BMI270_EXTI_PIN         PC13

#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI1
#define MAX7456_CS_PIN          PC14

/* I2C1: external barometer, compass, pitot and rangefinder bus. */
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9
/* The AT32 I2C driver instantiates all three descriptors. Mark the unused
 * controllers explicitly instead of assigning duplicate pins. */
#define I2C2_SCL                NONE
#define I2C2_SDA                NONE
#define I2C3_SCL                NONE
#define I2C3_SDA                NONE
#define DEFAULT_I2C_BUS         BUS_I2C1

#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C1
#define USE_BARO_ALL

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_ALL

#define PITOT_I2C_BUS           BUS_I2C1
#define RANGEFINDER_I2C_BUS     BUS_I2C1

/* USB and four UARTs. */
#define USE_VCP

#define USE_UART1
#define UART1_TX_PIN            PA9
#define UART1_RX_PIN            PA10

#define USE_UART2
#define UART2_TX_PIN            PA2
#define UART2_RX_PIN            PA3

#define USE_UART3
#define UART3_TX_PIN            PB10
#define UART3_RX_PIN            PB11

#define USE_UART4
#define UART4_TX_PIN            PA0
#define UART4_RX_PIN            PA1

#define SERIAL_PORT_COUNT       5

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_CRSF
#define SERIALRX_UART           SERIAL_PORT_USART2

/* Analog battery monitoring. */
#define USE_ADC
#define ADC_INSTANCE                ADC1
#define ADC1_DMA_STREAM             DMA2_CHANNEL1
#define ADC_CHANNEL_1_PIN           PB1
#define ADC_CHANNEL_2_PIN           PB0
#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2
#define VBAT_SCALE_DEFAULT          2100 // 200k:10k divider, 21:1

#define DEFAULT_FEATURES (FEATURE_TX_PROF_SEL | FEATURE_CURRENT_METER | FEATURE_TELEMETRY | FEATURE_VBAT | FEATURE_OSD | FEATURE_LED_STRIP)

#define USE_SERIAL_4WAY_BLHELI_INTERFACE
#define USE_DSHOT
#define USE_ESC_SENSOR

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         (BIT(13) | BIT(14) | BIT(15))
#define TARGET_IO_PORTH         (BIT(2) | BIT(3))

#define MAX_PWM_OUTPUT_PORTS    6
