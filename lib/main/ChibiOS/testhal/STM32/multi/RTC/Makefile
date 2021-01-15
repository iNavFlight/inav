##############################################################################
# Multi-project makefile rules
#

all:
	@echo
	@echo === Building for STM32F051-Discovery ===============================
	+@make --no-print-directory -f ./make/stm32f051_discovery.make all
	@echo ====================================================================
	@echo
	@echo === Building for STM32F303-Discovery ===============================
	+@make --no-print-directory -f ./make/stm32f303_discovery.make all
	@echo ====================================================================
	@echo
	@echo === Building for STM32F407-Discovery ===============================
	+@make --no-print-directory -f ./make/stm32f407_discovery.make all
	@echo ====================================================================
	@echo
	@echo === Building for STM32F746-Discovery ===============================
	+@make --no-print-directory -f ./make/stm32f746_discovery.make all
	@echo ====================================================================
	@echo
	@echo === Building for STM32L053-Discovery ===============================
	+@make --no-print-directory -f ./make/stm32l053_discovery.make all
	@echo ====================================================================
	@echo
	@echo === Building for STM32L476-Discovery ===============================
	+@make --no-print-directory -f ./make/stm32l476_discovery.make all
	@echo ====================================================================
	@echo
	@echo === Building for STM32L4R5-Nucleo144 ===============================
	+@make --no-print-directory -f ./make/stm32l4r5_nucleo144.make all
	@echo ====================================================================
	@echo

clean:
	@echo
	+@make --no-print-directory -f ./make/stm32f051_discovery.make clean
	@echo
	+@make --no-print-directory -f ./make/stm32f303_discovery.make clean
	@echo
	+@make --no-print-directory -f ./make/stm32f407_discovery.make clean
	@echo
	+@make --no-print-directory -f ./make/stm32f746_discovery.make clean
	@echo
	+@make --no-print-directory -f ./make/stm32l053_discovery.make clean
	@echo
	+@make --no-print-directory -f ./make/stm32l476_discovery.make clean
	@echo
	+@make --no-print-directory -f ./make/stm32l4r5_nucleo144.make clean
	@echo

#
##############################################################################
