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

#include <stdbool.h>
#include <stdint.h>

// Global flag for fast inline check - avoids function call in FAST_CODE path
extern bool motorLocateActive;

// Start motor locate cycle for specified motor index
// Runs jerk+beep pattern for ~2 seconds then stops automatically
// Returns false if locate is already running or motor index invalid
bool motorLocateStart(uint8_t motorIndex);

// Stop any running motor locate cycle
void motorLocateStop(void);

// Check if motor locate is currently active
bool motorLocateIsActive(void);

// Called from motor update loop to apply locate overrides
// Returns true if locate is active and motor values were overridden
bool motorLocateUpdate(void);
