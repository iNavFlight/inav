# List of the ChibiOS generic STM32L0xx startup and CMSIS files.
STARTUPSRC = $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC/crt1.c \
             $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC/vectors.c
          
STARTUPASM = $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC/crt0_v6m.s

STARTUPINC = $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC \
             $(CHIBIOS)/os/common/ports/ARMCMx/devices/STM32L0xx \
             $(CHIBIOS)/os/ext/CMSIS/include \
             $(CHIBIOS)/os/ext/CMSIS/ST/STM32L0xx

STARTUPLD  = $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC/ld
