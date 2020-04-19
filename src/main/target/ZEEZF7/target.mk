F7X2RE_TARGETS += $(TARGET)
FEATURES       += ONBOARDFLASH VCP MSC

TARGET_SRC = \
            drivers/accgyro/accgyro_mpu6000.c \
            drivers/light_ws2811strip.c \
            drivers/max7456.c \
