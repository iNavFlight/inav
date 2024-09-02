# MCUboot Release Notes

- Table of Contents
{:toc}

## Version 1.7.2

The 1.7.2 release of MCUboot is a minor patch release.  It makes a few
changes to the CI build process, and changes the exporting of the
function `boot_add_data_to_shared_area` so that it can be used within
the TF-M-specific bootloader.

There are no vulnerabilities addressed in this release.

## Version 1.7.1

The 1.7.1 release of MCUboot is a minor patch release that fixes a
vulnerability in the documentation generation, and adds an option for
setting a ROM address in the image header.

- CVE-2020-26247: The vulnerability doesn't appear to affect the usage
  within MCUboot (to generate documentation.  We have updated all
  dependencies in the docs/Gemfile.lock.
- Add an option `--rom-fixed` to imgtool.py to support setting the
  `ROM_FIXED` field in the image header.

## Version 1.7.0

The 1.7.0 release of MCUBoot adds support for the Mbed-OS platform,
Equal slots (direct-xip) upgrade mode, RAM loading upgrade mode,
hardening against hardware level fault injection and timing attacks
and single image mode.
There are bug fixes, and associated imgtool updates as well.

### About this release

- Initial support for the Mbed-OS platform.
- Added possibility to enter deep sleep mode after mcuboot app execution
  for cypress platform.
- Added hardening against hardware level fault injection and timing attacks.
- Introduced Abstract crypto primitives to simplify porting.
- Added RAM-load upgrade mode.
- Renamed single-image mode to single-slot mode.
- Allow larger primary slot in swap-move
- Fixed boostrapping in swap-move mode.
- Fixed issue causing that interrupted swap-move operation might brick device
  if the primary image was padded.
- Abstracting mcuboot crypto functions for cleaner porting
- Droped flash_area_read_is_empty() porting API.
- boot/zephyr: Added watchdog feed on nRF devices.
  See `CONFIG_BOOT_WATCHDOG_FEED` option.
- boot/zephyr: Added patch for turning off cache for Cortex M7 before
  chain-loading.
- boot/zephyr: added option to relocate interrupts to application
- boot/zephyr: clean ARM core configuration only when selected by user
- boot/boot_serial: allow nonaligned last image data chunk
- imgtool: added custom TLV support.
- imgtool: added possibility to set confirm flag for hex files as well.
- imgtool: Print image digest during verify.

### Zephyr-RTOS Compatibility

This release of MCUboot works with the Zephyr "master" at the time of the
release. It was tested as of has 7a3b253ce. This version of MCUboot also
works with the Zephyr v2.4.0, however it is recommended to enable
`CONFIG_MCUBOOT_CLEANUP_ARM_CORE` while using that version.

## Version 1.6.0

The 1.6.0 release of MCUboot adds support for the PSOC6 platform,
X25519 encrypted images, rollback protection, hardware keys, and a
shared boot record to communicate boot attestation information to
later boot stages.  There are bug fixes, and associated imgtool
updates as well.

### About this release

- Initial support for the Cypress PSOC6 plaformt.  This platform
  builds using the Cypress SDK, which has been added as submodules.
- CBOR decoding in serial recovery replaced by code generated from a
  CDDL description.
- Add support for X25519 encrypted images.
- Add rollback protection.  There is support for a HW rollback counter
  (which must be provided as part of the platform), as well as a SW
  solution that protects against some types of rollback.
- Add an optional boot record in shared memory to communicate boot
  attributes to later-run code.
- Add support for hardware keys.
- Various fixes to work with the latest Zephyr version.

### Security issues addressed

- CVE-2020-7595 "xmlStringLenDecodeEntities in parser.c in libxml2
  2.9.10 has an infinite loop in a certain end-of-file situation." Fix
  by updating a dependency in documentation generation.

### Zephyr-RTOS Compatibility

This release of MCUboot works the Zephyr "master" at the time of the
release.  It was tested as of has 1a89ca1238.  When Zephyr v2.3.0 is
released, there will be a possible 1.6.1 or similar release of Zephyr
if needed to address any issues.  There also may be branch releases of
MCUboot specifically for the current version of Zephyr, e.g.
v1.6.0-zephyr-2.2.1.

## Version 1.5.0

The 1.5.0 release of MCUboot adds support for encrypted images using
ECIES with secp256r1 as an Elliptic Curve alternative to RSA-OAEP. A
new swap method was added which allows for upgrades without using a
scratch partition. There are also lots of bug fixes, extra simulator
testing coverage and some imgtool updates.

### About this release

- TLVs were updated to use 16-bit lengths (from previous 8). This
  should work with no changes for little-endian targets, but will
  break compatibility with big-endian targets.
- A benchmark framework was added to Zephyr
- ed25519 signature validation can now build without using mbedTLS
  by relying on a bundled tinycrypt based sha-512 implementation.
- imgtool was updated to correctly detect trailer overruns by image.
- Encrypted image TLVs can be saved in swap metadata during a swap
  upgrade instead of the plain AES key.
- imgtool can dump private keys in C format (getpriv command), which
  can be added as decryption keys. Optionally can remove superfluous
  fields from the ASN1 by passing it `--minimal`.
- Lots of other smaller bugs fixes.
- Added downgrade prevention feature (available when the overwrite-based
  image update strategy is used)

### Known issues

- TLV size change breaks compatibility with big-endian targets.

## Version 1.4.0

The 1.4.0 release of MCUboot primarily adds support for multi-image
booting.  With this release, MCUboot can manage two images that can be
updated independently.  With this, it also supports additions to the
TLV that allow these dependencies to be specified.

Multi-image support adds backward-incompatible changes to the format
of the images: specifically adding support for protected TLV entries.
If multiple images and dependencies are not used, the images will be
compatible with previous releases of MCUboot.

### About this release

- Fixed CVE-2019-5477, and CVE-2019-16892.  These fix issue with
  dependencies used in the generation of the documentation on github.
- Numerous code cleanups and refactorings
- Documentation updates for multi-image features
- Update imgtool.py to support the new features
- Updated the mbed TLS submodule to current stable version 2.16.3
- Moved the mbed TLS submodule from within sim/mcuboot-sys to ext.
  This will make it easier for other board supports to use this code.
- Added some additional overflow and bound checks to data in the image
  header, and TLV data.
- Add a `-x` (or `--hex_addr`) flag to imgtool to set the base address
  written to a hex-format image.  This allows the image to be flashed
  at an offset, without having to use additional tools to modify the
  image.

## Version 1.3.1

The 1.3.1 release of MCUboot consists mostly of small bug fixes and updates.
There are no breaking changes in functionality. This release should work with
Mynewt 1.6.0 and up, and any Zephyr `master` after sha
f51e3c296040f73bca0e8fe1051d5ee63ce18e0d.

### About this release

- Fixed a revert interruption bug
- Added ed25519 signing support
- Added RSA-3072 signing support
- Allow ec256 to run on CC310 interface
- Some preparation work was done to allow for multi image support, which
  should land in 1.4.0. This includes a simulator update for testing
  multi-images, and a new name for slot0/slot1 which are now called
  "primary slot" and "secondary slot".
- Other minor bugfixes and improvements

## Version 1.3.0

The 1.3.0 release of MCUboot brings in many fixes and updates.  There
are no breaking changes in functionality.  Many of the changes are
refactorings that will make the code easier to maintain going forward.
In addition, support has been added for encrypted images.  See [the
docs](encrypted_images.md) for more information.

### About this release

- Modernize the Zephyr build scripts.
- Add a `ptest` utility to help run the simulator in different
  configurations.
- Migrate the simulator to Rust 2018 edition.  The sim now requires at
  least Rust 1.32 to build.
- Simulator cleanups.  The simulator code is now built the same way
  for every configuration, and queries the MCUboot code for how it was
  compiled.
- Abstract logging in MCUboot.  This was needed to support the new
  logging system used in Zephyr.
- Add multiple flash support.  Allows slot1/scratch to be stored in an
  external flash device.
- Add support for [encrypted images](encrypted_images.md).
- Add support for flash devices that read as '0' when erased.
- Add support to Zephyr for the `nrf52840_pca10059`.  This board
  supports serial recovery over USB with CDC ACM.
- imgtool is now also available as a python package on pypi.org.
- Add an option to erase flash pages progressively during recovery to
  avoid possible timeouts (required especially by serial recovery
  using USB with CDC ACM).
- imgtool: big-endian support
- imgtool: saves in intel-hex format when output filename has `.hex`
  extension; otherwise saves in binary format.

## Version 1.2.0

The 1.2.0 release of MCUboot brings a lot of fixes/updates, where much of the
changes were on the boot serial functionality and imgtool utility. There are
no breaking changes in MCUBoot functionality, but some of the CLI parameters
in imgtool were changed (either removed or added or updated).

### About this release

- imgtool accepts .hex formatted input
- Logging system is now configurable
- Most Zephyr configuration has been switched to Kconfig
- Build system accepts .pem files in build system to autogenerate required
  key arrays used internally
- Zephyr build switched to using built-in flash_map and TinyCBOR modules
- Serial boot has substantially decreased in space usage after refactorings
- Serial boot build doesn't require newlib-c anymore on Zephyr
- imgtool updates:
  + "create" subcommand can be used as an alias for "sign"
  + To allow imgtool to always perform the check that firmware does not
    overflow the status area, `--slot-size` was added and `--pad` was updated
    to act as a flag parameter.
  + `--overwrite-only` can be passed if not using swap upgrades
  + `--max-sectors` can be used to adjust the maximum amount of sectors that
    a swap can handle; this value must also be configured for the bootloader
  + `--pad-header` substitutes `--included-header` with reverted semantics,
    so it's not required for firmware built by Zephyr build system

### Known issues

None

## Version 1.1.0

The 1.1.0 release of MCUboot brings a lot of fixes/updates to its
inner workings, specially to its testing infrastructure which now
enables a more thorough quality assurance of many of the available
options. As expected of the 1.x.x release cycle, no breaking changes
were made. From the tooling perpective the main addition is
newt/imgtool support for password protected keys.

### About this release

- serial recovery functionality support under Zephyr
- simulator: lots of refactors were applied, which result in the
  simulator now leveraging the Rust testing infrastructure; testing
  of ecdsa (secp256r1) was added
- imgtool: removed PKCS1.5 support, added support for password
  protected keys
- tinycrypt 0.2.8 and the mbed-tls ASN1 parser are now bundled with
  mcuboot (eg secp256r1 is now free of external dependencies!)
- Overwrite-only mode was updated to erase/copy only sectors that
  actually store firmware
- A lot of small code and documentation fixes and updates.

### Known issues

None

## Version 1.0.0

The 1.0.0 release of MCUboot introduces a format change.  It is
important to either use the `imgtool.py` also from this release, or
pass the `-2` to recent versions of the `newt` tool in order to
generate image headers with the new format.  There should be no
incompatible format changes throughout the 1.x.y release series.

### About this release

- Header format change.  This change was made to move all of the
  information about signatures out of the header and into the TLV
  block appended to the image.  This allows
  - The signature to be replaced without changing the image.
  - Multiple signatures to be applied.  This can be used, for example,
    to sign an image with two algorithms, to support different
    bootloader configurations based on these image.
  - The public key is referred to by its SHA1 hash (or a prefix of the
    hash), instead of an index that has to be maintained with the
    bootloader.
  - Allow new types of signatures in the future.
- Support for PKCS#1 v1.5 signatures has been dropped.  All RSA
  signatures should be made with PSS.  The tools have been changed to
  reflect this.
- The source for Tinycrypt has been placed in the MCUboot tree.  A
  recent version of Tinycrypt introduced breaking API changes.  To
  allow MCUboot to work across various platforms, we stop using the
  Tinycrypt bundled with the OS platform, and use our own version.  A
  future release of MCUboot will update the Tinycrypt version.
- Support for some new targets:
  - Nordic nRF51 and nRF52832 dev kits
  - Hexiwear K64
- Clearer sample applications have been added under `samples`.
- Test plans for [zephyr](testplan-zephyr.md), and
  [mynewt](testplan-mynewt.md).
- The simulator is now able to test RSA signatures.
- There is an unimplemented `load_addr` header for future support for
  RAM loading in the bootloader.
- Numerous documentation.

### Known issues

None

## Version 0.9.0

This is the first release of MCUboot, a secure bootloader for 32-bit MCUs.
It is designed to be operating system-agnostic and works over any transport -
wired or wireless. It is also hardware independent, and relies  on hardware
porting layers from the operating system it works with. For the first release,
we have support for three open source operating systems: Apache Mynewt, Zephyr
and RIOT.

### About this release

- This release supports building with and running Apache Mynewt and Zephyr
  targets.
- RIOT is supported as a running target.
- Image integrity is provided with SHA256.
- Image originator authenticity is provided supporting the following
  signature algorithms:
  - RSA 2048 and RSA PKCS#1 v1.5 or v2.1
  - Elliptic curve DSA with secp224r1 and secp256r1
- Two firmware upgrade algorithms are provided:
  - An overwrite only which upgrades slot 0 with the image in slot 1.
  - A swapping upgrade which enables image test, allowing for rollback to a
    previous known good image.
- Supports both mbed-TLS and tinycrypt as backend crypto libraries. One of them
  must be defined and the chosen signing algorithm will require a particular
  library according to this list:
  - RSA 2048 needs mbed TLS
  - ECDSA secp224r1 needs mbed TLS
  - ECDSA secp256r1 needs tinycrypt as well as the ASN.1 code from mbed TLS
    (so still needs that present).

### Known issues

- The image header and TLV formats are planned to change with release 1.0:
  https://runtimeco.atlassian.net/browse/MCUB-66
