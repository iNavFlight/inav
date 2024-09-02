# Porting How-To

This document describes the requirements and necessary steps required to port
`mcuboot` to a new target `OS`.

# Requirements

* `mcuboot` requires a configuration file, which can be included as
   mcuboot_config/mcuboot_config.h, which configures various options
   (that begin with MCUBOOT_).

* `mcuboot` requires that the target provides a `flash` API with ability to
  get the flash's minimum write size, and read/write/erase individual sectors.

* `mcuboot` doesn't bundle a cryptographic library, which means the target
  OS must already have it bundled. The supported libraries at the moment are
  either `mbed TLS` or the set `tinycrypt` + `mbed TLS` (where `mbed TLS` is
  used to provide functionality not existing in `tinycrypt`).

# Steps to port

## Main app and calling the bootloader

From the perspective of the target OS, the bootloader can be seen as a library,
so an entry point must be provided. This is likely a typical `app` for the
target OS, and it must call the following function to run the bootloader:

```c
int boot_go(struct boot_rsp *rsp);
```

This function is located at `boot/bootutil/loader.c` and receives a `struct
boot_rsp` pointer. The `struct boot_rsp` is defined as:

```c
struct boot_rsp {
    /** A pointer to the header of the image to be executed. */
    const struct image_header *br_hdr;

    /**
     * The flash offset of the image to execute.  Indicates the position of
     * the image header.
     */
    uint8_t br_flash_id;
    uint32_t br_image_addr;
};
```

After running the management functions of the bootloader, `boot_go` returns
an initialized `boot_rsp` which has pointers to the location of the image
where the target firmware is located which can be used to jump to.

## Configuration file

You must provide a file, mcuboot_config/mcuboot_config.h. This is
included by several files in the "library" portion of MCUboot; it
provides preprocessor definitions that configure the library's
build.

See the file samples/mcuboot_config/mcuboot_config.template.h for a
starting point and more information. This is a good place to convert
settings in your environment's configuration system to those required
by MCUboot. For example, Mynewt uses MYNEWT_VAL() and Zephyr uses
Kconfig; these configuration systems are converted to MCUBOOT_ options
in the following files:

- boot/zephyr/include/mcuboot_config/mcuboot_config.h
- boot/mynewt/mcuboot_config/include/mcuboot_config/mcuboot_config.h

## Flash Map

The bootloader requires a `flash_map` to be able to know how the flash is
partitioned. A `flash_map` consists of `struct flash_area` entries
specifying the partitions, where a `flash_area` defined as follows:

```c
struct flash_area {
    uint8_t  fa_id;         /** The slot/scratch identification */
    uint8_t  fa_device_id;  /** The device id (usually there's only one) */
    uint16_t pad16;
    uint32_t fa_off;        /** The flash offset from the beginning */
    uint32_t fa_size;       /** The size of this sector */
};
```

`fa_id` is can be one of the following options:

```c
/* Independent from multiple image boot */
#define FLASH_AREA_BOOTLOADER         0
#define FLASH_AREA_IMAGE_SCRATCH      3
```
```c
/* Flash area IDs of the first image in case of multiple images */
#define FLASH_AREA_IMAGE_PRIMARY      1
#define FLASH_AREA_IMAGE_SECONDARY    2
```
```c
/* Flash area IDs of the second image in case of multiple images */
#define FLASH_AREA_IMAGE_PRIMARY      5
#define FLASH_AREA_IMAGE_SECONDARY    6
```

The functions that must be defined for working with the `flash_area`s are:

```c
/*< Opens the area for use. id is one of the `fa_id`s */
int     flash_area_open(uint8_t id, const struct flash_area **);
void    flash_area_close(const struct flash_area *);
/*< Reads `len` bytes of flash memory at `off` to the buffer at `dst` */
int     flash_area_read(const struct flash_area *, uint32_t off, void *dst,
                     uint32_t len);
/*< Writes `len` bytes of flash memory at `off` from the buffer at `src` */
int     flash_area_write(const struct flash_area *, uint32_t off,
                     const void *src, uint32_t len);
/*< Erases `len` bytes of flash memory at `off` */
int     flash_area_erase(const struct flash_area *, uint32_t off, uint32_t len);
/*< Returns this `flash_area`s alignment */
uint8_t flash_area_align(const struct flash_area *);
/*< What is value is read from erased flash bytes. */
uint8_t flash_area_erased_val(const struct flash_area *);
/*< Given flash area ID, return info about sectors within the area. */
int     flash_area_get_sectors(int fa_id, uint32_t *count,
                     struct flash_sector *sectors);
/*< Returns the `fa_id` for slot, where slot is 0 (primary) or 1 (secondary).
    `image_index` (0 or 1) is the index of the image. Image index is
    relevant only when multi-image support support is enabled */
int     flash_area_id_from_multi_image_slot(int image_index, int slot);
/*< Returns the slot (0 for primary or 1 for secondary), for the supplied
    `image_index` and `area_id`. `area_id` is unique and is represented by
    `fa_id` in the `flash_area` struct. */
int     flash_area_id_to_multi_image_slot(int image_index, int area_id);
```

## Memory management for mbed TLS

`mbed TLS` employs dynamic allocation of memory, making use of the pair
`calloc/free`. If `mbed TLS` is to be used for crypto, your target RTOS
needs to provide this pair of function.

To configure the what functions are called when allocating/deallocating
memory `mbed TLS` uses the following call:

```
int mbedtls_platform_set_calloc_free (void *(*calloc_func)(size_t, size_t),
                                      void (*free_func)(void *));
```

For reference see [mbed TLS platform.h](https://tls.mbed.org/api/platform_8h.html).
If your system already provides functions with compatible signatures, those can
be used directly here, otherwise create new functions that glue to your
`calloc/free` implementations.
