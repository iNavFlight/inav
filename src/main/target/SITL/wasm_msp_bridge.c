/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * WASM MSP Bridge
 *
 * Serial Byte Interface:
 *   - serialWriteByte() - send MSP packet bytes (implemented in serial_wasm.c)
 *   - serialReadByte() - receive MSP response bytes
 *   - serialAvailable() - check bytes available
 *   - Uses standard INAV MSP infrastructure (same as UART/TCP/BLE)
 *
 * This properly simulates serial communication and reuses all existing MSP code.
 */

#ifdef __EMSCRIPTEN__

#include <emscripten.h>
#include <stdint.h>

#include "platform.h"
#include "drivers/serial.h"
#include "msp/msp.h"
#include "msp/msp_serial.h"
#include "fc/fc_msp.h"
#include "serial_wasm.h"

// WASM MSP port
static mspPort_t *wasmMspPort = NULL;

/**
 * Initialize WASM MSP interface
 * Called automatically on first use
 */
static void wasmMspInit(void)
{
    if (wasmMspPort != NULL) {
        return;  // Already initialized
    }

    // Initialize WASM serial port
    serialPort_t *serialPort = wasmSerialInit();

    // Allocate MSP port (reuse existing infrastructure)
    // Note: We use the first available MSP port slot
    // In practice, SITL WASM probably won't have other MSP ports
    static mspPort_t wasmMspPortInstance;
    wasmMspPort = &wasmMspPortInstance;
    resetMspPort(wasmMspPort, serialPort);
}

/**
 * Process WASM MSP serial port
 * This should be called periodically (e.g., from main loop)
 * It processes incoming bytes and generates responses
 */
void wasmMspProcess(void)
{
    if (wasmMspPort == NULL) {
        wasmMspInit();
    }

    // Use standard MSP serial processing (same as UART/TCP/UDP/BLE!)
    mspSerialProcessOnePort(wasmMspPort, MSP_SKIP_NON_MSP_DATA, mspFcProcessCommand);
}

#endif  // __EMSCRIPTEN__
