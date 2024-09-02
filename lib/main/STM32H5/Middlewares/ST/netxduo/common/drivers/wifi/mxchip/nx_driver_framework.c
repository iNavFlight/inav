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


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Component                                                        */
/**                                                                       */
/**   NetX driver framework                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include <inttypes.h>

/* Check that this file is included by the driver source and not compiled directly  */
#ifndef NX_DRIVER_SOURCE
#error This file is included by the driver source, not compiled directly.
#endif /* NX_DRIVER_SOURCE */

/* Include driver framework include file.  */
#include "nx_driver_framework.h"

NX_DRIVER_INFORMATION nx_driver_information;

static VOID         nx_driver_framework_entry_default(NX_IP_DRIVER *driver_req_ptr);

/* Define the routines for processing each driver entry request.  */
static VOID         nx_driver_interface_attach(NX_IP_DRIVER *driver_req_ptr);
static VOID         nx_driver_initialize(NX_IP_DRIVER *driver_req_ptr);
static VOID         nx_driver_enable(NX_IP_DRIVER *driver_req_ptr);
static VOID         nx_driver_disable(NX_IP_DRIVER *driver_req_ptr);
static VOID         nx_driver_packet_send(NX_IP_DRIVER *driver_req_ptr);
static VOID         nx_driver_multicast_join(NX_IP_DRIVER *driver_req_ptr);
static VOID         nx_driver_multicast_leave(NX_IP_DRIVER *driver_req_ptr);
static VOID         nx_driver_get_status(NX_IP_DRIVER *driver_req_ptr);
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
static VOID         nx_driver_capability_get(NX_IP_DRIVER *driver_req_ptr);
static VOID         nx_driver_capability_set(NX_IP_DRIVER *driver_req_ptr);
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
#ifdef NX_DRIVER_ENABLE_DEFERRED
static VOID         nx_driver_deferred_processing(NX_IP_DRIVER *driver_req_ptr);
#endif /* NX_DRIVER_ENABLE_DEFERRED */
static VOID         nx_driver_transfer_to_netx(NX_IP *ip_ptr, NX_PACKET *packet_ptr);
static VOID         nx_driver_update_hardware_address(UCHAR hardware_address[6]);
#ifdef NX_DRIVER_INTERNAL_TRANSMIT_QUEUE
static VOID         nx_driver_transmit_packet_enqueue(NX_PACKET *packet_ptr)
static NX_PACKET   *nx_driver_transmit_packet_dequeue(VOID)
#endif /* NX_DRIVER_INTERNAL_TRANSMIT_QUEUE */


/* Define the pointers for the hardware implementation of this driver. */
static UINT(*nx_driver_hardware_initialize)(NX_IP_DRIVER *driver_req_ptr)      = NULL;
static UINT(*nx_driver_hardware_enable)(NX_IP_DRIVER *driver_req_ptr)          = NULL;
static UINT(*nx_driver_hardware_disable)(NX_IP_DRIVER *driver_req_ptr)         = NULL;
static UINT(*nx_driver_hardware_packet_send)(NX_PACKET *packet_ptr)            = NULL;
static UINT(*nx_driver_hardware_multicast_join)(NX_IP_DRIVER *driver_req_ptr)  = NULL;
static UINT(*nx_driver_hardware_multicast_leave)(NX_IP_DRIVER *driver_req_ptr) = NULL;
static UINT(*nx_driver_hardware_get_status)(NX_IP_DRIVER *driver_req_ptr)      = NULL;
static VOID (*nx_driver_hardware_packet_transmitted)(VOID)                      = NULL;
static VOID (*nx_driver_hardware_packet_received)(VOID)                         = NULL;
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
static UINT(*nx_driver_hardware_capability_set)(NX_IP_DRIVER *driver_req_ptr)  = NULL;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */



#ifdef NX_DEBUG
static const char *nx_driver_operation_to_string(UINT operation);
#endif /* NX_DEBUG */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_driver_framework_entry_default                   PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Andres Mlinar, Microsoft Corporation                                */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This is the default entry point processing for the NetX Driver.     */
/*    This function should be called by a specific driver to handle       */
/*    commands.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    driver_req_ptr                        The driver request from the   */
/*                                            IP layer.                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_driver_interface_attach            Process attach request        */
/*    nx_driver_initialize                  Process initialize request    */
/*    nx_driver_enable                      Process link enable request   */
/*    nx_driver_disable                     Process link disable request  */
/*    nx_driver_packet_send                 Process send packet requests  */
/*    nx_driver_multicast_join              Process multicast join request*/
/*    nx_driver_multicast_leave             Process multicast leave req   */
/*    nx_driver_get_status                  Process get status request    */
/*    nx_driver_deferred_processing         Drive deferred processing     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    IP layer                                                            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Andres Mlinar            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
static VOID nx_driver_framework_entry_default(NX_IP_DRIVER *driver_req_ptr)
{
#ifdef NX_DEBUG
  NX_IP *ip_ptr;
#endif /* NX_DEBUG */

#ifdef NX_DEBUG
  /* Setup the IP pointer from the driver request.  */
  ip_ptr =  driver_req_ptr -> nx_ip_driver_ptr;

  printf("\n[%06"PRIu32"] > %s\n", HAL_GetTick(),
         nx_driver_operation_to_string(driver_req_ptr -> nx_ip_driver_command));
#endif /* NX_DEBUG */

  /* Default to successful return.  */
  driver_req_ptr -> nx_ip_driver_status =  NX_SUCCESS;

  /* Process according to the driver request type in the IP control block.  */
  switch (driver_req_ptr -> nx_ip_driver_command)
  {

    case NX_LINK_INTERFACE_ATTACH:
    {
      /* Process link interface attach requests.  */
      nx_driver_interface_attach(driver_req_ptr);
      break;
    }

    case NX_LINK_INITIALIZE:
    {
#ifdef NX_DEBUG
      printf("\nNetX WiFi Driver Initialization - \"%s\"\n", ip_ptr -> nx_ip_name);
      printf("  IP Address = %08"PRIX32"\n", (uint32_t)ip_ptr -> nx_ip_address);
#endif /* NX_DEBUG */

      /* Process link initialize requests.  */
      nx_driver_initialize(driver_req_ptr);
      break;
    }

    case NX_LINK_ENABLE:
    {
      /* Process link enable requests.  */
      nx_driver_enable(driver_req_ptr);
#ifdef NX_DEBUG
      printf("\nNetX WiFi Driver Link Enabled - \"%s\"\n", ip_ptr -> nx_ip_name);
#endif /* NX_DEBUG */
      break;
    }

    case NX_LINK_DISABLE:
    {
      /* Process link disable requests.  */
      nx_driver_disable(driver_req_ptr);

#ifdef NX_DEBUG
      printf("\nNetX WiFi Driver Link Disabled - \"%s\"\n", ip_ptr -> nx_ip_name);
#endif /* NX_DEBUG */
      break;
    }

    case NX_LINK_ARP_SEND:
    case NX_LINK_ARP_RESPONSE_SEND:
    case NX_LINK_PACKET_BROADCAST:
    case NX_LINK_RARP_SEND:
    case NX_LINK_PACKET_SEND:
    {
      /* Process packet send requests.  */
      nx_driver_packet_send(driver_req_ptr);
      break;
    }

    case NX_LINK_MULTICAST_JOIN:
    {
      /* Process multicast join requests.  */
      nx_driver_multicast_join(driver_req_ptr);
      break;
    }

    case NX_LINK_MULTICAST_LEAVE:
    {
      /* Process multicast leave requests.  */
      nx_driver_multicast_leave(driver_req_ptr);
      break;
    }

    case NX_LINK_GET_STATUS:
    {
      /* Process get status requests.  */
      nx_driver_get_status(driver_req_ptr);
      break;
    }

#ifdef NX_DRIVER_ENABLE_DEFERRED
    case NX_LINK_DEFERRED_PROCESSING:
    {
      /* Process driver deferred requests.  */

      /* Process a device driver function on behave of the IP thread. */
      nx_driver_deferred_processing(driver_req_ptr);
      break;
    }
#endif /* NX_DRIVER_ENABLE_DEFERRED */

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    case NX_INTERFACE_CAPABILITY_GET:
    {
      /* Process get capability requests.  */
      nx_driver_capability_get(driver_req_ptr);
      break;
    }

    case NX_INTERFACE_CAPABILITY_SET:
    {
      /* Process set capability requests.  */
      nx_driver_capability_set(driver_req_ptr);
      break;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

    default:

      /* Invalid driver request.  */

      /* Return the unhandled command status.  */
      driver_req_ptr -> nx_ip_driver_status =  NX_UNHANDLED_COMMAND;

      /* Default to successful return.  */
      driver_req_ptr -> nx_ip_driver_status =  NX_DRIVER_ERROR;

#ifdef NX_DEBUG
      printf("\nNetX WiFi Driver Received invalid request - \"%s\"\n", ip_ptr -> nx_ip_name);
#endif /* NX_DEBUG */
  }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_driver_interface_attach                          PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Andres Mlinar, Microsoft Corporation                                */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processing the interface attach request.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    driver_req_ptr                        Driver command from the IP    */
/*                                            thread                      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Driver entry function                                               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Andres Mlinar            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
static VOID nx_driver_interface_attach(NX_IP_DRIVER *driver_req_ptr)
{
  /*
   * Setup the driver's interface.
   * This default implementation is for a simple one-interface driver.
   * Additional logic is necessary for multiple port devices.
   */
  nx_driver_information.nx_driver_information_interface = driver_req_ptr -> nx_ip_driver_interface;

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
  driver_req_ptr -> nx_ip_driver_interface -> nx_interface_capability_flag = NX_DRIVER_CAPABILITY;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

  /* Return successful status.  */
  driver_req_ptr -> nx_ip_driver_status =  NX_SUCCESS;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_driver_initialize                                PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Andres Mlinar, Microsoft Corporation                                */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processing the initialize request.  The processing    */
/*    in this function is generic. All hardware specific logic goes in    */
/*    nx_driver_hardware_initialize.                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    driver_req_ptr                        Driver command from the IP    */
/*                                            thread                      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_driver_hardware_initialize         Process initialize request    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Driver entry function                                               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Andres Mlinar            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
static VOID  nx_driver_initialize(NX_IP_DRIVER *driver_req_ptr)
{
  NX_IP           *ip_ptr;
  NX_INTERFACE    *interface_ptr;
  UINT            status;

  /* Setup the IP pointer from the driver request.  */
  ip_ptr =  driver_req_ptr -> nx_ip_driver_ptr;

  /* Setup interface pointer.  */
  interface_ptr = driver_req_ptr -> nx_ip_driver_interface;

  /* Initialize the driver's information structure.  */

  /* Default IP pointer to NULL.  */
  nx_driver_information.nx_driver_information_ip_ptr = NX_NULL;

  /* Setup the driver state to not initialized.  */
  nx_driver_information.nx_driver_information_state = NX_DRIVER_STATE_NOT_INITIALIZED;

  /* Setup the default packet pool for the driver's received packets.  */
  nx_driver_information.nx_driver_information_packet_pool_ptr = ip_ptr -> nx_ip_default_packet_pool;

  /* Clear the deferred events for the driver.  */
  nx_driver_information.nx_driver_information_deferred_events = 0;

#ifdef NX_DRIVER_INTERNAL_TRANSMIT_QUEUE
  /* Clear the transmit queue count and head pointer.  */
  nx_driver_information.nx_driver_transmit_packets_queued =  0;
  nx_driver_information.nx_driver_transmit_queue_head = NX_NULL;
  nx_driver_information.nx_driver_transmit_queue_tail = NX_NULL;
#endif /* NX_DRIVER_INTERNAL_TRANSMIT_QUEUE */

  /* Call the hardware-specific WiFi module initialization.  */
  if (!nx_driver_hardware_initialize)
  {
    status = NX_SUCCESS;
  }
  else
  {
    status = nx_driver_hardware_initialize(driver_req_ptr);
  }

  /* Determine if the request was successful.  */
  if (status == NX_SUCCESS)
  {
    /* Successful hardware initialization.  */

    /* Setup driver information to point to IP pointer.  */
    nx_driver_information.nx_driver_information_ip_ptr = driver_req_ptr -> nx_ip_driver_ptr;

    /* Setup the link maximum transfer unit. */
    interface_ptr -> nx_interface_ip_mtu_size = NX_DRIVER_MTU - NX_DRIVER_PHYSICAL_FRAME_SIZE;

    /* Indicate to the IP software that IP to physical mapping
       is required.  */
    interface_ptr -> nx_interface_address_mapping_needed = NX_TRUE;

    /* Move the driver's state to initialized.  */
    nx_driver_information.nx_driver_information_state = NX_DRIVER_STATE_INITIALIZED;

    /* Indicate successful initialize.  */
    driver_req_ptr -> nx_ip_driver_status = NX_SUCCESS;
  }
  else
  {
    /* Initialization failed. Indicate that the request failed.  */
    driver_req_ptr -> nx_ip_driver_status = NX_DRIVER_ERROR;
  }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_driver_enable                                    PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Andres Mlinar, Microsoft Corporation                                */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processing the initialize request. The processing     */
/*    in this function is generic. All hardware specific logic in         */
/*    nx_driver_hardware_enable.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    driver_req_ptr                        Driver command from the IP    */
/*                                            thread                      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_driver_hardware_enable             Process enable request        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Driver entry function                                               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Andres Mlinar            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
static VOID nx_driver_enable(NX_IP_DRIVER *driver_req_ptr)
{
  NX_IP *ip_ptr;
  UINT   status;

  /* Setup the IP pointer from the driver request.  */
  ip_ptr = driver_req_ptr -> nx_ip_driver_ptr;

  /* See if we can honor the NX_LINK_ENABLE request.  */
  if (nx_driver_information.nx_driver_information_state < NX_DRIVER_STATE_INITIALIZED)
  {
    /* Mark the request as not successful.  */
    driver_req_ptr -> nx_ip_driver_status = NX_DRIVER_ERROR;
    return;
  }

  /* Check if it is enabled by someone already */
  if (nx_driver_information.nx_driver_information_state >= NX_DRIVER_STATE_LINK_ENABLED)
  {
    /* Yes, the request has already been made.  */
    driver_req_ptr -> nx_ip_driver_status = NX_ALREADY_ENABLED;
    return;
  }

  /* Call hardware specific enable.  */
  if (!nx_driver_hardware_enable)
  {
    status = NX_SUCCESS;
  }
  else
  {
    status = nx_driver_hardware_enable(driver_req_ptr);
  }

  /* Was the hardware enable successful?  */
  if (status == NX_SUCCESS)
  {
    /* Update the driver state to link enabled.  */
    nx_driver_information.nx_driver_information_state = NX_DRIVER_STATE_LINK_ENABLED;

    /* Mark request as successful.  */
    driver_req_ptr -> nx_ip_driver_status = NX_SUCCESS;

    /* Mark the IP instance as link up.  */
    ip_ptr -> nx_ip_driver_link_up = NX_TRUE;
  }
  else
  {
    /* Enable failed.  Indicate that the request failed.  */
    driver_req_ptr -> nx_ip_driver_status = NX_DRIVER_ERROR;
  }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_driver_disable                                   PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Andres Mlinar, Microsoft Corporation                                */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processing the disable request. The processing        */
/*    in this function is generic. All hardware specific logic in         */
/*    nx_driver_hardware_disable.                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    driver_req_ptr                        Driver command from the IP    */
/*                                            thread                      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_driver_hardware_disable            Process disable request       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Driver entry function                                               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Andres Mlinar            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
static VOID nx_driver_disable(NX_IP_DRIVER *driver_req_ptr)
{
  NX_IP *ip_ptr;
  UINT   status;

  /* Setup the IP pointer from the driver request.  */
  ip_ptr = driver_req_ptr -> nx_ip_driver_ptr;

  /* Check if the link is enabled.  */
  if (nx_driver_information.nx_driver_information_state != NX_DRIVER_STATE_LINK_ENABLED)
  {
    /* The link is not enabled, so just return an error.  */
    driver_req_ptr -> nx_ip_driver_status = NX_DRIVER_ERROR;
    return;
  }

  /* Call hardware specific disable.  */
  if (!nx_driver_hardware_disable)
  {
    status = NX_SUCCESS;
  }
  else
  {
    status = nx_driver_hardware_disable(driver_req_ptr);
  }

  /* Was the hardware disable successful?  */
  if (status == NX_SUCCESS)
  {
    /* Mark the IP instance as link down.  */
    ip_ptr -> nx_ip_driver_link_up = NX_FALSE;

    /* Update the driver state back to initialized.  */
    nx_driver_information.nx_driver_information_state = NX_DRIVER_STATE_INITIALIZED;

    /* Mark request as successful.  */
    driver_req_ptr -> nx_ip_driver_status = NX_SUCCESS;
  }
  else
  {
    /* Disable failed, return an error.  */
    driver_req_ptr -> nx_ip_driver_status = NX_DRIVER_ERROR;
  }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_driver_packet_send                               PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Andres Mlinar, Microsoft Corporation                                */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processing the packet send request. The processing    */
/*    in this function is generic. All hardware specific logic is in      */
/*    nx_driver_hardware_packet_send.                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    driver_req_ptr                        Driver command from the IP    */
/*                                            thread                      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_driver_hardware_packet_send        Process packet send request   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Driver entry function                                               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Andres Mlinar            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
static VOID nx_driver_packet_send(NX_IP_DRIVER *driver_req_ptr)
{
  NX_IP           *ip_ptr;
  NX_PACKET       *packet_ptr;
  ULONG           *ethernet_frame_ptr;
  UINT            status;

  /* Setup the IP pointer from the driver request.  */
  ip_ptr = driver_req_ptr -> nx_ip_driver_ptr;

  /* Check to make sure the link is up.  */
  if (nx_driver_information.nx_driver_information_state != NX_DRIVER_STATE_LINK_ENABLED)
  {
    /* Indicate an unsuccessful packet send.  */
    driver_req_ptr -> nx_ip_driver_status =  NX_DRIVER_ERROR;

    /* Link is not up, simply free the packet.  */
    nx_packet_transmit_release(driver_req_ptr -> nx_ip_driver_packet);
    return;
  }

  /* Process driver send packet.  */

  /* Place the Ethernet frame at the front of the packet.  */
  packet_ptr = driver_req_ptr -> nx_ip_driver_packet;

#ifdef NX_DEBUG
  printf("\n[%06"PRIu32"] nx_driver_packet_send() with 0x%"PRIX32" bytes\n", HAL_GetTick(),
         (uint32_t)packet_ptr -> nx_packet_length);
#endif /* NX_DEBUG */

  /* Adjust the prepend pointer.  */
  packet_ptr -> nx_packet_prepend_ptr =
    packet_ptr -> nx_packet_prepend_ptr - NX_DRIVER_PHYSICAL_FRAME_SIZE;

  /* Adjust the packet length.  */
  packet_ptr -> nx_packet_length = packet_ptr -> nx_packet_length + NX_DRIVER_PHYSICAL_FRAME_SIZE;

  /* Setup the ethernet frame pointer to build the ethernet frame.
   * Backup another 2 * bytes to get 32-bit word alignment.
   */
  ethernet_frame_ptr = (ULONG *)(packet_ptr -> nx_packet_prepend_ptr - 2);

  /* Set up the hardware addresses in the Ethernet header. */
  *ethernet_frame_ptr       =  driver_req_ptr -> nx_ip_driver_physical_address_msw;
  *(ethernet_frame_ptr + 1) =  driver_req_ptr -> nx_ip_driver_physical_address_lsw;

  *(ethernet_frame_ptr + 2) = (ip_ptr -> nx_ip_arp_physical_address_msw << 16) |
                              (ip_ptr -> nx_ip_arp_physical_address_lsw >> 16);
  *(ethernet_frame_ptr + 3) = (ip_ptr -> nx_ip_arp_physical_address_lsw << 16);

  /* Set up the frame type field in the Ethernet header. */
  if ((driver_req_ptr -> nx_ip_driver_command == NX_LINK_ARP_SEND) ||
      (driver_req_ptr -> nx_ip_driver_command == NX_LINK_ARP_RESPONSE_SEND))
  {
    *(ethernet_frame_ptr + 3) |= NX_DRIVER_ETHERNET_ARP;
  }
  else if (driver_req_ptr -> nx_ip_driver_command == NX_LINK_RARP_SEND)
  {
    *(ethernet_frame_ptr + 3) |= NX_DRIVER_ETHERNET_RARP;
  }
#ifdef FEATURE_NX_IPV6
  else if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6)
  {
    *(ethernet_frame_ptr + 3) |= NX_DRIVER_ETHERNET_IPV6;
  }
#endif /* FEATURE_NX_IPV6 */
  else
  {
    *(ethernet_frame_ptr + 3) |= NX_DRIVER_ETHERNET_IP;
  }

  /* Endian swapping if NX_LITTLE_ENDIAN is defined.  */
  NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr));
  NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr + 1));
  NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr + 2));
  NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr + 3));

  /* Determine if the packet exceeds the driver's MTU.  */
  if (packet_ptr -> nx_packet_length > NX_DRIVER_MTU)
  {
    /* This packet exceeds the size of the driver's MTU. Simply throw it away! */

    /* Remove the Ethernet header.  */
    NX_DRIVER_PHYSICAL_HEADER_REMOVE(packet_ptr);

    /* Indicate an unsuccessful packet send.  */
    driver_req_ptr -> nx_ip_driver_status =  NX_DRIVER_ERROR;

    /* Link is not up, simply free the packet.  */
    nx_packet_transmit_release(packet_ptr);
    return;
  }

  /* Transmit the packet through the WiFi controller low level access routine. */
  if (!nx_driver_hardware_packet_send)
  {
    status = NX_DRIVER_ERROR;
  }
  else
  {
#ifdef NX_DEBUG
    printf("\n* %"PRIX32" %"PRIX32" %"PRIX32" %"PRIX32" *\n",
           ethernet_frame_ptr[0], ethernet_frame_ptr[1], ethernet_frame_ptr[2], ethernet_frame_ptr[3]);
#endif /* NX_DEBUG */

    status = nx_driver_hardware_packet_send(packet_ptr);
  }

  /* Determine if there was an error.  */
  if (status != NX_SUCCESS)
  {
    /* Driver's hardware send packet routine failed to send the packet.  */

    /* Remove the Ethernet header.  */
    NX_DRIVER_PHYSICAL_HEADER_REMOVE(packet_ptr);

    /* Indicate an unsuccessful packet send.  */
    driver_req_ptr -> nx_ip_driver_status = NX_DRIVER_ERROR;

    /* Link is not up, simply free the packet.  */
    nx_packet_transmit_release(packet_ptr);
  }
  else
  {
    /* Set the status of the request.  */
    driver_req_ptr -> nx_ip_driver_status = NX_SUCCESS;
  }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_driver_multicast_join                            PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Andres Mlinar, Microsoft Corporation                                */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processing the multicast join request. The processing */
/*    in this function is generic. All hardware specific logic is in      */
/*    nx_driver_hardware_multicast_join.                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    driver_req_ptr                        Driver command from the IP    */
/*                                            thread                      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_driver_hardware_multicast_join     Process multicast join request*/
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Driver entry function                                               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Andres Mlinar            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
static VOID nx_driver_multicast_join(NX_IP_DRIVER *driver_req_ptr)
{
  UINT status;

  /* Call hardware specific multicast join function. */
  if (!nx_driver_hardware_multicast_join)
  {
    status = NX_SUCCESS;
  }
  else
  {
    status = nx_driver_hardware_multicast_join(driver_req_ptr);
  }

  /* Determine if there was an error.  */
  if (status != NX_SUCCESS)
  {
    /* Indicate an unsuccessful request.  */
    driver_req_ptr -> nx_ip_driver_status =  NX_DRIVER_ERROR;
#ifdef NX_DEBUG
    printf("\nNetX WiFi Driver multicast join returns: NX_DRIVER_ERROR\n");
#endif /* NX_DEBUG */    /* Call hardware specific multicast join function. */
  }
  else
  {
    /* Indicate the request was successful.   */
    driver_req_ptr -> nx_ip_driver_status =  NX_SUCCESS;
  }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_driver_multicast_leave                           PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Andres Mlinar, Microsoft Corporation                                */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processing the multicast leave request. The code      */
/*    in this function is generic. All hardware specific logic is in      */
/*    nx_driver_hardware_multicast_leave.                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    driver_req_ptr                        Driver command from the IP    */
/*                                            thread                      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_driver_hardware_multicast_leave    Process multicast leave req   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Driver entry function                                               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Andres Mlinar            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
static VOID nx_driver_multicast_leave(NX_IP_DRIVER *driver_req_ptr)
{
  UINT status;

  /* Call hardware specific multicast leave function. */
  if (!nx_driver_hardware_multicast_leave)
  {
    status = NX_SUCCESS;
  }
  else
  {
    status = nx_driver_hardware_multicast_leave(driver_req_ptr);
  }

  /* Determine if there was an error.  */
  if (status != NX_SUCCESS)
  {
    /* Indicate an unsuccessful request.  */
    driver_req_ptr -> nx_ip_driver_status =  NX_DRIVER_ERROR;
#ifdef NX_DEBUG
    printf("\nNetX WiFi Driver multicast leave returns: NX_DRIVER_ERROR\n");
#endif /* NX_DEBUG */
  }
  else
  {
    /* Indicate the request was successful.   */
    driver_req_ptr -> nx_ip_driver_status =  NX_SUCCESS;
  }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_driver_get_status                                PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Andres Mlinar, Microsoft Corporation                                */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processing the get status request. The processing     */
/*    in this function is generic. All hardware specific logic is in      */
/*    nx_driver_hardware_get_status.                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    driver_req_ptr                        Driver command from the IP    */
/*                                            thread                      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_driver_hardware_get_status         Process get status request    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Driver entry function                                               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Andres Mlinar            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
static VOID nx_driver_get_status(NX_IP_DRIVER *driver_req_ptr)
{
  /* Call hardware specific get status function. */
  if (!nx_driver_hardware_get_status)
  {
    driver_req_ptr -> nx_ip_driver_status = NX_UNHANDLED_COMMAND;
  }
  else
  {
    UINT status = nx_driver_hardware_get_status(driver_req_ptr);
    if (status == NX_SUCCESS)
    {
      /* Indicate the request was successful.   */
      driver_req_ptr -> nx_ip_driver_status = NX_SUCCESS;
    }
    else
    {
      /* Indicate an unsuccessful request.  */
      driver_req_ptr -> nx_ip_driver_status = NX_DRIVER_ERROR;
    }
  }
}


#ifdef NX_ENABLE_INTERFACE_CAPABILITY
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_driver_capability_get                            PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Andres Mlinar, Microsoft Corporation                                */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processing the get capability request.                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    driver_req_ptr                        Driver command from the IP    */
/*                                            thread                      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Driver entry function                                               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Andres Mlinar            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
static VOID nx_driver_capability_get(NX_IP_DRIVER *driver_req_ptr)
{
  /* Return the capability of the WiFi controller. */
  *(driver_req_ptr -> nx_ip_driver_return_ptr) = NX_DRIVER_CAPABILITY;

  /* Return the success status.  */
  driver_req_ptr -> nx_ip_driver_status = NX_SUCCESS;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_driver_capability_set                          PORTABLE C        */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Andres Mlinar, Microsoft Corporation                                */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processing the set capability request.                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    driver_req_ptr                        Driver command from the IP    */
/*                                            thread                      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Driver entry function                                               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Andres Mlinar            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
static VOID nx_driver_capability_set(NX_IP_DRIVER *driver_req_ptr)
{
  UINT status;

  /* Call hardware specific get status function. */
  if (!nx_driver_hardware_capability_set)
  {
    status = NX_SUCCESS;
  }
  else
  {
    status = nx_driver_hardware_capability_set(driver_req_ptr);
  }

  /* Determine if there was an error.  */
  if (status != NX_SUCCESS)
  {
    /* Indicate an unsuccessful request.  */
    driver_req_ptr -> nx_ip_driver_status = NX_DRIVER_ERROR;
  }
  else
  {
    /* Indicate the request was successful.   */
    driver_req_ptr -> nx_ip_driver_status = NX_SUCCESS;
  }
}
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */


#ifdef NX_DRIVER_ENABLE_DEFERRED
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_driver_deferred_processing                       PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Andres Mlinar, Microsoft Corporation                                */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processing the deferred ISR action within the context */
/*    of the IP thread.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    driver_req_ptr                        Driver command from the IP    */
/*                                            thread                      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_driver_packet_transmitted         Clean up after transmission    */
/*    nx_driver_packet_received            Process a received packet      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Driver entry function                                               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Andres Mlinar            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
static VOID nx_driver_deferred_processing(NX_IP_DRIVER *driver_req_ptr)
{
  TX_INTERRUPT_SAVE_AREA

  ULONG deferred_events;

  /* Disable interrupts.  */
  TX_DISABLE

  /* Pickup deferred events.  */
  deferred_events = nx_driver_information.nx_driver_information_deferred_events;
  nx_driver_information.nx_driver_information_deferred_events = 0;

  /* Restore interrupts.  */
  TX_RESTORE

  /* Check for a transmit complete event.  */
  if (deferred_events & NX_DRIVER_DEFERRED_PACKET_TRANSMITTED)
  {
    /* Process transmitted packet(s).  */
    if (nx_driver_hardware_packet_transmitted)
    {
      nx_driver_hardware_packet_transmitted();
    }
  }

  /* Check for received packet.  */
  if (deferred_events & NX_DRIVER_DEFERRED_PACKET_RECEIVED)
  {
    /* Process received packet(s).  */
    if (nx_driver_hardware_packet_received)
    {
      nx_driver_hardware_packet_received();
    }
  }

  /* Mark request as successful.  */
  driver_req_ptr->nx_ip_driver_status = NX_SUCCESS;
}
#endif /* NX_DRIVER_ENABLE_DEFERRED */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_driver_transfer_to_netx                          PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Andres Mlinar, Microsoft Corporation                                */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processing incoming packets.  This routine would      */
/*    be called from the driver-specific receive packet processing        */
/*    function nx_driver_hardware_packet_received.                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP protocol block  */
/*    packet_ptr                            Packet pointer                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Error indication                                                    */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_packet_receive                 NetX IP packet receive        */
/*    _nx_ip_packet_deferred_receive        NetX IP packet receive        */
/*    _nx_arp_packet_deferred_receive       NetX ARP packet receive       */
/*    _nx_rarp_packet_deferred_receive      NetX RARP packet receive      */
/*    _nx_packet_release                    Release packet                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    nx_driver_hardware_packet_received    Driver packet receive function*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Andres Mlinar            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
static VOID nx_driver_transfer_to_netx(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

  /* Pickup the packet header to determine where the packet needs to be sent. */
  const USHORT packet_type = (USHORT)(((UINT)(*(packet_ptr -> nx_packet_prepend_ptr + 12))) << 8) |
                             ((UINT)(*(packet_ptr -> nx_packet_prepend_ptr + 13)));

  /* Set the interface for the incoming packet.  */
  packet_ptr -> nx_packet_ip_interface = nx_driver_information.nx_driver_information_interface;

  /* Route the incoming packet according to its Ethernet type. */
  if ((packet_type == NX_DRIVER_ETHERNET_IP) || (packet_type == NX_DRIVER_ETHERNET_IPV6))
  {
    /* Note:  The length reported by some Ethernet hardware includes
       bytes after the packet as well as the Ethernet header.  In some
       cases, the actual packet length after the Ethernet header should
       be derived from the length in the IP header (lower 16 bits of
       the first 32-bit word).  */

    /* Clean off the Ethernet header.  */
    packet_ptr -> nx_packet_prepend_ptr =
      packet_ptr -> nx_packet_prepend_ptr + NX_DRIVER_PHYSICAL_FRAME_SIZE;

    /* Adjust the packet length.  */
    packet_ptr -> nx_packet_length =
      packet_ptr -> nx_packet_length - NX_DRIVER_PHYSICAL_FRAME_SIZE;

#ifdef NX_DEBUG
    printf("\n[%06"PRIu32"] < NX_DRIVER_ETHERNET_IP(V6) with %"PRIu32" bytes\n", HAL_GetTick(),
           (uint32_t)packet_ptr -> nx_packet_length);

    const ULONG ip_tcp_headers_size = 40;
    for (ULONG i = ip_tcp_headers_size ; i < (packet_ptr -> nx_packet_length - ip_tcp_headers_size); i++)
    {
      printf("%02X", packet_ptr -> nx_packet_prepend_ptr[i]);
    }
#endif /* NX_DEBUG */

    /* Route to the ip receive function.  */
#ifdef NX_DRIVER_ENABLE_DEFERRED
    _nx_ip_packet_deferred_receive(ip_ptr, packet_ptr);
#else
    _nx_ip_packet_receive(ip_ptr, packet_ptr);
#endif /* NX_DRIVER_ENABLE_DEFERRED */
  }
  else if (packet_type == NX_DRIVER_ETHERNET_ARP)
  {
    /* Clean off the Ethernet header.  */
    packet_ptr -> nx_packet_prepend_ptr =
      packet_ptr -> nx_packet_prepend_ptr + NX_DRIVER_PHYSICAL_FRAME_SIZE;

    /* Adjust the packet length.  */
    packet_ptr -> nx_packet_length =
      packet_ptr -> nx_packet_length - NX_DRIVER_PHYSICAL_FRAME_SIZE;

    /* Route to the ARP receive function.  */
    _nx_arp_packet_deferred_receive(ip_ptr, packet_ptr);
  }
  else if (packet_type == NX_DRIVER_ETHERNET_RARP)
  {
    /* Clean off the Ethernet header.  */
    packet_ptr -> nx_packet_prepend_ptr =
      packet_ptr -> nx_packet_prepend_ptr + NX_DRIVER_PHYSICAL_FRAME_SIZE;

    /* Adjust the packet length.  */
    packet_ptr -> nx_packet_length =
      packet_ptr -> nx_packet_length - NX_DRIVER_PHYSICAL_FRAME_SIZE;

    /* Route to the RARP receive function.  */
    _nx_rarp_packet_deferred_receive(ip_ptr, packet_ptr);
  }
  else
  {
    /* Invalid Ethernet header... release the packet. */
    nx_packet_release(packet_ptr);
  }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_driver_update_hardware_address                   PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Andres Mlinar, Microsoft Corporation                                */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function updates the hardware address kept by the driver.      */
/*    The hardware address is used when constructing send packets and to  */
/*    filter incoming packets.                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hardware_address                      The new hardware address      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    nx_driver_initialize                  Driver initialization function*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Andres Mlinar            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
static VOID nx_driver_update_hardware_address(UCHAR hardware_address[6])
{
  NX_INTERFACE *interface_ptr;

  /* Setup interface pointer.  */
  interface_ptr = nx_driver_information.nx_driver_information_interface;

  /**
    * Setup the physical address of this IP instance. Increment the
    * physical address lsw to simulate multiple nodes hanging on the
    * Ethernet.
    */
  interface_ptr -> nx_interface_physical_address_msw =
    ((ULONG)(((ULONG)hardware_address[0]) << 8)) |
    ((ULONG)hardware_address[1]);
  interface_ptr -> nx_interface_physical_address_lsw =
    ((ULONG)(((ULONG)hardware_address[2]) << 24)) |
    ((ULONG)((ULONG)(hardware_address[3]) << 16)) |
    ((ULONG)((ULONG)(hardware_address[4]) <<  8)) |
    ((ULONG)hardware_address[5]);
}

#ifdef NX_DRIVER_INTERNAL_TRANSMIT_QUEUE
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_driver_transmit_packet_enqueue                   PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Andres Mlinar, Microsoft Corporation                                */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function queues a transmit packet when the hardware transmit   */
/*    queue does not have the resources (buffer descriptors, etc.) to     */
/*    send the packet.  The queue is maintained as a singularly linked-   */
/*    list with head and tail pointers. The maximum number of packets on  */
/*    the transmit queue is regulated by the constant                     */
/*    NX_DRIVER_MAX_TRANSMIT_QUEUE_DEPTH. When this number is exceeded,   */
/*    the oldest packet is discarded after the new packet is queued.      */
/*                                                                        */
/*    Note: that it is assumed further driver interrupts are locked out   */
/*    during the call to this driver utility.                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Packet pointer                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_transmit_release           Release packet                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    nx_driver_hardware_packet_send        Driver packet send function   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Andres Mlinar            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
static VOID nx_driver_transmit_packet_enqueue(NX_PACKET *packet_ptr)
{
  /* Determine if there is anything on the queue.  */
  if (nx_driver_information.nx_driver_transmit_queue_tail)
  {
    /* Yes, something is on the transmit queue. Simply add the new packet to the tail. */
    nx_driver_information.nx_driver_transmit_queue_tail -> nx_packet_queue_next = packet_ptr;

    /* Update the tail pointer.  */
    nx_driver_information.nx_driver_transmit_queue_tail = packet_ptr;
  }
  else
  {
    /* First packet on the transmit queue.  */

    /* Setup head pointers.  */
    nx_driver_information.nx_driver_transmit_queue_head = packet_ptr;
    nx_driver_information.nx_driver_transmit_queue_tail = packet_ptr;

    /* Set the packet's next pointer to NULL.  */
    packet_ptr -> nx_packet_queue_next = NX_NULL;
  }

  /* Increment the total packets queued.  */
  nx_driver_information.nx_driver_transmit_packets_queued++;

  /* Determine if the total packet queued exceeds the driver's maximum transmit
     queue depth.  */
  if (nx_driver_information.nx_driver_transmit_packets_queued > NX_DRIVER_MAX_TRANSMIT_QUEUE_DEPTH)
  {
    /* Yes, remove the head packet (oldest) packet in the transmit queue and release it.  */
    packet_ptr = nx_driver_information.nx_driver_transmit_queue_head;

    /* Adjust the head pointer to the next packet.  */
    nx_driver_information.nx_driver_transmit_queue_head = packet_ptr -> nx_packet_queue_next;

    /* Decrement the transmit packet queued count.  */
    nx_driver_information.nx_driver_transmit_packets_queued--;

    /* Remove the Ethernet header. */
    NX_DRIVER_PHYSICAL_HEADER_REMOVE(packet_ptr);

    /* Release the packet.  */
    nx_packet_transmit_release(packet_ptr);
  }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_driver_transmit_packet_dequeue                   PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Andres Mlinar, Microsoft Corporation                                */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function removes the oldest transmit packet when the hardware  */
/*    transmit queue has new resources (usually after a transmit complete */
/*    interrupt) to send the packet. If there are no packets in the       */
/*    transmit queue, a NULL is returned.                                 */
/*                                                                        */
/*    Note: that it is assumed further driver interrupts are locked out   */
/*    during the call to this driver utility.                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    packet_ptr                            Packet pointer                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    nx_driver_hardware_packet_send        Driver packet send function   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Andres Mlinar            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
static NX_PACKET *nx_driver_transmit_packet_dequeue(VOID)
{
  NX_PACKET   *packet_ptr;

  /* Pickup the head pointer of the transmit packet queue. */
  packet_ptr = nx_driver_information.nx_driver_transmit_queue_head;

  /* Determine if there is anything on the queue.  */
  if (packet_ptr)
  {
    /* Yes, something is on the transmit queue. Simply the packet from the head of the queue.  */

    /* Update the head pointer.  */
    nx_driver_information.nx_driver_transmit_queue_head = packet_ptr -> nx_packet_queue_next;

    /* Clear the next pointer in the packet.  */
    packet_ptr -> nx_packet_queue_next = NX_NULL;

    /* Decrement the transmit packet queued count.  */
    nx_driver_information.nx_driver_transmit_packets_queued--;
  }

  /* Return the packet pointer - NULL if there are no packets queued.  */
  return (packet_ptr);
}
#endif /* NX_DRIVER_INTERNAL_TRANSMIT_QUEUE */

#ifdef NX_DEBUG
#define CASE(x) case x: return #x
#define DEFAULT default: return "UNKNOWN"
static const char *nx_driver_operation_to_string(UINT operation)
{
  switch (operation)
  {
      CASE(NX_LINK_PACKET_SEND);
      CASE(NX_LINK_INITIALIZE);
      CASE(NX_LINK_ENABLE);
      CASE(NX_LINK_DISABLE);
      CASE(NX_LINK_PACKET_BROADCAST);
      CASE(NX_LINK_ARP_SEND);
      CASE(NX_LINK_ARP_RESPONSE_SEND);
      CASE(NX_LINK_RARP_SEND);
      CASE(NX_LINK_MULTICAST_JOIN);
      CASE(NX_LINK_MULTICAST_LEAVE);
      CASE(NX_LINK_GET_STATUS);
      CASE(NX_LINK_GET_SPEED);
      CASE(NX_LINK_GET_DUPLEX_TYPE);
      CASE(NX_LINK_GET_ERROR_COUNT);
      CASE(NX_LINK_GET_RX_COUNT);
      CASE(NX_LINK_GET_TX_COUNT);
      CASE(NX_LINK_GET_ALLOC_ERRORS);
      CASE(NX_LINK_UNINITIALIZE);
      CASE(NX_LINK_DEFERRED_PROCESSING);
      CASE(NX_LINK_INTERFACE_ATTACH);
      CASE(NX_LINK_SET_PHYSICAL_ADDRESS);
      CASE(NX_INTERFACE_CAPABILITY_GET);
      CASE(NX_INTERFACE_CAPABILITY_SET);
      CASE(NX_LINK_INTERFACE_DETACH);
      CASE(NX_LINK_FACTORY_ADDRESS_GET);
      CASE(NX_LINK_RX_ENABLE);
      CASE(NX_LINK_RX_DISABLE);
      CASE(NX_LINK_6LOWPAN_COMMAND);
      CASE(NX_LINK_GET_INTERFACE_TYPE);

      DEFAULT;
  }
}
#endif /* NX_DEBUG */
