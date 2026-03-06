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

/**
 * Initialize the WASM PG registry by copying scattered registry entries
 * into a contiguous array for PG_FOREACH iteration.
 *
 * This must be called early in init(), before any PG_FOREACH usage.
 * Native builds don't need this - the linker handles registry layout.
 */
void wasmPgRegistryInit(void);

#endif // __EMSCRIPTEN__
