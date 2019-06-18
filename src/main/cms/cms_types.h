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

//
// Menu element types
// XXX Upon separation, all OME would be renamed to CME_ or similar.
//

#pragma once

#include <stdint.h>

//type of elements

typedef enum
{
    OME_Label,
    OME_LabelFunc, // bool func(char *buf, unsigned bufsize) - returns wether buf should be printed
    OME_Back,
    OME_OSD_Exit,
    OME_Submenu,
    OME_Funcall,
    OME_Bool,
    OME_BoolFunc, // bool func(bool*):
    OME_INT8,
    OME_UINT8,
    OME_UINT16,
    OME_INT16,
    OME_String,
    OME_FLOAT, //only up to 255 value and cant be 2.55 or 25.5, just for PID's
    OME_Setting,
    //wlasciwosci elementow
    OME_TAB,
    OME_END,
    OME_BACK_AND_END,

    // Debug aid
    OME_MENU,

    OME_MAX = OME_MENU
} OSD_MenuElement;

typedef long (*CMSEntryFuncPtr)(displayPort_t *displayPort, const void *ptr);
// This is a function used in the func member if the type is OME_Submenu.
typedef char * (*CMSMenuOptFuncPtr)(void);

typedef struct
{
    const char * const text;
    union
    {
        const CMSEntryFuncPtr func;
        const CMSMenuOptFuncPtr menufunc;
        int itemId;
    };
    const void * const data;
    const uint8_t type; // from OSD_MenuElement
    uint8_t flags;
} __attribute__((packed)) OSD_Entry;

// Bits in flags
#define PRINT_VALUE    (1 << 0)  // Value has been changed, need to redraw
#define PRINT_LABEL    (1 << 1)  // Text label should be printed
#define DYNAMIC        (1 << 2)  // Value should be updated dynamically
#define OPTSTRING      (1 << 3)  // (Temporary) Flag for OME_Submenu, indicating func should be called to get a string to display.
#define READONLY       (1 << 4)  // Indicates that the value is read-only and p->data points directly to it - applies to [U]INT(8|16)

#define OSD_LABEL_ENTRY(label)                  ((OSD_Entry){ label, {.func = NULL}, NULL, OME_Label, 0 })
#define OSD_LABEL_DATA_ENTRY(label, data)       ((OSD_Entry){ label, {.func = NULL}, data, OME_Label, 0 })
#define OSD_LABEL_DATA_DYN_ENTRY(label, data)   ((OSD_Entry){ label, {.func = NULL}, data, OME_Label, DYNAMIC })
#define OSD_LABEL_FUNC_DYN_ENTRY(label, fn)     ((OSD_Entry){ label, {.func = NULL}, fn, OME_LabelFunc, DYNAMIC })
#define OSD_BACK_ENTRY                          ((OSD_Entry){ "BACK", {.func = NULL}, NULL, OME_Back, 0 })
#define OSD_BACK_AND_END_ENTRY                  ((OSD_Entry){ "BACK", {.func = NULL}, NULL, OME_BACK_AND_END, 0 })
#define OSD_SUBMENU_ENTRY(label, menu)          ((OSD_Entry){ label, {.func = NULL}, menu, OME_Submenu, 0 })
#define OSD_FUNC_CALL_ENTRY(label, fn)          ((OSD_Entry){ label, {.func = fn}, NULL, OME_Funcall, 0 })
#define OSD_BOOL_ENTRY(label, val)              ((OSD_Entry){ label, {.func = NULL}, val, OME_Bool, 0 })
#define OSD_BOOL_CALLBACK_ENTRY(label, cb, val) ((OSD_Entry){ label, {.func = cb}, val, OME_Bool, 0 })
#define OSD_BOOL_FUNC_ENTRY(label, fn)          ((OSD_Entry){ label, {.func = NULL}, fn, OME_BoolFunc, 0 })
#define OSD_INT8_DYN_ENTRY(label, val)          ((OSD_Entry){ label, {.func = NULL}, val, OME_INT8, DYNAMIC })
#define OSD_UINT8_ENTRY(label, val)             ((OSD_Entry){ label, {.func = NULL}, val, OME_UINT8, 0 })
#define OSD_UINT8_DYN_ENTRY(label, val)         ((OSD_Entry){ label, {.func = NULL}, val, OME_UINT8, DYNAMIC })
#define OSD_UINT8_CALLBACK_ENTRY(label, cb, val)((OSD_Entry){ label, {.func = cb}, val, OME_UINT8, 0 })
#define OSD_UINT16_ENTRY(label, val)            ((OSD_Entry){ label, {.func = NULL}, val, OME_UINT16, 0 })
#define OSD_UINT16_DYN_ENTRY(label, val)        ((OSD_Entry){ label, {.func = NULL}, val, OME_UINT16, DYNAMIC })
#define OSD_UINT16_RO_ENTRY(label, val)         ((OSD_Entry){ label, {.func = NULL}, val, OME_UINT16, DYNAMIC | READONLY })
#define OSD_INT16_ENTRY(label, val)             ((OSD_Entry){ label, {.func = NULL}, val, OME_INT16, 0 })
#define OSD_INT16_DYN_ENTRY(label, val)         ((OSD_Entry){ label, {.func = NULL}, val, OME_INT16, DYNAMIC })
#define OSD_INT16_RO_ENTRY(label, val)          ((OSD_Entry){ label, {.func = NULL}, val, OME_INT16, DYNAMIC | READONLY })
#define OSD_STRING_ENTRY(label, str)            ((OSD_Entry){ label, {.func = NULL}, str, OME_String, 0 })
#define OSD_TAB_ENTRY(label, val)               ((OSD_Entry){ label, {.func = NULL}, val, OME_TAB, 0 })
#define OSD_TAB_DYN_ENTRY(label, val)           ((OSD_Entry){ label, {.func = NULL}, val, OME_TAB, DYNAMIC })
#define OSD_TAB_CALLBACK_ENTRY(label, cb, val)  ((OSD_Entry){ label, {.func = cb}, val, OME_TAB, 0 })

#define OSD_END_ENTRY                           ((OSD_Entry){ NULL, {.func = NULL}, NULL, OME_END, 0 })

// Data type for OME_Setting. Uses upper 4 bits
// of flags, leaving 16 data types.
#define CMS_DATA_TYPE_OFFSET (4)
typedef enum {
    CMS_DATA_TYPE_ANGULAR_RATE = (1 << CMS_DATA_TYPE_OFFSET),
} CMSDataType_e;

// Use a function and data type to make sure switches are exhaustive
static inline CMSDataType_e CMS_DATA_TYPE(const OSD_Entry *entry) { return entry->flags & 0xF0; }

typedef long (*CMSMenuFuncPtr)(const OSD_Entry *from);

// Special return value(s) for function chaining by CMSMenuFuncPtr
#define MENU_CHAIN_BACK  (-1) // Causes automatic cmsMenuBack

/*
onExit function is called with self:
(1) Pointer to an OSD_Entry when cmsMenuBack() was called.
    Point to an OSD_Entry with type == OME_Back if BACK was selected.
(2) NULL if called from menu exit (forced exit at top level).
*/

typedef long (*CMSMenuOnExitPtr)(const OSD_Entry *self);

typedef struct
{
#ifdef CMS_MENU_DEBUG
    // These two are debug aids for menu content creators.
    const char *GUARD_text;
    const OSD_MenuElement GUARD_type;
#endif
    const CMSMenuFuncPtr onEnter;
    const CMSMenuOnExitPtr onExit;
    const CMSMenuFuncPtr onGlobalExit;
    const OSD_Entry *entries;
} CMS_Menu;

typedef struct
{
    uint8_t *val;
    uint8_t min;
    uint8_t max;
    uint8_t step;
} OSD_UINT8_t;

typedef struct
{
    int8_t *val;
    int8_t min;
    int8_t max;
    int8_t step;
} OSD_INT8_t;

typedef struct
{
    int16_t *val;
    int16_t min;
    int16_t max;
    int16_t step;
} OSD_INT16_t;

typedef struct
{
    uint16_t *val;
    uint16_t min;
    uint16_t max;
    uint16_t step;
} OSD_UINT16_t;

typedef struct
{
    uint8_t *val;
    uint8_t min;
    uint8_t max;
    uint8_t step;
    uint16_t multipler;
} OSD_FLOAT_t;

typedef struct OSD_SETTING_s {
    const uint16_t val; // setting number, from the constants in settings_generated.h
    const uint8_t step;
} __attribute__((packed)) OSD_SETTING_t;

#define OSD_SETTING_ENTRY_STEP_TYPE(name, setting, step, type)  { name, NULL, &(const OSD_SETTING_t){ setting, step }, OME_Setting, type }
#define OSD_SETTING_ENTRY_TYPE(name, setting, type)             OSD_SETTING_ENTRY_STEP_TYPE(name, setting, 0, type)
#define OSD_SETTING_ENTRY_STEP(name, setting, step)             OSD_SETTING_ENTRY_STEP_TYPE(name, setting, step, 0)
#define OSD_SETTING_ENTRY(name, setting)                        OSD_SETTING_ENTRY_STEP(name, setting, 0)

typedef struct
{
    uint8_t *val;
    uint8_t max;
    const char * const *names;
} OSD_TAB_t;

typedef struct
{
    char *val;
} OSD_String_t;
