# Cmake Usage

## Introduction

This guide documents inav usage of the `cmake` build tool.

## Target Defintion

A target requires `CMakeLists.txt` file. This file contains one of more lines of the form:

```
target_hardware_definition(name optional_parameters)
```

For example:

```
target_stm32f405xg(QUARKVISION HSE_MHZ 16)
```

* The hardware is `stm32f405xg`, a F405 with a 1MiB flash
* The name is `QUARKVISION` (a board that never reached production)
* The optional parameter is `HSE_MHZ 16` defining the non-default high-speed external (HSE) oscillator clock.

## Hardware names

As of inav 2.6, the following target hardware platforms are recognised:

* stm32f303xc
* stm32f405xg
* stm32f411xe
* stm32f427xg
* stm32f722xe
* stm32f745xg
* stm32f765xg
* stm32f765xi

The device characteristics for these names may be found at [stm32-base.org](https://stm32-base.org/cheatsheets/linker-memory-regions/).

## Optional Parameters

The following optional parameters are recognised:

| Paramater | Usage |
| --------- | ----- |
| `SKIP_RELEASES` | The target is disabled for releases and CI. It still may be possible to build the target directly. |
| `COMPILE_DEFINITIONS "VAR[=value]"` | Sets a preprocessor define. |
| `HSE_MZ value` | The target uses a high-speed external crystal (HSE) oscillator clock with a frequency different from the 8MHz default. The `value` is the desired clock, for example `HSE_MHZ 24` |

Multiple optional parameters may be specified, for example `HSE_MHZ 16 SKIP_RELEASES`.

## Target variations

A number of targets support multiple variants, either successive versions of the same hardware, or varations that enable different resources (soft serial, leds etc.) This is defined by adding additional `target_` lines to `CMakeLists.txt`. For example, the OMNIBUSF4 and its multiple clones / variations, `src/main/target/OMNIBUSF4/CMakeLists.txt`:

```
target_stm32f405xg(DYSF4PRO)
target_stm32f405xg(DYSF4PROV2)
target_stm32f405xg(OMNIBUSF4)
# the OMNIBUSF4SD has an SDCARD instead of flash, a BMP280 baro and therefore a slightly different ppm/pwm and SPI mapping
target_stm32f405xg(OMNIBUSF4PRO)
target_stm32f405xg(OMNIBUSF4PRO_LEDSTRIPM5)
target_stm32f405xg(OMNIBUSF4V3_S5_S6_2SS)
target_stm32f405xg(OMNIBUSF4V3_S5S6_SS)
target_stm32f405xg(OMNIBUSF4V3_S6_SS)
# OMNIBUSF4V3 is a (almost identical) variant of OMNIBUSF4PRO target,
# except for an inverter on UART6.
target_stm32f405xg(OMNIBUSF4V3)
```
