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

#include "asc_security_core/utils/event_loop_be.h"
#include "asc_security_core/utils/ievent_loop.h"

ievent_loop_t *ievent_loop_get_instance(void)
{
    return event_loop_be_instance_attach();
}