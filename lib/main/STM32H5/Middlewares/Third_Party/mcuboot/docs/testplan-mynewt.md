## mcuboot test plan

The current target for running the tests is the Freedom K64F board.

### Basic sign support (RSA/EC/EC256)

For each supported signing algorithm, check that non-signed, and signed
with wrong key images are not swapped to, and image signed with correct key
is swapped to.

For the 3 algorithms supported, rsa, ec and ec256, two files are provided:
key_<sign-algo>.pem, key_<sign-algo>_2.pem. And a keys file with the C public
key data for key_<sign-algo>.pem.

Build and load mcuboot:

* `newt build k64f_boot_<sign-algo>`
* `newt load k64f_boot_<sign-algo>`

Build and load good image in slot 0:

* `newt create-image k64f_blinky 1.0.1 key_<sign-algo>.pem`
* `newt load k64f_blinky`

NOTE: If testing RSA/PSS `newt create-image` needs to be passed in the extra
flag `--rsa-pss` eg:

`newt create-image k64f_blinky 1.0.1 key_rsa.pem --rsa-pss`

Build and load image in slot 1 with no signing, signed with
key_<sign-algo>_2.pem and signed with key_<sign-algo>.pem. Mark each one as
test image and check that swap only happens for image signed with
key_<sign-algo>.pem. Both others should be erased.

* `newt create-image k64f_blinky2 1.0.2 <one-of-the-sign-keys-or-none>`
* `newtmgr image upload k64f_blinky2`
* `newtmgr image list`
* `newtmgr image test <hash of slot 1>`

### Image signed with more than one key

FIXME: this is currently not functional, skip this section!

Build and load mcuboot:

* `newt build k64f_boot_rsa_ec`
* `newt load k64f_boot_rsa_ec`

Build and load good image in slot 0:

* `newt create-image k64f_blinky 1.0.1 key_rsa.pem`
* `newt load k64f_blinky`

Build and load image in slot 1 with no signing, signed with
key_<sign-algo>_2.pem and signed with key_<sign-algo>.pem. Mark each one as
test image and check that swap only happens for image signed with
key_<sign-algo>.pem. Both others should be erased.

Use all of this options:

* `newt create-image k64f_blinky2 1.0.2`

And load

* `newtmgr image upload k64f_blinky2`
* `newtmgr image list`
* `newtmgr image test <hash of slot 1>`

### Overwrite only functionality

Build/load mcuboot:

* `newt build k64f_boot_rsa_noswap`
* `newt load k64f_boot_rsa_noswap`

Build/load blinky to slot 0:

* `newt create-image k64f_blinky 1.0.1 key_rsa.pem`
* `newt load k64f_blinky`

Build/load blinky2 both with bad and good key, followed by a permanent swap
request:

* `newt create-image k64f_blinky2 1.0.2 <bad and good rsa keys>.pem`
* `newtmgr image upload k64f_blinky2`
* `newtmgr image list`
* `newtmgr image confirm <hash of slot 1>`

This should not swap and delete the image in slot 1 when signed with the wrong
key, otherwise the image in slot 1 should be *moved* to slot 0 and slot 1 should
be empty.

### Validate slot 0 option

Build/load mcuboot:

* `newt build k64f_boot_rsa_validate0`
* `newt load k64f_boot_rsa_validate0`

Build non-signed image:

* `newt create-image k64f_blinky 1.0.1`
* `newt load k64f_blinky`
* Reset and no image should be run

Build signed image with invalid key:

* `newt create-image k64f_blinky 1.0.1 key_rsa_2.pem`
* `newt load k64f_blinky`
* Reset and no image should be run

Build signed image with *valid* key:

* `newt create-image k64f_blinky 1.0.1 key_rsa.pem`
* `newt load k64f_blinky`
* Reset and image *should* run

### Swap with random failures

DISCLAIMER: be careful with copy/paste of commands, this test uses another
target/app!

Build/load mcuboot:

* `newt build k64f_boot_rsa`
* `newt load k64f_boot_rsa`

Build/load slinky to slot 0:

* `newt create-image k64f_slinky 1.0.1 key_rsa.pem`
* `newt load k64f_slinky`

Build/load slinky2 to slot 1:

* `newt create-image k64f_slinky2 1.0.2 key_rsa.pem`
* `newtmgr image upload k64f_slinky2`

Confirm that both images are installed, request a permanent request to the
image in slot 1 and check that it works.

* `newtmgr image list`
* `newtmgr image confirm <hash of slot 1>`

If everything works, now proceed with requests for permanent swap to the image
in slot 1 and do random swaps (as much as you like!). When the swap finishes
confirm that the swap was finished with the previous slot 1 image now in
slot 0 and vice-versa.

### Help

* Mass erase MCU

        $ pyocd erase --chip

* Flashing image in slot 1:

        $ pyocd flash -e sector -a 0x80000 ${IMG_FILE} bin
