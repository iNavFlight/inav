GDB ?= $(ARM_SDK_PREFIX)gdb
GDB_OPENOCD_REMOTE ?= localhost:3333

GDB_OPENOCD_INIT_CMDS ?= -ex "monitor reset halt"
ifneq ($(LOAD),)
GDB_OPENOCD_INIT_CMDS += -ex load
endif
ifneq ($(SEMIHOSTING),)
GDB_OPENOCD_INIT_CMDS += -ex "monitor arm semihosting enable"
endif

gdb-openocd: $(TARGET_ELF)
	(nc -z $(subst :, ,$(GDB_OPENOCD_REMOTE)) 2> /dev/null && \
	$(GDB) $< -ex "target remote $(GDB_OPENOCD_REMOTE)" $(GDB_OPENOCD_INIT_CMDS)) || \
	$(GDB) $< -ex "target remote | $(OPENOCD_CMDLINE) -c \"gdb_port pipe;\"" $(GDB_OPENOCD_INIT_CMDS)
