/*
    ChibiOS/HAL - Copyright (C) 2015 Uladzimir Pylinsky aka barthess

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    bitmap.h
 * @brief   Bit map structures and macros.
 *
 * @addtogroup bitmap
 * @{
 */

#ifndef BITMAP_H_
#define BITMAP_H_

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

typedef unsigned int bitmap_word_t;

/**
 * @brief   Type of a event timer structure.
 */
typedef struct {
  bitmap_word_t   *array;
  size_t          len;    /* Array length in _words_ NOT bytes */
} bitmap_t;

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void bitmapObjectInit(bitmap_t *map, bitmap_word_t val);
  void bitmapSet(bitmap_t *map, size_t bit);
  void bitmapClear(bitmap_t *map, size_t bit);
  void bitmapInvert(bitmap_t *map, size_t bit);
  bitmap_word_t bitmapGet(const bitmap_t *map, size_t bit);
  size_t bitmapGetBitsCount(const bitmap_t *map);
#ifdef __cplusplus
}
#endif

#endif /* BITMAP_H_ */

/** @} */
