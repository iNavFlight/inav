#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "drivers/display.h"
#include "drivers/osd.h"

bool aghOSDInit(videoSystem_e videoSystem);
bool aghOSDIsReady(void);
void aghOSDUpdate(void);
bool aghOSDReadFontCharacter(unsigned char_address, osdCharacter_t *chr);
bool aghOSDWriteFontCharacter(unsigned char_address, const osdCharacter_t *chr);
unsigned aghOSDGetGridRows(void);
unsigned aghOSDGetGridCols(void);
void aghOSDDrawStringInGrid(unsigned x, unsigned y, const char *buff, textAttributes_t attr);
void aghOSDDrawCharInGrid(unsigned x, unsigned y, uint16_t chr, textAttributes_t attr);
void aghOSDClearScreen(void);
