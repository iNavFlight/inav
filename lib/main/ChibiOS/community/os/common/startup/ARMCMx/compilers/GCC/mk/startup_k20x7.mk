include $(CHIBIOS_CONTRIB)/os/common/startup/ARMCMx/compilers/GCC/mk/startup_k20x.mk

STARTUPINC += $(CHIBIOS_CONTRIB)/os/common/startup/ARMCMx/devices/K20x7

# Shared variables
ALLXASMSRC += $(STARTUPASM)
ALLCSRC    += $(STARTUPSRC)
ALLINC     += $(STARTUPINC)
