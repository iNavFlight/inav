# This makefile expects the following variables to be externally
# defined:
# XOPT     - Compiler extra options
# XDEFS    - Extra definitions

##############################################################################################
# Start of default section
#

TRGT = mingw32-
CC   = $(TRGT)gcc
AS   = $(TRGT)gcc -x assembler-with-cpp
AR   = $(TRGT)ar
COV  = gcov

# List all default C defines here, like -D_DEBUG=1
DDEFS = -DSIMULATOR

# List all default ASM defines here, like -D_DEBUG=1
DADEFS =

# List all default directories to look for include files here
DINCDIR =

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
DLIBS = -lws2_32

#
# End of default section
##############################################################################################

##############################################################################################
# Start of user section
#

# Define project name here
PROJECT = ch

# Define linker script file here
LDSCRIPT=

# List all user C define here, like -D_DEBUG=1
UDEFS =

# Define ASM defines here
UADEFS =

# Imported source files
CHIBIOS = ../../..
include $(CHIBIOS)/os/hal/boards/simulator/board.mk
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/hal/ports/simulator/win32/platform.mk
include $(CHIBIOS)/os/hal/osal/rt/osal.mk
include $(CHIBIOS)/os/rt/ports/SIMIA32/compilers/GCC/port.mk
include $(CHIBIOS)/os/rt/rt.mk
include $(CHIBIOS)/test/rt/test.mk

# List C source files here
SRC =  $(PORTSRC) \
       $(KERNSRC) \
       $(TESTSRC) \
       $(HALSRC) \
       $(OSALSRC) \
       $(PLATFORMSRC) \
       $(BOARDSRC) \
       main.c

# List ASM source files here
ASRC = 

# List all user directories here
UINCDIR = $(PORTINC) $(KERNINC) $(TESTINC) \
          $(HALINC) $(OSALINC) $(PLATFORMINC) $(BOARDINC) \
          $(CHIBIOS)/os/various

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS =

# Define optimisation level here
OPT = $(XOPT)

#
# End of user defines
##############################################################################################


INCDIR  = $(patsubst %,-I%,$(DINCDIR) $(UINCDIR))
LIBDIR  = $(patsubst %,-L%,$(DLIBDIR) $(ULIBDIR))
DEFS    = $(DDEFS) $(UDEFS) $(XDEFS)
ADEFS   = $(DADEFS) $(UADEFS)
OBJS    = $(ASRC:.s=.o) $(SRC:.c=.o)
LIBS    = $(DLIBS) $(ULIBS)

LDFLAGS = -Wl,-Map=$(PROJECT).map,--cref,--no-warn-mismatch -lgcov $(LIBDIR)
ASFLAGS = -Wa,-amhls=$(<:.s=.lst) $(ADEFS)
CPFLAGS = $(OPT) -Wall -Wextra -Wundef -Wstrict-prototypes -fverbose-asm -Wa,-ahlms=$(<:.c=.lst) $(DEFS)

# Generate dependency information
CPFLAGS += -MD -MP -MF .dep/$(@F).d

#
# makefile rules
#

all: $(OBJS) $(PROJECT).exe

%.o : %.c
	$(CC) -c $(CPFLAGS) -I . $(INCDIR) $< -o $@

%.o : %.s
	$(AS) -c $(ASFLAGS) $< -o $@

%.exe: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $@

.PHONY: gcov
gcov:
	$(COV) -u -f -b -o $(CHIBIOS)/os/rt/src $(KERNSRC)

clean:
	-rm -f $(OBJS)
	-rm -f $(PROJECT).exe
	-rm -f $(PROJECT).map
	-rm -f $(SRC:.c=.c.bak)
	-rm -f $(SRC:.c=.lst)
	-rm -f $(SRC:.c=.gcno)
	-rm -f $(SRC:.c=.gcda)
	-rm -f $(ASRC:.s=.s.bak)
	-rm -f $(ASRC:.s=.lst)
	-rm -fR .dep

misra:
	@lint-nt -v -w3 $(DEFS) pclint/co-gcc.lnt pclint/au-misra3.lnt pclint/waivers.lnt $(INCDIR) $(KERNSRC)

#
# Include the dependency files, should be the last of the makefile
#
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

# *** EOF ***
