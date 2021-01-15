# List of the ChibiOS generic SAMA5D2 file.
STARTUPSRC = $(CHIBIOS)/os/common/startup/ARM/compilers/GCC/crt1.c \
			 $(CHIBIOS)/os/common/startup/ARM/devices/SAMA5D2/mmu.c

STARTUPASM = $(CHIBIOS)/os/common/startup/ARM/devices/SAMA5D2/boot.S \
             $(CHIBIOS)/os/common/startup/ARM/compilers/GCC/vectors.S \
             $(CHIBIOS)/os/common/startup/ARM/compilers/GCC/crt0.S

STARTUPINC = $(CHIBIOS)/os/common/portability/GCC \
             ${CHIBIOS}/os/common/startup/ARM/devices/SAMA5D2 \
             $(CHIBIOS)/os/common/ext/ARM/CMSIS/Core_A/Include \
             $(CHIBIOS)/os/common/portability/GCC 

STARTUPLD  = ${CHIBIOS}/os/common/startup/ARM/compilers/GCC/ld

# Shared variables
ALLXASMSRC += $(STARTUPASM)
ALLCSRC    += $(STARTUPSRC)
ALLINC     += $(STARTUPINC)
