# List of all the ChibiOS/NIL kernel files, there is no need to remove the files
# from this list, you can disable parts of the kernel by editing chconf.h.
ifeq ($(USE_SMART_BUILD),yes)

# Configuration files directory
ifeq ($(CHCONFDIR),)
  ifeq ($(CONFDIR),)
    CHCONFDIR = .
  else
    CHCONFDIR := $(CONFDIR)
  endif
endif

CHCONF := $(strip $(shell cat $(CHCONFDIR)/chconf.h | egrep -e "\#define"))

KERNSRC := ${CHIBIOS}/os/nil/src/ch.c
else
KERNSRC := ${CHIBIOS}/os/nil/src/ch.c
endif

# Required include directories
KERNINC := ${CHIBIOS}/os/nil/include

# Shared variables
ALLCSRC += $(KERNSRC)
ALLINC  += $(KERNINC)

# OS Library
include $(CHIBIOS)/os/oslib/oslib.mk
