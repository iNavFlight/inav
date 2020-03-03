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

/*
 Original OSD code created by Marcin Baliniak
 OSD-CMS separation by jflyper
 CMS-displayPort separation by jflyper and martinbudden
 */

//#define CMS_PAGE_DEBUG // For multi-page/menu debugging
//#define CMS_MENU_DEBUG // For external menu content creators
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "platform.h"

#ifdef USE_CMS

#include "build/build_config.h"
#include "build/debug.h"
#include "build/version.h"

#include "cms/cms.h"
#include "cms/cms_menu_builtin.h"
#include "cms/cms_menu_saveexit.h"
#include "cms/cms_menu_osd.h"
#include "cms/cms_types.h"

#include "common/maths.h"
#include "common/printf.h"
#include "common/typeconversion.h"
#include "common/utils.h"

#include "drivers/system.h"
#include "drivers/time.h"

// For rcData, stopAllMotors, stopPwmAllMotors
#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

// For 'ARM' related
#include "fc/fc_core.h"
#include "fc/config.h"
#include "fc/rc_controls.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "flight/mixer.h"
#include "flight/servos.h"

// For VISIBLE*
#include "io/osd.h"
#include "io/rcdevice_cam.h"

#include "rx/rx.h"

// DisplayPort management

#ifndef CMS_MAX_DEVICE
#define CMS_MAX_DEVICE 4
#endif

// Should be as big as the maximum number of rows displayed
// simultaneously in the tallest supported screen.
static uint8_t entry_flags[32];

#define IS_PRINTVALUE(p, row) (entry_flags[row] & PRINT_VALUE)
#define SET_PRINTVALUE(p, row) { entry_flags[row] |= PRINT_VALUE; }
#define CLR_PRINTVALUE(p, row) { entry_flags[row] &= ~PRINT_VALUE; }

#define IS_PRINTLABEL(p, row) (entry_flags[row] & PRINT_LABEL)
#define SET_PRINTLABEL(p, row) { entry_flags[row] |= PRINT_LABEL; }
#define CLR_PRINTLABEL(p, row) { entry_flags[row] &= ~PRINT_LABEL; }

#define IS_DYNAMIC(p)   ((p)->flags & DYNAMIC)
#define IS_READONLY(p)  ((p)->flags & READONLY)

#define SETTING_INVALID_VALUE_NAME "INVALID"

static displayPort_t *pCurrentDisplay;

static displayPort_t *cmsDisplayPorts[CMS_MAX_DEVICE];
static int cmsDeviceCount;
static int cmsCurrentDevice = -1;
static timeMs_t cmsYieldUntil = 0;

bool cmsDisplayPortRegister(displayPort_t *pDisplay)
{
    if (cmsDeviceCount == CMS_MAX_DEVICE)
        return false;

    cmsDisplayPorts[cmsDeviceCount++] = pDisplay;

    return true;
}

static displayPort_t *cmsDisplayPortSelectCurrent(void)
{
    if (cmsDeviceCount == 0)
        return NULL;

    if (cmsCurrentDevice < 0)
        cmsCurrentDevice = 0;

    return cmsDisplayPorts[cmsCurrentDevice];
}

static displayPort_t *cmsDisplayPortSelectNext(void)
{
    if (cmsDeviceCount == 0)
        return NULL;

    cmsCurrentDevice = (cmsCurrentDevice + 1) % cmsDeviceCount; // -1 Okay

    return cmsDisplayPorts[cmsCurrentDevice];
}

bool cmsDisplayPortSelect(displayPort_t *instance)
{
    if (cmsDeviceCount == 0) {
        return false;
    }
    for (int i = 0; i < cmsDeviceCount; i++) {
        if (cmsDisplayPortSelectNext() == instance) {
            return true;
        }
    }
    return false;
}

displayPort_t *cmsDisplayPortGetCurrent(void)
{
    return pCurrentDisplay;
}

#define CMS_UPDATE_INTERVAL_US  50000   // Interval of key scans (microsec)
#define CMS_POLL_INTERVAL_US   100000   // Interval of polling dynamic values (microsec)

// XXX LEFT_MENU_COLUMN and RIGHT_MENU_COLUMN must be adjusted
// dynamically depending on size of the active output device,
// or statically to accomodate sizes of all supported devices.
//
// Device characteristics
// OLED
//   21 cols x 8 rows
//     128x64 with 5x7 (6x8) : 21 cols x 8 rows
// MAX7456 (PAL)
//   30 cols x 16 rows
// MAX7456 (NTSC)
//   30 cols x 13 rows
// HoTT Telemetry Screen
//   21 cols x 8 rows
//

#define NORMAL_SCREEN_MIN_COLS 18      // Less is a small screen
static bool smallScreen;
static uint8_t leftMenuColumn;
static uint8_t rightMenuColumn;
static uint8_t maxMenuItems;
static uint8_t linesPerMenuItem;
static cms_key_e externKey = CMS_KEY_NONE;

bool cmsInMenu = false;

typedef struct cmsCtx_s {
    const CMS_Menu *menu;         // menu for this context
    uint8_t page;                 // page in the menu
    int8_t cursorRow;             // cursorRow in the page
} cmsCtx_t;

static cmsCtx_t menuStack[10];
static uint8_t menuStackIdx = 0;

static int8_t pageCount;            // Number of pages in the current menu
static const OSD_Entry *pageTop;    // First entry for the current page
static uint8_t pageMaxRow;          // Max row in the current page

static cmsCtx_t currentCtx;

#ifdef CMS_MENU_DEBUG // For external menu content creators

static char menuErrLabel[21 + 1] = "RANDOM DATA";

static const OSD_Entry menuErrEntries[] = {
    { "BROKEN MENU", OME_Label, NULL, NULL, 0 },
    { menuErrLabel, OME_Label, NULL, NULL, 0 },

    OSD_BACK_ENTRY,
    OSD_END_ENTRY,
};

static const CMS_Menu menuErr = {
    "MENUERR",
    OME_MENU,
    NULL,
    NULL,
    NULL,
    menuErrEntries,
};
#endif

#ifdef CMS_PAGE_DEBUG
#define cmsPageDebug() { \
    debug[0] = pageCount; \
    debug[1] = currentCtx.page; \
    debug[2] = pageMaxRow; \
    debug[3] = currentCtx.cursorRow; } struct _dummy
#else
#define cmsPageDebug()
#endif

static void cmsUpdateMaxRow(displayPort_t *instance)
{
    UNUSED(instance);
    pageMaxRow = 0;

    for (const OSD_Entry *ptr = pageTop; ptr->type != OME_END; ptr++) {
        pageMaxRow++;
        if (ptr->type == OME_BACK_AND_END) {
            break;
        }
    }

    if (pageMaxRow > maxMenuItems) {
        pageMaxRow = maxMenuItems;
    }

    pageMaxRow--;
}

static uint8_t cmsCursorAbsolute(displayPort_t *instance)
{
    UNUSED(instance);
    return currentCtx.cursorRow + currentCtx.page * maxMenuItems;
}

static void cmsPageSelect(displayPort_t *instance, int8_t newpage)
{
    currentCtx.page = (newpage + pageCount) % pageCount;
    pageTop = &currentCtx.menu->entries[currentCtx.page * maxMenuItems];
    cmsUpdateMaxRow(instance);
    displayClearScreen(instance);
}

static void cmsPageNext(displayPort_t *instance)
{
    cmsPageSelect(instance, currentCtx.page + 1);
}

static void cmsPagePrev(displayPort_t *instance)
{
    cmsPageSelect(instance, currentCtx.page - 1);
}

static bool cmsElementIsLabel(OSD_MenuElement element)
{
    return element == OME_Label || element == OME_LabelFunc;
}

static void cmsFormatFloat(int32_t value, char *floatString)
{
    uint8_t k;
    // np. 3450

    itoa(100000 + value, floatString, 10); // Create string from abs of integer value

    // 103450

    floatString[0] = floatString[1];
    floatString[1] = floatString[2];
    floatString[2] = '.';

    // 03.450
    // usuwam koncowe zera i kropke
    // Keep the first decimal place
    for (k = 5; k > 3; k--)
        if (floatString[k] == '0' || floatString[k] == '.')
            floatString[k] = 0;
        else
            break;

    // oraz zero wiodonce
    if (floatString[0] == '0')
        floatString[0] = ' ';
}

// Pad buffer to the left, i.e. align right
static void cmsPadLeftToSize(char *buf, int size)
{
    int i, j;
    int len = strlen(buf);

    for (i = size - 1, j = size - len; i - j >= 0; i--) {
        buf[i] = buf[i - j];
    }

    for (; i >= 0; i--) {
        buf[i] = ' ';
    }

    buf[size] = 0;
}

static void cmsPadToSize(char *buf, int size)
{
    // Make absolutely sure the string terminated.
    buf[size] = 0x00,

    cmsPadLeftToSize(buf, size);
}

static int cmsDrawMenuItemValue(displayPort_t *pDisplay, char *buff, uint8_t row, uint8_t maxSize)
{
    int colpos;
    int cnt;

    cmsPadToSize(buff, maxSize);
    colpos = rightMenuColumn - maxSize;
    cnt = displayWrite(pDisplay, colpos, row, buff);
    return cnt;
}

static int cmsDrawMenuEntry(displayPort_t *pDisplay, const OSD_Entry *p, uint8_t row, uint8_t screenRow)
{
#define CMS_DRAW_BUFFER_LEN 12
#define CMS_NUM_FIELD_LEN 5
#define CMS_CURSOR_BLINK_DELAY_MS 500

    char buff[CMS_DRAW_BUFFER_LEN + 1]; // Make room for null terminator.
    int cnt = 0;

    if (smallScreen) {
        row++;
    }

    switch (p->type) {
    case OME_String:
        if (IS_PRINTVALUE(p, screenRow) && p->data) {
            strncpy(buff, p->data, CMS_DRAW_BUFFER_LEN);
            cnt = cmsDrawMenuItemValue(pDisplay, buff, row, CMS_DRAW_BUFFER_LEN);
            CLR_PRINTVALUE(p, screenRow);
        }
        break;

    case OME_Submenu:
    case OME_Funcall:
        if (IS_PRINTVALUE(p, screenRow)) {
            buff[0] = 0x0;
            if ((p->type == OME_Submenu) && p->func && (p->flags & OPTSTRING)) {

                // Special case of sub menu entry with optional value display.
                char *str = p->menufunc();
                strncpy(buff, str, CMS_DRAW_BUFFER_LEN);
            }
            strncat(buff, ">", CMS_DRAW_BUFFER_LEN);

            row = smallScreen ? row - 1 : row;
            cnt = cmsDrawMenuItemValue(pDisplay, buff, row, strlen(buff));
            CLR_PRINTVALUE(p, screenRow);
        }
        break;

    case OME_Bool:
        if (IS_PRINTVALUE(p, screenRow) && p->data) {
            if (*((uint8_t *)(p->data))) {
                strcpy(buff, "YES");
            } else {
                strcpy(buff, "NO");
            }

            cnt = cmsDrawMenuItemValue(pDisplay, buff, row, 3);
            CLR_PRINTVALUE(p, screenRow);
        }
        break;

    case OME_BoolFunc:
        if (IS_PRINTVALUE(p, screenRow) && p->data) {
            bool (*func)(bool *arg) = p->data;
            if (func(NULL)) {
                strcpy(buff, "YES");
            } else {
                strcpy(buff, "NO");
            }

            cnt = cmsDrawMenuItemValue(pDisplay, buff, row, 3);
            CLR_PRINTVALUE(p, screenRow);
        }
        break;

    case OME_TAB:
        if (IS_PRINTVALUE(p, screenRow)) {
            const OSD_TAB_t *ptr = p->data;
            char * str = (char *)ptr->names[*ptr->val];
            strncpy(buff, str, CMS_DRAW_BUFFER_LEN);
            cnt = cmsDrawMenuItemValue(pDisplay, buff, row, CMS_DRAW_BUFFER_LEN);
            CLR_PRINTVALUE(p, screenRow);
        }
        break;

    case OME_UINT8:
        if (IS_PRINTVALUE(p, screenRow) && p->data) {
            const uint8_t *val;
            if (IS_READONLY(p)) {
                val = p->data;
            } else {
                const OSD_UINT8_t *ptr = p->data;
                val = ptr->val;
            }
            itoa(*val, buff, 10);
            cnt = cmsDrawMenuItemValue(pDisplay, buff, row, CMS_NUM_FIELD_LEN);
            CLR_PRINTVALUE(p, screenRow);
        }
        break;

    case OME_INT8:
        if (IS_PRINTVALUE(p, screenRow) && p->data) {
            const int8_t *val;
            if (IS_READONLY(p)) {
                val = p->data;
            } else {
                const OSD_INT8_t *ptr = p->data;
                val = ptr->val;
            }
            itoa(*val, buff, 10);
            cnt = cmsDrawMenuItemValue(pDisplay, buff, row, CMS_NUM_FIELD_LEN);
            CLR_PRINTVALUE(p, screenRow);
        }
        break;

    case OME_UINT16:
        if (IS_PRINTVALUE(p, screenRow) && p->data) {
            const uint16_t *val;
            if (IS_READONLY(p)) {
                val = p->data;
            } else {
                const OSD_UINT16_t *ptr = p->data;
                val = ptr->val;
            }
            itoa(*val, buff, 10);
            cnt = cmsDrawMenuItemValue(pDisplay, buff, row, CMS_NUM_FIELD_LEN);
            CLR_PRINTVALUE(p, screenRow);
        }
        break;

    case OME_INT16:
        if (IS_PRINTVALUE(p, screenRow) && p->data) {
            const int16_t *val;
            if (IS_READONLY(p)) {
                val = p->data;
            } else {
                const OSD_INT16_t *ptr = p->data;
                val = ptr->val;
            }
            itoa(*val, buff, 10);
            cnt = cmsDrawMenuItemValue(pDisplay, buff, row, CMS_NUM_FIELD_LEN);
            CLR_PRINTVALUE(p, screenRow);
        }
        break;

    case OME_FLOAT:
        if (IS_PRINTVALUE(p, screenRow) && p->data) {
            const OSD_FLOAT_t *ptr = p->data;
            cmsFormatFloat(*ptr->val * ptr->multipler, buff);
            cnt = cmsDrawMenuItemValue(pDisplay, buff, row, CMS_NUM_FIELD_LEN);
            CLR_PRINTVALUE(p, screenRow);
        }
        break;

    case OME_Setting:
        if (IS_PRINTVALUE(p, screenRow) && p->data) {
            uint8_t maxSize = CMS_NUM_FIELD_LEN;
            buff[0] = '\0';
            const OSD_SETTING_t *ptr = p->data;
            const setting_t *var = settingGet(ptr->val);
            int32_t value = 0;
            const void *valuePointer = settingGetValuePointer(var);
            switch (SETTING_TYPE(var)) {
                case VAR_UINT8:
                    value = *(uint8_t *)valuePointer;
                    break;
                case VAR_INT8:
                    value = *(int8_t *)valuePointer;
                    break;
                case VAR_UINT16:
                    value = *(uint16_t *)valuePointer;
                    break;
                case VAR_INT16:
                    value = *(int16_t *)valuePointer;
                    break;
                case VAR_UINT32:
                    value = *(uint32_t *)valuePointer;
                    break;
                case VAR_FLOAT:
                    // XXX: This bypasses the data types. However, we
                    // don't have any VAR_FLOAT settings which require
                    // a data type yet.
                    ftoa(*(float *)valuePointer, buff);
                    break;
                case VAR_STRING:
                    strncpy(buff, valuePointer, sizeof(buff));
                    break;
            }
            if (buff[0] == '\0') {
                const char *suffix = NULL;
                switch (CMS_DATA_TYPE(p)) {
                    case CMS_DATA_TYPE_ANGULAR_RATE:
                        // Setting is in degrees/10 per second
                        value *= 10;
                        suffix = " DPS";
                        break;
                }
                switch (SETTING_MODE(var)) {
                    case MODE_DIRECT:
                        if (SETTING_TYPE(var) == VAR_UINT32) {
                            tfp_sprintf(buff, "%u", (unsigned)value);
                        } else {
                            tfp_sprintf(buff, "%d", (int)value);
                        }
                        break;
                    case MODE_LOOKUP:
                        {
                            const char *str = settingLookupValueName(var, value);
                            strncpy(buff, str ? str : SETTING_INVALID_VALUE_NAME, sizeof(buff) - 1);
                            maxSize = MAX(settingGetValueNameMaxSize(var), strlen(SETTING_INVALID_VALUE_NAME));
                        }
                        break;
                }
                if (suffix) {
                    strcat(buff, suffix);
                }
            }
            cnt = cmsDrawMenuItemValue(pDisplay, buff, row, maxSize);
            CLR_PRINTVALUE(p, screenRow);
        }
        break;

    case OME_Label:
    case OME_LabelFunc:
        if (IS_PRINTVALUE(p, screenRow)) {
            // A label with optional string, immediately following text
            const char *text = p->data;
            if (p->type == OME_LabelFunc) {
                // Label is generated by a function
                bool (*label_func)(char *buf, unsigned size) = p->data;
                if (label_func(buff, sizeof(buff))) {
                    text = buff;
                } else {
                    text = NULL;
                }
            }
            if (text) {
                cnt = displayWrite(pDisplay,
                        leftMenuColumn + 1 + (uint8_t) strlen(p->text), row, text);
            }
            CLR_PRINTVALUE(p, screenRow);
        }
        break;

    case OME_OSD_Exit:
    case OME_END:
    case OME_Back:
    case OME_BACK_AND_END:
        break;

    case OME_MENU:
        // Fall through
    default:
#ifdef CMS_MENU_DEBUG
        // Shouldn't happen. Notify creator of this menu content.
        cnt = displayWrite(pDisplay, rightMenuColumn - 6), row, "BADENT");
#endif
        break;
    }

    return cnt;
}

static void cmsDrawMenu(displayPort_t *pDisplay, uint32_t currentTimeUs)
{
    if (!pageTop)
        return;

    uint8_t i;
    const OSD_Entry *p;
    uint8_t top = smallScreen ? 1 : (pDisplay->rows - pageMaxRow) / 2;

    // Polled (dynamic) value display denominator.

    bool drawPolled = false;
    static uint32_t lastPolledUs = 0;

    if (currentTimeUs > lastPolledUs + CMS_POLL_INTERVAL_US) {
        drawPolled = true;
        lastPolledUs = currentTimeUs;
    }

    uint32_t room = displayTxBytesFree(pDisplay);

    if (pDisplay->cleared) {
        // Mark all labels and values for printing
        memset(entry_flags, PRINT_LABEL | PRINT_VALUE, sizeof(entry_flags));
        pDisplay->cleared = false;
    } else if (drawPolled) {
        for (p = pageTop, i = 0; p <= pageTop + pageMaxRow; p++, i++) {
            if (IS_DYNAMIC(p))
                SET_PRINTVALUE(p, i);
        }
    }

    // Cursor manipulation

    while (cmsElementIsLabel((pageTop + currentCtx.cursorRow)->type)) // skip label
        currentCtx.cursorRow++;

    cmsPageDebug();

    if (pDisplay->cursorRow >= 0 && currentCtx.cursorRow != pDisplay->cursorRow) {
        room -= displayWrite(pDisplay, leftMenuColumn, top + pDisplay->cursorRow * linesPerMenuItem, " ");
    }

    if (room < 30)
        return;

    if (pDisplay->cursorRow != currentCtx.cursorRow) {
        room -= displayWrite(pDisplay, leftMenuColumn, top + currentCtx.cursorRow * linesPerMenuItem, ">");
        pDisplay->cursorRow = currentCtx.cursorRow;
    }

    if (room < 30)
        return;

    // Print text labels
    for (i = 0, p = pageTop; i < maxMenuItems && p->type != OME_END; i++, p++) {
        if (IS_PRINTLABEL(p, i)) {
            uint8_t coloff = leftMenuColumn;
            coloff += cmsElementIsLabel(p->type) ? 0 : 1;
            room -= displayWrite(pDisplay, coloff, top + i * linesPerMenuItem, p->text);
            CLR_PRINTLABEL(p, i);
            if (room < 30) {
                return;
            }
        }
        if (p->type == OME_BACK_AND_END) {
            break;
        }
    }
    // Print values

    // XXX Polled values at latter positions in the list may not be
    // XXX printed if not enough room in the middle of the list.
    for (i = 0, p = pageTop; i < maxMenuItems && p->type != OME_END; i++, p++) {
        if (IS_PRINTVALUE(p, i)) {
            room -= cmsDrawMenuEntry(pDisplay, p, top + i * linesPerMenuItem, i);
            if (room < 30) {
                return;
            }
        }
        if (p->type == OME_BACK_AND_END) {
            break;
        }
    }
}

static void cmsMenuCountPage(displayPort_t *pDisplay)
{
    UNUSED(pDisplay);
    const OSD_Entry *p;
    for (p = currentCtx.menu->entries; p->type != OME_END; p++) {
        if (p->type == OME_BACK_AND_END) {
            p++;
            break;
        }
    }
    pageCount = (p - currentCtx.menu->entries - 1) / maxMenuItems + 1;
}

STATIC_UNIT_TESTED long cmsMenuBack(displayPort_t *pDisplay); // Forward; will be resolved after merging

long cmsMenuChange(displayPort_t *pDisplay, const CMS_Menu *pMenu, const OSD_Entry *from)
{
    if (!pMenu) {
        return 0;
    }

#ifdef CMS_MENU_DEBUG
    if (pMenu->GUARD_type != OME_MENU) {
        // ptr isn't pointing to a CMS_Menu.
        if (pMenu->GUARD_type <= OME_MAX) {
            strncpy(menuErrLabel, pMenu->GUARD_text, sizeof(menuErrLabel) - 1);
        } else {
            strncpy(menuErrLabel, "LABEL UNKNOWN", sizeof(menuErrLabel) - 1);
        }
        pMenu = &menuErr;
    }
#endif

    if (pMenu != currentCtx.menu) {
        // Stack the current menu and move to a new menu.

        menuStack[menuStackIdx++] = currentCtx;

        currentCtx.menu = pMenu;
        currentCtx.cursorRow = 0;

        if (pMenu->onEnter && (pMenu->onEnter(from) == MENU_CHAIN_BACK)) {
            return cmsMenuBack(pDisplay);
        }

        cmsMenuCountPage(pDisplay);
        cmsPageSelect(pDisplay, 0);
    } else {
        // The (pMenu == curretMenu) case occurs when reopening for display cycling
        // currentCtx.cursorRow has been saved as absolute; convert it back to page + relative

        int8_t cursorAbs = currentCtx.cursorRow;
        currentCtx.cursorRow = cursorAbs % maxMenuItems;
        cmsMenuCountPage(pDisplay);
        cmsPageSelect(pDisplay, cursorAbs / maxMenuItems);
    }

    cmsPageDebug();

    return 0;
}

STATIC_UNIT_TESTED long cmsMenuBack(displayPort_t *pDisplay)
{
    // Let onExit function decide whether to allow exit or not.

    if (currentCtx.menu->onExit && currentCtx.menu->onExit(pageTop + currentCtx.cursorRow) < 0) {
        return -1;
    }

    if (!menuStackIdx) {
        return 0;
    }

    currentCtx = menuStack[--menuStackIdx];

    cmsMenuCountPage(pDisplay);
    cmsPageSelect(pDisplay, currentCtx.page);

    cmsPageDebug();

    return 0;
}

void cmsMenuOpen(void)
{
    if (!cmsInMenu) {
        // New open
	setServoOutputEnabled(false);
        pCurrentDisplay = cmsDisplayPortSelectCurrent();
        if (!pCurrentDisplay)
            return;
        cmsInMenu = true;
        currentCtx = (cmsCtx_t){ &menuMain, 0, 0 };
        ENABLE_ARMING_FLAG(ARMING_DISABLED_CMS_MENU);
    } else {
        // Switch display
        displayPort_t *pNextDisplay = cmsDisplayPortSelectNext();
        if (pNextDisplay != pCurrentDisplay) {
            // DisplayPort has been changed.
            // Convert cursorRow to absolute value
            currentCtx.cursorRow = cmsCursorAbsolute(pCurrentDisplay);
            displayRelease(pCurrentDisplay);
            pCurrentDisplay = pNextDisplay;
        } else {
            return;
        }
    }
    displayGrab(pCurrentDisplay); // grab the display for use by the CMS

    if (pCurrentDisplay->cols < NORMAL_SCREEN_MIN_COLS) {
        smallScreen = true;
        linesPerMenuItem = 2;
        leftMenuColumn = 0;
        rightMenuColumn = pCurrentDisplay->cols;
        maxMenuItems = (pCurrentDisplay->rows) / linesPerMenuItem;
    } else {
        smallScreen = false;
        linesPerMenuItem = 1;
        leftMenuColumn = 2;
        rightMenuColumn = pCurrentDisplay->cols - 2;
        maxMenuItems = pCurrentDisplay->rows - 2;
    }

    if (pCurrentDisplay->useFullscreen) {
        leftMenuColumn = 0;
        rightMenuColumn = pCurrentDisplay->cols;
        maxMenuItems = pCurrentDisplay->rows;
    }

    cmsMenuChange(pCurrentDisplay, currentCtx.menu, NULL);
}

static void cmsTraverseGlobalExit(const CMS_Menu *pMenu)
{
    for (const OSD_Entry *p = pMenu->entries; p->type != OME_END; p++) {
        if (p->type == OME_Submenu) {
            cmsTraverseGlobalExit(p->data);
        }
        if (p->type == OME_BACK_AND_END) {
            break;
        }
    }

    if (pMenu->onGlobalExit) {
        pMenu->onGlobalExit(NULL);
    }
}

long cmsMenuExit(displayPort_t *pDisplay, const void *ptr)
{
    int exitType = (int)ptr;
    switch (exitType) {
    case CMS_EXIT_SAVE:
    case CMS_EXIT_SAVEREBOOT:
    case CMS_POPUP_SAVE:
    case CMS_POPUP_SAVEREBOOT:

        cmsTraverseGlobalExit(&menuMain);

        if (currentCtx.menu->onExit)
            currentCtx.menu->onExit((OSD_Entry *)NULL); // Forced exit

        if ((exitType == CMS_POPUP_SAVE) || (exitType == CMS_POPUP_SAVEREBOOT)) {
            // traverse through the menu stack and call their onExit functions
            for (int i = menuStackIdx - 1; i >= 0; i--) {
                if (menuStack[i].menu->onExit) {
                    menuStack[i].menu->onExit((OSD_Entry *) NULL);
                }
            }
        }

        saveConfigAndNotify();
        break;

    case CMS_EXIT:
        break;
    }

    cmsInMenu = false;

    displayRelease(pDisplay);
    currentCtx.menu = NULL;

    setServoOutputEnabled(true);

    if ((exitType == CMS_EXIT_SAVEREBOOT) || (exitType == CMS_POPUP_SAVEREBOOT)) {
        displayClearScreen(pDisplay);
        displayWrite(pDisplay, 5, 3, "REBOOTING...");

        displayResync(pDisplay); // Was max7456RefreshAll(); why at this timing?

        fcReboot(false);
    }

    DISABLE_ARMING_FLAG(ARMING_DISABLED_CMS_MENU);

    return 0;
}

void cmsYieldDisplay(displayPort_t *pPort, timeMs_t duration)
{
    // Check if we're already yielding, in that case just extend
    // the yield time without releasing the display again, otherwise
    // the yield/grab become unbalanced.
    if (cmsYieldUntil == 0) {
        displayRelease(pPort);
    }
    cmsYieldUntil = millis() + duration;
}

// Stick/key detection and key codes

#define IS_HI(X)  (rxGetChannelValue(X) > 1750)
#define IS_LO(X)  (rxGetChannelValue(X) < 1250)
#define IS_MID(X) (rxGetChannelValue(X) > 1250 && rxGetChannelValue(X) < 1750)

#define BUTTON_TIME   250 // msec
#define BUTTON_PAUSE  500 // msec

STATIC_UNIT_TESTED uint16_t cmsHandleKey(displayPort_t *pDisplay, uint8_t key)
{
    uint16_t res = BUTTON_TIME;
    const OSD_Entry *p;

    if (!currentCtx.menu)
        return res;

    if (key == CMS_KEY_MENU) {
        cmsMenuOpen();
        return BUTTON_PAUSE;
    }

    if (key == CMS_KEY_ESC) {
        cmsMenuBack(pDisplay);
        return BUTTON_PAUSE;
    }

    if (key == CMS_KEY_SAVEMENU) {
        cmsMenuChange(pDisplay, &cmsx_menuSaveExit, NULL);
        return BUTTON_PAUSE;
    }

    if (key == CMS_KEY_DOWN) {
        if (currentCtx.cursorRow < pageMaxRow) {
            currentCtx.cursorRow++;
        } else {
            cmsPageNext(pDisplay);
            currentCtx.cursorRow = 0;    // Goto top in any case
        }
    }

    if (key == CMS_KEY_UP) {
        currentCtx.cursorRow--;

        // Skip non-title labels
        if (cmsElementIsLabel((pageTop + currentCtx.cursorRow)->type) && currentCtx.cursorRow > 0)
            currentCtx.cursorRow--;

        if (currentCtx.cursorRow == -1 || cmsElementIsLabel((pageTop + currentCtx.cursorRow)->type)) {
            // Goto previous page
            cmsPagePrev(pDisplay);
            currentCtx.cursorRow = pageMaxRow;
        }
    }

    if (key == CMS_KEY_DOWN || key == CMS_KEY_UP)
        return res;

    p = pageTop + currentCtx.cursorRow;

    switch (p->type) {
        case OME_Submenu:
            if (key == CMS_KEY_RIGHT) {
                cmsMenuChange(pDisplay, p->data, p);
                res = BUTTON_PAUSE;
            }
            break;

        case OME_Funcall:
            if (p->func && key == CMS_KEY_RIGHT) {
                long retval = p->func(pDisplay, p->data);
                if (retval == MENU_CHAIN_BACK)
                    cmsMenuBack(pDisplay);
                res = BUTTON_PAUSE;
            }
            break;

        case OME_OSD_Exit:
            if (p->func && key == CMS_KEY_RIGHT) {
                p->func(pDisplay, p->data);
                res = BUTTON_PAUSE;
            }
            break;

        case OME_Back:
        case OME_BACK_AND_END:
            cmsMenuBack(pDisplay);
            res = BUTTON_PAUSE;
            break;

        case OME_Bool:
            if (p->data) {
                uint8_t *val = (uint8_t *)p->data;
                if (key == CMS_KEY_RIGHT)
                    *val = 1;
                else
                    *val = 0;
                SET_PRINTVALUE(p, currentCtx.cursorRow);
                if (p->func) {
                    p->func(pDisplay, p);
                }
            }
            break;

        case OME_BoolFunc:
            if (p->data) {
                bool (*func)(bool *arg) = p->data;
                bool val = key == CMS_KEY_RIGHT;
                func(&val);
                SET_PRINTVALUE(p, currentCtx.cursorRow);
            }
            break;

        case OME_UINT8:
        case OME_FLOAT:
            if (IS_READONLY(p)) {
                break;
            }
            if (p->data) {
                const OSD_UINT8_t *ptr = p->data;
                if (key == CMS_KEY_RIGHT) {
                    if (*ptr->val < ptr->max)
                        *ptr->val += ptr->step;
                } else {
                    if (*ptr->val > ptr->min)
                        *ptr->val -= ptr->step;
                }
                SET_PRINTVALUE(p, currentCtx.cursorRow);
                if (p->func) {
                    p->func(pDisplay, p);
                }
            }
            break;

        case OME_TAB:
            if (p->type == OME_TAB) {
                const OSD_TAB_t *ptr = p->data;

                if (key == CMS_KEY_RIGHT) {
                    if (*ptr->val < ptr->max)
                        *ptr->val += 1;
                } else {
                    if (*ptr->val > 0)
                        *ptr->val -= 1;
                }
                if (p->func) {
                    p->func(pDisplay, p->data);
                }
                SET_PRINTVALUE(p, currentCtx.cursorRow);
            }
            break;

        case OME_INT8:
            if (IS_READONLY(p)) {
                break;
            }
            if (p->data) {
                const OSD_INT8_t *ptr = p->data;
                if (key == CMS_KEY_RIGHT) {
                    if (*ptr->val < ptr->max)
                        *ptr->val += ptr->step;
                } else {
                    if (*ptr->val > ptr->min)
                        *ptr->val -= ptr->step;
                }
                SET_PRINTVALUE(p, currentCtx.cursorRow);
                if (p->func) {
                    p->func(pDisplay, p);
                }
            }
            break;

        case OME_UINT16:
            if (IS_READONLY(p)) {
                break;
            }
            if (p->data) {
                const OSD_UINT16_t *ptr = p->data;
                if (key == CMS_KEY_RIGHT) {
                    if (*ptr->val < ptr->max)
                        *ptr->val += ptr->step;
                } else {
                    if (*ptr->val > ptr->min)
                        *ptr->val -= ptr->step;
                }
                SET_PRINTVALUE(p, currentCtx.cursorRow);
                if (p->func) {
                    p->func(pDisplay, p);
                }
            }
            break;

        case OME_INT16:
            if (IS_READONLY(p)) {
                break;
            }
            if (p->data) {
                const OSD_INT16_t *ptr = p->data;
                if (key == CMS_KEY_RIGHT) {
                    if (*ptr->val < ptr->max)
                        *ptr->val += ptr->step;
                } else {
                    if (*ptr->val > ptr->min)
                        *ptr->val -= ptr->step;
                }
                SET_PRINTVALUE(p, currentCtx.cursorRow);
                if (p->func) {
                    p->func(pDisplay, p);
                }
            }
            break;

        case OME_Setting:
            if (p->data) {
                const OSD_SETTING_t *ptr = p->data;
                const setting_t *var = settingGet(ptr->val);
                setting_min_t min = settingGetMin(var);
                setting_max_t max = settingGetMax(var);
                float step = ptr->step ?: 1;
                if (key != CMS_KEY_RIGHT) {
                    step = -step;
                }
                const void *valuePointer = settingGetValuePointer(var);
                switch (SETTING_TYPE(var)) {
                    case VAR_UINT8:
                        {
                            uint8_t val = *(uint8_t *)valuePointer;
                            val = MIN(MAX(val + step, (uint8_t)min), (uint8_t)max);
                            *(uint8_t *)valuePointer = val;
                            break;
                        }
                    case VAR_INT8:
                        {
                            int8_t val = *(int8_t *)valuePointer;
                            val = MIN(MAX(val + step, (int8_t)min), (int8_t)max);
                            *(int8_t *)valuePointer = val;
                            break;
                        }
                    case VAR_UINT16:
                        {
                            uint16_t val = *(uint16_t *)valuePointer;
                            val = MIN(MAX(val + step, (uint16_t)min), (uint16_t)max);
                            *(uint16_t *)valuePointer = val;
                            break;
                        }
                    case VAR_INT16:
                        {
                            int16_t val = *(int16_t *)valuePointer;
                            val = MIN(MAX(val + step, (int16_t)min), (int16_t)max);
                            *(int16_t *)valuePointer = val;
                            break;
                        }
                    case VAR_UINT32:
                        {
                            uint32_t val = *(uint32_t *)valuePointer;
                            val = MIN(MAX(val + step, (uint32_t)min), (uint32_t)max);
                            *(uint32_t *)valuePointer = val;
                            break;
                        }
                    case VAR_FLOAT:
                        {
                            float val = *(float *)valuePointer;
                            val = MIN(MAX(val + step, (float)min), (float)max);
                            *(float *)valuePointer = val;
                            break;
                        }
                    case VAR_STRING:
                        break;
                }
                SET_PRINTVALUE(p, currentCtx.cursorRow);
                if (p->func) {
                    p->func(pDisplay, p);
                }
            }
            break;

        case OME_String:
            break;

        case OME_Label:
        case OME_LabelFunc:
        case OME_END:
            break;

        case OME_MENU:
            // Shouldn't happen
            break;
    }
    return res;
}

void cmsSetExternKey(cms_key_e extKey)
{
    if (externKey == CMS_KEY_NONE)
        externKey = extKey;
}

uint16_t cmsHandleKeyWithRepeat(displayPort_t *pDisplay, uint8_t key,
int repeatCount)
{
    uint16_t ret = 0;

    for (int i = 0; i < repeatCount; i++) {
        ret = cmsHandleKey(pDisplay, key);
    }

    return ret;
}

static uint16_t cmsScanKeys(timeMs_t currentTimeMs, timeMs_t lastCalledMs, int16_t rcDelayMs)
{
    static int holdCount = 1;
    static int repeatCount = 1;
    static int repeatBase = 0;

    //
    // Scan 'key' first
    //

    uint8_t key = CMS_KEY_NONE;

    if (externKey != CMS_KEY_NONE) {
        rcDelayMs = cmsHandleKey(pCurrentDisplay, externKey);
        externKey = CMS_KEY_NONE;
    } else {
        if (IS_MID(THROTTLE) && IS_LO(YAW) && IS_HI(PITCH) && !ARMING_FLAG(ARMED)) {
            key = CMS_KEY_MENU;
        } else if (IS_HI(PITCH)) {
            key = CMS_KEY_UP;
        } else if (IS_LO(PITCH)) {
            key = CMS_KEY_DOWN;
        } else if (IS_LO(ROLL)) {
            key = CMS_KEY_LEFT;
        } else if (IS_HI(ROLL)) {
            key = CMS_KEY_RIGHT;
        } else if (IS_LO(YAW)) {
            key = CMS_KEY_ESC;
        } else if (IS_HI(YAW)) {
            key = CMS_KEY_SAVEMENU;
        }

        if (key == CMS_KEY_NONE) {
            // No 'key' pressed, reset repeat control
            holdCount = 1;
            repeatCount = 1;
            repeatBase = 0;
        } else {
            // The 'key' is being pressed; keep counting
            ++holdCount;
        }

        if (rcDelayMs > 0) {
            rcDelayMs -= (currentTimeMs - lastCalledMs);
        } else if (key) {
            rcDelayMs = cmsHandleKeyWithRepeat(pCurrentDisplay, key,
            repeatCount);

            // Key repeat effect is implemented in two phases.
            // First phldase is to decrease rcDelayMs reciprocal to hold time.
            // When rcDelayMs reached a certain limit (scheduling interval),
            // repeat rate will not raise anymore, so we call key handler
            // multiple times (repeatCount).
            //
            // XXX Caveat: Most constants are adjusted pragmatically.
            // XXX Rewrite this someday, so it uses actual hold time instead
            // of holdCount, which depends on the scheduling interval.

            if (((key == CMS_KEY_LEFT) || (key == CMS_KEY_RIGHT)) && (holdCount > 20)) {

                // Decrease rcDelayMs reciprocally

                rcDelayMs /= (holdCount - 20);

                // When we reach the scheduling limit,

                if (rcDelayMs <= 50) {

                    // start calling handler multiple times.

                    if (repeatBase == 0)
                    repeatBase = holdCount;

                    if (holdCount < 100) {
                        repeatCount = repeatCount
                        + (holdCount - repeatBase) / 5;

                        if (repeatCount > 5) {
                            repeatCount = 5;
                        }
                    } else {
                        repeatCount = repeatCount + holdCount - repeatBase;

                        if (repeatCount > 50) {
                            repeatCount = 50;
                        }
                    }
                }
            }
        }
    }
    return rcDelayMs;
}

void cmsUpdate(uint32_t currentTimeUs)
{
#ifdef USE_RCDEVICE
    if(rcdeviceInMenu) {
        return ;
    }
#endif

    static int16_t rcDelayMs = BUTTON_TIME;

    static timeMs_t lastCalledMs = 0;
    static uint32_t lastCmsHeartBeatMs = 0;

    const timeMs_t currentTimeMs = currentTimeUs / 1000;

    if (!cmsInMenu) {
        // Detect menu invocation
        if (IS_MID(THROTTLE) && IS_LO(YAW) && IS_HI(PITCH) && !ARMING_FLAG(ARMED)) {
            cmsMenuOpen();
            rcDelayMs = BUTTON_PAUSE;    // Tends to overshoot if BUTTON_TIME
        }
    } else {

        // Check if we're yielding and its's time to stop it
        if (cmsYieldUntil > 0 && currentTimeMs > cmsYieldUntil) {
            cmsYieldUntil = 0;
            displayGrab(pCurrentDisplay);
            displayClearScreen(pCurrentDisplay);
        }

        // Only scan keys and draw if we're not yielding
        if (cmsYieldUntil == 0) {
            // XXX: Note that one call to cmsScanKeys() might generate multiple keypresses
            // when repeating, that's why cmsYieldDisplay() has to check for multiple calls.
            rcDelayMs = cmsScanKeys(currentTimeMs, lastCalledMs, rcDelayMs);
            // Check again, the keypress might have produced a yield
            if (cmsYieldUntil == 0) {
                cmsDrawMenu(pCurrentDisplay, currentTimeUs);
            }
        }

        if (currentTimeMs > lastCmsHeartBeatMs + 500) {
            // Heart beat for external CMS display device @ 500msec
            // (Timeout @ 1000msec)
            displayHeartbeat(pCurrentDisplay);
            lastCmsHeartBeatMs = currentTimeMs;
        }
    }

    // Some key (command), notably flash erase, takes too long to use the
    // currentTimeMs to be used as lastCalledMs (freezes CMS for a minute or so
    // if used).
    lastCalledMs = millis();
}

void cmsHandler(timeUs_t currentTimeUs)
{
    if (cmsDeviceCount < 0)
        return;

    static timeUs_t lastCalledUs = 0;

    if (currentTimeUs >= lastCalledUs + CMS_UPDATE_INTERVAL_US) {
        lastCalledUs = currentTimeUs;
        cmsUpdate(currentTimeUs);
    }
}

void cmsInit(void)
{
    cmsDeviceCount = 0;
    cmsCurrentDevice = -1;
}

#endif // CMS
