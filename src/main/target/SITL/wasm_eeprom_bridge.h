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

#pragma once

#ifdef __EMSCRIPTEN__

#include <stdint.h>
#include <stdbool.h>

/**
 * Get pointer to eepromData buffer for JavaScript access
 */
uint8_t* wasmGetEepromPtr(void);

/**
 * Get size of EEPROM storage in bytes
 */
uint32_t wasmGetEepromSize(void);

/**
 * Reload configuration from eepromData after JavaScript injection
 */
bool wasmReloadConfig(void);

/**
 * Check if EEPROM contains valid configuration
 */
bool wasmIsEepromValid(void);

/**
 * Notify JavaScript that EEPROM was saved (call from config_streamer)
 */
void wasmNotifyEepromSaved(void);

#endif // __EMSCRIPTEN__
