
F405_TARGETS   += $(TARGET)
FEATURES       += VCP ONBOARDFLASH

TARGET_SRC = stm32f4xx_crc.c \
             drivers/dma_spi.c \
             drivers/barometer/barometer_bmp085.c \
             drivers/barometer/barometer_bmp280.c \
             drivers/barometer/barometer_ms56xx.c \
             drivers/compass/compass_ak8963.c \
             drivers/compass/compass_ak8975.c \
             drivers/compass/compass_hmc5883l.c \
             drivers/compass/compass_mag3110.c \
             drivers/compass/compass_qmc5883l.c \
             drivers/compass/compass_ist8310.c \
             drivers/pitotmeter_ms4525.c \
             drivers/accgyro/accgyro_imuf9001.c \
             drivers/max7456.c