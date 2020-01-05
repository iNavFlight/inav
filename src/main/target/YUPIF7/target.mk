F7X2RE_TARGETS  += $(TARGET)
FEATURES        += VCP  ONBOARDFLASH MSC
TARGET_SRC = \
            drivers/accgyro/accgyro_mpu.c \
            drivers/accgyro/accgyro_mpu6050.c \
            drivers/accgyro/accgyro_mpu6500.c \
            drivers/barometer/barometer_bmp280.c \
            drivers/barometer/barometer_ms56xx.c \
            drivers/compass/compass_hmc5883l.c \
            drivers/compass/compass_qmc5883l.c \
            drivers/compass/compass_ist8310.c \
            drivers/compass/compass_mag3110.c \
            drivers/max7456.c \
	    drivers/display_ug2864hsweg01.c \
	    drivers/pitotmeter_ms4525.c \
            drivers/light_ws2811strip.c \
            drivers/serial_softserial.c
