# RELIANCE EDGE files.

RELEDGESRC=								\
	$(CHIBIOS)/ext/reliance-edge/core/driver/blockio.c			\
	$(CHIBIOS)/ext/reliance-edge/core/driver/buffer.c			\
	$(CHIBIOS)/ext/reliance-edge/core/driver/core.c				\
	$(CHIBIOS)/ext/reliance-edge/core/driver/dir.c				\
	$(CHIBIOS)/ext/reliance-edge/core/driver/format.c			\
	$(CHIBIOS)/ext/reliance-edge/core/driver/imap.c				\
	$(CHIBIOS)/ext/reliance-edge/core/driver/imapextern.c		\
	$(CHIBIOS)/ext/reliance-edge/core/driver/imapinline.c		\
	$(CHIBIOS)/ext/reliance-edge/core/driver/inode.c			\
	$(CHIBIOS)/ext/reliance-edge/core/driver/inodedata.c		\
	$(CHIBIOS)/ext/reliance-edge/core/driver/volume.c			\
	$(CHIBIOS)/ext/reliance-edge/fse/fse.c						\
	$(CHIBIOS)/ext/reliance-edge/posix/path.c					\
	$(CHIBIOS)/ext/reliance-edge/posix/posix.c					\
	$(CHIBIOS)/ext/reliance-edge/util/bitmap.c					\
	$(CHIBIOS)/ext/reliance-edge/util/crc.c						\
	$(CHIBIOS)/ext/reliance-edge/util/memory.c					\
	$(CHIBIOS)/ext/reliance-edge/util/namelen.c					\
	$(CHIBIOS)/ext/reliance-edge/util/sign.c					\
	$(CHIBIOS)/ext/reliance-edge/util/string.c

RELEDGEINC = $(CHIBIOS)/ext/reliance-edge/include   			\
			$(CHIBIOS)/ext/reliance-edge/core/include

RELEDGEBINDINC = $(CHIBIOS)/os/various/reledge_bindings/SAMA5D2x
			
RELEDGEBINDSRC	= 	\
	$(CHIBIOS)/os/various/reledge_bindings/SAMA5D2x/ch_sdmmc_reledge.c	\
	$(CHIBIOS)/os/various/reledge_bindings/SAMA5D2x/osassert.c			\
	$(CHIBIOS)/os/various/reledge_bindings/SAMA5D2x/osbdev.c			\
	$(CHIBIOS)/os/various/reledge_bindings/SAMA5D2x/osclock.c			\
	$(CHIBIOS)/os/various/reledge_bindings/SAMA5D2x/osmutex.c			\
	$(CHIBIOS)/os/various/reledge_bindings/SAMA5D2x/osoutput.c			\
	$(CHIBIOS)/os/various/reledge_bindings/SAMA5D2x/ostask.c
	
# Shared variables
ALLCSRC += $(RELEDGESRC) $(RELEDGEBINDSRC)
ALLINC  += $(RELEDGEINC) $(RELEDGEBINDINC)