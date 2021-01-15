# List of all the ChibiOS/LIB files, there is no need to remove the files
# from this list, you can disable parts of the kernel by editing chlibconf.h.
ifeq ($(USE_SMART_BUILD),yes)

# Configuration files directory
ifeq ($(CHCONFDIR),)
  ifeq ($(CONFDIR),)
    CHCONFDIR = .
  else
    CHCONFDIR := $(CONFDIR)
  endif
endif

CHLIBCONF := $(strip $(shell cat $(CHCONFDIR)/chconf.h | egrep -e "\#define"))

LIBSRC :=
ifneq ($(findstring CH_CFG_USE_MAILBOXES TRUE,$(CHLIBCONF)),)
LIBSRC += $(CHIBIOS)/os/oslib/src/chmboxes.c
endif
ifneq ($(findstring CH_CFG_USE_MEMCORE TRUE,$(CHLIBCONF)),)
LIBSRC += $(CHIBIOS)/os/oslib/src/chmemcore.c
endif
ifneq ($(findstring CH_CFG_USE_HEAP TRUE,$(CHLIBCONF)),)
LIBSRC += $(CHIBIOS)/os/oslib/src/chmemheaps.c
endif
ifneq ($(findstring CH_CFG_USE_MEMPOOLS TRUE,$(CHLIBCONF)),)
LIBSRC += $(CHIBIOS)/os/oslib/src/chmempools.c
endif
ifneq ($(findstring CH_CFG_USE_PIPES TRUE,$(CHLIBCONF)),)
LIBSRC += $(CHIBIOS)/os/oslib/src/chpipes.c
endif
ifneq ($(findstring CH_CFG_USE_FACTORY TRUE,$(CHLIBCONF)),)
LIBSRC += $(CHIBIOS)/os/oslib/src/chfactory.c
endif
else
LIBSRC := $(CHIBIOS)/os/oslib/src/chmboxes.c \
          $(CHIBIOS)/os/oslib/src/chmemcore.c \
          $(CHIBIOS)/os/oslib/src/chmemheaps.c \
          $(CHIBIOS)/os/oslib/src/chmempools.c \
          $(CHIBIOS)/os/oslib/src/chpipes.c \
          $(CHIBIOS)/os/oslib/src/chfactory.c
endif

# Required include directories
LIBINC := $(CHIBIOS)/os/oslib/include

# Shared variables
ALLCSRC += $(LIBSRC)
ALLINC  += $(LIBINC)
