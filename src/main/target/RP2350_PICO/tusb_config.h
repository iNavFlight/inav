/*
 * TinyUSB configuration for INAV RP2350 USB CDC stdio
 *
 * Minimal config: CDC only (serial console for debug printf)
 */

#ifndef _TUSB_CONFIG_H
#define _TUSB_CONFIG_H

// MCU selection — RP2040 DCD works for both RP2040 and RP2350
#define CFG_TUSB_MCU OPT_MCU_RP2040

// USB device mode
#define CFG_TUD_ENABLED 1
#define CFG_TUH_ENABLED 0

// CDC class — one interface for stdio
#define CFG_TUD_CDC 1
#define CFG_TUD_CDC_RX_BUFSIZE 256
#define CFG_TUD_CDC_TX_BUFSIZE 256

// No other USB classes needed for M2
#define CFG_TUD_MSC 0
#define CFG_TUD_HID 0
#define CFG_TUD_MIDI 0
#define CFG_TUD_VENDOR 0
#define CFG_TUD_DFU_RUNTIME 0

// Operating system abstraction — use Pico SDK
#define CFG_TUSB_OS OPT_OS_PICO

// Use default endpoint size
#define CFG_TUD_ENDPOINT0_SIZE 64

#endif /* _TUSB_CONFIG_H */
