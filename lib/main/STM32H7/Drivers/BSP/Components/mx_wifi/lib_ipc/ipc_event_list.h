/**
  ******************************************************************************
  * @file    lib_ipc_event_list.h
  * @author  MCD Application Team
  * @brief   Header for lib_ipc_event_list module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

 #ifdef __cplusplus
 extern "C" {
#endif

/* list ALL event APIs in this header file */

/* 3080 send a string event to ST */
IPC_CMD_API(ipc_echo_event)

/*
  * system event
  */
IPC_CMD_API(system_firmware_version_get_event)

/*
  * wifi event
  */
IPC_CMD_API(wifi_mac_get_event)
IPC_CMD_API(wifi_scan_event)
IPC_CMD_API(wifi_link_info_get_event)
IPC_CMD_API(wifi_ip_get_event)
IPC_CMD_API(wifi_status_event)

IPC_CMD_API(wifi_softap_start_event)
IPC_CMD_API(wifi_softap_stop_event)

/*
  * socket event
  */
IPC_CMD_API(socket_gethostbyname_event)
IPC_CMD_API(socket_ping_event)
IPC_CMD_API(socket_create_event)
IPC_CMD_API(socket_setsockopt_event)
IPC_CMD_API(socket_getsockopt_event)
IPC_CMD_API(socket_bind_event)
IPC_CMD_API(socket_listen_event)
IPC_CMD_API(socket_accept_event)
IPC_CMD_API(socket_connect_event)
IPC_CMD_API(socket_send_event)
IPC_CMD_API(socket_sendto_event)
IPC_CMD_API(socket_recv_event)
IPC_CMD_API(socket_recvfrom_event)
IPC_CMD_API(socket_close_event)
IPC_CMD_API(socket_shutdown_event)

/*
  * mDNS
  */
IPC_CMD_API(mdns_start_event)
IPC_CMD_API(mdns_stop_event)
IPC_CMD_API(mdns_announce_service_event)
IPC_CMD_API(mdns_deannounce_service_event)
IPC_CMD_API(mdns_deannounce_service_all_event)
IPC_CMD_API(mdns_iface_state_change_event)
IPC_CMD_API(mdns_set_hostname_event)
IPC_CMD_API(mdns_set_txt_rec_event)

/*
  * TLS
  */
IPC_CMD_API(tls_set_ver_event)
IPC_CMD_API(tls_set_client_certificate_event)
IPC_CMD_API(tls_set_client_private_key_event)
IPC_CMD_API(tls_connect_event)
IPC_CMD_API(tls_send_event)
IPC_CMD_API(tls_recv_event)
IPC_CMD_API(tls_close_event)
IPC_CMD_API(tls_set_nonblock_event)

/*
  * WEBSERVER
  */
IPC_CMD_API(web_start_event)
IPC_CMD_API(web_stop_event)

/*
  * FOTA
  */
IPC_CMD_API(fota_start_event)
IPC_CMD_API(fota_status_event)

#undef IPC_CMD_API


#ifdef __cplusplus
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
