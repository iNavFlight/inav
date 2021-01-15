# List of the required lwIP files.
LWIPDIR = $(CHIBIOS)/ext/lwip/src

# The various blocks of files are outlined in Filelists.mk.
include $(LWIPDIR)/Filelists.mk

LWBINDSRC = \
        $(CHIBIOS)/os/various/lwip_bindings/lwipthread.c \
        $(CHIBIOS)/os/various/lwip_bindings/arch/sys_arch.c


# Add blocks of files from Filelists.mk as required for enabled options
LWSRC = $(COREFILES) $(CORE4FILES) $(APIFILES) $(LWBINDSRC) $(NETIFFILES) $(HTTPDFILES)

LWINC = \
        $(CHIBIOS)/os/various/lwip_bindings \
        $(LWIPDIR)/include

# Shared variables
ALLCSRC += $(LWSRC)
ALLINC  += $(LWINC) \
           $(CHIBIOS)/os/various 
