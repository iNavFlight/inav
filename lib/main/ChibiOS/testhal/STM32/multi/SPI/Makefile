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
	@echo === Building for STM32H743-Nucleo144 ===============================
	@make --no-print-directory -f ./make/stm32h743_nucleo144.make all
	@echo ====================================================================
	@echo
	@echo === Building for STM32L476-Discovery ===============================
	@make --no-print-directory -f ./make/stm32l476_discovery.make all
	@echo ====================================================================
	@echo
	@echo === Building for STM32L4R5-Nucleo144 ===============================
	@make --no-print-directory -f ./make/stm32l4r5_nucleo144.make all
	@echo ====================================================================
	@echo
	@echo === Building for STM32L4R9-Discovery ===============================
	@make --no-print-directory -f ./make/stm32l4r9_discovery.make all
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
	-@make --no-print-directory -f ./make/stm32h743_nucleo144.make clean
	@echo
	-@make --no-print-directory -f ./make/stm32l476_discovery.make clean
	@echo
	-@make --no-print-directory -f ./make/stm32l4r5_nucleo144.make clean
	@echo
	-@make --no-print-directory -f ./make/stm32l4r9_discovery.make clean
	@echo

#
##############################################################################
