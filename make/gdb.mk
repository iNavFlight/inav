GDB ?= $(ARM_SDK_PREFIX)gdb
GDB_REMOTE ?= localhost:3333

GDB_OPENOCD_INIT_CMDS ?=
GDB_OPENOCD_INIT_CMDS += -ex "monitor reset halt"
ifneq ($(LOAD),)
GDB_OPENOCD_INIT_CMDS += -ex load
endif
ifneq ($(SEMIHOSTING),)
GDB_OPENOCD_INIT_CMDS += -ex "monitor arm semihosting enable"
endif

gdb-openocd: $(TARGET_ELF)
	$(GDB) $< -ex "target remote $(GDB_REMOTE)" $(GDB_OPENOCD_INIT_CMDS)
