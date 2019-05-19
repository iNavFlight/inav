#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "drivers/display.h"
#include "drivers/osd.h"

bool frskyOSDInit(videoSystem_e videoSystem);
bool frskyOSDIsReady(void);
void frskyOSDUpdate(void);
bool frskyOSDReadFontCharacter(unsigned char_address, osdCharacter_t *chr);
bool frskyOSDWriteFontCharacter(unsigned char_address, const osdCharacter_t *chr);
unsigned frskyOSDGetGridRows(void);
unsigned frskyOSDGetGridCols(void);
void frskyOSDDrawStringInGrid(unsigned x, unsigned y, const char *buff, textAttributes_t attr);
void frskyOSDDrawCharInGrid(unsigned x, unsigned y, uint16_t chr, textAttributes_t attr);
void frskyOSDClearScreen(void);
