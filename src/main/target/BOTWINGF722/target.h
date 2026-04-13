#pragma once 
#define TARGET_BOARD_IDENTIFIER                         "BLDY"
#define USBD_PRODUCT_STRING                             "BOTWINGF722"
#define USE_TARGET_CONFIG


#define LED0                                            PC4
#define LED1                                            PB2

// *************** UART *****************************
#define USE_VCP
#define USB_IO

#define USE_UART1
#define UART1_TX_PIN                                    PA9
#define UART1_RX_PIN                                    PA10

#define USE_UART2
#define UART2_TX_PIN                                    PA2
#define UART2_RX_PIN                                    PA3

#define USE_UART3
#define UART3_TX_PIN                                    PB10
#define UART3_RX_PIN                                    PB11

#define USE_UART4
#define UART4_TX_PIN                                    PA0
#define UART4_RX_PIN                                    PA1

#define USE_UART6
#define UART6_TX_PIN                                    PC6
#define UART6_RX_PIN                                    PC7

#define SERIAL_PORT_COUNT                               6

// *************** Gyro & ACC **********************
#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN                              CW0_DEG

#define USE_SPI
#define USE_SPI_DEVICE_3
// #define SPI3_NSS_PIN                                 PD2
#define SPI3_SCK_PIN                                    PC10
#define SPI3_MISO_PIN                                   PC11
#define SPI3_MOSI_PIN                                   PC12

// #define ICM42605_CS_PIN                              SPI3_NSS_PIN
#define ICM42605_CS_PIN                                 PD2
#define ICM42605_SPI_BUS                                BUS_SPI3

#define USE_BEEPER
#define BEEPER                                          PB7
#define BEEPER_INVERTED

// *************** I2C(Baro & I2C) **************************
#define USE_I2C
#define USE_I2C_DEVICE_3
#define I2C3_SCL                                        PA8
#define I2C3_SDA                                        PC9      


// Baro
#define USE_BARO
#define USE_BARO_DPS310
#define BARO_I2C_BUS          	                        BUS_I2C3	

// Mag
#define USE_MAG
#define USE_MAG_ALL
#define MAG_I2C_BUS                                     BUS_I2C3

// *************** Flash **************************
#define USE_SPI_DEVICE_1
#define SPI1_NSS_PIN                                 PA4
#define SPI1_SCK_PIN                                    PA5
#define SPI1_MISO_PIN                                   PA6
#define SPI1_MOSI_PIN                                   PA7

#define USE_FLASHFS
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT
#define USE_FLASH_M25P16
#define M25P16_CS_PIN                SPI1_NSS_PIN
#define M25P16_SPI_BUS               BUS_SPI1



// *************** OSD *****************************
#define USE_OSD
#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN                                    PB13
#define SPI2_MISO_PIN                                   PB14
#define SPI2_MOSI_PIN                                   PB15

#define USE_MAX7456
#define MAX7456_SPI_BUS                                 BUS_SPI2
#define MAX7456_CS_PIN                                  PB12

// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE                ADC1
#define ADC1_DMA_STREAM             DMA2_Stream0
#define ADC_CHANNEL_1_PIN                               PC0  // CURRENT
#define ADC_CHANNEL_2_PIN                               PC1  // RSSI
#define ADC_CHANNEL_3_PIN                               PC2  // VBAT
#define VBAT_ADC_CHANNEL                                ADC_CHN_3
#define CURRENT_METER_ADC_CHANNEL                       ADC_CHN_1
#define RSSI_ADC_CHANNEL                                ADC_CHN_2

// *************** LED *****************************
#define USE_LED_STRIP
#define WS2811_PIN                                      PB6


#define DEFAULT_FEATURES (FEATURE_TX_PROF_SEL | FEATURE_CURRENT_METER | FEATURE_TELEMETRY | FEATURE_VBAT | FEATURE_OSD | FEATURE_BLACKBOX)
#define USE_DSHOT
#define USE_ESC_SENSOR
#define USE_SERIAL_4WAY_BLHELI_INTERFACE
#define USE_SERIALSHOT

#define SERIALRX_UART                                   SERIAL_PORT_USART2
#define DEFAULT_RX_TYPE                                 RX_TYPE_SERIAL
#define SERIALRX_PROVIDER                               SERIALRX_SBUS

#define MAX_PWM_OUTPUT_PORTS                            6

#define CURRENT_METER_SCALE                             102

#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN                                      PC3 // RF Switch
#define PINIO2_PIN                                      PC14 // RF Switch
#define PINIO1_FLAGS                                    PINIO_FLAGS_INVERTED
#define PINIO2_FLAGS                                    PINIO_FLAGS_INVERTED

#define TARGET_IO_PORTA                                 0xffff
#define TARGET_IO_PORTB                                 0xffff
#define TARGET_IO_PORTC                                 0xffff
#define TARGET_IO_PORTD                                 (BIT(2))

