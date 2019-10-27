GDB ?= $(ARM_SDK_PREFIX)gdb
GDB_OPENOCD_REMOTE ?= localhost:3333

GDB_OPENOCD_INIT_CMDS = -ex "arm semihosting enable" -ex "monitor reset halt"

gdb-openocd: $(TARGET_ELF)
	(nc -z $(subst :, ,$(GDB_OPENOCD_REMOTE)) 2> /dev/null && \
	$(GDB) $< -ex "target remote $(GDB_OPENOCD_REMOTE)" $(GDB_OPENOCD_RESET)) || \
	$(GDB) $< -ex "target remote | $(OPENOCD_CMDLINE) -c \"gdb_port pipe;\"" $(GDB_OPENOCD_RESET)
