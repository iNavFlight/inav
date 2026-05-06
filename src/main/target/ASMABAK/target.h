#pragma once
#define MAX_PWM_OUTPUT_PORTS    8
#define TARGET_BOARD_IDENTIFIER "ASMB"
#define USBD_PRODUCT_STRING     "AsMaBak FC"

// LED и Buzzer (по ТЗ)
#define LED0                    PC0
#define BEEPER                  PC15
#define BEEPER_INVERTED         // если buzzer не пищит — уберите строку

// ============== IMU MPU9255 (гиро + аксел + магнитометр) ==============
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PB3
#define SPI1_MISO_PIN           PB4
#define SPI1_MOSI_PIN           PB5

#define MPU9250_CS_PIN          PC5
#define MPU9250_SPI_BUS         BUS_SPI1
#define USE_IMU_MPU9250
#define IMU_MPU9250_ALIGN       CW0_DEG        // подберите по ориентации платы (CW0/CW90/CW180/CW270)

#define USE_MAG_MPU9250
#define MAG_MPU9250_ALIGN       CW0_DEG

#define GYRO_INT_EXTI_PIN       PC4            // прерывание от гироскопа (EXTI4)

// ============== Барометр BMP280 ==============
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL_PIN            PB6
#define I2C1_SDA_PIN            PB7

#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C1
#define USE_BARO_BMP280

// ============== OSD MAX7456 ==============
#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15
#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI2
#define MAX7456_CS_PIN          PB12

// ============== UARTы ==============
#define USE_VCP                                     // USB

#define USE_UART1
#define UART1_TX_PIN            PA9
#define UART1_RX_PIN            PA10                // SBUS/IBUS

#define USE_UART2
#define UART2_TX_PIN            PA2
#define UART2_RX_PIN            PA3                 // Motor_Serial (если нужно)

#define USE_UART3
#define UART3_TX_PIN            PC10
#define UART3_RX_PIN            PC11                // GPS

#define USE_UART4
#define UART4_TX_PIN            PA0
#define UART4_RX_PIN            PA1                 // MSP

#define USE_UART5
#define UART5_TX_PIN            PC12
#define UART5_RX_PIN            PD2                 // SmartAudio / Tramp

#define SERIAL_PORT_COUNT       6

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART1

// ============== Моторы (DShot/ESC) ==============
#define USE_DSHOT
#define USE_ESC_SENSOR
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

// ============== ADC (RSSI) ==============
#define USE_ADC
#define ADC_INSTANCE            ADC1
#define RSSI_ADC_PIN            PC3                 // ADC123_IN13

// ============== Дополнительно (опционально по ТЗ) ==============
#define USE_LED_STRIP
#define WS2811_PIN              PA15                // если есть LED-стрип (можно изменить)

// Gimbal (подвес камеры) — PWM
#define USE_SERVOS
#define GIMBAL_PITCH_PIN        PA8                 // TIM1_CH1
#define GIMBAL_ROLL_PIN         PB2                 // TIM2_CH4
#define GIMBAL_YAW_PIN          PB10                // TIM2_CH3
#define GIMBAL_ZOOM_PIN         PA15                // TIM2_CH1 (если нужно)

// GPIO (навесные устройства)
#define PINIO1_PIN              PC13
#define PINIO2_PIN              PC14

// IO порты
#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         (BIT(2))

#define DEFAULT_FEATURES        (FEATURE_OSD | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TELEMETRY | FEATURE_BLACKBOX | FEATURE_GPS)