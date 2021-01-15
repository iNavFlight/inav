##############################################################################
# Multi-project makefile rules
#

all:
	@echo
	@echo === Building for STM32F091-Nucleo64 ================================
	@make --no-print-directory -f ./make/stm32f091_nucleo64.make all
	@echo ====================================================================
	@echo
	@echo === Building for STM32F303-Discovery ===============================
	@make --no-print-directory -f ./make/stm32f303_discovery.make all
	@echo ====================================================================
	@echo
	@echo === Building for STM32F407-Discovery ===============================
	@make --no-print-directory -f ./make/stm32f407_discovery.make all
	@echo ====================================================================
	@echo
	@echo === Building for STM32F746-Discovery ===============================
	@make --no-print-directory -f ./make/stm32f746_discovery.make all
	@echo ====================================================================
	@echo

clean:
	@echo
	-@make --no-print-directory -f ./make/stm32f091_nucleo64.make clean
	@echo
	-@make --no-print-directory -f ./make/stm32f303_discovery.make clean
	@echo
	-@make --no-print-directory -f ./make/stm32f407_discovery.make clean
	@echo
	-@make --no-print-directory -f ./make/stm32f746_discovery.make clean
	@echo

#
##############################################################################
