/**
  ******************************************************************************
  * @file    net_cellular.h
  * @author  MCD Application Team
  * @brief   Header for the network Cellular class.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
#ifndef NET_CELLULAR_H
#define NET_CELLULAR_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NET_CELLULAR_CREDENTIAL_V2

/* SIM socket type */
#define NET_SIM_SLOT_MODEM_SOCKET        0    /* Modem Socket SIM Slot   */
#define NET_SIM_SLOT_MODEM_EMBEDDED_SIM  1    /* Modem Embedded SIM Slot */
#define NET_CELLULAR_MAX_SUPPORTED_SLOT  2   /* Number max of supported SIM slot */
#define NET_CELLULAR_DEFAULT_SIM_SLOT   NET_SIM_SLOT_MODEM_SOCKET   /* Default SIM Slot   */

typedef char_t net_sim_slot_type_t ;

/* SIM Slot parameters */
typedef struct net_cellular_sim_slot_s
{
  net_sim_slot_type_t  sim_slot_type;   /* sim slot type                               */
  char_t                *apn;           /* APN (string)                                */
  char_t               cid;             /* CID (1-9)                                   */
  char_t                *username;      /* username: empty string => no username       */
  char_t                *password;      /* password (used only is username is defined) */
} net_cellular_sim_slot_t;

/* Credential configuration */
typedef struct net_cellular_credentials_s
{
  uint8_t sim_socket_nb;                                                /* number of sim slot used */
  net_cellular_sim_slot_t  sim_slot[NET_CELLULAR_MAX_SUPPORTED_SLOT];    /* sim slot parameters     */
} net_cellular_credentials_t;

#else /*NET_CELLULAR_CREDENTIAL_V2 */

/* Credential configuration */
typedef struct net_cellular_credentials_s
{
  const char_t *apn;
  const char_t *username;
  const char_t *password;
  bool_t use_internal_sim;
} net_cellular_credentials_t;

#endif /* NET_CELLULAR_CREDENTIAL_V2 */


/* Network radio results */
typedef struct net_cellular_radio_results_s
{
  int8_t  signal_level_db;
} net_cellular_radio_results_t;

/* network extension for Cellular class interface */
int32_t net_cellular_set_credentials(net_if_handle_t *pnetif, const net_cellular_credentials_t *credentials);
int32_t net_cellular_get_radio_results(net_if_handle_t *pnetif, net_cellular_radio_results_t *results);
/* Declaration of cellular network interface constructor */
int32_t cellular_net_driver(net_if_handle_t *pnetif);

#ifdef __cplusplus
}
#endif

#endif /* NET_CELLULAR_H */
