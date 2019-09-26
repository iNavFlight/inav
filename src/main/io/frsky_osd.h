#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "drivers/display.h"
#include "drivers/osd.h"

typedef enum {
    FRSKY_OSD_TRANSACTION_OPT_PROFILED = 1 << 0,
    FRSKY_OSD_TRANSACTION_OPT_RESET_DRAWING = 1 << 1,
} frskyOSDTransactionOptions_e;

typedef enum {
    FRSKY_OSD_COLOR_BLACK = 0,
    FRSKY_OSD_COLOR_TRANSPARENT = 1,
    FRSKY_OSD_COLOR_WHITE = 2,
    FRSKY_OSD_COLOR_GRAY = 3,
} frskyOSDColor_e;

typedef enum {
    FRSKY_OSD_OUTLINE_TYPE_NONE = 0,
    FRSKY_OSD_OUTLINE_TYPE_TOP = 1 << 0,
    FRSKY_OSD_OUTLINE_TYPE_RIGHT = 1 << 1,
    FRSKY_OSD_OUTLINE_TYPE_BOTTOM = 1 << 2,
    FRSKY_OSD_OUTLINE_TYPE_LEFT = 1 << 3,
} frskyOSDLineOutlineType_e;

bool frskyOSDInit(videoSystem_e videoSystem);
bool frskyOSDIsReady(void);
void frskyOSDUpdate(void);
void frskyOSDBeginTransaction(frskyOSDTransactionOptions_e opts);
void frskyOSDCommitTransaction(void);
void frskyOSDFlushSendBuffer(void);
bool frskyOSDReadFontCharacter(unsigned char_address, osdCharacter_t *chr);
bool frskyOSDWriteFontCharacter(unsigned char_address, const osdCharacter_t *chr);

unsigned frskyOSDGetGridRows(void);
unsigned frskyOSDGetGridCols(void);

unsigned frskyOSDGetPixelWidth(void);
unsigned frskyOSDGetPixelHeight(void);

void frskyOSDDrawStringInGrid(unsigned x, unsigned y, const char *buff, textAttributes_t attr);
void frskyOSDDrawCharInGrid(unsigned x, unsigned y, uint16_t chr, textAttributes_t attr);
bool frskyOSDReadCharInGrid(unsigned x, unsigned y, uint16_t *c, textAttributes_t *attr);
void frskyOSDClearScreen(void);

void frskyOSDSetStrokeColor(frskyOSDColor_e color);
void frskyOSDSetFillColor(frskyOSDColor_e color);
void frskyOSDSetStrokeAndFillColor(frskyOSDColor_e color);
void frskyOSDSetColorInversion(bool inverted);
void frskyOSDSetPixel(int x, int y, frskyOSDColor_e color);
void frskyOSDSetPixelToStrokeColor(int x, int y);
void frskyOSDSetPixelToFillColor(int x, int y);
void frskyOSDSetStrokeWidth(unsigned width);
void frskyOSDSetLineOutlineType(frskyOSDLineOutlineType_e outlineType);
void frskyOSDSetLineOutlineColor(frskyOSDColor_e outlineColor);

void frskyOSDClipToRect(int x, int y, int w, int h);
void frskyOSDClearRect(int x, int y, int w, int h);
void frskyOSDResetDrawingState(void);
void frskyOSDDrawCharacter(int x, int y, uint16_t chr, uint8_t opts);
void frskyOSDDrawCharacterMask(int x, int y, uint16_t chr, frskyOSDColor_e color, uint8_t opts);
void frskyOSDDrawString(int x, int y, const char *s, uint8_t opts);
void frskyOSDDrawStringMask(int x, int y, const char *s, frskyOSDColor_e color, uint8_t opts);
void frskyOSDMoveToPoint(int x, int y);
void frskyOSDStrokeLineToPoint(int x, int y);
void frskyOSDStrokeTriangle(int x1, int y1, int x2, int y2, int x3, int y3);
void frskyOSDFillTriangle(int x1, int y1, int x2, int y2, int x3, int y3);
void frskyOSDFillStrokeTriangle(int x1, int y1, int x2, int y2, int x3, int y3);
void frskyOSDStrokeRect(int x, int y, int w, int h);
void frskyOSDFillRect(int x, int y, int w, int h);
void frskyOSDFillStrokeRect(int x, int y, int w, int h);
void frskyOSDStrokeEllipseInRect(int x, int y, int w, int h);
void frskyOSDFillEllipseInRect(int x, int y, int w, int h);
void frskyOSDFillStrokeEllipseInRect(int x, int y, int w, int h);

void frskyOSDCtmReset(void);
void frskyOSDCtmSet(float m11, float m12, float m21, float m22, float m31, float m32);
void frskyOSDCtmTranslate(float tx, float ty);
void frskyOSDCtmScale(float sx, float sy);
void frskyOSDCtmRotate(float r);

void frskyOSDContextPush(void);
void frskyOSDContextPop(void);
