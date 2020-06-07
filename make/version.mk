FORKNAME		:= inav

FC_VER_MAJOR	:= $(shell grep " FC_VERSION_MAJOR" src/main/build/version.h | awk '{print $$3}' )
FC_VER_MINOR	:= $(shell grep " FC_VERSION_MINOR" src/main/build/version.h | awk '{print $$3}' )
FC_VER_PATCH	:= $(shell grep " FC_VERSION_PATCH" src/main/build/version.h | awk '{print $$3}' )
FC_VER 			:= $(FC_VER_MAJOR).$(FC_VER_MINOR).$(FC_VER_PATCH)

FC_VER_SUFFIX	?=
ifneq ($(FC_VER_SUFFIX),)
	FC_VER += -$(FC_VER_SUFFIX)
endif

REVISION 		:= $(shell git rev-parse --short HEAD)

.PHONY: print_version

print_version:
	@echo $(FC_VER)
