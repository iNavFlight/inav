/*
 * This file is part of iNav.
 *
 * iNav are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * iNav are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdint.h>
#include "flash.h"
#include "drivers/io_types.h"

bool w25n01g_init(int flashNumToUse);

void w25n01g_eraseSector(uint32_t address);
void w25n01g_eraseCompletely(void);

uint32_t w25n01g_pageProgram(uint32_t address, const uint8_t *data, int length);

int w25n01g_readBytes(uint32_t address, uint8_t *buffer, int length);

bool w25n01g_isReady(void);

const flashGeometry_t* w25n01g_getGeometry(void);