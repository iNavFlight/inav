# Building in Fedora
Fedora's built-in CodeSourcery vesrion of ARM tools produces broken builds, so you have to install toolchanin from Atmel (now Microchip): https://www.microchip.com/avr-support/avr-and-arm-toolchains-(c-compilers)

These steps was tested on Fedora 25 and 6.3.1 compiler version.
## Remove built-in toolchain
```
su dnf remove arm-none-eabi-gcc arm-none-eabi-newlib
```
## Download AVR toolchain
Go to https://www.microchip.com/avr-support/avr-and-arm-toolchains-(c-compilers) and download appropriate tarbal (64-bit only).
Extract somewhere.
## Install Ruby and json gem
```
su dnf install ruby
su gem install json
```
## Create local clone
```
cd <your_work_dir>
git clone https://github.com/iNavFlight/inav.git
cd inav
```
## Build away
For build options run
```
make help
```
```
export PATH=$PATH:<path_to_extracted_toolchain>/arm-none-eabi/bin
make <your_board>
```
