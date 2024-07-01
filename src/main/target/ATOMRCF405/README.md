# ATOMRC F405 config
Tested against a "ATOMRC Exceed F405 DJI Mini FC"

## Betaflight config

```
# Betaflight / STM32F405 (S405) 4.1.0 May 22 2019 / 02:24:46 (1541466da) MSP API: 1.42

#define USE_ACC_SPI_MPU6000
#define USE_GYRO_SPI_MPU6000
#define USE_FLASH_W25Q128FV
#define USE_MAX7456

board_name ATOMRCF405
manufacturer_id SKZO

# resources
resource BEEPER 1 C13
resource MOTOR 1 B00
resource MOTOR 2 B01
resource MOTOR 3 C07
resource MOTOR 4 B04
resource MOTOR 5 C08
resource MOTOR 6 C09
resource PPM 1 B15
resource LED_STRIP 1 B06
resource SERIAL_TX 1 A09
resource SERIAL_TX 2 A02
resource SERIAL_TX 3 B10
resource SERIAL_TX 4 A00
resource SERIAL_TX 5 C12
resource SERIAL_RX 1 A10
resource SERIAL_RX 2 A03
resource SERIAL_RX 3 B11
resource SERIAL_RX 4 A01
resource SERIAL_RX 5 D02
resource I2C_SCL 1 B08
resource I2C_SDA 1 B09
resource LED 1 C14
resource LED 2 B02
resource SPI_SCK 1 A05
resource SPI_SCK 2 B13
resource SPI_SCK 3 C10
resource SPI_MISO 1 A06
resource SPI_MISO 2 B14
resource SPI_MISO 3 C11
resource SPI_MOSI 1 A07
resource SPI_MOSI 2 C03
resource SPI_MOSI 3 B05
resource ESCSERIAL 1 A03
resource CAMERA_CONTROL 1 B03
resource ADC_BATT 1 C02
resource ADC_RSSI 1 C00
resource ADC_CURR 1 C01
resource FLASH_CS 1 B12
resource OSD_CS 1 A15
resource GYRO_EXTI 1 C04
resource GYRO_CS 1 A04

# timer
timer B15 AF9
# pin B15: TIM12 CH2 (AF9)
timer B00 AF2
# pin B00: TIM3 CH3 (AF2)
timer B01 AF2
# pin B01: TIM3 CH4 (AF2)
timer C07 AF2
# pin C07: TIM3 CH2 (AF2)
timer B04 AF2
# pin B04: TIM3 CH1 (AF2)
timer C08 AF3
# pin C08: TIM8 CH3 (AF3)
timer C09 AF3
# pin C09: TIM8 CH4 (AF3)
timer B06 AF2
# pin B06: TIM4 CH1 (AF2)
timer B03 AF1
# pin B03: TIM2 CH2 (AF1)

# dma
dma ADC 1 1
# ADC 1: DMA2 Stream 4 Channel 0
dma pin B00 0
# pin B00: DMA1 Stream 7 Channel 5
dma pin B01 0
# pin B01: DMA1 Stream 2 Channel 5
dma pin C07 0
# pin C07: DMA1 Stream 5 Channel 5
dma pin B04 0
# pin B04: DMA1 Stream 4 Channel 5
dma pin C08 0
# pin C08: DMA2 Stream 2 Channel 0
dma pin C09 0
# pin C09: DMA2 Stream 7 Channel 7
dma pin B06 0
# pin B06: DMA1 Stream 0 Channel 2
dma pin B03 0
# pin B03: DMA1 Stream 6 Channel 3

# feature
feature OSD

# master
set baro_bustype = I2C
set baro_i2c_device = 1
set blackbox_device = SPIFLASH
set dshot_burst = ON
set current_meter = ADC
set battery_meter = ADC
set beeper_inversion = ON
set beeper_od = OFF
set system_hse_mhz = 8
set max7456_spi_bus = 3
set flash_spi_bus = 2
set gyro_1_bustype = SPI
set gyro_1_spibus = 1
set gyro_1_sensor_align = CW90
set gyro_1_align_yaw = 900
```

## Board timers
```
# timer

# pin B15: TIM12 CH2 (AF9) # PPM 1
timer B15 AF9

# pin B00: TIM3 CH3 (AF2) # M1
timer B00 AF2

# pin B01: TIM3 CH4 (AF2) # M2
timer B01 AF2

# pin C07: TIM3 CH2 (AF2) # M3
timer C07 AF2

# pin B04: TIM3 CH1 (AF2) # M4 
timer B04 AF2

# pin C08: TIM8 CH3 (AF3) # M5
timer C08 AF3

# pin C09: TIM8 CH4 (AF3) # M6
timer C09 AF3

# pin B06: TIM4 CH1 (AF2) # LED_STRIP 1
timer B06 AF2

# pin B03: TIM2 CH2 (AF1) # CAMERA_CONTROL 1
timer B03 AF1
```