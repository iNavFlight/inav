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

#ifndef IRAND_H
#define IRAND_H

#include <stdint.h>

#include <asc_config.h>

/**
 * @brief Initialize the seed for rand generator.
 */
void irand_srand(uint32_t seed);

/**
 * @brief Generate an integer value between 0 and RAND_MAX.
 *
 * @details The C library function int rand(void) returns a pseudo-random number in the range of 0 to RAND_MAX.
 * RAND_MAX is a constant whose default value may vary between implementations but it is granted to be at least 32767.
 */
uint32_t irand_int(void);


#endif /* IRAND_H */