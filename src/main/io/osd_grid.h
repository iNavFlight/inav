/*
 * This file is part of INAV.
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
 *
 * @author Alberto Garcia Hierro <alberto@garciahierro.com>
 */

#pragma once

typedef struct displayPort_s displayPort_t;

void osdGridDrawThrottleGauge(displayPort_t *display, unsigned gx, unsigned gy, uint8_t thrPos);
void osdGridDrawVario(displayPort_t *display, unsigned gx, unsigned gy, float zvel);
void osdGridDrawDirArrow(displayPort_t *display, unsigned gx, unsigned gy, float degrees);
void osdGridDrawArtificialHorizon(displayPort_t *display, unsigned gx, unsigned gy, float pitchAngle, float rollAngle);
void osdGridDrawHeadingGraph(displayPort_t *display, unsigned gx, unsigned gy, int heading);
void osdGridDrawSidebars(displayPort_t *display);
