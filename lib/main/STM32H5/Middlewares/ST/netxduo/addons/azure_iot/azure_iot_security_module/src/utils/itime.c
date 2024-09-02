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
#include <string.h>

#include <asc_config.h>

#include "asc_security_core/utils/itime.h"

static unix_time_callback_t _time_callback = NULL;

void itime_init(unix_time_callback_t time_callback)
{
    _time_callback = time_callback;
}

unsigned long itime_time(unsigned long *timer)
{
    if (_time_callback == NULL)
    {
        return ITIME_FAILED;
    }

    return _time_callback(timer);
}
