##############################################################################
# Compiler settings
#

TRGT = aarch32-
CC   = clang
CPPC = clang++
LD   = clang
CP   = $(TRGT)objcopy
AS   = $(TRGT)as -x assembler-with-cpp
AR   = $(TRGT)ar
OD   = $(TRGT)objdump
SZ   = $(TRGT)size
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary

#
# Compiler settings
##############################################################################
