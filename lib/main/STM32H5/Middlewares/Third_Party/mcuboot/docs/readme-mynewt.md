# Running mynewt apps with MCUboot

Due to small differences between Mynewt's bundled bootloader and MCUboot,
when building an app that will be run with MCUboot as the bootloader and
which at the same time requires to use `newtmgr` to manage images, MCUboot
must be added as a new dependency for this app.

First you need to add the repo to your `project.yml`:

```
    project.repositories:
        - mcuboot

    repository.mcuboot:
        type: github
        vers: 0-dev
        user: mcu-tools
        repo: mcuboot
```

Then update your app's `pkg.yml` adding the extra dependency:

```
    pkg.deps:
        - "@mcuboot/boot/bootutil"
```

Also remove any dependency on `boot/bootutil` (mynewt's bundled bootloader)
which might exist.

To configure MCUboot check all the options available in
`boot/mynewt/mcuboot_config/syscfg.yml`.

Also, MCUboot uses a different image header struct as well as slightly
different TLV structure, so images created by `newt` have to be generated
in this new format. That is done by passing the extra parameter `-2` as in:

`newt create-image <target> <version> <pubkey> -2`

# Boot serial functionality with Mynewt

Building with `BOOT_SERIAL: 1` enables some basic management functionality
like listing images and uploading a new image to `slot0`. The serial bootloader
requires that `mtu` is set to a value that is less than or equal to `256`.
This can be done either by editing `~/.newtmgr.cp.json` and setting the `mtu`
for the connection profile, or specifying you connection string manually as in:

```
newtmgr --conntype serial --connstring "dev=/dev/ttyUSB0,mtu=256" image upload -e blinky.img
```

where `/dev/ttyUSB0` is your serial port.
