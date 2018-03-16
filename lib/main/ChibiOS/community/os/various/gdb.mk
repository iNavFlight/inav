GDB               ?= arm-none-eabi-gdb
GDB_PROGRAM       ?= $(BUILDDIR)/$(PROJECT).elf
GDB_PORT          ?= 2331
GDB_START_ADDRESS ?= 0
GDB_BREAK         ?= main

gdb-debug:
	printf "target remote localhost:$(GDB_PORT)\nmem $(GDB_START_ADDRESS) 0\nbreak $(GDB_BREAK)\nload\nmon reset\ncontinue" > .gdbinit
	$(GDB) --command=.gdbinit $(GDB_PROGRAM)



.PHONY: gdb-debug
