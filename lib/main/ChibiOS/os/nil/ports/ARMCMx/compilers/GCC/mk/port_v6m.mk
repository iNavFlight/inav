# List of the ChibiOS/NIL ARMv6M generic port files.
PORTSRC = $(CHIBIOS)/os/nil/ports/ARMCMx/nilcore.c \
          $(CHIBIOS)/os/nil/ports/ARMCMx/nilcore_v6m.c
          
PORTASM = $(CHIBIOS)/os/nil/ports/ARMCMx/compilers/GCC/nilcoreasm_v6m.s

PORTINC = $(CHIBIOS)/os/nil/ports/ARMCMx \
          $(CHIBIOS)/os/nil/ports/ARMCMx/compilers/GCC
