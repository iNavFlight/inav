# Building and using MCUboot with Zephyr

MCUboot began its life as the bootloader for Mynewt.  It has since
acquired the ability to be used as a bootloader for Zephyr as well.
There are some pretty significant differences in how apps are built
for Zephyr, and these are documented here.

Please see the [design document](design.md) for documentation on the design
and operation of the bootloader itself. This functionality should be the same
on all supported RTOSs.

The first step required for Zephyr is making sure your board has flash
partitions defined in its device tree. These partitions are:

- `boot_partition`: for MCUboot itself
- `image_0_primary_partition`: the primary slot of Image 0
- `image_0_secondary_partition`: the secondary slot of Image 0
- `scratch_partition`: the scratch slot

Currently, the two image slots must be contiguous. If you are running
MCUboot as your stage 1 bootloader, `boot_partition` must be configured
so your SoC runs it out of reset. If there are multiple updateable images
then the corresponding primary and secondary partitions must be defined for
the rest of the images too (e.g. `image_1_primary_partition` and
`image_1_secondary_partition` for Image 1).

The flash partitions are typically defined in the Zephyr boards folder, in a
file named `boards/<arch>/<board>/<board>.dts`. An example `.dts` file with
flash partitions defined is the frdm_k64f's in
`boards/arm/frdm_k64f/frdm_k64f.dts`. Make sure the labels in your board's
`.dts` file match the ones used there.

## Installing Requirements and Dependencies

Install additional packages required for development with mcuboot:

```
  cd ~/mcuboot  # or to your directory where mcuboot is cloned
  pip3 install --user -r scripts/requirements.txt
```

## Building the bootloader itself

The bootloader is an ordinary Zephyr application, at least from
Zephyr's point of view.  There is a bit of configuration that needs to
be made before building it.  Most of this can be done as documented in
the `CMakeLists.txt` file in boot/zephyr.  There are comments there for
guidance.  It is important to select a signature algorithm, and decide
if the primary slot should be validated on every boot.

To build MCUboot, create a build directory in boot/zephyr, and build
it as usual:

```
  cd boot/zephyr
  mkdir build && cd build
  cmake -GNinja -DBOARD=<board> ..
  ninja
```

In addition to the partitions defined in DTS, some additional
information about the flash layout is currently required to build
MCUboot itself. All the needed configuration is collected in
`boot/zephyr/include/target.h`. Depending on the board, this information
may come from board-specific headers, Device Tree, or be configured by
MCUboot on a per-SoC family basis.

After building the bootloader, the binaries should reside in
`build/zephyr/zephyr.{bin,hex,elf}`, where `build` is the build
directory you chose when running `cmake`. Use the Zephyr build
system `flash` target to flash these binaries, usually by running
`make flash` (or `ninja flash`, etc.) from the build directory. Depending
on the target and flash tool used, this might erase the whole of the flash
memory (mass erase) or only the sectors where the boot loader resides prior to
programming the bootloader image itself.

## Building Applications for the bootloader

In addition to flash partitions in DTS, some additional configuration
is required to build applications for MCUboot.

This is handled internally by the Zephyr configuration system and is wrapped
in the `CONFIG_BOOTLOADER_MCUBOOT` Kconfig variable, which must be enabled in
the application's `prj.conf` file.

The directory `samples/zephyr/hello-world` in the MCUboot tree contains
a simple application with everything you need. You can try it on your
board and then just make a copy of it to get started on your own
application; see samples/zephyr/README.md for a tutorial.

The Zephyr `CONFIG_BOOTLOADER_MCUBOOT` configuration option
[documentation](http://docs.zephyrproject.org/reference/kconfig/CONFIG_BOOTLOADER_MCUBOOT.html)
provides additional details regarding the changes it makes to the image
placement and generation in order for an application to be bootable by
MCUboot.

With this, build the application as your normally would.

### Signing the application

In order to upgrade to an image (or even boot it, if
`MCUBOOT_VALIDATE_PRIMARY_SLOT` is enabled), the images must be signed.
To make development easier, MCUboot is distributed with some example
keys.  It is important to stress that these should never be used for
production, since the private key is publicly available in this
repository.  See below on how to make your own signatures.

Images can be signed with the `scripts/imgtool.py` script.  It is best
to look at `samples/zephyr/Makefile` for examples on how to use this.

### Flashing the application

The application itself can flashed with regular flash tools, but will
need to be programmed at the offset of the primary slot for this particular
target. Depending on the platform and flash tool you might need to manually
specify a flash offset corresponding to the primary slot starting address. This
is usually not relevant for flash tools that use Intel Hex images (.hex) instead
of raw binary images (.bin) since the former include destination address
information. Additionally you will need to make sure that the flash tool does
not perform a mass erase (erasing the whole of the flash) or else you would be
deleting MCUboot.
These images can also be marked for upgrade, and loaded into the secondary slot,
at which point the bootloader should perform an upgrade.  It is up to
the image to mark the primary slot as "image ok" before the next reboot,
otherwise the bootloader will revert the application.

## Managing signing keys

The signing keys used by MCUboot are represented in standard formats,
and can be generated and processed using conventional tools.  However,
`scripts/imgtool.py` is able to generate key pairs in all of the
supported formats.  See [the docs](imgtool.md) for more details on
this tool.

### Generating a new keypair

Generating a keypair with imgtool is a matter of running the keygen
subcommand:

```
    $ ./scripts/imgtool.py keygen -k mykey.pem -t rsa-2048
```

The argument to `-t` should be the desired key type.  See the
[the docs](imgtool.md) for more details on the possible key types.

### Extracting the public key

The generated keypair above contains both the public and the private
key.  It is necessary to extract the public key and insert it into the
bootloader.  The keys live in `boot/zephyr/keys.c`, and can be
extracted using imgtool:

```
    $ ./scripts/imgtool.py getpub -k mykey.pem
```

This will output the public key as a C array that can be dropped
directly into the `keys.c` file.

Once this is done, this new keypair file (`mykey.pem` in this
example) can be used to sign images.
