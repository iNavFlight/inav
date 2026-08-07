#pragma once

#include <stdint.h>

extern const uint16_t vtx58frequencyTable[5][8];
extern const char * const vtx58BandNames[];
extern const char * const vtx58ChannelNames[];
extern const char * const vtx58DefaultPowerNames[];
extern const char vtx58BandLetter[];

extern const uint16_t vtx1G3frequencyTable[2][8];
extern const char * const vtx1G3BandNames[];
extern const char * const vtx1G3ChannelNames[];
extern const char * const vtx1G3DefaultPowerNames[];
extern const char vtx51G3BandLetter[];

bool vtx58_Freq2Bandchan(uint16_t freq, uint8_t *pBand, uint8_t *pChannel);
uint16_t vtx58_Bandchan2Freq(uint8_t band, uint8_t channel);
uint16_t vtx1G3_Bandchan2Freq(uint8_t band, uint8_t channel);
