F765XG_TARGETS += $(TARGET)
FEATURES       += SDCARD VCP

TARGET_SRC = \
            drivers/accgyro/accgyro_mpu6000.c \
            drivers/accgyro/accgyro_mpu6500.c \
            drivers/barometer/barometer_spl06.c \
            drivers/compass/compass_hmc5883l.c \
            drivers/compass/compass_qmc5883l.c \
            drivers/compass/compass_ist8310.c \
            drivers/compass/compass_ist8308.c \
            drivers/compass/compass_mag3110.c \
            drivers/compass/compass_lis3mdl.c \
            drivers/pitotmeter_adc.c
