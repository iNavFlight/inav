#pragma once

#include "drivers/display.h"

#include "common/time.h"

#include "cms/cms_types.h"

typedef enum {
    CMS_KEY_NONE,
    CMS_KEY_UP,
    CMS_KEY_DOWN,
    CMS_KEY_LEFT,
    CMS_KEY_RIGHT,
    CMS_KEY_ESC,
    CMS_KEY_MENU,
    CMS_KEY_SAVEMENU
} cms_key_e;


extern bool cmsInMenu;

// Device management
bool cmsDisplayPortRegister(displayPort_t *pDisplay);

// For main.c and scheduler
void cmsInit(void);
void cmsHandler(timeUs_t currentTimeUs);

bool cmsDisplayPortSelect(displayPort_t *instance);
displayPort_t *cmsDisplayPortGetCurrent(void);

void cmsMenuOpen(void);
long cmsMenuChange(displayPort_t *pPort, const CMS_Menu *menu, const OSD_Entry *from);
long cmsMenuExit(displayPort_t *pPort, const void *ptr);
void cmsYieldDisplay(displayPort_t *pPort, timeMs_t duration);
void cmsUpdate(uint32_t currentTimeUs);
void cmsSetExternKey(cms_key_e extKey);

#define CMS_STARTUP_HELP_TEXT1 "MENU: THR MID"
#define CMS_STARTUP_HELP_TEXT2     "+ YAW LEFT"
#define CMS_STARTUP_HELP_TEXT3     "+ PITCH UP"

// cmsMenuExit special ptr values
#define CMS_EXIT                (0)
#define CMS_EXIT_SAVE           (1)
#define CMS_EXIT_SAVEREBOOT     (2)
#define CMS_POPUP_SAVE          (3)
#define CMS_POPUP_SAVEREBOOT    (4)

