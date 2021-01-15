##############################################################################
# Multi-project makefile rules
#

all:
	@echo
	@echo === Building for STM32L4R9-Discovery ===============================
	+@make --no-print-directory -f ./make/stm32l4r9_discovery.make all
	@echo
	@echo === Building for STM32L476-Discovery ===============================
	+@make --no-print-directory -f ./make/stm32l476_discovery.make all
	@echo ====================================================================
	@echo

clean:
	@echo
	+@make --no-print-directory -f ./make/stm32l4r9_discovery.make clean
	@echo
	+@make --no-print-directory -f ./make/stm32l476_discovery.make clean
	@echo

#
##############################################################################
