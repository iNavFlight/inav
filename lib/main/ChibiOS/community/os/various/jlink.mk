JLINK               ?= JLinkExe
JLINK_GDB_SERVER    ?= JLinkGDBServer
JLINK_GDB_PORT      ?= 2331
JLINK_IF            ?= swd
JLINK_SPEED         ?= 2000
JLINK_START_ADDRESS ?= 0
JLINK_BURN          ?= $(BUILDDIR)/$(PROJECT).bin
JLINK_COMMON_OPTS   ?= -device $(JLINK_DEVICE) -if $(JLINK_IF) -speed $(JLINK_SPEED)

jlink-flash:
	printf "$(JLINK_PRE_FLASH)\nloadbin $(JLINK_BURN) $(JLINK_START_ADDRESS)\nverifybin $(JLINK_BURN) $(JLINK_START_ADDRESS)\nr\ng\nexit\n" > $(BUILDDIR)/flash.jlink
	$(JLINK) $(JLINK_COMMON_OPTS) $(BUILDDIR)/flash.jlink

ifneq ($(SOFTDEVICE),)
jlink-flash-softdevice:
	printf "w4 4001e504 1\nloadbin $(NRF51SDK)/components/softdevice/$(SOFTDEVICE)/hex/$(SOFTDEVICE)_nrf51_$(SOFTDEVICE_RELEASE)_softdevice.hex 0\nr\ng\nexit\n" > $(BUILDDIR)/flash.softdevice.jlink
	$(JLINK) $(JLINK_COMMON_OPTS) $(BUILDDIR)/flash.softdevice.jlink
endif

ifneq ($(JLINK_ERASE_ALL),)
jlink-erase-all: 
	printf "$(JLINK_ERASE_ALL)\nr\nexit\n" > $(BUILDDIR)/erase-all.jlink
	$(JLINK) $(JLINK_COMMON_OPTS) $(BUILDDIR)/erase-all.jlink
endif

jlink-reset: 
	printf "r\nexit\n" > $(BUILDDIR)/reset.jlink
	$(JLINK) $(JLINK_COMMON_OPTS) $(BUILDDIR)/reset.jlink

jlink-pin-reset: 
	printf "$(JLINK_PIN_RESET)\nexit\n" > $(BUILDDIR)/pin-reset.jlink
	$(JLINK) $(JLINK_COMMON_OPTS) $(BUILDDIR)/pin-reset.jlink

jlink-debug-server:
	$(JLINK_GDB_SERVER) $(JLINK_COMMON_OPTS) -port $(JLINK_GDB_PORT)

.PHONY: jlink-flash jlink-flash-softdevice jlink-erase-all jlink-reset jlink-debug-server
