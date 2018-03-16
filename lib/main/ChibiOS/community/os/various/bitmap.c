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
 * @file    bitmap.c
 * @brief   Bit map code.
 *
 * @addtogroup bitmap
 * @{
 */

#include "string.h" /* for memset() */

#include "hal.h"
#include "bitmap.h"

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Module local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

/**
 * @brief Get word number.
 *
 * @param[in] bit       number of the bit
 *
 * @return              Index of the word containing specified bit.
 */
static inline size_t word(size_t bit) {
  return bit / (sizeof(bitmap_word_t) * 8);
}

/**
 * @brief Get bit position in word.
 *
 * @param[in] bit       number of the bit
 *
 * @return              Position of the specified bit related to word start.
 */
static inline size_t pos_in_word(size_t bit) {
  return bit % (sizeof(bitmap_word_t) * 8);
}

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/
/**
 * @brief Initializes an @p bitmap_t structure.
 *
 * @param[out] map      the @p bitmap_t structure to be initialized
 * @param[in] val       the value to be written in all bitmap
 */
void bitmapObjectInit(bitmap_t *map, bitmap_word_t val) {
  uint8_t pattern;

  osalDbgCheck(val == 1 || val == 0);

  if (val == 1)
    pattern = 0xFF;
  else
    pattern = 0;

  memset(map->array, pattern, map->len*sizeof(bitmap_word_t));
}

/**
 * @brief Set single bit in an @p bitmap_t structure.
 *
 * @param[out] map      the @p bitmap_t structure
 * @param[in] bit       number of the bit to be set
 */
void bitmapSet(bitmap_t *map, size_t bit) {
  size_t w = word(bit);

  osalDbgCheck(w < map->len);
  map->array[w] |= (bitmap_word_t)1 << pos_in_word(bit);
}

/**
 * @brief Clear single bit in an @p bitmap_t structure.
 *
 * @param[out] map      the @p bitmap_t structure
 * @param[in] bit       number of the bit to be cleared
 */
void bitmapClear(bitmap_t *map, size_t bit) {
  size_t w = word(bit);

  osalDbgCheck(w < map->len);
  map->array[w] &= ~((bitmap_word_t)1 << pos_in_word(bit));
}

/**
 * @brief Invert single bit in an @p bitmap_t structure.
 *
 * @param[out] map      the @p bitmap_t structure
 * @param[in] bit       number of the bit to be inverted
 */
void bitmapInvert(bitmap_t *map, size_t bit) {
  size_t w = word(bit);

  osalDbgCheck(w < map->len);
  map->array[w] ^= (bitmap_word_t)1 << pos_in_word(bit);
}

/**
 * @brief Get bit value from an @p bitmap_t structure.
 *
 * @param[in] map       the @p bitmap_t structure
 * @param[in] bit       number of the requested bit
 *
 * @return              Requested bit value.
 */
bitmap_word_t bitmapGet(const bitmap_t *map, size_t bit) {
  size_t w = word(bit);

  osalDbgCheck(w < map->len);
  return (map->array[w] >> pos_in_word(bit)) & 1;
}

/**
 * @brief Get total amount of bits in an @p bitmap_t structure.
 *
 * @param[in] map       the @p bitmap_t structure
 *
 * @return              Bit number.
 */
size_t bitmapGetBitsCount(const bitmap_t *map) {
  return map->len * sizeof(bitmap_word_t) * 8;
}
/** @} */
