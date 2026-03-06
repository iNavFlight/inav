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

/**
 * WASM EEPROM Bridge
 *
 * Provides JavaScript access to the firmware's EEPROM storage for
 * implementing persistent settings via IndexedDB.
 *
 * Usage from JavaScript:
 *   1. On load: Read IndexedDB, copy to wasmGetEepromPtr(), call wasmReloadConfig()
 *   2. On save: wasmEepromSavedCallback fires, read from wasmGetEepromPtr(), store to IndexedDB
 */

#ifdef __EMSCRIPTEN__

#include <emscripten.h>
#include <stdint.h>
#include <stdbool.h>

#include "platform.h"
#include "config/config_eeprom.h"
#include "config/config_streamer.h"

// eepromData is defined in config_streamer.c
extern uint8_t eepromData[];

// Notify JavaScript when EEPROM has been saved
// JavaScript should set: Module.wasmEepromSavedCallback = function() { ... }
EM_JS(void, notifyEepromSaved, (), {
    if (Module.wasmEepromSavedCallback) {
        Module.wasmEepromSavedCallback();
    }
});

/**
 * Get pointer to eepromData buffer
 * JavaScript uses this to read/write EEPROM contents directly
 *
 * @return Pointer to eepromData array
 */
EMSCRIPTEN_KEEPALIVE
uint8_t* wasmGetEepromPtr(void)
{
    return eepromData;
}

/**
 * Get size of EEPROM storage
 *
 * @return Size in bytes
 */
EMSCRIPTEN_KEEPALIVE
uint32_t wasmGetEepromSize(void)
{
    return EEPROM_SIZE;
}

/**
 * Reload configuration from eepromData
 *
 * Call this after JavaScript has copied stored settings into eepromData.
 * This re-reads the EEPROM and updates all parameter groups.
 *
 * @return true if EEPROM content was valid and loaded, false otherwise
 */
EMSCRIPTEN_KEEPALIVE
bool wasmReloadConfig(void)
{
    if (!isEEPROMContentValid()) {
        return false;
    }
    return loadEEPROM();
}

/**
 * Check if EEPROM contains valid configuration
 *
 * JavaScript can call this after copying data to verify it's valid
 * before calling wasmReloadConfig().
 *
 * @return true if valid, false otherwise
 */
EMSCRIPTEN_KEEPALIVE
bool wasmIsEepromValid(void)
{
    return isEEPROMContentValid();
}

/**
 * Called by config_streamer after EEPROM write completes
 * This notifies JavaScript to persist the data to IndexedDB
 */
void wasmNotifyEepromSaved(void)
{
    notifyEepromSaved();
}

#endif // __EMSCRIPTEN__
