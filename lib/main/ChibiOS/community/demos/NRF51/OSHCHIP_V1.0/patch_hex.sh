#!/bin/bash

# The official [OSHChip CMSIS-DAP programmer] provides also
# a MBED-type Mass Storage flashing mechanism - copying
# a hex file to the external "drive" flashes the firmware
# to the attached OSHChip.

# However this MSD flasher expect the hex file to begin with the
# "Extended Linear Address" (e.g. ":020000040000FA"); this happens
# automatically if the firmware is >= 64kB, but MBED online compiler
# does this always. ARM GCC suite does not. Hence this script.

# [OSHChip CMSIS-DAP programmer]: http://oshchip.org/products/OSHChip_CMSIS_DAP_V1.0_Product.html

DIR=build
FILEBASE=ch
FILEEXT=hex
FILE="${FILEBASE}.${FILEEXT}"

if [ ! -f "${DIR}/${FILE}" ]; then
  echo "Build the firmware first."
  exit 1
fi

if $(head "${DIR}/${FILE}" | grep -q ":02000004") ; then
	echo "The format of ${DIR}/${FILE} is already good."
	exit 2
fi

echo ":020000040000FA" > "${DIR}/${FILEBASE}_patched.hex"
cat "${DIR}/${FILE}" >> "${DIR}/${FILEBASE}_patched.hex"
