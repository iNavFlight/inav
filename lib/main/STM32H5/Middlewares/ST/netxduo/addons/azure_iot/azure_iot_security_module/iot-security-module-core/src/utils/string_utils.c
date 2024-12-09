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
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <asc_config.h>

#include "asc_security_core/logger.h"
#include "asc_security_core/utils/string_utils.h"

size_t str_len(const char *s)
{
    return s ? strlen(s) : 0;
}

char *str_str(const char *s1, const char *s2)
{
    if (!s1 || !s2) {
        return NULL;
    }
    return strstr(s1, s2);
}

int str_cmp(const char *s1, const char *s2)
{
    return strcmp(s1 ? s1 : "", s2 ? s2 : "");
}

void str_trim_last_newline(char *s)
{
    size_t len;

    if (!s) {
        return;
    }
    len = str_len(s);

    while (len) {
        if (s[len-1] == '\n' || s[len-1] == '\r') {
            s[len-1] = '\0';
        } else {
           break;
        }
        len --;
    }
}

char *str_trim(char *s)
{
    char *end;

    if (!s) {
        return NULL;
    }
    // Trim leading space
    while(isspace((unsigned char)*s)) {
        s++;
    }
    // All spaces?
    if(*s == '\0') {
        return s;
    }
    // Trim trailing space
    end = s + str_len(s) - 1;
    while(end > s && isspace((unsigned char)*end)) {
        end--;
    }
    // Write new null terminator character
    end[1] = '\0';

    return s;
}

bool str_isempty(const char *s)
{
    return !s || !*s;
}

const char *str_value_or_empty(const char *s)
{
    return (s == NULL) ? "" : s;
}

bool str_is_blank(const char *s)
{
    if (s == NULL) {
        return true;
    }

    while (*s != '\0') {
        if (!isspace((unsigned char)*s)) {
            return false;
        }

        s++;
    }

    return true;
}

asc_result_t str_split(char *s, char **token, size_t *token_len, char **rest, size_t *rest_len, char *delimiter)
{
    asc_result_t result = ASC_RESULT_BAD_ARGUMENT;
    char *p = str_str(s, delimiter);
    size_t index = 0;

    if (p == NULL) {
        log_error("Failed to find '%s' in string (s=%s)", delimiter, s);
        goto cleanup;
    }

    index = (str_len(s) - str_len(p));

    if (index <= 0 || index+1 >= str_len(s)) {
        log_error("Invalid key format (s=%s)", s);
        goto cleanup;
    }

    *token = s;
    *token_len = index;
    *rest = p + str_len(delimiter);
    *rest_len = str_len(s) - index - str_len(delimiter);
    result = ASC_RESULT_OK;

cleanup:
    return result;
}

int str_ncmp(const char *s1, size_t s1_len, const char *s2, size_t s2_len)
{
    if (s1_len < s2_len) {
        return -1;
    }

    if (s2_len < s1_len) {
        return 1;
    }

    /* s1_len == s2_len */
    return strncmp(s1 ? s1 : "", s2 ? s2 : "", s1_len);
}

bool str_starts_from(const char *s1, const char *s2)
{
    char *p = str_str(s1, s2);
    return (p==s1);
}

char *replace_chars(char *char_array, size_t len, char replace_from, char replace_to)
{
    for (size_t j = 0 ; j < len; j++)
    {
        if (char_array[j] == replace_from) {
             char_array[j] = replace_to;
        }           
    }

    return char_array;
}
