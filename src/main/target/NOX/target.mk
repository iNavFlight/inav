F411_TARGETS    += $(TARGET)
FEATURES        += VCP ONBOARDFLASH MSC

TARGET_SRC = \
            drivers/accgyro/accgyro_mpu6000.c \
            drivers/barometer/barometer_bmp280.c \
            drivers/light_ws2811strip.c \
            drivers/flash_m25p16.c \
            drivers/max7456.c
