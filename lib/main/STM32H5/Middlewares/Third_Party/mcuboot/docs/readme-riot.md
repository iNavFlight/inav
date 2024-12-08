# Building and using MCUboot with RIOT

MCUboot began its life as the bootloader for Mynewt.  It has since
acquired the ability to be used as a bootloader for RIOT as well.
Currently the support is limited to the nrf52dk platform.

## Building the bootloader itself

In this first version, a prebuilt Mynewt binary is downloaded at
compile time.  This binary was compiled to do an integrity check, but
not a signature check. In order to configure the bootloader for
signature check it is necessary to re-compile it either with Mynewt
or Zephyr, following the provided instructions.

In the next version, it is planned to compile MCUboot using RIOT,
which should be able to boot any of the supported OS images.

## Building Applications for the bootloader

A compatible MCUboot image can be compiled by typing: `make mcuboot`.

The only variable which needs to be set is `IMAGE_VERSION` loaded
with a valid formatted value. The format is `major.minor.patch+other`
(e.g. `export IMAGE_VERSION= 1.1.1+1`. This variable can be either
exported in the Makefile or manually, prior to the compilation process.

The goal is to produce an ELF file which is linked to be flashed at a
`BOOTLOADER_OFFSET` offset rather than the beginning of ROM.  MCUboot
also expects an image padded with some specific headers containing the
version information, and trailer type-length-value records (TLVs) with
hash and signing information. This is done through the imgtool.py
application, which is executed automatically by the RIOT build system.

### Signing the application

The application will be automatically signed with the provided key.
If no key is provided, a new key will be automatically generated. The
default key type is RSA-2048.

In order to use your provided key, you need to recompile the bootloader
using you public key, either in Zephyr or Mynewt by following the
provided procedure for the selected OS.

### Flashing the application

The application can be flashed by typing: `make flash-mcuboot`.
This will flash both the bootloader and the application.
