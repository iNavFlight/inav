/**
  ******************************************************************************
  * @file    lib_ipc.h
  * @author  MCD Application Team
  * @brief   Header for lib_ipc module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef LIB_IPC_H
#define LIB_IPC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ipc_cmd_event_define.h"

extern ipc_api_t ipc_event_tbl[];

/******************************************************************************
  *  LIB_IPC API
  *****************************************************************************/
/* ipc packet process(parse and do real ipc cmd/event function),
  * return 0 if success, < 0 if failed, > 0 means event銆�async cmd to process).*/
int32_t ipc_api_recv(uint8_t *data, int32_t len);

/* send cmd/event data with ipc packet format
  * return 0: success, <0: error code */
int32_t ipc_api_send(uint16_t cmd, uint8_t arg_type[], ...);

/* low level packet send function called by ipc_api_send,
  * implemented by user for different io interfaces.
  * return 0: success, <0: error code */
extern int32_t ipc_output(uint8_t *data, int32_t len);

/*cstat -MISRAC2012-Dir-4.4 */
/**
  * @defgroup MX_WIFI_IPC Raw_CMD
  * @brief Raw commands & events for MXCHIP Wi-Fi module, see lib_ipc.h
  * @{
  */
/*cstat +MISRAC2012-Dir-4.4 */

/******************************************************************************
  *  COMMAND & EVENTS
  *****************************************************************************/
/*cstat -MISRAC2012-Dir-4.4 */
/**
  * @defgroup MX_WIFI_IPC_ECHO Echo
  * @brief Raw data echo cmd & events (for testing)
  * @{
  */
/*cstat +MISRAC2012-Dir-4.4 */
int32_t ipc_echo_cmd(void *arg1, int32_t len, int32_t p_ret_flag);
int32_t ipc_echo_event(void *arg1, int32_t len, int32_t p_ret_flag);
/*cstat -MISRAC2012-Dir-4.4 */
/**
  * @}
  */

/**
  * @defgroup MX_WIFI_IPC_SYSTEM System
  * @brief System cmds & events
  * @{
  */
/*cstat +MISRAC2012-Dir-4.4 */
int32_t system_reboot_cmd(void);
int32_t system_factory_cmd(void);
int32_t system_firmware_version_get_cmd(int32_t version_buf_addr, int32_t version_buf_len,
                                        int32_t ret_addr, int32_t p_ret_flag);
int32_t system_firmware_version_get_event(uint8_t *ret_version, int32_t ret_version_len,
                                          uint8_t *ret_data, int32_t ret_data_len, int32_t p_ret_flag);
/*cstat -MISRAC2012-Dir-4.4 */
/**
  * @}
  */

/**
  * @defgroup MX_WIFI_IPC_WIFI_STATION Station
  * @brief Wi-Fi station cmds & events
  * @{
  */
/*cstat +MISRAC2012-Dir-4.4 */
int32_t wifi_mac_get_cmd(int32_t mac_buf_addr, int32_t mac_buf_len,
                         int32_t ret_addr, int32_t p_ret_flag);
int32_t wifi_mac_get_event(uint8_t *ret_mac, int32_t ret_mac_len,
                           uint8_t *ret, int32_t ret_len, int32_t p_ret_flag);
int32_t wifi_scan_cmd(int32_t mode, void *ssid, int32_t ssid_len, int32_t p_ret_flag);
int32_t wifi_scan_event(int32_t num, uint8_t *ap_list, int32_t ap_list_len, int32_t p_ret_flag);

int wifi_eap_set_rootca_cmd(void *rootca, int rootca_len);
int wifi_eap_set_client_cert_cmd(void *client_cert, int client_cert_len);
int wifi_eap_set_client_key_cmd(void *client_key, int client_key_len);
int wifi_eap_connect_cmd(void *ssid, int ssid_len, void *identity, int identity_len, 
                        void *password, int password_len, void *attr, int attr_len,
                        void *ip, int ip_len);
int32_t wifi_connect_cmd(const void *ssid, int32_t ssid_len, const void *key, int32_t key_len,
                         void *connect_attr, int32_t connect_attr_len, void *ip_attr, int32_t ip_attr_len);
int32_t wifi_disconnect_cmd(void);

int32_t wifi_link_info_get_cmd(int32_t p_ret_flag);
int32_t wifi_link_info_get_event(uint8_t *info, int32_t len, int32_t p_ret_flag);
int32_t wifi_ip_get_cmd(int32_t mode, int32_t p_ret_flag);
int32_t wifi_ip_get_event(uint8_t *ip, int32_t len, int32_t p_ret_flag);

int32_t wifi_status_event(int32_t mode, int32_t state);
/*cstat -MISRAC2012-Dir-4.4 */
/**
  * @}
  */

/**
  * @defgroup MX_WIFI_IPC_WIFI_SOFTAP SoftAP
  * @brief Wi-Fi softAP cmds & events
  * @{
  */
/*cstat +MISRAC2012-Dir-4.4 */
int32_t wifi_softap_start_cmd(void *ssid, int32_t ssid_len, void *key, int32_t key_len, int32_t channel, void *ip_attr,
                              int32_t ip_attr_len,
                              int32_t p_ret, int32_t p_ret_flag);
int32_t wifi_softap_start_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);
int32_t wifi_softap_stop_cmd(int32_t p_ret, int32_t p_ret_flag);
int32_t wifi_softap_stop_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);
/*cstat -MISRAC2012-Dir-4.4 */
/**
  * @}
  */

/**
  * @defgroup MX_WIFI_IPC_SOCKET Socket
  * @brief Socket cmds & events
  * @{
  */
/*cstat +MISRAC2012-Dir-4.4 */
int32_t socket_gethostbyname_cmd(const void *hostname, int32_t namelen, int32_t p_host_addr, int32_t host_addr_len,
                                 int32_t p_ret_flag);
int32_t socket_gethostbyname_event(uint8_t *ret_host_addr, int32_t ret_host_addr_len,
                                   int32_t p_ret_flag);
int32_t socket_ping_cmd(const void *hostname, int32_t namelen, int32_t count, int32_t delay_ms, int32_t p_ret_buf,
                        int32_t p_ret_flag);
int32_t socket_ping_event(uint8_t *ping_result, int32_t result_len, int32_t p_ret_flag);

int32_t socket_create_cmd(int32_t domain, int32_t type, int32_t protocol, int32_t p_ret, int32_t p_ret_flag);
int32_t socket_create_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);

int32_t socket_setsockopt_cmd(int32_t sockfd, int32_t level, int32_t optname, const void *optval, int32_t optlen,
                              int32_t p_ret, int32_t p_ret_flag);
int32_t socket_setsockopt_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);

int32_t socket_getsockopt_cmd(int32_t sockfd, int32_t level, int32_t optname, int32_t p_optbuf, int32_t p_optlen,
                              int32_t optlen,
                              int32_t p_ret, int32_t p_ret_flag);
int32_t socket_getsockopt_event(uint8_t *optval_buf, int32_t optval_len, uint8_t *ret_optlen_buf, int32_t ret_optlen,
                                uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);
int32_t socket_bind_cmd(int32_t sockfd, const void *addr, int32_t addrlen, int32_t p_ret, int32_t p_ret_flag);
int32_t socket_bind_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);
int32_t socket_listen_cmd(int32_t sockfd, int32_t n, int32_t p_ret, int32_t p_ret_flag);
int32_t socket_listen_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);
int32_t socket_accept_cmd(int32_t sockfd, int32_t p_addr, int32_t p_addrlen, int32_t addrlen, int32_t p_ret_fd,
                          int32_t p_ret_flag);
int32_t socket_accept_event(uint8_t *addr_buf, int32_t addr_buf_len, uint8_t *p_addrlen, int32_t addrlen,
                            uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);
int32_t socket_connect_cmd(int32_t sockfd, const void *addr, int32_t addrlen, int32_t p_ret, int32_t p_ret_flag);
int32_t socket_connect_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);

int32_t socket_send_cmd(int32_t sockfd, const uint8_t *buffer, size_t size, int32_t flags,
                        int32_t p_ret, int32_t p_ret_flag);
int32_t socket_send_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);

int32_t socket_sendto_cmd(int32_t sockfd, const uint8_t *buffer, size_t size, int32_t flags,
                          const void *addr, int32_t addrlen,
                          int32_t p_ret, int32_t p_ret_flag);
int32_t socket_sendto_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);

int32_t socket_recv_cmd(int32_t sockfd, int32_t p_buf, size_t size, int32_t flags,
                        int32_t p_ret, int32_t p_ret_flag);
int32_t socket_recv_event(uint8_t *recv_buf, size_t size,
                          uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);

int32_t socket_recvfrom_cmd(int32_t sockfd, int32_t p_buf, size_t size, int32_t flags,
                            int32_t p_fromaddr, int32_t p_fromaddr_len, int32_t fromaddr_len,
                            int32_t p_ret, int32_t p_ret_flag);
int32_t socket_recvfrom_event(uint8_t *recv_buf, size_t size, uint8_t *fromaddr, int32_t fromaddr_size,
                              uint8_t *fromaddr_len_buf, int32_t fromaddr_len_buf_size,
                              uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);

int32_t socket_shutdown_cmd(int32_t filedes, int32_t how, int32_t p_ret, int32_t p_ret_flag);
int32_t socket_shutdown_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);
int32_t socket_close_cmd(int32_t filedes, int32_t p_ret, int32_t p_ret_flag);
int32_t socket_close_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);
/*cstat -MISRAC2012-Dir-4.4 */
/**
  * @}
  */

/**
  * @defgroup MX_WIFI_IPC_MDNS mDNS
  * @brief mDNS cmds & events
  * @{
  */
/*cstat +MISRAC2012-Dir-4.4 */
int32_t mdns_start_cmd(const char *domain, char *hostname, int32_t p_ret, int32_t p_ret_flag);
int32_t mdns_start_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);
int32_t mdns_stop_cmd(int32_t p_ret, int32_t p_ret_flag);
int32_t mdns_stop_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);

int32_t mdns_announce_service_cmd(void *service, int32_t service_len, int32_t iface,
                                  int32_t p_ret, int32_t p_ret_flag);
int32_t mdns_announce_service_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);

int32_t mdns_deannounce_service_cmd(void *service, int32_t service_len, int32_t iface,
                                    int32_t p_ret, int32_t p_ret_flag);
int32_t mdns_deannounce_service_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);

int32_t mdns_deannounce_service_all_cmd(int32_t iface, int32_t p_ret, int32_t p_ret_flag);
int32_t mdns_deannounce_service_all_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);

int32_t mdns_iface_state_change_cmd(int32_t iface, int32_t state, int32_t p_ret, int32_t p_ret_flag);
int32_t mdns_iface_state_change_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);

int32_t mdns_set_hostname_cmd(void *hostname, int32_t len, int32_t p_ret, int32_t p_ret_flag);
int32_t mdns_set_hostname_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);

int32_t mdns_set_txt_rec_cmd(void *service, int32_t service_len, int32_t p_ret, int32_t p_ret_flag);
int32_t mdns_set_txt_rec_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);
/*cstat -MISRAC2012-Dir-4.4 */
/**
  * @}
  */

/**
  * @defgroup MX_WIFI_IPC_TLS TLS
  * @brief TLS cmds & events
  * @{
  */
/*cstat +MISRAC2012-Dir-4.4 */
int32_t tls_set_ver_cmd(int32_t version, int32_t p_ret, int32_t p_ret_flag);
int32_t tls_set_ver_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);

int32_t tls_set_client_certificate_cmd(uint8_t *cert, int32_t len,  int32_t p_ret, int32_t p_ret_flag);
int32_t tls_set_client_certificate_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);
int32_t tls_set_client_private_key_cmd(uint8_t *key, int32_t len,  int32_t p_ret, int32_t p_ret_flag);
int32_t tls_set_client_private_key_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);

int32_t tls_connect_cmd(int32_t domain, int32_t type, int32_t protocol,
                        const void *addr, int32_t addrlen,
                        void *ca, int32_t calen, int32_t p_ret, int32_t p_ret_flag);
int tls_connect_sni_cmd(const char* sni_servername, int sni_servername_len,
                    const void *addr, int32_t addrlen,
                    void* ca, int calen, int p_ret, int p_ret_flag);
int32_t tls_connect_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);

int32_t tls_send_cmd(int32_t tls, void *data, int32_t len,  int32_t p_ret, int32_t p_ret_flag);
int32_t tls_send_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);

int32_t tls_recv_cmd(int32_t tls, int32_t p_buf, int32_t len,  int32_t p_ret, int32_t p_ret_flag);
int32_t tls_recv_event(uint8_t *recv_buf, size_t size,
                       uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);

int32_t tls_close_cmd(int32_t tls,  int32_t p_ret, int32_t p_ret_flag);
int32_t tls_close_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);

int32_t tls_set_nonblock_cmd(int32_t tls, int32_t nonblock, int32_t p_ret, int32_t p_ret_flag);
int32_t tls_set_nonblock_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);
/*cstat -MISRAC2012-Dir-4.4 */
/**
  * @}
  */

/**
  * @defgroup MX_WIFI_IPC_WEBSERVER Webserver
  * @brief Webserver cmds & events
  * @{
  */
/*cstat +MISRAC2012-Dir-4.4 */
int32_t web_start_cmd(int32_t p_ret, int32_t p_ret_flag);
int32_t web_start_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);

int32_t web_stop_cmd(int32_t p_ret, int32_t p_ret_flag);
int32_t web_stop_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);
/*cstat -MISRAC2012-Dir-4.4 */
/**
  * @}
  */

/**
  * @defgroup MX_WIFI_IPC_FOTA FOTA
  * @brief FOTA cmds & events
  * @{
  */
/*cstat +MISRAC2012-Dir-4.4 */
int32_t fota_start_cmd(const char *url, int32_t url_len, const char *md5, int32_t md5_len,
                       int32_t p_ret, int32_t p_ret_flag);
int32_t fota_start_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag);
int32_t fota_status_event(int32_t status);
/*cstat -MISRAC2012-Dir-4.4 */
/**
  * @}
  */


/**
  * @}
  */
/*cstat +MISRAC2012-Dir-4.4 */


#ifdef __cplusplus
}
#endif

#endif /* LIB_IPC_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
