F405_TARGETS   += $(TARGET)
FEATURES       = VCP 

TARGET_SRC = \
            io/osd.c \
            drivers/accgyro/accgyro_mpu.c \
            drivers/accgyro/accgyro_fake.c \
            drivers/accgyro/accgyro_mpu6500.c \
            drivers/accgyro/accgyro_mpu9250.c \
            drivers/barometer/barometer_bmp280.c \
            drivers/compass/compass_mpu9250.c \
            drivers/light_ws2811strip.c \
            drivers/max7456.c
