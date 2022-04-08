/*
 * This file is part of iNav.
 *
 * iNav is free software. You can redistribute this software
 * and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * iNav is distributed in the hope that they will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

bool firmwareUpdatePrepare(uint32_t firmwareSize);
bool firmwareUpdateStore(uint8_t *data, uint16_t length);
void firmwareUpdateExec(uint8_t expectCRC);
bool firmwareUpdateRollbackPrepare(void);
void firmwareUpdateRollbackExec(void);

bool writeMeta(void); // XXX temp
