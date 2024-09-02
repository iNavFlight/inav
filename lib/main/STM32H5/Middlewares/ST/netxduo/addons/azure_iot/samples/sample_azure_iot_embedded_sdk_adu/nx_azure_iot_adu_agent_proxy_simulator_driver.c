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

void nx_azure_iot_adu_agent_proxy_driver(NX_AZURE_IOT_ADU_AGENT_DRIVER *driver_req_ptr);

/****** DRIVER SPECIFIC ******/
void nx_azure_iot_adu_agent_proxy_driver(NX_AZURE_IOT_ADU_AGENT_DRIVER *driver_req_ptr)
{

    /* Default to successful return.  */
    driver_req_ptr -> nx_azure_iot_adu_agent_driver_status = NX_AZURE_IOT_SUCCESS;
        
    /* Process according to the driver request type.  */
    switch (driver_req_ptr -> nx_azure_iot_adu_agent_driver_command)
    {
        
        case NX_AZURE_IOT_ADU_AGENT_DRIVER_INITIALIZE:
        {

            /* Process initialize requests.  */
            printf("Proxy driver initalized successfully.\r\n");
            break;
        }

        case NX_AZURE_IOT_ADU_AGENT_DRIVER_UPDATE_CHECK:
        {

            /* Compare the installed_criteria (such as: version) to check if the update is installed or not.
               If installed, return NX_TRUE, else return NX_FALSE.  */
            *(driver_req_ptr -> nx_azure_iot_adu_agent_driver_return_ptr) = NX_FALSE;

            printf("Proxy driver update check successfully.\r\n");
            break;
        }

        case NX_AZURE_IOT_ADU_AGENT_DRIVER_PREPROCESS:
        {
        
            /* Process firmware preprocess requests before writing firmware.
               Such as: erase the flash at once to improve the speed.  */
            printf("Proxy driver flash erased successfully.\r\n");
    
            break;
        }
            
        case NX_AZURE_IOT_ADU_AGENT_DRIVER_WRITE:
        {
        
            /* Process firmware write requests.  */
            
            /* Write firmware contents.
               1. This function must be able to figure out which bank it should write to.
               2. Write firmware contents into new bank.
               3. Decrypt and authenticate the firmware itself if needed.
            */
            printf("Proxy driver firmware writing...\r\n");
            
            break;
        } 
            
        case NX_AZURE_IOT_ADU_AGENT_DRIVER_INSTALL:
        {

            /* Set the new firmware for next boot.  */
            printf("Proxy driver firmware installed successfully.\r\n");

            break;
        } 
            
        case NX_AZURE_IOT_ADU_AGENT_DRIVER_APPLY:
        {

            /* Apply the new firmware, and reboot device from there.*/
            printf("Proxy driver firmware apply successfully.\r\n");
        
            break;
        } 
        default:
        {

            /* Invalid driver request.  */

            /* Default to successful return.  */
            driver_req_ptr -> nx_azure_iot_adu_agent_driver_status =  NX_AZURE_IOT_FAILURE;
        }
    }
}
