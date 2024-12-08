# MCUboot port for Mbed OS

This is an MCUboot port for Mbed OS.

## Using MCUboot

Note: The following is a general overview. It does not cover MCUboot or Mbed OS basics.

See https://github.com/AGlass0fMilk/mbed-mcuboot-demo as a detailed example.

### Basic configurations

To use MCUboot, you need to create an Mbed OS project with the following configurations:
* `"mcuboot.primary-slot-address"`: address of the primary slot in the internal flash
* `"mcuboot.slot-size"`: size of an image slot (only one image, two slots are currently supported)
* `"mcuboot.max-img-sectors"`: maximum number of sectors, should be at least the number of sectors in each slot
* `"target.restrict_size"`: the maximum size of the bootloader, such that it does not overlap with the primary slot

More configurations such as signing algorithm, slot swapping, etc. can be found in [mbed_lib.json](https://github.com/mcu-tools/mcuboot/tree/master/boot/mbed/mbed_lib.json). Please note that certain features are not currently supported.

### Providing a secondary slot

You need to provide an instance of `mbed::BlockDevice` as the secondary slot. It can be any types of internal or external storage provided that:
* Its size equals the `"mcuboot.slot-size"` you have set
* Its minimum supported read and write sizes (granularities) are _no larger than_ 16 byte, which MCUboot's read/write operations are aligned to. If the read size is larger than _one byte_, you need to set `"mcuboot.read-granularity"` to the read size of the storage - this buffers smaller read operations.

In order for MCUboot to access your secondary slot, the interface to implement is
```cpp
mbed::BlockDevice* get_secondary_bd(void);
```
which should return an uninitialized instance of BlockDevice.

### Building the bootloader

To build a bootloader based on MCUboot, make sure `"mcuboot.bootloader-build"` is `true` (already the default) and you have provided configurations and a secondary slot BlockDevice as explained above.

### Building a user application

To build a user application, set `"mcuboot.bootloader-build"` to `false` so MCUboot is built as a _library only_ without a bootloader application. This is useful if your user application needs to confirm the current image with `boot_set_confirmed()` after an update, or set a new image in the secondary slot as pending with `boot_set_pending()` in order to trigger an update upon reboot.

As your application starts in the primary slots (instead of the beginning of the whole flash), you need to set the start address (`"target.mbed_app_start"`) to be equal to `"mcuboot.primary-slot-address"` + `"mcuboot.header-size"` of your bootloader. And its size (`"target.mbed_app_size"`) must be no larger than `"mcuboot.slot-size"` - `"mcuboot.header-size"`, and some space must be left for the image trailer too (see [this](design.md#image-trailer)).
