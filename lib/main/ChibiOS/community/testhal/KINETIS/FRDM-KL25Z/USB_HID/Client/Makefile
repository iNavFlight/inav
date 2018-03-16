CC=gcc
CFLAGS = -Wall -Wextra -O2 -g

SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=%.o)
EXE = test-usb-hid

all: $(EXE)

$(EXE): $(OBJS)

-include $(subst .c,.d,$(SRCS))

%.d: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -MM -MF $@ -MP -MT $(subst .c,.o,$<) $<

clean:
	rm -f $(EXE)
	rm -f $(OBJS)
	rm -f $(subst .c,.d,$(SRCS))
	rm -f *~

.PHONY: clean all
