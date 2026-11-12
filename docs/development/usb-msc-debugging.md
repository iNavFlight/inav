# USB MSC Debugging

Debugging USB Mass Storage (MSC) and CDC (serial) issues on flight controllers.

## Key Insight: Composite USB Devices

When a target runs CDC (MSP serial) and MSC (mass storage) together, the device descriptors must be built as a genuinely composite device — mixing standalone-MSC descriptors with a device that was actually initialized in composite mode produces conflicting descriptors that confuse the host OS.

On H7 targets this is controlled by `usbDevConfig()->type == COMPOSITE` (`src/main/vcp_hal/usbd_conf_stm32h7xx.c`); `src/main/drivers/usb_msc_h7xx.c` explicitly notes where it falls back to standalone-MSC descriptors for the non-composite case.

**Lesson:** USB library updates can change how composite-device descriptor selection is wired up. If USB stops enumerating correctly after a library update, check how the composite/standalone descriptor choice is made before assuming a hardware or wiring problem.

## Symptoms of MSC Issues

**Windows:**
- Device Manager shows "Virtual COM Port in FS Mode"
- Missing drivers (Code 28)
- No storage drive appears in MSC mode

**Linux:**
- MSC may still work (different enumeration handling)
- Check `dmesg` for USB errors

## Debugging Commands

```bash
# Check USB device enumeration
lsusb -v | grep -A20 "STM"

# Check kernel messages
dmesg | tail -50

# Check USB device modes
cat /sys/bus/usb/devices/*/product

# List all USB devices with details
lsusb -t
```

## H7-Specific Notes

H7 boards typically use SDIO for SD card access, unlike F4/F7/AT32 boards which more commonly use SPI. SDIO-based MSC and SPI-based MSC go through different code paths, so a USB/MSC regression that only reproduces on H7 boards is worth checking against the SDIO-specific path first (`src/main/drivers/usb_msc_h7xx.c`).

## USB Library Changes to Watch

When the underlying STM32 USB device library is updated, check for:
- Composite vs standalone descriptor selection logic changes
- API changes (e.g. endpoint-size macro renames)
- Function signature changes
- Descriptor handling differences

## Related Files

INAV USB implementation:
- `src/main/drivers/usb_msc_h7xx.c` - H7 MSC implementation
- `src/main/vcp_hal/usbd_desc.c` - USB descriptors
- `src/main/vcp_hal/usbd_conf_stm32h7xx.c` - composite device configuration
- `lib/main/STM32H7/Middlewares/ST/STM32_USB_Device_Library/` - USB library
