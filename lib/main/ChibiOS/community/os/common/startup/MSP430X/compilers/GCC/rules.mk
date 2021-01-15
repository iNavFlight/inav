# ARM Cortex-Mx common makefile scripts and rules.

##############################################################################
# Processing options coming from the upper Makefile.
#

# Compiler options
OPT = $(USE_OPT)
COPT = $(USE_COPT)
CPPOPT = $(USE_CPPOPT)

# Garbage collection
ifeq ($(USE_LINK_GC),yes)
  OPT += -ffunction-sections -fdata-sections -fno-common
  LDOPT := ,--gc-sections
else
  LDOPT :=
endif

# Linker extra options
ifneq ($(USE_LDOPT),)
  LDOPT := $(LDOPT),$(USE_LDOPT)
endif

# Link time optimizations
ifeq ($(USE_LTO),yes)
  OPT += -flto
endif

# HWMULT-related options
ifeq ($(USE_HWMULT),)
  USE_HWMULT = none
endif
ifneq ($(USE_HWMULT),none)
  OPT += -mhwmult=$(USE_HWMULT)
endif

# Idle thread stack size
ifeq ($(USE_IDLE_STACKSIZE),)
  LDOPT := $(LDOPT),--defsym=__idle_stack_size__=0x40
else
  LDOPT := $(LDOPT),--defsym=__idle_stack_size__=$(USE_IDLE_STACKSIZE)
endif

# Output directory and files
ifeq ($(BUILDDIR),)
  BUILDDIR = build
endif
ifeq ($(BUILDDIR),.)
  BUILDDIR = build
endif
OUTFILES = $(BUILDDIR)/$(PROJECT).elf \
           $(BUILDDIR)/$(PROJECT).hex \
           $(BUILDDIR)/$(PROJECT).bin \
           $(BUILDDIR)/$(PROJECT).dmp \
           $(BUILDDIR)/$(PROJECT).list

ifdef SREC
  OUTFILES += $(BUILDDIR)/$(PROJECT).srec
endif

# Source files groups and paths
ACSRC = $(CSRC)
ACPPSRC = $(CPPSRC)
ASRC    = $(CSRC)$(CPPSRC)
SRCPATHS  = $(sort $(dir $(ASMXSRC)) $(dir $(ASMSRC)) $(dir $(ASRC)) $(dir $(TSRC)))

# Various directories
OBJDIR    = $(BUILDDIR)/obj
LSTDIR    = $(BUILDDIR)/lst

# Object files groups
ACOBJS    = $(addprefix $(OBJDIR)/, $(notdir $(ACSRC:.c=.o)))
ACPPOBJS  = $(addprefix $(OBJDIR)/, $(notdir $(ACPPSRC:.cpp=.o)))
ASMOBJS   = $(addprefix $(OBJDIR)/, $(notdir $(ASMSRC:.s=.o)))
ASMXOBJS  = $(addprefix $(OBJDIR)/, $(notdir $(ASMXSRC:.S=.o)))
OBJS    = $(ASMXOBJS) $(ASMOBJS) $(ACOBJS) $(TCOBJS) $(ACPPOBJS) $(TCPPOBJS)

# Paths
IINCDIR   = $(patsubst %,-I%,$(INCDIR) $(DINCDIR) $(UINCDIR))
LLIBDIR   = $(patsubst %,-L%,$(DLIBDIR) $(ULIBDIR))

# Macros
DEFS      = $(DDEFS) $(UDEFS)
ADEFS     = $(DADEFS) $(UADEFS)

# Libs
LIBS      = $(DLIBS) $(ULIBS)

# Various settings
MCFLAGS   = -mmcu=$(MCU) $(MOPT)
ODFLAGS    = -x --syms
ASFLAGS   = $(MCFLAGS) -Wa,-amhls=$(LSTDIR)/$(notdir $(<:.s=.lst)) $(ADEFS)
ASXFLAGS  = $(MCFLAGS) -Wa,-amhls=$(LSTDIR)/$(notdir $(<:.S=.lst)) $(ADEFS)
CFLAGS    = $(MCFLAGS) $(OPT) $(COPT) $(CWARN) -Wa,-alms=$(LSTDIR)/$(notdir $(<:.c=.lst)) $(DEFS)
CPPFLAGS  = $(MCFLAGS) $(OPT) $(CPPOPT) $(CPPWARN) -Wa,-alms=$(LSTDIR)/$(notdir $(<:.cpp=.lst)) $(DEFS)
LDFLAGS   =  $(MCFLAGS) $(OPT) -minrt $(LLIBDIR) -Wl,-Map=$(BUILDDIR)/$(PROJECT).map,--cref,--no-warn-mismatch,--library-path=$(RULESPATH)/ld,--script=$(LDSCRIPT)$(LDOPT)

# Temporary specfile to deal with messed-up msp430-elf default spec file
SPECFILE  := $(shell mktemp -u) 

# Generate dependency information
ASFLAGS  += -MD -MP -MF .dep/$(@F).d
CFLAGS   += -MD -MP -MF .dep/$(@F).d
CPPFLAGS += -MD -MP -MF .dep/$(@F).d

# Paths where to search for sources
VPATH     = $(SRCPATHS)

#
# Makefile rules
#

all: PRE_MAKE_ALL_RULE_HOOK $(OBJS) $(OUTFILES) POST_MAKE_ALL_RULE_HOOK

PRE_MAKE_ALL_RULE_HOOK:

POST_MAKE_ALL_RULE_HOOK:

$(OBJS): | $(BUILDDIR) $(OBJDIR) $(LSTDIR)

$(BUILDDIR):
ifneq ($(USE_VERBOSE_COMPILE),yes)
	@echo Compiler Options
	@echo $(CC) -c $(CFLAGS) -I. $(IINCDIR) main.c -o main.o
	@echo
endif
	@mkdir -p $(BUILDDIR)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(LSTDIR):
	@mkdir -p $(LSTDIR)

$(ACPPOBJS) : $(OBJDIR)/%.o : %.cpp Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CPPC) -c $(CPPFLAGS) $(AOPT) -I. $(IINCDIR) $< -o $@
else
	@echo Compiling $(<F)
	@$(CPPC) -c $(CPPFLAGS) $(AOPT) -I. $(IINCDIR) $< -o $@
endif

$(TCPPOBJS) : $(OBJDIR)/%.o : %.cpp Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CPPC) -c $(CPPFLAGS) $(TOPT) -I. $(IINCDIR) $< -o $@
else
	@echo Compiling $(<F)
	@$(CPPC) -c $(CPPFLAGS) $(TOPT) -I. $(IINCDIR) $< -o $@
endif

$(ACOBJS) : $(OBJDIR)/%.o : %.c Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CC) -c $(CFLAGS) $(AOPT) -I. $(IINCDIR) $< -o $@
else
	@echo Compiling $(<F)
	@$(CC) -c $(CFLAGS) $(AOPT) -I. $(IINCDIR) $< -o $@
endif

$(TCOBJS) : $(OBJDIR)/%.o : %.c Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CC) -c $(CFLAGS) $(TOPT) -I. $(IINCDIR) $< -o $@
else
	@echo Compiling $(<F)
	@$(CC) -c $(CFLAGS) $(TOPT) -I. $(IINCDIR) $< -o $@
endif

$(ASMOBJS) : $(OBJDIR)/%.o : %.s Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(AS) -c $(ASFLAGS) -I. $(IINCDIR) $< -o $@
else
	@echo Compiling $(<F)
	@$(AS) -c $(ASFLAGS) -I. $(IINCDIR) $< -o $@
endif

$(ASMXOBJS) : $(OBJDIR)/%.o : %.S Makefile
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(CC) -c $(ASXFLAGS) $(TOPT) -I. $(IINCDIR) $< -o $@
else
	@echo Compiling $(<F)
	@$(CC) -c $(ASXFLAGS) $(TOPT) -I. $(IINCDIR) $< -o $@
endif

# This is gcc-specific - if LD isn't gcc it will fail
$(SPECFILE) : 
	$(LD) -dumpspecs > $(SPECFILE)
	sed -i 's/%{!T.*}//' $(SPECFILE)

$(BUILDDIR)/$(PROJECT).elf: $(OBJS) $(LDSCRIPT) $(SPECFILE)
ifeq ($(USE_VERBOSE_COMPILE),yes)
	@echo
	$(LD) $(LDFLAGS) -specs=$(SPECFILE) $(OBJS) $(LIBS) -o $@
else
	@echo Linking $@
	@$(LD) $(LDFLAGS) -specs=$(SPECFILE) $(OBJS) $(LIBS) -o $@
endif

%.hex: %.elf
ifeq ($(USE_VERBOSE_COMPILE),yes)
	$(HEX) $< $@
else
	@echo Creating $@
	@$(HEX) $< $@
endif

%.bin: %.elf
ifeq ($(USE_VERBOSE_COMPILE),yes)
	$(BIN) $< $@
else
	@echo Creating $@
	@$(BIN) $< $@
endif

%.srec: %.elf
ifdef SREC
	ifeq ($(USE_VERBOSE_COMPILE),yes)
		$(SREC) $< $@
  else
		@echo Creating $@
		@$(SREC) $< $@
	endif
endif

%.dmp: %.elf
ifeq ($(USE_VERBOSE_COMPILE),yes)
	$(OD) $(ODFLAGS) $< > $@
	$(SZ) $<
else
	@echo Creating $@
	@$(OD) $(ODFLAGS) $< > $@
	@echo
	@$(SZ) $<
endif

%.list: %.elf
ifeq ($(USE_VERBOSE_COMPILE),yes)
	$(OD) -S $< > $@
else
	@echo Creating $@
	@$(OD) -S $< > $@
	@echo
	@echo Done
endif

lib: $(OBJS) $(BUILDDIR)/lib$(PROJECT).a

$(BUILDDIR)/lib$(PROJECT).a: $(OBJS)
	@$(AR) -r $@ $^
	@echo
	@echo Done

clean:
	@echo Cleaning
	-rm -fR .dep $(BUILDDIR)
	@echo
	@echo Done

#
# Include the dependency files, should be the last of the makefile
#
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

# *** EOF ***
