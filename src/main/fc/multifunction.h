/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#pragma once

extern uint8_t multiFunctionFlags;

#define MULTI_FUNC_FLAG_DISABLE(mask) (multiFunctionFlags &= ~(mask))
#define MULTI_FUNC_FLAG_ENABLE(mask) (multiFunctionFlags |= (mask))
#define MULTI_FUNC_FLAG(mask) (multiFunctionFlags & (mask))

typedef enum {
    SUSPEND_SAFEHOMES = (1 << 0),
    SUSPEND_TRACKBACK = (1 << 1),
} multiFunctionFlags_e;

typedef enum {
    MULTI_FUNC_NONE,
    MULTI_FUNC_1,
    MULTI_FUNC_2,
    MULTI_FUNC_3,
    MULTI_FUNC_4,
    MULTI_FUNC_5,
    MULTI_FUNC_END,
} multi_function_e;

multi_function_e multiFunctionSelection(void);
