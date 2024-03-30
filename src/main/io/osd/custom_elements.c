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


#include "config/config_reset.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "common/string_light.h"
#include "common/maths.h"

#include "programming/logic_condition.h"
#include "programming/global_variables.h"

#include "io/osd.h"
#include "io/osd/custom_elements.h"

#include "drivers/osd_symbols.h"

PG_REGISTER_ARRAY_WITH_RESET_FN(osdCustomElement_t, MAX_CUSTOM_ELEMENTS, osdCustomElements, PG_OSD_CUSTOM_ELEMENTS_CONFIG, 1);

void pgResetFn_osdCustomElements(osdCustomElement_t *instance)
{
    for (int i = 0; i < MAX_CUSTOM_ELEMENTS; i++) {
        RESET_CONFIG(osdCustomElement_t, &instance[i],
            .part[0]              = {.type = CUSTOM_ELEMENT_TYPE_NONE, .value = 0},
            .part[1]              = {.type = CUSTOM_ELEMENT_TYPE_NONE, .value = 0},
            .part[2]              = {.type = CUSTOM_ELEMENT_TYPE_NONE, .value = 0},
            .visibility           = {.type = CUSTOM_ELEMENT_VISIBILITY_ALWAYS, .value = 0},
            .osdCustomElementText   = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        );
    }
}

bool isCustomelementVisible(const osdCustomElement_t* customElement){
    if(customElement->visibility.type == CUSTOM_ELEMENT_VISIBILITY_ALWAYS){
        return true;
    }

    if(customElement->visibility.type == CUSTOM_ELEMENT_VISIBILITY_GV && gvGet(customElement->visibility.value)){
        return true;
    }

    if(customElement->visibility.type == CUSTOM_ELEMENT_VISIBILITY_LOGIC_CON && logicConditionGetValue(customElement->visibility.value)){
        return true;
    }

    return false;
}

uint8_t customElementDrawPart(char *buff, uint8_t customElementIndex, uint8_t customElementItemIndex){
    const osdCustomElement_t* customElement = osdCustomElements(customElementIndex);
    const int customPartType = osdCustomElements(customElementIndex)->part[customElementItemIndex].type;
    const int customPartValue = osdCustomElements(customElementIndex)->part[customElementItemIndex].value;

    switch (customPartType) {
        case CUSTOM_ELEMENT_TYPE_GV:
        {
            osdFormatCentiNumber(buff, (int32_t) gvGet(customPartValue) * (int32_t) 100, 1, 0, 0, 6, false);
            return 6;
        }
        case CUSTOM_ELEMENT_TYPE_GV_FLOAT:
        {
            osdFormatCentiNumber(buff, (int32_t) gvGet(customPartValue), 1, 2, 0, 6, false);
            return 6;
        }
        case CUSTOM_ELEMENT_TYPE_GV_SMALL:
        {
            osdFormatCentiNumber(buff, (int32_t) ((gvGet(customPartValue) % 1000 ) * (int32_t) 100), 1, 0, 0, 3, false);
            return 3;
        }
        case CUSTOM_ELEMENT_TYPE_GV_SMALL_FLOAT:
        {
            osdFormatCentiNumber(buff, (int32_t) ((gvGet(customPartValue) % 100)  * (int32_t) 10), 1, 1, 0, 2, false);
            return 2;
        }
        case CUSTOM_ELEMENT_TYPE_ICON_GV:
        {
            *buff = (uint8_t)gvGet(customPartValue);
            return 1;
        }
        case CUSTOM_ELEMENT_TYPE_ICON_STATIC:
        {
            *buff = (uint8_t)customPartValue;
            return 1;
        }
        case CUSTOM_ELEMENT_TYPE_TEXT:
        {
            for (int i = 0; i < OSD_CUSTOM_ELEMENT_TEXT_SIZE; i++) {
                if (customElement->osdCustomElementText[i] == 0){
                    return i;
                }
                *buff = sl_toupper((unsigned char)customElement->osdCustomElementText[i]);
                buff++;
            }
            return OSD_CUSTOM_ELEMENT_TEXT_SIZE;
        }
    }

    return 0;
}

void customElementDrawElement(char *buff, uint8_t customElementIndex){

    if(customElementIndex >= MAX_CUSTOM_ELEMENTS){
        return;
    }

    static uint8_t prevLength[MAX_CUSTOM_ELEMENTS];

    uint8_t buffSeek = 0;
    const osdCustomElement_t* customElement = osdCustomElements(customElementIndex);
    if(isCustomelementVisible(customElement))
    {
        for (uint8_t i = 0; i < CUSTOM_ELEMENTS_PARTS; ++i) {
            uint8_t currentSeek = customElementDrawPart(buff, customElementIndex, i);
            buff += currentSeek;
            buffSeek += currentSeek;
        }
    }

    for (uint8_t i = buffSeek; i < prevLength[customElementIndex]; i++) {
        *buff++ = SYM_BLANK;
    }
    prevLength[customElementIndex] = buffSeek;
}

