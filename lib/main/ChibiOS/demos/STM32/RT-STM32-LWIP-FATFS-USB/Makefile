##############################################################################
# Multi-project makefile rules
#

all:
	@echo
	@echo === Building for STM32F407-Olimex ==================================
	+@make --no-print-directory -f make/stm32f407_olimex.make all
	@echo ====================================================================
	@echo
	@echo === Building for STM32F746-Discovery ===============================
	+@make --no-print-directory -f make/stm32f746_discovery.make all
	@echo ====================================================================
	@echo
	@echo === Building for STM32F796-Discovery ===============================
	+@make --no-print-directory -f make/stm32f769_discovery.make all
	@echo ====================================================================
	@echo
	@echo === Building for STM32F107-GoldBull ================================
	+@make --no-print-directory -f make/stm32f107_goldbull.make all
	@echo ====================================================================
	@echo

clean:
	@echo
	+@make --no-print-directory -f make/stm32f407_olimex.make clean
	@echo
	+@make --no-print-directory -f make/stm32f746_discovery.make clean
	@echo
	+@make --no-print-directory -f make/stm32f769_discovery.make clean
	@echo
	+@make --no-print-directory -f make/stm32f107_goldbull.make clean
	@echo
#
##############################################################################
