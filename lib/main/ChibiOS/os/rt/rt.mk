# List of all the ChibiOS/RT kernel files, there is no need to remove the files
# from this list, you can disable parts of the kernel by editing chconf.h.
ifeq ($(USE_SMART_BUILD),yes)
CHCONF := $(strip $(shell cat chconf.h | egrep -e "define"))

KERNSRC := $(CHIBIOS)/os/rt/src/chsys.c \
           $(CHIBIOS)/os/rt/src/chdebug.c \
           $(CHIBIOS)/os/rt/src/chvt.c \
           $(CHIBIOS)/os/rt/src/chschd.c \
           $(CHIBIOS)/os/rt/src/chthreads.c
ifneq ($(findstring CH_CFG_USE_TM TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/os/rt/src/chtm.c
endif
ifneq ($(findstring CH_DBG_STATISTICS TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/os/rt/src/chstats.c
endif
ifneq ($(findstring CH_CFG_USE_DYNAMIC TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/os/rt/src/chdynamic.c
endif
ifneq ($(findstring CH_CFG_USE_REGISTRY TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/os/rt/src/chregistry.c
endif
ifneq ($(findstring CH_CFG_USE_SEMAPHORES TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/os/rt/src/chsem.c
endif
ifneq ($(findstring CH_CFG_USE_MUTEXES TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/os/rt/src/chmtx.c
endif
ifneq ($(findstring CH_CFG_USE_CONDVARS TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/os/rt/src/chcond.c
endif
ifneq ($(findstring CH_CFG_USE_EVENTS TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/os/rt/src/chevents.c
endif
ifneq ($(findstring CH_CFG_USE_MESSAGES TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/os/rt/src/chmsg.c
endif
ifneq ($(findstring CH_CFG_USE_MAILBOXES TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/os/rt/src/chmboxes.c
endif
ifneq ($(findstring CH_CFG_USE_QUEUES TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/os/rt/src/chqueues.c
endif
ifneq ($(findstring CH_CFG_USE_MEMCORE TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/os/rt/src/chmemcore.c
endif
ifneq ($(findstring CH_CFG_USE_HEAP TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/os/rt/src/chheap.c
endif
ifneq ($(findstring CH_CFG_USE_MEMPOOLS TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/os/rt/src/chmempools.c
endif
else
KERNSRC = $(CHIBIOS)/os/rt/src/chsys.c \
          $(CHIBIOS)/os/rt/src/chdebug.c \
          $(CHIBIOS)/os/rt/src/chvt.c \
          $(CHIBIOS)/os/rt/src/chschd.c \
          $(CHIBIOS)/os/rt/src/chthreads.c \
          $(CHIBIOS)/os/rt/src/chtm.c \
          $(CHIBIOS)/os/rt/src/chstats.c \
          $(CHIBIOS)/os/rt/src/chdynamic.c \
          $(CHIBIOS)/os/rt/src/chregistry.c \
          $(CHIBIOS)/os/rt/src/chsem.c \
          $(CHIBIOS)/os/rt/src/chmtx.c \
          $(CHIBIOS)/os/rt/src/chcond.c \
          $(CHIBIOS)/os/rt/src/chevents.c \
          $(CHIBIOS)/os/rt/src/chmsg.c \
          $(CHIBIOS)/os/rt/src/chmboxes.c \
          $(CHIBIOS)/os/rt/src/chqueues.c \
          $(CHIBIOS)/os/rt/src/chmemcore.c \
          $(CHIBIOS)/os/rt/src/chheap.c \
          $(CHIBIOS)/os/rt/src/chmempools.c
endif

# Required include directories
KERNINC = $(CHIBIOS)/os/rt/include
