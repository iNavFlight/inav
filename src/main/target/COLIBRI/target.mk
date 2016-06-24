F405_TARGETS   += $(TARGET)
FEATURES       += VCP ONBOARDFLASH

TARGET_SRC = \
            drivers/accgyro_mpu.c \
            drivers/accgyro_spi_mpu6000.c \
            drivers/barometer_ms5611.c \
            drivers/compass_hmc5883l.c \
            drivers/pitotmeter_ms4525.c \
            target/$(TARGET)/system_stm32f4xx.c

