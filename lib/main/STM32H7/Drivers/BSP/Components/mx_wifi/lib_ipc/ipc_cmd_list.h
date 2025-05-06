/**
  ******************************************************************************
  * @file    lib_ipc_cmd_list.h
  * @author  MCD Application Team
  * @brief   Header for lib_ipc_cmd_list module
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

/* list all command APIs in this header file */

/*
  * test cmd
  */

/* ST send a string to 3080 */
IPC_CMD_API(ipc_echo_cmd)

/*
  * system cmd
  */
IPC_CMD_API(system_reboot_cmd)
IPC_CMD_API(system_factory_cmd)
IPC_CMD_API(system_firmware_version_get_cmd)

/*
  * wifi cmd
  */
IPC_CMD_API(wifi_mac_get_cmd)
IPC_CMD_API(wifi_scan_cmd)
IPC_CMD_API(wifi_eap_set_rootca_cmd)
IPC_CMD_API(wifi_eap_set_client_cert_cmd)
IPC_CMD_API(wifi_eap_set_client_key_cmd)
IPC_CMD_API(wifi_eap_connect_cmd)
IPC_CMD_API(wifi_connect_cmd)
IPC_CMD_API(wifi_disconnect_cmd)
IPC_CMD_API(wifi_link_info_get_cmd)
IPC_CMD_API(wifi_ip_get_cmd)

IPC_CMD_API(wifi_softap_start_cmd)
IPC_CMD_API(wifi_softap_stop_cmd)

/*
  * socket cmd
  */
IPC_CMD_API(socket_gethostbyname_cmd)
IPC_CMD_API(socket_ping_cmd)
IPC_CMD_API(socket_create_cmd)
IPC_CMD_API(socket_setsockopt_cmd)
IPC_CMD_API(socket_getsockopt_cmd)
IPC_CMD_API(socket_bind_cmd)
IPC_CMD_API(socket_listen_cmd)
IPC_CMD_API(socket_accept_cmd)
IPC_CMD_API(socket_connect_cmd)
IPC_CMD_API(socket_send_cmd)
IPC_CMD_API(socket_sendto_cmd)
IPC_CMD_API(socket_recv_cmd)
IPC_CMD_API(socket_recvfrom_cmd)
IPC_CMD_API(socket_close_cmd)
IPC_CMD_API(socket_shutdown_cmd)

/*
  * mDNS
  */
IPC_CMD_API(mdns_start_cmd)
IPC_CMD_API(mdns_stop_cmd)
IPC_CMD_API(mdns_announce_service_cmd)
IPC_CMD_API(mdns_deannounce_service_cmd)
IPC_CMD_API(mdns_deannounce_service_all_cmd)
IPC_CMD_API(mdns_iface_state_change_cmd)
IPC_CMD_API(mdns_set_hostname_cmd)
IPC_CMD_API(mdns_set_txt_rec_cmd)

/*
  * TLS
  */
IPC_CMD_API(tls_set_ver_cmd)
IPC_CMD_API(tls_set_client_certificate_cmd)
IPC_CMD_API(tls_set_client_private_key_cmd)
IPC_CMD_API(tls_connect_cmd)
IPC_CMD_API(tls_connect_sni_cmd)
IPC_CMD_API(tls_send_cmd)
IPC_CMD_API(tls_recv_cmd)
IPC_CMD_API(tls_close_cmd)
IPC_CMD_API(tls_set_nonblock_cmd)

/*
  * WEBSERVER
  */
IPC_CMD_API(web_start_cmd)
IPC_CMD_API(web_stop_cmd)

/*
  * FOTA
  */
IPC_CMD_API(fota_start_cmd)

#undef IPC_CMD_API


#ifdef __cplusplus
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
