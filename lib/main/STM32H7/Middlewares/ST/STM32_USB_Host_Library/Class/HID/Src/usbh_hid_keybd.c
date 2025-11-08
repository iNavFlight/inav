/**
  ******************************************************************************
  * @file    usbh_hid_keybd.c
  * @author  MCD Application Team
  * @brief   This file is the application layer for USB Host HID Keyboard handling
  *          QWERTY and AZERTY Keyboard are supported as per the selection in
  *          usbh_hid_keybd.h
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
- "stm32xxxxx_{eval}{discovery}{adafruit}_lcd.c"
- "stm32xxxxx_{eval}{discovery}_sdram.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbh_hid_keybd.h"
#include "usbh_hid_parser.h"

/** @addtogroup USBH_LIB
  * @{
  */

/** @addtogroup USBH_CLASS
  * @{
  */

/** @addtogroup USBH_HID_CLASS
  * @{
  */

/** @defgroup USBH_HID_KEYBD
  * @brief    This file includes HID Layer Handlers for USB Host HID class.
  * @{
  */

/** @defgroup USBH_HID_KEYBD_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBH_HID_KEYBD_Private_Defines
  * @{
  */
/**
  * @}
  */
#ifndef AZERTY_KEYBOARD
#define QWERTY_KEYBOARD
#endif
#define  KBD_LEFT_CTRL                                  0x01
#define  KBD_LEFT_SHIFT                                 0x02
#define  KBD_LEFT_ALT                                   0x04
#define  KBD_LEFT_GUI                                   0x08
#define  KBD_RIGHT_CTRL                                 0x10
#define  KBD_RIGHT_SHIFT                                0x20
#define  KBD_RIGHT_ALT                                  0x40
#define  KBD_RIGHT_GUI                                  0x80
#define  KBR_MAX_NBR_PRESSED                            6

/** @defgroup USBH_HID_KEYBD_Private_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_HID_KEYBD_Private_FunctionPrototypes
  * @{
  */
static USBH_StatusTypeDef USBH_HID_KeybdDecode(USBH_HandleTypeDef *phost);
/**
  * @}
  */

/** @defgroup USBH_HID_KEYBD_Private_Variables
  * @{
  */

HID_KEYBD_Info_TypeDef    keybd_info;
uint8_t                   keybd_rx_report_buf[USBH_HID_KEYBD_REPORT_SIZE];
uint8_t                   keybd_report_data[USBH_HID_KEYBD_REPORT_SIZE];

static const HID_Report_ItemTypedef imp_0_lctrl =
{
  keybd_report_data, /*data*/
  1,     /*size*/
  0,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min vale device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};
static const HID_Report_ItemTypedef imp_0_lshift =
{
  keybd_report_data, /*data*/
  1,     /*size*/
  1,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min vale device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};
static const HID_Report_ItemTypedef imp_0_lalt =
{
  keybd_report_data, /*data*/
  1,     /*size*/
  2,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min vale device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};
static const HID_Report_ItemTypedef imp_0_lgui =
{
  keybd_report_data, /*data*/
  1,     /*size*/
  3,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min vale device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};
static const HID_Report_ItemTypedef imp_0_rctrl =
{
  keybd_report_data, /*data*/
  1,     /*size*/
  4,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min vale device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};
static const HID_Report_ItemTypedef imp_0_rshift =
{
  keybd_report_data, /*data*/
  1,     /*size*/
  5,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min vale device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};
static const HID_Report_ItemTypedef imp_0_ralt =
{
  keybd_report_data, /*data*/
  1,     /*size*/
  6,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min vale device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};
static const HID_Report_ItemTypedef imp_0_rgui =
{
  keybd_report_data, /*data*/
  1,     /*size*/
  7,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min vale device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};

static const HID_Report_ItemTypedef imp_0_key_array =
{
  keybd_report_data + 2U, /*data*/
  8,     /*size*/
  0,     /*shift*/
  6,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  101,   /*max value read can return*/
  0,     /*min vale device can report*/
  101,   /*max value device can report*/
  1      /*resolution*/
};

#ifdef QWERTY_KEYBOARD
static const uint8_t HID_KEYBRD_Key[] =
{
  /*  0 */ '\0',  /*  1 */ '`',   /*  2 */ '1',   /*  3 */ '2',
  /*  4 */ '3',   /*  5 */ '4',   /*  6 */ '5',   /*  7 */ '6',
  /*  8 */ '7',   /*  9 */ '8',   /* 10 */ '9',   /* 11 */ '0',
  /* 12 */ '-',   /* 13 */ '=',   /* 14 */ '\0',  /* 15 */ '\b',
  /* 16 */ '\t',  /* 17 */ 'q',   /* 18 */ 'w',   /* 19 */ 'e',
  /* 20 */ 'r',   /* 21 */ 't',   /* 22 */ 'y',   /* 23 */ 'u',
  /* 24 */ 'i',   /* 25 */ 'o',   /* 26 */ 'p',   /* 27 */ '[',
  /* 28 */ ']',   /* 29 */ '\\',  /* 30 */ '\0',  /* 31 */ 'a',
  /* 32 */ 's',   /* 33 */ 'd',   /* 34 */ 'f',   /* 35 */ 'g',
  /* 36 */ 'h',   /* 37 */ 'j',   /* 38 */ 'k',   /* 39 */ 'l',
  /* 40 */ ';',   /* 41 */ '\'',  /* 42 */ '\0',  /* 43 */ '\n',
  /* 44 */ '\0',  /* 45 */ '\0',  /* 46 */ 'z',   /* 47 */ 'x',
  /* 48 */ 'c',   /* 49 */ 'v',   /* 50 */ 'b',   /* 51 */ 'n',
  /* 52 */ 'm',   /* 53 */ ',',   /* 54 */ '.',   /* 55 */ '/',
  /* 56 */ '\0',  /* 57 */ '\0',  /* 58 */ '\0',  /* 59 */ '\0',
  /* 60 */ '\0',  /* 61 */ ' ',   /* 62 */ '\0',  /* 63 */ '\0',
  /* 64 */ '\0',  /* 65 */ '\0',  /* 66 */ '\0',  /* 67 */ '\0',
  /* 68 */ '\0',  /* 69 */ '\0',  /* 70 */ '\0',  /* 71 */ '\0',
  /* 72 */ '\0',  /* 73 */ '\0',  /* 74 */ '\0',  /* 75 */ '\0',
  /* 76 */ '\0',  /* 77 */ '\0',  /* 78 */ '\0',  /* 79 */ '\0',
  /* 80 */ '\r',  /* 81 */ '\0',  /* 82 */ '\0',  /* 83 */ '\0',
  /* 84 */ '\0',  /* 85 */ '\0',  /* 86 */ '\0',  /* 87 */ '\0',
  /* 88 */ '\0',  /* 89 */ '\0',  /* 90 */ '\0',  /* 91 */ '7',
  /* 92 */ '4',   /* 93 */ '1',   /* 94 */ '\0',  /* 95 */ '/',
  /* 96 */ '8',   /* 97 */ '5',   /* 98 */ '2',   /* 99 */ '0',
  /*100 */ '*',   /*101 */ '9',   /*102 */ '6',   /*103 */ '3',
  /*104 */ '.',   /*105 */ '-',   /*106 */ '+',   /*107 */ '\0',
  /*108 */ '\n',  /*109 */ '\0',  /*110 */ 0x1B,  /*111 */ '\0',
  /*112 */ '\0',  /*113 */ '\0',  /*114 */ '\0',  /*115 */ '\0',
  /*116 */ '\0',  /*117 */ '\0',  /*118 */ '\0',  /*119 */ '\0',
  /*120 */ '\0',  /*121 */ '\0',  /*122 */ '\0',  /*123 */ '\0',
  /*124 */ '\0',  /*125 */ '\0',  /*126 */ '\0',  /*127 */ '\0',
  /*128 */ '\0',  /*129 */ '\0',  /*130 */ '\0',  /*131 */ '\0'
};

static const uint8_t HID_KEYBRD_ShiftKey[] =
{
  /*  0 */ '\0',  /*  1 */ '~',   /*  2 */ '!',   /*  3 */ '@',
  /*  4 */ '#',   /*  5 */ '$',   /*  6 */ '%',   /*  7 */ '^',
  /*  8 */ '&',   /*  9 */ '*',   /* 10 */ '(',   /* 11 */ ')',
  /* 12 */ '_',   /* 13 */ '+',   /* 14 */ '\0',  /* 15 */ '\b',
  /* 16 */ '\0',  /* 17 */ 'Q',   /* 18 */ 'W',   /* 19 */ 'E',
  /* 20 */ 'R',   /* 21 */ 'T',   /* 22 */ 'Y',   /* 23 */ 'U',
  /* 24 */ 'I',   /* 25 */ 'O',   /* 26 */ 'P',   /* 27 */ '{',
  /* 28 */ '}',   /* 29 */ '|',   /* 30 */ '\0',  /* 31 */ 'A',
  /* 32 */ 'S',   /* 33 */ 'D',   /* 34 */ 'F',   /* 35 */ 'G',
  /* 36 */ 'H',   /* 37 */ 'J',   /* 38 */ 'K',   /* 39 */ 'L',
  /* 40 */ ':',   /* 41 */ '"',   /* 42 */ '\0',  /* 43 */ '\n',
  /* 44 */ '\0',  /* 45 */ '\0',  /* 46 */ 'Z',   /* 47 */ 'X',
  /* 48 */ 'C',   /* 49 */ 'V',   /* 50 */ 'B',   /* 51 */ 'N',
  /* 52 */ 'M',   /* 53 */ '<',   /* 54 */ '>',   /* 55 */ '?',
  /* 56 */ '\0',  /* 57 */ '\0',  /* 58 */ '\0',  /* 59 */ '\0',
  /* 60 */ '\0',  /* 61 */ ' ',   /* 62 */ '\0',  /* 63 */ '\0',
  /* 64 */ '\0',  /* 65 */ '\0',  /* 66 */ '\0',  /* 67 */ '\0',
  /* 68 */ '\0',  /* 69 */ '\0',  /* 70 */ '\0',  /* 71 */ '\0',
  /* 72 */ '\0',  /* 73 */ '\0',  /* 74 */ '\0',  /* 75 */ '\0',
  /* 76 */ '\0',  /* 77 */ '\0',  /* 78 */ '\0',  /* 79 */ '\0',
  /* 80 */ '\0',  /* 81 */ '\0',  /* 82 */ '\0',  /* 83 */ '\0',
  /* 84 */ '\0',  /* 85 */ '\0',  /* 86 */ '\0',  /* 87 */ '\0',
  /* 88 */ '\0',  /* 89 */ '\0',  /* 90 */ '\0',  /* 91 */ '\0',
  /* 92 */ '\0',  /* 93 */ '\0',  /* 94 */ '\0',  /* 95 */ '/',
  /* 96 */ '\0',  /* 97 */ '\0',  /* 98 */ '\0',  /* 99 */ '\0',
  /*100 */ '*',   /*101 */ '\0',  /*102 */ '\0',  /*103 */ '\0',
  /*104 */ '\0',  /*105 */ '-',   /*106 */ '+',   /*107 */ '\0',
  /*108 */ '\n',  /*109 */ '\0',  /*110 */ 0x1B,  /*111 */ '\0',
  /*112 */ '\0',  /*113 */ '\0',  /*114 */ '\0',  /*115 */ '\0',
  /*116 */ '\0',  /*117 */ '\0',  /*118 */ '\0',  /*119 */ '\0',
  /*120 */ '\0',  /*121 */ '\0',  /*122 */ '\0',  /*123 */ '\0',
  /*124 */ '\0',  /*125 */ '\0',  /*126 */ '\0',  /*127 */ '\0',
  /*128 */ '\0',  /*129 */ '\0',  /*130 */ '\0',  /*131 */ '\0'
};

#else /* AZERTY Keyboard defined */

static const uint8_t HID_KEYBRD_Key[] =
{
  /*  0 */ '\0',  /*  1 */ '²',   /*  2 */ '&',   /*  3 */ 'é',
  /*  4 */ '"',   /*  5 */ '\'',  /*  6 */ '(',   /*  7 */ '-',
  /*  8 */ 'è',   /*  9 */ '_',   /* 10 */ 'ç',   /* 11 */ 'à',
  /* 12 */ ')',   /* 13 */ '=',   /* 14 */ '\0',  /* 15 */ '\b',
  /* 16 */ '\t',  /* 17 */ 'a',   /* 18 */ 'z',   /* 19 */ 'e',
  /* 20 */ 'r',   /* 21 */ 't',   /* 22 */ 'y',   /* 23 */ 'u',
  /* 24 */ 'i',   /* 25 */ 'o',   /* 26 */ 'p',   /* 27 */ '^',
  /* 28 */ '$',   /* 29 */ '\\',  /* 30 */ '\0',  /* 31 */ 'q',
  /* 32 */ 's',   /* 33 */ 'd',   /* 34 */ 'f',   /* 35 */ 'g',
  /* 36 */ 'h',   /* 37 */ 'j',   /* 38 */ 'k',   /* 39 */ 'l',
  /* 40 */ 'm',   /* 41 */ 'ù',   /* 42 */ '*',   /* 43 */ '\n',
  /* 44 */ '\0',  /* 45 */ '<',   /* 46 */ 'w',   /* 47 */ 'x',
  /* 48 */ 'c',   /* 49 */ 'v',   /* 50 */ 'b',   /* 51 */ 'n',
  /* 52 */ ',',   /* 53 */ ';',   /* 54 */ ':',   /* 55 */ '!',
  /* 56 */ '\0',  /* 57 */ '\0',  /* 58 */ '\0',  /* 59 */ '\0',
  /* 60 */ '\0',  /* 61 */ ' ',   /* 62 */ '\0',  /* 63 */ '\0',
  /* 64 */ '\0',  /* 65 */ '\0',  /* 66 */ '\0',  /* 67 */ '\0',
  /* 68 */ '\0',  /* 69 */ '\0',  /* 70 */ '\0',  /* 71 */ '\0',
  /* 72 */ '\0',  /* 73 */ '\0',  /* 74 */ '\0',  /* 75 */ '\0',
  /* 76 */ '\0',  /* 77 */ '\0',  /* 78 */ '\0',  /* 79 */ '\0',
  /* 80 */ '\0',  /* 81 */ '\0',  /* 82 */ '\0',  /* 83 */ '\0',
  /* 84 */ '\0',  /* 85 */ '\0',  /* 86 */ '\0',  /* 87 */ '\0',
  /* 88 */ '\0',  /* 89 */ '\0',  /* 90 */ '\0',  /* 91 */ '7',
  /* 92 */ '4',   /* 93 */ '1',   /* 94 */ '\0',  /* 95 */ '/',
  /* 96 */ '8',   /* 97 */ '5',   /* 98 */ '2',   /* 99 */ '0',
  /*100 */ '*',   /*101 */ '9',   /*102 */ '6',   /*103 */ '3',
  /*104 */ '.',   /*105 */ '-',   /*106 */ '+',   /*107 */ '\0',
  /*108 */ '\n',  /*109 */ '\0',  /*110 */ 0x1B,  /*111 */ '\0',
  /*112 */ '\0',  /*113 */ '\0',  /*114 */ '\0',  /*115 */ '\0',
  /*116 */ '\0',  /*117 */ '\0',  /*118 */ '\0',  /*119 */ '\0',
  /*120 */ '\0',  /*121 */ '\0',  /*122 */ '\0',  /*123 */ '\0',
  /*124 */ '\0',  /*125 */ '\0',  /*126 */ '\0',  /*127 */ '\0',
  /*128 */ '\0',  /*129 */ '\0',  /*130 */ '\0',  /*131 */ '\0'
};

static const uint8_t HID_KEYBRD_ShiftKey[] =
{
  /*  0 */ '\0',  /*  1 */ '\0',  /*  2 */ '1',   /*  3 */ '2',
  /*  4 */ '3',   /*  5 */ '4',   /*  6 */ '5',   /*  7 */ '6',
  /*  8 */ '7',   /*  9 */ '8',   /* 10 */ '9',   /* 11 */ '0',
  /* 12 */ '°',   /* 13 */ '+',   /* 14 */ '\0',  /* 15 */ '\b',
  /* 16 */ '\t',  /* 17 */ 'A',   /* 18 */ 'Z',   /* 19 */ 'E',
  /* 20 */ 'R',   /* 21 */ 'T',   /* 22 */ 'Y',   /* 23 */ 'U',
  /* 24 */ 'I',   /* 25 */ 'O',   /* 26 */ 'P',   /* 27 */ '¨',
  /* 28 */ '£',   /* 29 */ 'µ',   /* 30 */ '\0',  /* 31 */ 'Q',
  /* 32 */ 'S',   /* 33 */ 'D',   /* 34 */ 'F',   /* 35 */ 'G',
  /* 36 */ 'H',   /* 37 */ 'J',   /* 38 */ 'K',   /* 39 */ 'L',
  /* 40 */ 'M',   /* 41 */ '%',   /* 42 */ 'µ',   /* 43 */ '\n',
  /* 44 */ '\0',  /* 45 */ '>',   /* 46 */ 'W',   /* 47 */ 'X',
  /* 48 */ 'C',   /* 49 */ 'V',   /* 50 */ 'B',   /* 51 */ 'N',
  /* 52 */ '?',   /* 53 */ '.',   /* 54 */ '/',   /* 55 */ '§',
  /* 56 */ '\0',  /* 57 */ '\0',  /* 58 */ '\0',  /* 59 */ '\0',
  /* 60 */ '\0',  /* 61 */ ' ',   /* 62 */ '\0',  /* 63 */ '\0',
  /* 64 */ '\0',  /* 65 */ '\0',  /* 66 */ '\0',  /* 67 */ '\0',
  /* 68 */ '\0',  /* 69 */ '\0',  /* 70 */ '\0',  /* 71 */ '\0',
  /* 72 */ '\0',  /* 73 */ '\0',  /* 74 */ '\0',  /* 75 */ '\0',
  /* 76 */ '\0',  /* 77 */ '\0',  /* 78 */ '\0',  /* 79 */ '\0',
  /* 80 */ '\0',  /* 81 */ '\0',  /* 82 */ '\0',  /* 83 */ '\0',
  /* 84 */ '\0',  /* 85 */ '\0',  /* 86 */ '\0',  /* 87 */ '\0',
  /* 88 */ '\0',  /* 89 */ '\0',  /* 90 */ '\0',  /* 91 */ '7',
  /* 92 */ '\0',  /* 93 */ '\0',  /* 94 */ '\0',  /* 95 */ '/',
  /* 96 */ '\0',  /* 97 */ '\0',  /* 98 */ '\0',  /* 99 */ '\0',
  /*100 */ '*',   /*101 */ '\0',  /*102 */ '\0',  /*103 */ '\0',
  /*104 */ '\0',  /*105 */ '-',   /*106 */ '+',   /*107 */ '\0',
  /*108 */ '\n',  /*109 */ '\0',  /*110 */ 0x1B,  /*111 */ '\0',
  /*112 */ '\0',  /*113 */ '\0',  /*114 */ '\0',  /*115 */ '\0',
  /*116 */ '\0',  /*117 */ '\0',  /*118 */ '\0',  /*119 */ '\0',
  /*120 */ '\0',  /*121 */ '\0',  /*122 */ '\0',  /*123 */ '\0',
  /*124 */ '\0',  /*125 */ '\0',  /*126 */ '\0',  /*127 */ '\0',
  /*128 */ '\0',  /*129 */ '\0',  /*130 */ '\0',  /*131 */ '\0'
};
#endif

static  const  uint8_t  HID_KEYBRD_Codes[] =
{
  0,     0,    0,    0,   31,   50,   48,   33,
  19,   34,   35,   36,   24,   37,   38,   39,       /* 0x00 - 0x0F */
  52,   51,   25,   26,   17,   20,   32,   21,
  23,   49,   18,   47,   22,   46,    2,    3,       /* 0x10 - 0x1F */
  4,     5,    6,    7,    8,    9,   10,   11,
  43,  110,   15,   16,   61,   12,   13,   27,       /* 0x20 - 0x2F */
  28,   29,   42,   40,   41,    1,   53,   54,
  55,   30,  112,  113,  114,  115,  116,  117,       /* 0x30 - 0x3F */
  118, 119,  120,  121,  122,  123,  124,  125,
  126,  75,   80,   85,   76,   81,   86,   89,       /* 0x40 - 0x4F */
  79,   84,   83,   90,   95,  100,  105,  106,
  108,  93,   98,  103,   92,   97,  102,   91,       /* 0x50 - 0x5F */
  96,  101,   99,  104,   45,  129,    0,    0,
  0,     0,    0,    0,    0,    0,    0,    0,       /* 0x60 - 0x6F */
  0,     0,    0,    0,    0,    0,    0,    0,
  0,     0,    0,    0,    0,    0,    0,    0,       /* 0x70 - 0x7F */
  0,     0,    0,    0,    0,  107,    0,   56,
  0,     0,    0,    0,    0,    0,    0,    0,       /* 0x80 - 0x8F */
  0,     0,    0,    0,    0,    0,    0,    0,
  0,     0,    0,    0,    0,    0,    0,    0,       /* 0x90 - 0x9F */
  0,     0,    0,    0,    0,    0,    0,    0,
  0,     0,    0,    0,    0,    0,    0,    0,       /* 0xA0 - 0xAF */
  0,     0,    0,    0,    0,    0,    0,    0,
  0,     0,    0,    0,    0,    0,    0,    0,       /* 0xB0 - 0xBF */
  0,     0,    0,    0,    0,    0,    0,    0,
  0,     0,    0,    0,    0,    0,    0,    0,       /* 0xC0 - 0xCF */
  0,     0,    0,    0,    0,    0,    0,    0,
  0,     0,    0,    0,    0,    0,    0,    0,       /* 0xD0 - 0xDF */
  58,   44,   60,  127,   64,   57,   62,  128        /* 0xE0 - 0xE7 */
};

/**
  * @brief  USBH_HID_KeybdInit
  *         The function init the HID keyboard.
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_HID_KeybdInit(USBH_HandleTypeDef *phost)
{
  uint32_t x;
  HID_HandleTypeDef *HID_Handle = (HID_HandleTypeDef *) phost->pActiveClass->pData;

  keybd_info.lctrl = 0U;
  keybd_info.lshift = 0U;
  keybd_info.lalt = 0U;
  keybd_info.lgui = 0U;
  keybd_info.rctrl = 0U;
  keybd_info.rshift = 0U;
  keybd_info.ralt = 0U;
  keybd_info.rgui = 0U;

  for (x = 0U; x < sizeof(keybd_report_data); x++)
  {
    keybd_report_data[x] = 0U;
    keybd_rx_report_buf[x] = 0U;
  }

  if (HID_Handle->length > (sizeof(keybd_report_data)))
  {
    HID_Handle->length = (uint16_t)(sizeof(keybd_report_data));
  }

  HID_Handle->pData = keybd_rx_report_buf;

  if ((HID_QUEUE_SIZE * sizeof(keybd_report_data)) > sizeof(phost->device.Data))
  {
    return USBH_FAIL;
  }
  else
  {
    USBH_HID_FifoInit(&HID_Handle->fifo, phost->device.Data, (uint16_t)(HID_QUEUE_SIZE * sizeof(keybd_report_data)));
  }

  return USBH_OK;
}

/**
  * @brief  USBH_HID_GetKeybdInfo
  *         The function return keyboard information.
  * @param  phost: Host handle
  * @retval keyboard information
  */
HID_KEYBD_Info_TypeDef *USBH_HID_GetKeybdInfo(USBH_HandleTypeDef *phost)
{
  if (USBH_HID_KeybdDecode(phost) == USBH_OK)
  {
    return &keybd_info;
  }
  else
  {
    return NULL;
  }
}

/**
  * @brief  USBH_HID_KeybdDecode
  *         The function decode keyboard data.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_HID_KeybdDecode(USBH_HandleTypeDef *phost)
{
  uint8_t x;

  HID_HandleTypeDef *HID_Handle = (HID_HandleTypeDef *) phost->pActiveClass->pData;

  if ((HID_Handle->length == 0U) || (HID_Handle->fifo.buf == NULL))
  {
    return USBH_FAIL;
  }

  /*Fill report */
  if (USBH_HID_FifoRead(&HID_Handle->fifo, &keybd_report_data, HID_Handle->length) ==  HID_Handle->length)
  {
    keybd_info.lctrl = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &imp_0_lctrl, 0U);
    keybd_info.lshift = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &imp_0_lshift, 0U);
    keybd_info.lalt = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &imp_0_lalt, 0U);
    keybd_info.lgui = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &imp_0_lgui, 0U);
    keybd_info.rctrl = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &imp_0_rctrl, 0U);
    keybd_info.rshift = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &imp_0_rshift, 0U);
    keybd_info.ralt = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &imp_0_ralt, 0U);
    keybd_info.rgui = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &imp_0_rgui, 0U);

    for (x = 0U; x < sizeof(keybd_info.keys); x++)
    {
      keybd_info.keys[x] = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &imp_0_key_array, x);
    }

    return USBH_OK;
  }
  return   USBH_FAIL;
}

/**
  * @brief  USBH_HID_GetASCIICode
  *         The function decode keyboard data into ASCII characters.
  * @param  phost: Host handle
  * @param  info: Keyboard information
  * @retval ASCII code
  */
uint8_t USBH_HID_GetASCIICode(HID_KEYBD_Info_TypeDef *info)
{
  uint8_t output;

  if ((info->lshift != 0U) || (info->rshift != 0U))
  {
    output = HID_KEYBRD_ShiftKey[HID_KEYBRD_Codes[info->keys[0]]];
  }
  else
  {
    output = HID_KEYBRD_Key[HID_KEYBRD_Codes[info->keys[0]]];
  }
  return output;
}

