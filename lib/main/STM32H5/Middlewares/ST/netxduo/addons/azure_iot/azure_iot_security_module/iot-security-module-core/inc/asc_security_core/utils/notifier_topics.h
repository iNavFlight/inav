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

#ifndef __NOTIFIER_GROUPS_H__
#define __NOTIFIER_GROUPS_H__
#include <asc_config.h>

typedef enum {
    NOTIFY_TOPIC_SYSTEM,
    NOTIFY_TOPIC_COLLECT,
    NOTIFY_TOPIC_SECURITY_MODULE_STATE,
    NOTIFY_TOPICS_NUMBER
} notify_topic_t;

enum {
#ifdef ASC_COLLECTORS_INFO_SUPPORT
    NOTIFY_SYSTEM_CONFIGURATION,
#endif

    NOTIFY_MESSAGE_READY,

    NOTIFY_SECURITY_MODULE_CONNECTED,
    NOTIFY_SECURITY_MODULE_PENDING,
    NOTIFY_SECURITY_MODULE_SUSPENDED,
#ifdef ASC_COMPONENT_CORE_SUPPORTS_RESTART
    NOTIFY_SECURITY_MODULE_RESTART
#endif
};

#endif