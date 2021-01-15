# List of the ChibiOS generic KL2x startup and CMSIS files.
STARTUPSRC = $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/crt1.c

STARTUPASM = $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/crt0_v6m.S \
             $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/vectors.S

STARTUPINC = $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC \
             $(CHIBIOS_CONTRIB)/os/common/startup/ARMCMx/devices/KL2x \
             $(CHIBIOS)/os/common/ext/CMSIS/include \
             $(CHIBIOS)/os/common/ext/ARM/CMSIS/Core/Include \
             $(CHIBIOS_CONTRIB)/os/common/ext/CMSIS/KINETIS

STARTUPLD  = $(CHIBIOS_CONTRIB)/os/common/startup/ARMCMx/compilers/GCC/ld

# Shared variables
ALLXASMSRC += $(STARTUPASM)
ALLCSRC    += $(STARTUPSRC)
ALLINC     += $(STARTUPINC)
