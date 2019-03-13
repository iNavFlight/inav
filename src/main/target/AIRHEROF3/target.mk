F3_TARGETS  += $(TARGET)
HSE_VALUE = 12000000

TARGET_SRC = \
            drivers/accgyro/accgyro_mpu.c \
            drivers/accgyro/accgyro_mpu6500.c \
            drivers/barometer/barometer_bmp280.c \
            drivers/compass/compass_hmc5883l.c \
            drivers/compass/compass_mag3110.c \
            drivers/compass/compass_qmc5883l.c \
            drivers/compass/compass_ist8310.c \
            drivers/compass/compass_ist8308.c \
            drivers/compass/compass_lis3mdl.c \
            drivers/rangefinder/rangefinder_hcsr04.c \
            drivers/light_ws2811strip.c \
            drivers/serial_softserial.c \
            drivers/pitotmeter_adc.c
