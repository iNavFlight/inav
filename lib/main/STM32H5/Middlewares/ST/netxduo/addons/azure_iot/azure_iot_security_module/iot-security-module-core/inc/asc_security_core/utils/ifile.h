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

#ifndef __IFILE_H__
#define __IFILE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <asc_config.h>

#include "asc_security_core/asc_result.h"

#include "asc_security_core/utils/istream.h"

/**
 * @brief Read a file into the given buffer.
 *
 * @param path  The path of the file
 * @param mode  The mode of the file
 * @param buf   The buffer to write to
 * @param size  The size of the buffer in bytes
 *
 * @return size_t The number of bytes read or 0 in case of an error.
 */
size_t ifile_read(const char *path, const char *mode, char *buf, size_t size);

/**
 * @brief Read a file data with allocating buffer.
 *
 * @param path  The path of the file.
 * @param mode  The mode of the file.
 * @param size  The number of bytes read.
 *
 * @return char* The allocated buffer or NULL in case of an error.
 */
char *ifile_alloc_read(const char *path, const char *mode, size_t *size);

/**
 * @brief Write a buffer into a file.
 *
 * @param path  The path of the file
 * @param mode  The mode of the file
 * @param buf   The buffer that should be written in the file
 * @param size  The size of the buffer in bytes.
 *
 * @return ASC_RESULT_OK on success
 */
asc_result_t ifile_write(const char *path, const char *mode, char *buf, size_t size);

typedef struct {
    void *username;                 // Pointer on previous allocated valid uid's [user1, user2]
    void *groupname;                // Pointer on previous allocated valid gid's [group1, group2]
    const char *permissions;        // Pointer on previous allocated 2 initial bytes for restriction (== or <=) and the rest for decimal repr of permissions (e.g <=777)
    istream_match_t istream_match; // The istream_match_t struct
} ifile_stat_t;

/**
 * @brief Check if the given stat struct is matching to the real stat of the file given by path
 *
 * @param path  The path of the file
 * @param s     stat struct describing the exepected stat of the file at path
 * @param buf       The target buffer that is used to read result or error
 * @param size      The size of the buffer in bytes.
 * 
 * @return ASC_RESULT_OK on equals, ASC_RESULT_EMPTY on not equal, otherwise ASC_RESULT_error_code.
 */
asc_result_t ifile_validate_stat(const char *path, ifile_stat_t *expected_stat, char **buf, size_t *size);

#endif /* __IFILE_H__ */
