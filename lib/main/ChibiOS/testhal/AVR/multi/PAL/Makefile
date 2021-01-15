##############################################################################
# Multi-project makefile rules
#

all:
	@echo
	@echo === Building for Arduino-Mega ======================================
	@make --no-print-directory -f make/Makefile-arduino_mega.make all
	@echo ====================================================================
	@echo
	@echo === Building for Arduino-Mini ======================================
	@make --no-print-directory -f make/Makefile-arduino_mini.make all
	@echo ====================================================================
	@echo
	@echo === Building for Arduino-Nano ======================================
	@make --no-print-directory -f make/Makefile-arduino_nano.make all
	@echo ====================================================================
	@echo
	@echo === Building for Arduino-Uno =======================================
	@make --no-print-directory -f make/Makefile-arduino_uno.make all
	@echo ====================================================================
	@echo

clean:
	@echo
	-@make --no-print-directory -f make/Makefile-arduino_mega.make clean
	@echo
	-@make --no-print-directory -f make/Makefile-arduino_mini.make clean
	@echo
	-@make --no-print-directory -f make/Makefile-arduino_nano.make clean
	@echo
	-@make --no-print-directory -f make/Makefile-arduino_uno.make clean
	@echo

#
##############################################################################
