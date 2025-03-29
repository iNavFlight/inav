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

#include "drivers/osd.h"
#include "msp/msp_serial.h"

// MSP displayport V2 attribute byte bit functions
#define DISPLAYPORT_MSP_ATTR_FONTPAGE   0 // Select bank of 256 characters as per displayPortSeverity_e
#define DISPLAYPORT_MSP_ATTR_BLINK      6 // Device local blink
#define DISPLAYPORT_MSP_ATTR_VERSION    7 // Format indicator; must be zero for V2 (and V1)

#define DISPLAYPORT_MSP_ATTR_FONTPAGE_MASK   0x3
#define DISPLAYPORT_MSP_ATTR_BLINK_MASK      (1 << DISPLAYPORT_MSP_ATTR_BLINK)
#define DISPLAYPORT_MSP_ATTR_VERSION_MASK    (1 << DISPLAYPORT_MSP_ATTR_VERSION)

// PAL screen size
#define PAL_COLS 30
#define PAL_ROWS 16
// NTSC screen size
#define NTSC_COLS 30
#define NTSC_ROWS 13
// HDZERO screen size
#define HDZERO_COLS 50
#define HDZERO_ROWS 18
// Avatar screen size
#define AVATAR_COLS 53
#define AVATAR_ROWS 20
// DJIWTF screen size
#define DJI_COLS 60
#define DJI_ROWS 22

// set COLS and ROWS to largest size available
#define MSP_DISPLAYPORT_MAX_COLS DJI_COLS
#define MSP_DISPLAYPORT_MAX_ROWS DJI_ROWS



typedef struct displayPort_s displayPort_t;

displayPort_t *mspOsdDisplayPortInit(const videoSystem_e videoSystem);
void mspOsdSerialProcess(mspProcessCommandFnPtr mspProcessCommandFn);
mspPort_t *getMspOsdPort(void);

#define getAttrPage(attr) (attr & DISPLAYPORT_MSP_ATTR_FONTPAGE_MASK)
#define getAttrBlink(attr) ((attr & DISPLAYPORT_MSP_ATTR_BLINK_MASK) >> DISPLAYPORT_MSP_ATTR_BLINK)
#define getAttrVersion(attr) ((attr & DISPLAYPORT_MSP_ATTR_VERSION_MASK) >> DISPLAYPORT_MSP_ATTR_VERSION)

uint8_t setAttrPage(uint8_t origAttr, uint8_t page);
uint8_t setAttrBlink(uint8_t origAttr, uint8_t page);
uint8_t setAttrVersion(uint8_t origAttr, uint8_t page);
