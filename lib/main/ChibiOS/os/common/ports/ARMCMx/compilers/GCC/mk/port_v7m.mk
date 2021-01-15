# List of the ChibiOS/RT ARMv7M generic port files.
PORTSRC = $(CHIBIOS)/os/common/ports/ARMCMx/chcore.c \
          $(CHIBIOS)/os/common/ports/ARMCMx/chcore_v7m.c
          
PORTASM = $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC/chcoreasm_v7m.S

PORTINC = $(CHIBIOS)/os/common/ports/ARMCMx \
          $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC

# Shared variables
ALLXASMSRC += $(PORTASM)
ALLCSRC    += $(PORTSRC)
ALLINC     += $(PORTINC)
