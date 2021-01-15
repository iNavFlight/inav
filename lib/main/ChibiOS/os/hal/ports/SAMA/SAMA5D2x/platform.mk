# Required platform files.

PLATFORMSRC := $(CHIBIOS)/os/hal/ports/SAMA/SAMA5D2x/hal_lld.c        \
               $(CHIBIOS)/os/hal/ports/SAMA/SAMA5D2x/hal_st_lld.c     \
               $(CHIBIOS)/os/hal/ports/SAMA/SAMA5D2x/sama_aic.c       \
               $(CHIBIOS)/os/hal/ports/SAMA/SAMA5D2x/sama_matrix.c    \
               $(CHIBIOS)/os/hal/ports/SAMA/SAMA5D2x/sama_secumod.c   \
               $(CHIBIOS)/os/hal/ports/SAMA/SAMA5D2x/sama_onewire.c   \
               $(CHIBIOS)/os/hal/ports/SAMA/SAMA5D2x/sama_classd.c    \
               $(CHIBIOS)/os/hal/ports/SAMA/SAMA5D2x/sama_lcdc.c      \
               $(CHIBIOS)/os/hal/ports/SAMA/SAMA5D2x/sama_cache.c     \
               $(CHIBIOS)/os/hal/ports/SAMA/SAMA5D2x/sama_tc_lld.c    \
               $(CHIBIOS)/os/hal/ports/SAMA/SAMA5D2x/hal_crypto_lld.c
               
# Required include directories.
PLATFORMINC := $(CHIBIOS)/os/hal/ports/SAMA/SAMA5D2x

# Optional platform files.
ifeq ($(USE_SMART_BUILD),yes)

# Configuration files directory
ifeq ($(CONFDIR),)
  CONFDIR = .
endif

HALCONF := $(strip $(shell cat $(CONFDIR)/halconf.h | egrep -e "\#define"))

else
endif

# Drivers compatible with the platform.
include $(CHIBIOS)/os/hal/ports/SAMA/LLD/DMAv1/driver.mk
include $(CHIBIOS)/os/hal/ports/SAMA/LLD/I2Cv1/driver.mk
include $(CHIBIOS)/os/hal/ports/SAMA/LLD/MACv1/driver.mk
include $(CHIBIOS)/os/hal/ports/SAMA/LLD/PIOv1/driver.mk
include $(CHIBIOS)/os/hal/ports/SAMA/LLD/QUADSPIv1/driver.mk
include $(CHIBIOS)/os/hal/ports/SAMA/LLD/SPIv1/driver.mk
include $(CHIBIOS)/os/hal/ports/SAMA/LLD/RTCv1/driver.mk
include $(CHIBIOS)/os/hal/ports/SAMA/LLD/xWDGv1/driver.mk
include $(CHIBIOS)/os/hal/ports/SAMA/LLD/USARTv1/driver.mk
include $(CHIBIOS)/os/hal/ports/SAMA/LLD/CRYPTOv1/driver.mk
include $(CHIBIOS)/os/hal/ports/SAMA/LLD/SDMMCv1/driver.mk 
include $(CHIBIOS)/os/hal/ports/SAMA/LLD/RNGv1/driver.mk 

# Shared variables
ALLCSRC += $(PLATFORMSRC)
ALLINC  += $(PLATFORMINC)
