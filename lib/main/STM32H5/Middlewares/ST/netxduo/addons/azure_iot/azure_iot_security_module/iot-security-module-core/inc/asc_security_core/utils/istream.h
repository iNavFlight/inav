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

#ifndef __ISTREAM_H__
#define __ISTREAM_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <asc_config.h>

#include "asc_security_core/asc_result.h"

/**
 * @brief Enum to define the data source.
*/
typedef enum {
    ISTREAM_DATA_SRC_FILE = 0,
    ISTREAM_DATA_SRC_CMD_OUTPUT,
    ISTREAM_DATA_SRC_CMD_OUTPUT_TO_TMP,
    ISTREAM_DATA_SRC_BASH_CMD_OUTPUT
} istream_data_src_t;

typedef struct {
    istream_data_src_t data_src;    // Data source
    bool full;                      // Search on full buffer (true) or line by line (false - default)
    bool negative;                  // Make logic to be opposite, if not match - pass
    bool data_not_exist_pass;       // Add logic if src not exist - pass
    bool data_empty_pass;           // Add logic if stream is empty - pass
    bool scandir_match_or;          // Add logic if one of the files match the condition in scandir (asterisk) - result is pass
    bool case_ignore;               // Is case sensitive
    bool each_line;                 // Matching on each line
    int sub_group_or;               // Add logic of sub group 'or' if one of the tests in this sub group passed - result of this sub group is pass - max 10
    int sub_group_and;              // Add logic of sub group 'and' if all tests in sub this group passed - result of this sub group is pass - max 10
    const char *path;               // Pointer on previous allocated stream path
    const char *regex;              // Pointer on previous allocated matching string
    const char *base_dir;           // Base dir for 'path' parameter
    const char *hash;               // Pointer on previous allocated hash
    bool suppress_err_log;          // Flag for error log suppress on low layer
} istream_match_t;

/**
 * @brief Check if there is a stream contains the specified string.
 *
 * @param path      The stream path
 * @param s         The string
 * @param params    The parameters defined in @c istream_match_t struct
 * @param buf       The target buffer that is used to read result or error
 * @param size      The size of the buffer in bytes.
 *
 * @return ASC_RESULT_OK on contains, ASC_RESULT_EMPTY on not contains, otherwise ASC_RESULT_error_code.
 */
asc_result_t istream_contains_string(const char *path, const char *s, istream_match_t *params, char **buf, size_t *size);

/**
 * @brief Check if the content of the whole stream (without newline or return) given by path, is equal specified buffer.
 *
 * @param path      The stream path
 * @param s         Compare the content of the stream with this string
 * @param params    The parameters defined in @c istream_match_t struct
 * @param buf       The target buffer that is used to read result or error
 * @param size      The size of the buffer in bytes.
 *
 * @return ASC_RESULT_OK on equals, ASC_RESULT_EMPTY on not equal, otherwise ASC_RESULT_error_code.
 */
asc_result_t istream_trimmed_equals_to_buffer(const char *path, const char *s, istream_match_t *params, char **buf, size_t *size);

typedef struct {
    int deep_of_substrings;         // Max deep of regular expression substrings
    int start_index_substring;      // Start substring index to be stored
    int end_index_substring;        // End substring index to be stored
    const char *var_delim;          // Delimiter between values in output format
    int regcomp_flags;              // Regular expression compilation flags (see <regex.h>)
    istream_match_t istream_match;  // The istream_match_t struct
} istream_regex_extract_t;

/**
 * @brief Fill given buffer by blocks from stream based on regular expression.
 * 
 * @param path      The stream path
 * @param regex     The regular expression to be extracted
 * @param params    The parameters defined in @c istream_get_regex_t struct
 * @param buf       The target buffer that is used to read result or error
 * @param size      The size of the buffer in bytes.
 *
 * @return ASC_RESULT_OK on success
 */
asc_result_t istream_regex_extract(const char *path, const char *regex, istream_regex_extract_t *params, char **buf, size_t *size);

/**
 * @brief Check if there is a line in the stream that contains the specified regular expression.
 * 
 * @param path      The stream path
 * @param regex     The regular expression to be matched
 * @param params    The parameters defined in @c istream_match_t struct
 * @param buf       The target buffer that is used to read result or error
 * @param size      The size of the buffer in bytes.
 *
 * @return ASC_RESULT_OK on contains, ASC_RESULT_EMPTY on not contains, otherwise ASC_RESULT_error_code.
 */
asc_result_t istream_regex_match(const char *path, const char *regex, istream_match_t *params, char **buf, size_t *size);

#endif /* __ISTREAM_H__ */
