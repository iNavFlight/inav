#!/bin/bash

if [ -z `which blhost` ]; then
	echo "You'll need to get the 'blhost' utility from Freescale."
	echo 'http://www.freescale.com/products/arm-processors/kinetis-cortex-m/kinetis-symbols-footprints-and-models/kinetis-bootloader:KBOOT'
	exit 1
fi

if [ ! -f build/ch.bin ]; then
	echo "Perhaps you should compile the firmware first."
	exit 2
fi

if [[ `blhost -u -- get-property 1` == *"cannot open USB HID device"* ]]; then
	echo "Perhaps you should put the device in the bootloader mode first."
	exit 3
fi

echo "-> Erasing flash..."
blhost -u -- flash-erase-all

echo "-> Flashing firmware..."
blhost -u -- write-memory 0 build/ch.bin

echo "-> Resetting MCU (allow 5 seconds for the firmware to start)..."
blhost -u -- reset
