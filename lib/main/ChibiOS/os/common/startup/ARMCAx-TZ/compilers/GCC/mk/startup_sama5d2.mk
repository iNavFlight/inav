# List of the ChibiOS generic SAMA5D2 file.
STARTUPSRC = $(CHIBIOS)/os/common/startup/ARMCAx-TZ/compilers/GCC/crt1.c \
			 $(CHIBIOS)/os/common/startup/ARMCAx-TZ/devices/SAMA5D2/mmu.c

STARTUPASM = $(CHIBIOS)/os/common/startup/ARMCAx-TZ/devices/SAMA5D2/boot.S \
             $(CHIBIOS)/os/common/startup/ARMCAx-TZ/compilers/GCC/vectors.S \
             $(CHIBIOS)/os/common/startup/ARMCAx-TZ/compilers/GCC/crt0.S

STARTUPINC = $(CHIBIOS)/os/common/portability/GCC \
             ${CHIBIOS}/os/common/startup/ARMCAx-TZ/devices/SAMA5D2 \
             $(CHIBIOS)/os/common/ext/ARM/CMSIS/Core_A/Include \
             $(CHIBIOS)/os/common/portability/GCC

STARTUPLD  = ${CHIBIOS}/os/common/startup/ARMCAx-TZ/compilers/GCC/ld

# Shared variables
ALLXASMSRC += $(STARTUPASM)
ALLCSRC    += $(STARTUPSRC)
ALLINC     += $(STARTUPINC)
