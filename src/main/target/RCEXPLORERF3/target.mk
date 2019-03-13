F3_TARGETS  += $(TARGET)
FEATURES    = VCP HIGHEND

TARGET_SRC = \
           drivers/accgyro/accgyro_mpu.c \
           drivers/accgyro/accgyro_mpu6000.c \
           drivers/barometer/barometer_ms56xx.c \
           drivers/compass/compass_hmc5883l.c \
           drivers/compass/compass_qmc5883l.c \
           drivers/compass/compass_ist8310.c \
           drivers/compass/compass_ist8308.c \
           drivers/compass/compass_mag3110.c \
           drivers/compass/compass_lis3mdl.c \
           drivers/display_ug2864hsweg01.c \
           drivers/serial_usb_vcp.c \
           drivers/flash_m25p16.c \
           drivers/light_ws2811strip.c
