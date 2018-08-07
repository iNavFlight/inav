F446_TARGETS    += $(TARGET)
FEATURES        += VCP SDCARD
HSE_VALUE       = 16000000

TARGET_SRC = \
            drivers/accgyro/accgyro_bmi160.c \
            drivers/barometer/barometer_bmp280.c \
            drivers/compass/compass_hmc5883l.c \
            drivers/compass/compass_qmc5883l.c \
            drivers/compass/compass_ist8310.c \
            drivers/compass/compass_mag3110.c \
            drivers/compass/compass_lis3mdl.c
