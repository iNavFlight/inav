CC = gcc
AR=ar cr
DEFINES = -DTX_LINUX_DEBUG_ENABLE -D_GNU_SOURCE -DTX_LINUX_MULTI_CORE -DTX_ENABLE_EVENT_TRACE
ifdef NO_IDLE
DEFINES += -DTX_LINUX_NO_IDLE_ENABLE
TITLE = "No idle"
else
TITLE = "TX"
endif
ifdef ARCH64
TITLE+=":64"
else
TITLE+=":32"
ARCH = -m32
endif
COMMON_PATH=$(DIR)/../../../../common
INCLUDES = -I$(COMMON_PATH)/inc -I$(DIR)/../inc
CFLAGS = -g3 $(ARCH) -g3 -fPIC -gdwarf-2 -std=c99 $(DEFINES) $(INCLUDES)
LINK = gcc $(ARCH)
LIBS = -lpthread -lrt
FILE_LIST = file_list.mk
DEPEND_LIST = $(OUTPUT_FOLDER)/*.d
OUTPUT_FOLDER= .tmp
DIR=$(shell pwd)

include $(FILE_LIST)

all: $(OUTPUT_FOLDER) sample_threadx tx.so
	echo $(TITLE) build done

$(OUTPUT_FOLDER):
	mkdir -p $@
	mkdir -p $@/generic/

sample_threadx: $(OUTPUT_FOLDER)/sample_threadx.o tx.a
	echo LD $@
	$(LINK) -o $@ $^ $(LIBS) 

tx.a: $(OUTPUT_FOLDER) $(LINUX_OBJS) $(GENERIC_OBJS)
	echo AR $@
	$(AR) $@ $(LINUX_OBJS) $(GENERIC_OBJS)

tx.so: $(OUTPUT_FOLDER) $(LINUX_OBJS) $(GENERIC_OBJS)
	cc -fPIC -shared -Wl,-soname,$@ -o $@ $(LINUX_OBJS) $(GENERIC_OBJS) $(LIBS) $(ARCH)

$(OUTPUT_FOLDER)/sample_threadx.o: sample_threadx.c $(DIR)/Makefile
	filename=`basename $<`; \
	echo CC $$filename; \
	$(CC) $(CFLAGS) -MT $@ -MD -MP -MF $(OUTPUT_FOLDER)/$$filename.d -c -o $@ $<

$(OUTPUT_FOLDER)/%.o: ../src/%.c $(DIR)/Makefile
	filename=`basename $<`; \
	echo CC $$filename; \
	$(CC) $(CFLAGS) -MT $@ -MD -MP -MF $(OUTPUT_FOLDER)/$$filename.d -c -o $@ $<

$(OUTPUT_FOLDER)/generic/%.o: $(COMMON_PATH)/src/%.c $(DIR)/Makefile
	filename=`basename $<`; \
	echo CC $$filename; \
	$(CC) $(CFLAGS) -MT $@ -MD -MP -MF $(OUTPUT_FOLDER)/$$filename.d -c -o $@ $<

-include $(DEPEND_LIST)

.SILENT:
.PHONY: files clean
files:
	@echo "LINUX_SRCS = \\" > $(FILE_LIST);
	-@for file in *.c; \
	do \
		filename=`basename $$file`; \
		[ "$$file" == "sample_threadx.c" ] || echo "$$filename \\" >> $(FILE_LIST); \
	done; 
	@printf "\n" >> $(FILE_LIST);
	@echo 'LINUX_OBJS = $$(LINUX_SRCS:%.c=$(OUTPUT_FOLDER)/%.o)' >> $(FILE_LIST);
	@printf "\n\n" >> $(FILE_LIST);
	@echo "GENERIC_SRCS = \\" >> $(FILE_LIST);
	-@for file in $(COMMON_PATH)/src/*.c; \
	do \
		filename=`basename $$file`; \
		[ "$$file" == "sample_threadx.c" ] || echo "$$filename \\" >> $(FILE_LIST); \
	done; 
	@printf "\n" >> $(FILE_LIST);
	@echo 'GENERIC_OBJS = $$(GENERIC_SRCS:%.c=$(OUTPUT_FOLDER)/generic/%.o)' >> $(FILE_LIST);

clean:
	-rm -f -r $(OUTPUT_FOLDER) tx.a sample_threadx tx.so
