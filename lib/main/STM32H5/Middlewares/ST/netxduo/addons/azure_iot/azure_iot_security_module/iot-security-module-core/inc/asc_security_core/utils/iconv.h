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

#ifndef ICONV_H
#define ICONV_H

#include <stdint.h>

#include <asc_config.h>

typedef struct {
    int code;
    char *string;
} code2string_t;

/**
 * @brief Return the first string from the list which matches code
 *
 * @param list List of strings and their matching code
 * @param code The code that that should be located
 *
 * @return The string that matches the code or NULL in case of failure.
 */
char *code2string(code2string_t *list, int code);

/**
 * @brief Return the first code from the list which matches the string
 *
 * @param list    List of strings and their matching code
 * @param string  The string that should be located
 * @parma len     The length of the string
 *
 * @return The code that matches the string or -1 in case of failure.
 */
int string2code(code2string_t *list, char *string, uint32_t len);

#endif /* ICONV_H */