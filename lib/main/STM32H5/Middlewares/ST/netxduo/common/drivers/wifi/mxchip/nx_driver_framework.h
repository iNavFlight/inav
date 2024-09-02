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

#ifndef NX_DRIVER_FRAMEWORK_H
#define NX_DRIVER_FRAMEWORK_H


#ifdef __cplusplus
/* Yes, C++ compiler is present.  Use standard C. */
extern   "C" {
#endif /* __cplusplus */


/* Include ThreadX header file, if not already. */

#ifndef TX_API_H
#include "tx_api.h"
#endif /* TX_API_H */


/* Include NetX header file, if not already.  */

#ifndef NX_API_H
#include "nx_api.h"
#endif /* NX_API_H */


#define NX_DRIVER_MTU                           (1514)
#define NX_DRIVER_PHYSICAL_FRAME_SIZE           (14)

/* MIPC_HEADER_SIZE(6) + sizeof(wifi_bypass_in_rparams_t)(22) */

#define NX_DRIVER_PACKET_SIZE                   (NX_DRIVER_MTU + 28 + 2)

#define NX_DRIVER_PHYSICAL_HEADER_REMOVE(p)                        \
  do {                                                             \
    p -> nx_packet_prepend_ptr += NX_DRIVER_PHYSICAL_FRAME_SIZE;   \
    p -> nx_packet_length -= NX_DRIVER_PHYSICAL_FRAME_SIZE;        \
  } while (0)

struct NX_DRIVER_INFORMATION_STRUCT;


/**
  * Determine if the driver's source file is being compiled.
  * The constants and typdefs are only valid within
  * the driver's source file compilation.
  */

#ifdef NX_DRIVER_SOURCE


/* Define generic constants and macros for all NetX drivers. */

#define NX_DRIVER_STATE_NOT_INITIALIZED         1
#define NX_DRIVER_STATE_INITIALIZE_FAILED       2
#define NX_DRIVER_STATE_INITIALIZED             3
#define NX_DRIVER_STATE_LINK_ENABLED            4

#ifdef NX_DRIVER_INTERNAL_TRANSMIT_QUEUE
#ifndef NX_DRIVER_MAX_TRANSMIT_QUEUE_DEPTH
#define NX_DRIVER_MAX_TRANSMIT_QUEUE_DEPTH      10
#endif /* NX_DRIVER_MAX_TRANSMIT_QUEUE_DEPTH */
#endif /* NX_DRIVER_INTERNAL_TRANSMIT_QUEUE */

#define NX_DRIVER_DEFERRED_PACKET_RECEIVED      1
#define NX_DRIVER_DEFERRED_DEVICE_RESET         2
#define NX_DRIVER_DEFERRED_PACKET_TRANSMITTED   4

#define NX_DRIVER_ERROR                         90

#ifndef NX_DRIVER_CAPABILITY
#define NX_DRIVER_CAPABILITY ( 0 )
#endif /* NX_DRIVER_CAPABILITY */

/* Define generic constants and macros for all NetX Ethernet drivers. */

#define NX_DRIVER_ETHERNET_IP                   0x0800
#define NX_DRIVER_ETHERNET_IPV6                 0x86dd
#define NX_DRIVER_ETHERNET_ARP                  0x0806
#define NX_DRIVER_ETHERNET_RARP                 0x8035


/**
  * Define basic Ethernet driver information typedef.
  * Note that this typedefs is designed to be used only
  * in the driver's C file.
  */

typedef struct NX_DRIVER_INFORMATION_STRUCT
{
  /* NetX IP instance that this driver is attached to.  */
  NX_IP               *nx_driver_information_ip_ptr;

  /* Driver's current state.  */
  ULONG               nx_driver_information_state ;

  /* Packet pool used for receiving packets. */
  NX_PACKET_POOL      *nx_driver_information_packet_pool_ptr;

  /* Define the driver interface association.  */
  NX_INTERFACE        *nx_driver_information_interface;

  /* Define the deferred event field. This will contain bits representing events
     deferred from the ISR for processing in the thread context.  */
  ULONG               nx_driver_information_deferred_events;

} NX_DRIVER_INFORMATION;

#endif /* NX_DRIVER_SOURCE */


/* Define default driver entry function. */

VOID nx_driver_framework_default_entry(NX_IP_DRIVER *driver_req_ptr);

#ifdef __cplusplus
/* Yes, C++ compiler is present.  Use standard C. */
}
#endif /* __cplusplus */

#endif /* NX_DRIVER_FRAMEWORK_H */
