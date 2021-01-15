# Configuration files directory
ifeq ($(CONFDIR),)
  CONFDIR = .
endif

MCUCONF := $(strip $(shell cat $(CONFDIR)/mcuconf.h | egrep -e "\#define"))

ifneq ($(findstring SAMA_USE_SDMMC TRUE,$(MCUCONF)),)

PLATFORMSRC +=	$(CHIBIOS)/os/hal/ports/SAMA/LLD/SDMMCv1/sama_sdmmc_lld.c	\
				$(CHIBIOS)/os/hal/ports/SAMA/LLD/SDMMCv1/ch_sdmmc_device.c	\
				$(CHIBIOS)/os/hal/ports/SAMA/LLD/SDMMCv1/ch_sdmmc_sdio.c	\
				$(CHIBIOS)/os/hal/ports/SAMA/LLD/SDMMCv1/ch_sdmmc_cmds.c	\
				$(CHIBIOS)/os/hal/ports/SAMA/LLD/SDMMCv1/ch_sdmmc_mmc.c		\
				$(CHIBIOS)/os/hal/ports/SAMA/LLD/SDMMCv1/ch_sdmmc_sd.c		\
				$(CHIBIOS)/os/hal/ports/SAMA/LLD/SDMMCv1/ch_sdmmc.c			
					
PLATFORMINC +=  $(CHIBIOS)/os/hal/ports/SAMA/LLD/SDMMCv1

endif
