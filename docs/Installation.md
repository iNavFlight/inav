# Installation

## Using the configurator
This is a generic procedure to flash a board using the configurator. The configurator does not yet support all boards, so please check the documentation corresponding to your board before proceeding.

Make sure you have the [INAV Configurator](https://github.com/iNavFlight/inav-configurator) installed, then:

* Connect the flight controller to the PC.
* Start the INAV Configurator.
* Click on "Disconnect" if the configurator connected to the board automatically.
* Click on the "Firmware Flasher" tab.
* Make sure you have internet connectivity and click on the "Load Firmware [Online]" button.
* Click on the "Choose a Firmware / Board" dropdown menu, and select the latest stable version for your flight controller.
* IMPORTANT: Read and understand the release notes that are displayed.  When upgrading review all release notes since your current firmware.
* If this is the first time INAV is flashed to the board, tick the "Full Chip Erase" checkbox.
* Connect the flight controller board to the PC.  Ensure the correct serial port is selected.
* Click on the "Flash Firmware" button and hold still (do not breathe, too).
* When the progress bar becomes green and reads "Programming: SUCCESSFUL" you are done!

## Manually

See the board specific flashing instructions.

# Upgrading

When upgrading be sure to backup / dump your existing settings.  Some firmware releases are not backwards compatible and default settings are restored when the FC detects an out of date configuration.

## Backup/Restore process

The INAV Configurator (after version 9.0.x) automatically backs up your configuration before flashing and offers to restore it afterwards (when Full Chip Erase is enabled) — including automatic settings migration when upgrading across major versions.

For details on automatic and manual backup/restore, see [Backup and Restore](Backup%20and%20Restore.md).

For CLI-based backup and restore, see the [CLI documentation](Cli.md#backup-via-cli).
