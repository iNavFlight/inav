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

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stdint.h>
#include <stdbool.h>

#include <asc_config.h>

#include "asc_security_core/asc_result.h"

/**
 * @brief   safe strstr.
 *
 * @param   s   the buffer
 *
 * @return  buffer size
 */
char *str_str(const char *s1, const char *s2);

/**
 * @brief   Replace last newline by NULL - no reallocation memory.
 *
 * @param   s   the buffer
 *
 * @return  non
 */
void str_trim_last_newline(char *s);

/**
 * @brief   Trim whitespaces. This function returns a pointer to a substring of the original string.
 *
 * @param   s   the original string
 *
 * @return  a pointer to a substring of the original string
 */
char *str_trim(char *s);

/**
 * @brief   safe strlen.
 *
 * @param   s   the buffer
 *
 * @return  buffer size
 */
size_t str_len(const char *s);

/**
 * @brief   safe strcmp.
 *
 * @param   s   the buffer
 *
 * @return  comparison result
 */
int str_cmp(const char *s1, const char *s2);

/**
 * @brief   check if string is NULL or empty.
 *
 * @param   s   the string for check
 *
 * @return  true if empty, otherwise false.
 */
bool str_isempty(const char *s);

/**
 * @brief   char* value getter
 *
 * @param   s    string
 *
 * @return char* value if is not NULL, empty otherwise
 */
const char *str_value_or_empty(const char *s);

/**
 * @brief   Check if string is blank
 *
 * @param   s    string
 *
 * @return true iff string is blank
 */
bool str_is_blank(const char *s);

/**
 * @brief   Split a string to two parts by a delimiter
 *
 * @param   s           string
 * @param   token       A pointer to a variable to receive the token
 * @param   token_len   A pointer to a variable to receive the length of 'token'
 * @param   rest        A pointer to a variable to receive the rest of the string
 * @param   rest_len    A pointer to a variable to receive the length of 'rest'
 * @param   delimiter   The delimiter to 'split' the string
 *
 * @return ASC_RESULT_OK on success.
 */
asc_result_t str_split(char *s, char **token, size_t *token_len, char **rest, size_t *rest_len, char *delimiter);

/**
 * @brief   Compare two strings
 *
 * @param   s1        The first string
 * @param   s1_len    The length of s1
 * @param   s2        The second string
 * @param   s2_len    The length of s2
 *
 * @return comparison result
 */
int str_ncmp(const char *s1, size_t s1_len, const char *s2, size_t s2_len);

/**
 * @brief   Check if one string is a prefix of the other string
 *
 * @param   s1        The first string
 * @param   s2        The second string
 *
 * @return true if s2 is a prefix of s1
 */
bool str_starts_from(const char *s1, const char *s2);

/**
 * @brief   replace all instances of a char in a char array with a different char
 *  
 * @param   char_array      The character array
 * @param   len             The length to look at
 * @param   replace_from    The character to be replaced
 * @param   replace_to      The character to replace with
 *
 * @return a reference to the char array
 */
char *replace_chars(char *char_array, size_t len, char replace_from, char replace_to);

#endif /* STRING_UTILS_H */