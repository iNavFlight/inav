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

static uint8_t prevLength[MAX_CUSTOM_ELEMENTS];

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

static void customElementWriteIcon(osdCustomElementScreenBuffer_t *buf, uint8_t partIndex, uint16_t iconValue)
{
    if (iconValue > UINT8_MAX) {
        *buf->buff = SYM_BLANK;
        buf->twoByteChar[partIndex].index = buf->totalSeek;
        buf->twoByteChar[partIndex].icon = iconValue;
    } else {
        *buf->buff = (uint8_t)iconValue;
    }
}

void customElementDrawPart(osdCustomElementScreenBuffer_t *osdCustomElementScreenBuffer, uint8_t customElementIndex, uint8_t customElementItemIndex){
    const osdCustomElement_t* customElement = osdCustomElements(customElementIndex);
    const int customPartType = osdCustomElements(customElementIndex)->part[customElementItemIndex].type;
    const int customPartValue = osdCustomElements(customElementIndex)->part[customElementItemIndex].value;
    uint8_t currentSeek = 0;

    switch (customPartType) {
        case CUSTOM_ELEMENT_TYPE_GV_1:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff,(int32_t) (constrain(gvGet(customPartValue), -9, 9) * (int32_t) 100), 1, 0, 0, 2, false);
            currentSeek = 2;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_GV_2:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff, (int32_t) (constrain(gvGet(customPartValue), -99, 99) * (int32_t) 100), 1, 0, 0, 3, false);
            currentSeek = 3;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_GV_3:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff, (int32_t) (constrain(gvGet(customPartValue), -999, 999) * (int32_t) 100), 1, 0, 0, 4, false);
            currentSeek = 4;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_GV_4:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff, (int32_t) (constrain(gvGet(customPartValue), -9999, 9999) * (int32_t) 100), 1, 0, 0, 5, false);
            currentSeek = 5;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_GV_5:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff, (int32_t) (constrain(gvGet(customPartValue), -99999, 99999) * (int32_t) 100), 1, 0, 0, 6, false);
            currentSeek = 6;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_GV_FLOAT_1_1:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff, (int32_t) (constrain(gvGet(customPartValue), -99, 99) * (int32_t) 10), 1, 1, 0, 3, false);
            currentSeek = 3;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_GV_FLOAT_1_2:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff, (int32_t) (constrain(gvGet(customPartValue), -999, 999)), 1, 2, 0, 4, false);
            currentSeek = 4;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_GV_FLOAT_2_1:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff, (int32_t) (constrain(gvGet(customPartValue), -999, 999) * (int32_t) 10), 1, 1, 0, 4, false);
            currentSeek = 4;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_GV_FLOAT_2_2:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff, (int32_t) constrain(gvGet(customPartValue), -9999, 9999), 1, 2, 0, 5, false);
            currentSeek = 5;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_GV_FLOAT_3_1:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff, (int32_t) (constrain(gvGet(customPartValue), -9999, 9999) * (int32_t) 10), 1, 1, 0, 5, false);
            currentSeek = 5;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_GV_FLOAT_3_2:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff, (int32_t) constrain(gvGet(customPartValue), -99999, 99999), 1, 2, 0, 6, false);
            currentSeek = 6;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_GV_FLOAT_4_1:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff, (int32_t) (constrain(gvGet(customPartValue), -99999, 99999) * (int32_t) 10), 1, 1, 0, 6, false);
            currentSeek = 6;
            break;
        }

        case CUSTOM_ELEMENT_TYPE_LC_1:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff,(int32_t) (constrain(logicConditionGetValue(customPartValue), -9, 9) * (int32_t) 100), 1, 0, 0, 2, false);
            currentSeek = 2;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_LC_2:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff, (int32_t) (constrain(logicConditionGetValue(customPartValue), -99, 99) * (int32_t) 100), 1, 0, 0, 3, false);
            currentSeek = 3;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_LC_3:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff, (int32_t) (constrain(logicConditionGetValue(customPartValue), -999, 999) * (int32_t) 100), 1, 0, 0, 4, false);
            currentSeek = 4;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_LC_4:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff, (int32_t) (constrain(logicConditionGetValue(customPartValue), -9999, 9999) * (int32_t) 100), 1, 0, 0, 5, false);
            currentSeek = 5;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_LC_5:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff, (int32_t) (constrain(logicConditionGetValue(customPartValue), -99999, 99999) * (int32_t) 100), 1, 0, 0, 6, false);
            currentSeek = 6;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_LC_FLOAT_1_1:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff, (int32_t) (constrain(logicConditionGetValue(customPartValue), -99, 99) * (int32_t) 10), 1, 1, 0, 3, false);
            currentSeek = 3;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_LC_FLOAT_1_2:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff, (int32_t) (constrain(logicConditionGetValue(customPartValue), -999, 999)), 1, 2, 0, 4, false);
            currentSeek = 4;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_LC_FLOAT_2_1:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff, (int32_t) (constrain(logicConditionGetValue(customPartValue), -999, 999) * (int32_t) 10), 1, 1, 0, 4, false);
            currentSeek = 4;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_LC_FLOAT_2_2:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff, (int32_t) constrain(logicConditionGetValue(customPartValue), -9999, 9999), 1, 2, 0, 5, false);
            currentSeek = 5;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_LC_FLOAT_3_1:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff, (int32_t) (constrain(logicConditionGetValue(customPartValue), -9999, 9999)  * (int32_t) 10), 1, 1, 0, 5, false);
            currentSeek = 5;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_LC_FLOAT_3_2:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff, (int32_t) constrain(logicConditionGetValue(customPartValue), -99999, 99999), 1, 2, 0, 6, false);
            currentSeek = 6;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_LC_FLOAT_4_1:
        {
            osdFormatCentiNumber(osdCustomElementScreenBuffer->buff, (int32_t) (constrain(logicConditionGetValue(customPartValue), -99999, 99999) * (int32_t) 10), 1, 1, 0, 6, false);
            currentSeek = 6;
            break;
        }

        case CUSTOM_ELEMENT_TYPE_ICON_GV:
        {
            customElementWriteIcon(osdCustomElementScreenBuffer, customElementItemIndex, (uint16_t)gvGet(customPartValue));
            currentSeek = 1;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_ICON_LC:
        {
            customElementWriteIcon(osdCustomElementScreenBuffer, customElementItemIndex, (uint16_t)logicConditionGetValue(customPartValue));
            currentSeek = 1;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_ICON_STATIC:
        {
            customElementWriteIcon(osdCustomElementScreenBuffer, customElementItemIndex, (uint16_t)customPartValue);
            currentSeek = 1;
            break;
        }
        case CUSTOM_ELEMENT_TYPE_TEXT:
        {
            uint8_t i;
            for (i = 0; i < OSD_CUSTOM_ELEMENT_TEXT_SIZE; i++) {
                if (customElement->osdCustomElementText[i] == 0) {
                    break;
                }
                osdCustomElementScreenBuffer->buff[i] = sl_toupper((unsigned char)customElement->osdCustomElementText[i]);
            }
            currentSeek = i;
            break;
        }
    }

    osdCustomElementScreenBuffer->totalSeek += currentSeek;
    osdCustomElementScreenBuffer->buff += currentSeek;
}

void customElementDrawElement(displayPort_t *osdDisplayPort, char *buff, uint8_t customElementIndex, uint8_t x, uint8_t y){

    osdCustomElementScreenBuffer_t osdCustomElementScreenBuffer = {
            .buffStart = buff,
            .buff = buff,
            .totalSeek = 0
    };

    if(customElementIndex >= MAX_CUSTOM_ELEMENTS){
        return;
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    // prepare buffer
    const osdCustomElement_t* customElement = osdCustomElements(customElementIndex);
    if(isCustomelementVisible(customElement))
    {
        for (uint8_t i = 0; i < CUSTOM_ELEMENTS_PARTS; ++i) {
            customElementDrawPart(&osdCustomElementScreenBuffer, customElementIndex, i);
        }
    }
    ////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////
    // add blank symbol to clear prev osd element symbols
    for (uint8_t i = osdCustomElementScreenBuffer.totalSeek; i < prevLength[customElementIndex]; i++) {
        *osdCustomElementScreenBuffer.buff++ = SYM_BLANK;
    }
    prevLength[customElementIndex] = osdCustomElementScreenBuffer.totalSeek;
    /////////////////////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////////////////////////
    // draw simple buffer
    displayWrite(osdDisplayPort, x, y, osdCustomElementScreenBuffer.buffStart);
    //////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////////
    // draw 16 bit icons
    for (int i = 0; i < CUSTOM_ELEMENTS_PARTS; ++i) {
        twoByteChar_t *twoByteChar = &(osdCustomElementScreenBuffer.twoByteChar[i]);
        if(twoByteChar->icon > UINT8_MAX){
            displayWriteChar(osdDisplayPort, x + twoByteChar->index, y, twoByteChar->icon);
        }
    }
    //////////////////////////////////////////////////////////////////////////////////

}

uint8_t customElementLength(uint8_t customElementIndex){
    return prevLength[customElementIndex] ? prevLength[customElementIndex] : 1;
}
