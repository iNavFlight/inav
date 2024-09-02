/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

#include <asc_config.h>

#include "nx_api.h"
#include "asc_security_core/utils/irand.h"

void irand_srand(uint32_t seed)
{
    NX_SRAND(seed);
}

uint32_t irand_int(void)
{
    return (uint32_t)NX_RAND();
}
