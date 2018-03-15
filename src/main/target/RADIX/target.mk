F446_TARGETS    += $(TARGET)
FEATURES        += VCP SDCARD BRAINFPV

HSE_VALUE       = 16000000

TARGET_SRC =  \
              drivers/light_ws2811strip.c \
              drivers/barometer/barometer_bmp280.c \
              drivers/accgyro/accgyro_fake.c \
              
