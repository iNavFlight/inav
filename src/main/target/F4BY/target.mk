F405_TARGETS    += $(TARGET)
FEATURES        += SDCARD VCP MSC

TARGET_SRC = \
            drivers/accgyro/accgyro_mpu6000.c \
            drivers/accgyro/accgyro_icm20689.c \
            drivers/barometer/barometer_ms56xx.c \
            drivers/compass/compass_hmc5883l.c \
            drivers/compass/compass_qmc5883l.c \
            drivers/compass/compass_ist8310.c \
            drivers/compass/compass_ist8308.c \
            drivers/compass/compass_mag3110.c \
            drivers/compass/compass_lis3mdl.c \
            drivers/rangefinder/rangefinder_hcsr04.c \
            drivers/max7456.c
