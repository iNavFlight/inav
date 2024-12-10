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
#include "nx_azure_iot_adu_agent.h"

/* Device properties.  */
#ifndef SAMPLE_DEVICE_MANUFACTURER
#define SAMPLE_DEVICE_MANUFACTURER                                      "Contoso"
#endif /* SAMPLE_DEVICE_MANUFACTURER*/

#ifndef SAMPLE_DEVICE_MODEL
#define SAMPLE_DEVICE_MODEL                                             "IoTDevice"
#endif /* SAMPLE_DEVICE_MODEL */

#ifndef SAMPLE_DEVICE_INSTALLED_CRITERIA
#define SAMPLE_DEVICE_INSTALLED_CRITERIA                                "1.0.0"
#endif /* SAMPLE_DEVICE_INSTALLED_CRITERIA */

#if (NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT >= 1)
/* Device properties.  */
#ifndef SAMPLE_LEAF_DEVICE_MANUFACTURER
#define SAMPLE_LEAF_DEVICE_MANUFACTURER                                 "Contoso"
#endif /* SAMPLE_LEAF_DEVICE_MANUFACTURER*/

#ifndef SAMPLE_LEAF_DEVICE_MODEL
#define SAMPLE_LEAF_DEVICE_MODEL                                        "IoTDevice-Leaf"
#endif /* SAMPLE_LEAF_DEVICE_MODEL */

#ifndef SAMPLE_LEAF_DEVICE_INSTALLED_CRITERIA
#define SAMPLE_LEAF_DEVICE_INSTALLED_CRITERIA                           "1.0.0"
#endif /* SAMPLE_LEAF_DEVICE_INSTALLED_CRITERIA */
#endif /* NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT */

static NX_AZURE_IOT_ADU_AGENT adu_agent;
static UINT adu_agent_started = NX_FALSE;

VOID sample_adu_start(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);

extern void nx_azure_iot_adu_agent_driver(NX_AZURE_IOT_ADU_AGENT_DRIVER *driver_req_ptr);
#if (NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT >= 1)
extern void nx_azure_iot_adu_agent_proxy_driver(NX_AZURE_IOT_ADU_AGENT_DRIVER *driver_req_ptr);
#endif /* NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT */

static void adu_agent_update_notify(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr,
                                    UINT update_state,
                                    UCHAR *provider, UINT provider_length,
                                    UCHAR *name, UINT name_length,
                                    UCHAR *version, UINT version_length)
{

    if (update_state == NX_AZURE_IOT_ADU_AGENT_UPDATE_RECEIVED)
    {

        /* Received new update.  */
        printf("Received new update: Provider: %.*s; Name: %.*s, Version: %.*s\r\n",
               provider_length, provider, name_length, name, version_length, version);

        /* Start to download and install update immediately for testing.  */
        nx_azure_iot_adu_agent_update_download_and_install(adu_agent_ptr);
    }
    else if(update_state == NX_AZURE_IOT_ADU_AGENT_UPDATE_INSTALLED)
    {

        /* Start to apply update immediately for testing.  */
        nx_azure_iot_adu_agent_update_apply(adu_agent_ptr);
    }
}

VOID sample_adu_start(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
    if (adu_agent_started == NX_FALSE)
    {

        /* Start ADU agent.  */
        if (nx_azure_iot_adu_agent_start(&adu_agent, hub_client_ptr,
                                         (const UCHAR *)SAMPLE_DEVICE_MANUFACTURER, sizeof(SAMPLE_DEVICE_MANUFACTURER) - 1,
                                         (const UCHAR *)SAMPLE_DEVICE_MODEL, sizeof(SAMPLE_DEVICE_MODEL) - 1,
                                         (const UCHAR *)SAMPLE_DEVICE_INSTALLED_CRITERIA, sizeof(SAMPLE_DEVICE_INSTALLED_CRITERIA) - 1,
                                         adu_agent_update_notify,
                                         nx_azure_iot_adu_agent_driver))
        {
            printf("Failed on nx_azure_iot_adu_agent_start!\r\n");
            return;
        }

        printf("Device Properties: manufacturer: %s, model: %s, installed criteria: %s.\r\n", SAMPLE_DEVICE_MANUFACTURER, SAMPLE_DEVICE_MODEL, SAMPLE_DEVICE_INSTALLED_CRITERIA);

#if (NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT >= 1)
        /* Enable proxy update for leaf device.  */
        if (nx_azure_iot_adu_agent_proxy_update_add(&adu_agent,
                                                    (const UCHAR *)SAMPLE_LEAF_DEVICE_MANUFACTURER, sizeof(SAMPLE_LEAF_DEVICE_MANUFACTURER) - 1,
                                                    (const UCHAR *)SAMPLE_LEAF_DEVICE_MODEL, sizeof(SAMPLE_LEAF_DEVICE_MODEL) - 1,
                                                    (const UCHAR *)SAMPLE_LEAF_DEVICE_INSTALLED_CRITERIA, sizeof(SAMPLE_LEAF_DEVICE_INSTALLED_CRITERIA) - 1,
                                                    nx_azure_iot_adu_agent_proxy_driver))
        {
            printf("Failed on nx_azure_iot_adu_agent_proxy_update_add!\r\n");
            return;
        }

        printf("Leaf Device Properties: manufacturer: %s, model: %s, installed criteria: %s.\r\n", SAMPLE_LEAF_DEVICE_MANUFACTURER, SAMPLE_LEAF_DEVICE_MODEL, SAMPLE_LEAF_DEVICE_INSTALLED_CRITERIA);
#endif /* NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT */

        adu_agent_started = NX_TRUE;
    }
}
