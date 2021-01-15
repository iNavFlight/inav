##############################################################################
# Multi-project makefile rules
#

all:
	@echo
	@echo === Building for STM32L476-Discovery ===============================
	+@make --no-print-directory -f ./make/stm32l476_discovery.make all
	@echo ====================================================================
	@echo
	@echo === Building for STM32L4R5ZI-Nucleo144 =============================
	+@make --no-print-directory -f ./make/stm32l4r5zi_nucleo144.make all
	@echo ====================================================================
	@echo

clean:
	@echo
	+@make --no-print-directory -f ./make/stm32l476_discovery.make clean
	@echo
	+@make --no-print-directory -f ./make/stm32l4r5zi_nucleo144.make clean
	@echo

#
##############################################################################
