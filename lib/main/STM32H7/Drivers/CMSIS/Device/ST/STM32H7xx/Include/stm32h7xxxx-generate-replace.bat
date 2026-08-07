@color 0B
@echo off

REM Generate all CMSIS files 
REM Active perl must be installed

set scriptPATH=%CD%\..\..\..\..\..\..\..\__INTERNAL__tools\tools\_CmsisDeviceGenerator\

if not exist "%scriptPATH%"  (
	echo Input directory does not exist!
	pause
	exit
)

cd %scriptPATH%

perl DeviceGeneration.pl  --target STM32H7xx -replace

pause
:EOF
