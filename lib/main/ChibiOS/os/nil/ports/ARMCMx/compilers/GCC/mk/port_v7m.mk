# List of the ChibiOS/NIL ARMv7M generic port files.
PORTSRC = $(CHIBIOS)/os/nil/ports/ARMCMx/nilcore.c \
          $(CHIBIOS)/os/nil/ports/ARMCMx/nilcore_v7m.c
          
PORTASM = $(CHIBIOS)/os/nil/ports/ARMCMx/compilers/GCC/nilcoreasm_v7m.s

PORTINC = $(CHIBIOS)/os/nil/ports/ARMCMx \
          $(CHIBIOS)/os/nil/ports/ARMCMx/compilers/GCC
