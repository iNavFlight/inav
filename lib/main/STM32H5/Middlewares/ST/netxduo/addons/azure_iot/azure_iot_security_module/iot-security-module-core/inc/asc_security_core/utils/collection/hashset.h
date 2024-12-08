/*******************************************************************************/
/*                                                                             */
/* Copyright (c) Microsoft Corporation. All rights reserved.                   */
/*                                                                             */
/* This software is licensed under the Microsoft Software License              */
/* Terms for Microsoft Azure Defender for IoT. Full text of the license can be */
/* found in the LICENSE file at https://aka.ms/AzureDefenderForIoT_EULA        */
/* and in the root directory of this software.                                 */
/*                                                                             */
/*******************************************************************************/

#ifndef HASHSET_H
#define HASHSET_H

#include <string.h>

#include <asc_config.h>

#include "asc_security_core/utils/collection/collection.h"

/**
 * @brief   A generic Hashset implementation.
 *
 *          Usage:
 *          - Use HASHSET_DECLARATIONS(type) to emit function declaration for struct-type "type".
 *          - Use HASHSET_DEFINITIONS(type, size) to emite function definitions for struct-type "type".
 *          - Implement the following functions:
 *            - hashset_##type##_hash
 *            - hashset_##type##_equals
 *            - hashset_##type##_update
 *
 *          Additional Notes:
 *          - Make sure the struct-type uses COLLECTION_INTERFACE (see collection.h) This macro must be first in object.
 *          - The Hashset does NOT allocate any memory
 *          - The size defined in HASHSET_DEFINITION is the expected size for any hashset instance
 *          - When implementing hashset_##type##_hash and hashset_##type##_equals, make sure that
 *            when 2 elements are equal (equals returns true) then they produce the same hash.
 */

#define HASHSET_DECLARATIONS(type)                                                                                        \
/**                                                                                                                       \
 * @brief   A Hashset "for each" function                                                                                 \
 */                                                                                                                       \
typedef void (*hashset_##type##_for_each_function)(type *element, void *context);                                         \
/**                                                                                                                       \
 * @brief   Initialize a new Hashset                                                                                      \
 *                                                                                                                        \
 * @param   table A pre-allocated array of ##type##*                                                                      \
 *                                                                                                                        \
 * @return  void                                                                                                          \
 */                                                                                                                       \
extern void hashset_##type##_init(type *table[]);                                                                         \
                                                                                                                          \
/**                                                                                                                       \
 * @brief   Find an element in the Hashset. Note: the returned element is still in the Hashset!                           \
 *                                                                                                                        \
 * @param   table   An initialized hashset                                                                                \
 * @param   element The element to find                                                                                   \
 *                                                                                                                        \
 * @return  The found elemnt, NULL otherwise                                                                              \
 */                                                                                                                       \
extern type *hashset_##type##_find(type *table[], type *element);                                                         \
                                                                                                                          \
/**                                                                                                                       \
 * @brief   Find an element in the Hashset and remove it from the Hashset                                                 \
 *                                                                                                                        \
 * @param   table   An initialized hashset                                                                                \
 * @param   element The element to remove                                                                                 \
 *                                                                                                                        \
 * @return  The found elemnt, NULL otherwise                                                                              \
 */                                                                                                                       \
extern type *hashset_##type##_remove(type *table[], type *element);                                                       \
                                                                                                                          \
/**                                                                                                                       \
 * @brief   Adds an element to the Hashset if it's not there already                                                      \
 *          Otherwise updates the existing element using the hashset_##type##_update function.                            \
 *                                                                                                                        \
 * @param   table   An initialized hashset                                                                                \
 * @param   element The element to add or update                                                                          \
 *                                                                                                                        \
 * @return  void                                                                                                          \
 */                                                                                                                       \
extern void hashset_##type##_add_or_update(type *table[], type *element);                                                 \
                                                                                                                          \
/**                                                                                                                       \
 * @brief   Iterates the Hashset and executes for_each_funcion for each element                                           \
 *                                                                                                                        \
 * @param   table             An initialized hashset                                                                      \
 * @param   for_each_funcion  The function to execute                                                                     \
 * @param   context           A context for the function                                                                  \
 *                                                                                                                        \
 * @return void                                                                                                           \
 */                                                                                                                       \
extern void hashset_##type##_for_each(type *table[], hashset_##type##_for_each_function for_each_funcion, void *context); \
                                                                                                                          \
/**                                                                                                                       \
 * @brief   Same as hashset_##type##_for_each, but removes the element from the hashset before executing                  \
 *                                                                                                                        \
 * @param   table             An initialized hashset                                                                      \
 * @param   for_each_funcion  The function to execute                                                                     \
 * @param   context           A context for the function                                                                  \
 *                                                                                                                        \
 * @return void                                                                                                           \
 */                                                                                                                       \
extern void hashset_##type##_clear(type *table[], hashset_##type##_for_each_function for_each_funcion, void *context);    \
                                                                                                                          \
/**                                                                                                                       \
 * @brief   The element hash function                                                                                     \
 *                                                                                                                        \
 * @param   element The element to compute the hash for                                                                   \
 *                                                                                                                        \
 * @return  The element hash                                                                                              \
 */                                                                                                                       \
extern unsigned int hashset_##type##_hash(type *element);                                                                 \
                                                                                                                          \
/**                                                                                                                       \
 * @brief   The element equality function                                                                                 \
 *                                                                                                                        \
 * @param   a The first element to compare                                                                                \
 * @param   b The second element to compare                                                                               \
 *                                                                                                                        \
 * @return  Non-zero if the elements are equal, zero otherwise                                                            \
 */                                                                                                                       \
extern int hashset_##type##_equals(type *a, type *b);                                                                     \
                                                                                                                          \
/**                                                                                                                       \
 * @brief   The element update function. Note: the new_element stays out of the hashset!                                  \
 *                                                                                                                        \
 * @param   old_element The element to update                                                                             \
 * @param   new_element The element to update from                                                                        \
 *                                                                                                                        \
 * @return  void                                                                                                          \
 */                                                                                                                       \
extern void hashset_##type##_update(type *old_element, type *new_element);                                                \
                                                                                                                          \
/**                                                                                                                       \
 * @brief   Get depth of hash to exam the hash function!                                                                  \
 *                                                                                                                        \
 * @param   table             An initialized hashset                                                                      \
 * @param   count             Count of non null or one element linked lists (out parameter)                               \
 *                                                                                                                        \
 * @return  zero if all hash elements have list with 0 or 1 length - otherwise the longest list length                    \
 */                                                                                                                       \
unsigned int hashset_##type##_get_depth(type *table[], unsigned int *count);                                              \
                                                                                                                          \
/**                                                                                                                       \
 * @brief   Check if hash table is empty                                                                                  \
 *                                                                                                                        \
 * @param   table             An initialized hashset                                                                      \
 *                                                                                                                        \
 * @return  true if hash table is empty - otherwise false                                                                 \
 */                                                                                                                       \
 bool hashset_##type##_is_empty(type *table[]);

#define HASHSET_DEFINITIONS(type, size)                                                                                   \
void hashset_##type##_init(type *table[])                                                                                 \
{                                                                                                                         \
  if (table == NULL) return;                                                                                              \
  memset(table, 0, sizeof(type*) * size);                                                                                 \
}                                                                                                                         \
                                                                                                                          \
type *hashset_##type##_find(type *table[], type *element)                                                                 \
{                                                                                                                         \
  if (element == NULL || table == NULL) return NULL;                                                                      \
  unsigned int index = hashset_##type##_hash(element) % (size);                                                           \
  type *current_element = table[index];                                                                                   \
  while (current_element != NULL) {                                                                                       \
    if (hashset_##type##_equals(element, current_element) != 0) break;                                                    \
    current_element = current_element->next;                                                                              \
  }                                                                                                                       \
  return current_element;                                                                                                 \
}                                                                                                                         \
                                                                                                                          \
type *hashset_##type##_remove(type *table[], type *element)                                                               \
{                                                                                                                         \
  if (element == NULL || table == NULL) return NULL;                                                                      \
  type *existing_element = hashset_##type##_find(table, element);                                                         \
  if (existing_element == NULL) return NULL;                                                                              \
  unsigned int index = hashset_##type##_hash(element) % (size);                                                           \
  type *previous_element = (element)->previous;                                                                           \
  type *next_element = (element)->next;                                                                                   \
  if (next_element != NULL) {                                                                                             \
    next_element->previous = previous_element;                                                                            \
  }                                                                                                                       \
  if (previous_element == NULL) {                                                                                         \
    table[index] = next_element;                                                                                          \
  } else {                                                                                                                \
    previous_element->next = next_element;                                                                                \
  }                                                                                                                       \
  return existing_element;                                                                                                \
}                                                                                                                         \
                                                                                                                          \
extern void hashset_##type##_add_or_update(type *table[], type *element)                                                  \
{                                                                                                                         \
  if (element == NULL || table == NULL) return;                                                                           \
  type *existing_element = hashset_##type##_find(table, element);                                                         \
  if (existing_element == NULL) {                                                                                         \
    unsigned int index = hashset_##type##_hash(element) % (size);                                                         \
    element->previous = NULL;                                                                                             \
    element->next = table[index];                                                                                         \
    if (table[index] != NULL) {                                                                                           \
      table[index]->previous = element;                                                                                   \
    }                                                                                                                     \
    table[index] = element;                                                                                               \
  } else {                                                                                                                \
    hashset_##type##_update(existing_element, element);                                                                   \
  }                                                                                                                       \
}                                                                                                                         \
                                                                                                                          \
void hashset_##type##_for_each(type *table[], hashset_##type##_for_each_function for_each_funcion, void *context)         \
{                                                                                                                         \
  if (table == NULL || for_each_funcion == NULL) return;                                                                  \
  type *current_element = NULL, *tmp = NULL;                                                                                           \
  for (unsigned int i = 0; i < size; ++i) {                                                                               \
    current_element = table[i];                                                                                           \
    while (current_element != NULL) {                                                                                     \
      tmp = current_element->next;                                                                                        \
      for_each_funcion(current_element, context);                                                                         \
      current_element = tmp;                                                                                              \
    }                                                                                                                     \
  }                                                                                                                       \
}                                                                                                                         \
                                                                                                                          \
void hashset_##type##_clear(type *table[], hashset_##type##_for_each_function for_each_funcion, void *context)            \
{                                                                                                                         \
  if (table == NULL) return;                                                                                              \
  type *current_element = NULL;                                                                                           \
  for (unsigned int i = 0; i < size; ++i) {                                                                               \
    current_element = table[i];                                                                                           \
    while (current_element != NULL) {                                                                                     \
      table[i] = current_element->next;                                                                                   \
      if (table[i]) {                                                                                                     \
        table[i]->previous = NULL;                                                                                        \
      }                                                                                                                   \
      current_element->previous = NULL;                                                                                   \
      current_element->next = NULL;                                                                                       \
      if (for_each_funcion != NULL) {                                                                                     \
        for_each_funcion(current_element, context);                                                                       \
      }                                                                                                                   \
      current_element = table[i];                                                                                         \
    }                                                                                                                     \
  }                                                                                                                       \
}                                                                                                                         \
                                                                                                                          \
unsigned int hashset_##type##_get_depth(type *table[], unsigned int *count)                                               \
{                                                                                                                         \
  unsigned int max = 0;                                                                                                   \
  *count = 0;                                                                                                             \
  if (table == NULL) return max;                                                                                          \
  type *current_element = NULL;                                                                                           \
  for (unsigned int i = 0; i < size; ++i) {                                                                               \
    unsigned int cnt = 0;                                                                                                 \
    current_element = table[i];                                                                                           \
    while (current_element != NULL) {                                                                                     \
      cnt++;                                                                                                              \
      current_element = current_element->next;                                                                            \
    }                                                                                                                     \
    if (cnt > 1) {                                                                                                        \
      if (cnt > max) max = cnt;                                                                                           \
      (*count)++;                                                                                                         \
    }                                                                                                                     \
  }                                                                                                                       \
  return max;                                                                                                             \
}                                                                                                                         \
                                                                                                                          \
bool hashset_##type##_is_empty(type *table[])                                                                             \
{                                                                                                                         \
  if (table == NULL) return true;                                                                                         \
  for (unsigned int i = 0; i < size; ++i) {                                                                               \
    if (table[i]) return false;                                                                                           \
  }                                                                                                                       \
  return true;                                                                                                            \
}

static inline unsigned int hashset_buffer2hash(const char *str, size_t len)
{
    unsigned int hash = 5381;

    for (size_t i = 0 ; i < len; i++) {
        hash = ((hash << 5) + hash) + (unsigned int)str[i];
    }

    return hash;
}

#endif /* HASHSET_H */
