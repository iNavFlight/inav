
#pragma once

#define TARGET_BOARD_IDENTIFIER "BLDY"
#define USBD_PRODUCT_STRING     "BOTWINGF405"

/*** Indicators ***/
#define LED0                       PC0
#define LED1                       PC5

#define BEEPER                     PB0
#define BEEPER_INVERTED

// *************** SPI **********************
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN               PA5
#define SPI1_MISO_PIN               PA6
#define SPI1_MOSI_PIN               PA7

#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN               PB13
#define SPI2_MISO_PIN               PB14
#define SPI2_MOSI_PIN               PB15

#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN               PC10
#define SPI3_MISO_PIN               PC11
#define SPI3_MOSI_PIN               PB5

/*
* I2C
*/
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9

/*
* Serial
*/
// *************** UART *****************************
#define USB_IO
#define USE_VCP
#define SERIAL_PORT_COUNT       7

#define USE_UART1
#define UART1_TX_PIN               PB6
#define UART1_RX_PIN               PB7

#define USE_UART2
#define UART2_TX_PIN               PA2
#define UART2_RX_PIN               PA3

#define USE_UART3
#define UART3_TX_PIN               PB10
#define UART3_RX_PIN               PB11

#define USE_UART4
#define UART4_TX_PIN               PA0
#define UART4_RX_PIN               PA1

#define USE_UART5
#define UART5_TX_PIN               PC12
#define UART5_RX_PIN               PD2

#define USE_UART6
#define UART6_TX_PIN               PC6
#define UART6_RX_PIN               PC7

/*
* Gyro
*/
#define USE_IMU_ICM42605
#define ICM42605_CS_PIN       PA4
#define ICM42605_SPI_BUS      BUS_SPI1
#define IMU_ICM42605_ALIGN    CW0_DEG

/*
* Other
*/
#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C1
#define USE_BARO_DPS310

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_ALL


/*
* OSD
*/
#define USE_MAX7456
#define MAX7456_CS_PIN          PA15
#define MAX7456_SPI_BUS         BUS_SPI3

/*
* Blackbox
*/
#define USE_FLASHFS
#define USE_FLASH_M25P16
#define M25P16_SPI_BUS          BUS_SPI2
#define M25P16_CS_PIN           PB12
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART1

// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE                ADC1
#define ADC1_DMA_STREAM             DMA2_Stream0

#define ADC_CHANNEL_1_PIN           PC1
#define ADC_CHANNEL_2_PIN           PC2
#define ADC_CHANNEL_3_PIN           PC3

#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_3
#define RSSI_ADC_CHANNEL            ADC_CHN_2
#define VBAT_SCALE_DEFAULT              1420
#define CURRENT_METER_SCALE             206


// *************** PINIO ***************************
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN                  PB1 // RF Switch
#define PINIO2_PIN                  PC13 // RF Switch
#define PINIO1_FLAGS                PINIO_FLAGS_INVERTED
#define PINIO2_FLAGS                PINIO_FLAGS_INVERTED

// *************** LEDSTRIP ************************
#define USE_LED_STRIP
#define WS2811_PIN                  PB3


#define DEFAULT_FEATURES        (FEATURE_OSD | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TX_PROF_SEL | FEATURE_BLACKBOX)

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA 0xffff
#define TARGET_IO_PORTB 0xffff
#define TARGET_IO_PORTC 0xffff
#define TARGET_IO_PORTD 0xffff



#define MAX_PWM_OUTPUT_PORTS        4


#define USE_DSHOT
#define USE_SERIALSHOT
#define USE_ESC_SENSOR