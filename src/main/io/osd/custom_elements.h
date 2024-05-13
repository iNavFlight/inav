/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "config/parameter_group.h"

#define OSD_CUSTOM_ELEMENT_TEXT_SIZE 16
#define CUSTOM_ELEMENTS_PARTS 3
#define MAX_CUSTOM_ELEMENTS 3

typedef enum {
    CUSTOM_ELEMENT_TYPE_NONE            = 0,
    CUSTOM_ELEMENT_TYPE_TEXT            = 1,
    CUSTOM_ELEMENT_TYPE_ICON_STATIC     = 2,
    CUSTOM_ELEMENT_TYPE_ICON_GV         = 3,
    CUSTOM_ELEMENT_TYPE_GV              = 4,
    CUSTOM_ELEMENT_TYPE_GV_FLOAT        = 5,
    CUSTOM_ELEMENT_TYPE_GV_SMALL        = 6,
    CUSTOM_ELEMENT_TYPE_GV_SMALL_FLOAT  = 7,
} osdCustomElementType_e;

typedef enum {
    CUSTOM_ELEMENT_VISIBILITY_ALWAYS    = 0,
    CUSTOM_ELEMENT_VISIBILITY_GV        = 1,
    CUSTOM_ELEMENT_VISIBILITY_LOGIC_CON = 2,
} osdCustomElementTypeVisibility_e;

typedef struct {
    osdCustomElementType_e  type;
    uint16_t                value;
} osdCustomElementItem_t;

typedef struct {
    osdCustomElementTypeVisibility_e    type;
    uint16_t                            value;
} osdCustomElementVisibility_t;

typedef struct  {
    osdCustomElementItem_t          part[CUSTOM_ELEMENTS_PARTS];
    osdCustomElementVisibility_t    visibility;
    char                            osdCustomElementText[OSD_CUSTOM_ELEMENT_TEXT_SIZE];
} osdCustomElement_t;

PG_DECLARE_ARRAY(osdCustomElement_t, MAX_CUSTOM_ELEMENTS, osdCustomElements);

void customElementDrawElement(char *buff, uint8_t customElementIndex);