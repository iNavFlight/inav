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

#include "config/parameter_group.h"

/**
 * Ensure a parameter group has allocated memory.
 *
 * This function is called by the PG_DECLARE accessor macros on every config access.
 * It performs lazy allocation: if memory hasn't been allocated yet, it:
 * 1. Allocates memory via malloc()
 * 2. Initializes with defaults from reset template
 * 3. For profile configs, allocates storage for all profiles
 *
 * Thread safety: NOT thread-safe. Safe only for single-threaded WASM builds.
 *
 * @param reg  The parameter group registry entry
 * @return Pointer to the allocated memory (system config or current profile),
 *         or NULL on allocation failure
 */
void* wasmPgEnsureAllocated(const pgRegistry_t *reg);

/**
 * Eagerly allocate all parameter groups at once.
 *
 * This is optional - the lazy allocation in wasmPgEnsureAllocated() will handle
 * on-demand allocation. However, calling this at boot can:
 * - Detect memory allocation failures early
 * - Avoid runtime allocation overhead on first access
 * - Ensure consistent initialization order
 */
void wasmPgInitAll(void);

#endif  // __EMSCRIPTEN__
