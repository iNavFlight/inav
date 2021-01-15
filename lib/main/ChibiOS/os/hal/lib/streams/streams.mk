# RT Shell files.
STREAMSSRC = $(CHIBIOS)/os/hal/lib/streams/chprintf.c \
             $(CHIBIOS)/os/hal/lib/streams/memstreams.c \
             $(CHIBIOS)/os/hal/lib/streams/nullstreams.c

STREAMSINC = $(CHIBIOS)/os/hal/lib/streams

# Shared variables
ALLCSRC += $(STREAMSSRC)
ALLINC  += $(STREAMSINC)