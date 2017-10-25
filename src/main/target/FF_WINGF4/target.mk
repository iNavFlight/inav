F405_TARGETS    += $(TARGET)
FEATURES        += VCP

TARGET_SRC = \
             drivers/accgyro/accgyro_mpu6500.c \
             drivers/accgyro/accgyro_spi_mpu6500.c \
             drivers/compass/compass_ak8963.c \
             drivers/barometer/barometer_ms56xx.c \
             drivers/barometer/barometer_spi_ms56xx.c \
             drivers/compass/compass_hmc5883l.c \
             drivers/rangefinder_hcsr04.c \
             drivers/serial_usb_vcp.c \
             drivers/max7456.c
             