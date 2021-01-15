##############################################################################
# Multi-project makefile rules
#

all:
	@echo
	@echo === Building for TM4C123G LaunchPad ================================
	@make --no-print-directory -f ./make/tm4c123g_launchpad all
	@echo ====================================================================
	@echo
	@echo === Building for TM4C1294 Connected LaunchPad ======================
	@make --no-print-directory -f ./make/tm4c1294_launchpad all
	@echo ====================================================================
	@echo

clean:
	@echo
	-@make --no-print-directory -f ./make/tm4c123g_launchpad clean
	@echo
	-@make --no-print-directory -f ./make/tm4c1294_launchpad clean
	@echo

#
##############################################################################
