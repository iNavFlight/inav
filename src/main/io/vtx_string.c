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

/* Created by jflyper */

#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "platform.h"
#include "build/debug.h"

#define VTX_STRING_5G8_BAND_COUNT  5
#define VTX_STRING_5G8_CHAN_COUNT  8
#define VTX_STRING_5G8_POWER_COUNT 5

#define VTX_STRING_1G3_BAND_COUNT  2
#define VTX_STRING_1G3_CHAN_COUNT  8
#define VTX_STRING_1G3_POWER_COUNT 3

const uint16_t vtx58frequencyTable[VTX_STRING_5G8_BAND_COUNT][VTX_STRING_5G8_CHAN_COUNT] =
{
    { 6110, 6130, 6150, 6170, 6190, 6220, 6230, 6250 }, // A
    { 6270, 6290, 6310, 6330, 6350, 6370, 6390, 6410 }, // B
    { 6430, 6450, 6470, 6490, 6510, 6530, 6550, 6570 }, // E
    { 6590, 6610, 6630, 6650, 6670, 6690, 6710, 6730 }, // F
    { 6750, 6770, 6790, 6810, 6830, 6850, 6870, 6890 }, // R
};

const char * const vtx58BandNames[VTX_STRING_5G8_BAND_COUNT + 1] = {
    "-",
    "A",
    "B",
    "E",
    "F",
    "R",
};

const char vtx58BandLetter[VTX_STRING_5G8_BAND_COUNT + 1] = {'-', 'A', 'B', 'E', 'F', 'R'};

const char * const vtx58ChannelNames[VTX_STRING_5G8_CHAN_COUNT + 1] = {
    "-", "1", "2", "3", "4", "5", "6", "7", "8",
};

const char * const vtx58DefaultPowerNames[VTX_STRING_5G8_POWER_COUNT + 1] = {
    "---", "PL1", "PL2", "PL3", "PL4", "PL5"
};

const uint16_t vtx1G3frequencyTable[VTX_STRING_1G3_BAND_COUNT][VTX_STRING_1G3_CHAN_COUNT] =
{
    { 1080, 1120, 1160, 1200, 1240, 1280, 1320, 1360 }, // A
    { 1080, 1120, 1160, 1200, 1258, 1280, 1320, 1360 }, // B
};

const char * const vtx1G3BandNames[VTX_STRING_1G3_BAND_COUNT + 1] = {
    "-",
    "A",
    "B",
};

const char vtx1G3BandLetter[VTX_STRING_1G3_BAND_COUNT + 1] = {'-', 'A', 'B'};

const char * const vtx1G3ChannelNames[VTX_STRING_1G3_CHAN_COUNT + 1] = {
    "-", "1", "2", "3", "4", "5", "6", "7", "8",
};

const char * const vtx1G3DefaultPowerNames[VTX_STRING_1G3_POWER_COUNT + 1] = {
    "---", "PL1", "PL2", "PL3"
};

bool vtx58_Freq2Bandchan(uint16_t freq, uint8_t *pBand, uint8_t *pChannel)
{
    int8_t band;
    uint8_t channel;

    // Use reverse lookup order so that 5880Mhz
    // get Raceband 7 instead of Fatshark 8.
    for (band = 4 ; band >= 0 ; band--) {
        for (channel = 0 ; channel < 8 ; channel++) {
            if (vtx58frequencyTable[band][channel] == freq) {
                *pBand = band + 1;
                *pChannel = channel + 1;
                return true;
            }
        }
    }

    *pBand = 0;
    *pChannel = 0;

    return false;
}

// Converts band and channel values to a frequency (in MHz) value.
// band: Band value (1 to 5).
// channel:  Channel value (1 to 8).
// Returns frequency value (in MHz), or 0 if band/channel out of range.
uint16_t vtx58_Bandchan2Freq(uint8_t band, uint8_t channel)
{
    if (band > 0 && band <= VTX_STRING_5G8_BAND_COUNT &&
                          channel > 0 && channel <= VTX_STRING_5G8_CHAN_COUNT) {
        return vtx58frequencyTable[band - 1][channel - 1];
    }
    return 0;
}

// Converts band and channel values to a frequency (in MHz) value.
// band: Band value (1 to 2).
// channel:  Channel value (1 to 8).
// Returns frequency value (in MHz), or 0 if band/channel out of range.
uint16_t vtx1G3_Bandchan2Freq(uint8_t band, uint8_t channel)
{
    if (band > 0 && band <= VTX_STRING_1G3_BAND_COUNT &&
                          channel > 0 && channel <= VTX_STRING_1G3_CHAN_COUNT) {
        return vtx1G3frequencyTable[band - 1][channel - 1];
    }
    return 0;
}
