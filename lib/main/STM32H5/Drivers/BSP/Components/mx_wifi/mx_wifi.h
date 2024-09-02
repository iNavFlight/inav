/**
  ******************************************************************************
  * @file    mx_wifi.h
  * @author  MCD Application Team
  * @brief   Header for mx_wifi.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */


/**
  * @file mx_wifi.h
  * @mainpage README
  * @author MXCHIP
  * @version V2.3.3
  * @date 2021-09-26
  * @brief EMW3080B Wi-Fi module driver API for STM32 MCU
  * @details
  * # Change log
  *
  * ## V2.3.3 2021-09-26
  * - Update wifi API documents for params and return value.
  *
  * ## V2.3.3 2021-09-07
  * - Fix TCP server in softAP+STA mode.
  * - Fix UART driver for new Discovery Kit RevC board.
  * - Add new wifi connect API MX_WIFI_Connect_Adv.
  * - Add wifi module firmware binary v2.3.3.
  * - Update wifi API documents for more details.
  *
  * ## V2.3.0 2021-08-27
  * - Fix for FOSS problem.
  * - Update FOTA for AWS problem.
  * - Add new API for EAP, WPS, config server.
  * - Support get MAC for softAP.
  * - Update STM32 network library from ST.
  * - **Problem**: TCP server in softAP mode still not working.
  *
  * ## v2.1.0 2021-03-25
  * - Support STM32 U5 board for microsoft project.
  * - Redesign SPI protocol to fix data transfer bug and increasing speed.
  */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MX_WIFI_H
#define MX_WIFI_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "mx_wifi_conf.h"
#include "core/mx_address.h"

/**
  * @defgroup MX_WIFI Wi-Fi_API
  * @brief Driver API on STM32 for MXCHIP Wi-Fi module, see mx_wifi.h
  * @{ **
  */

/* API version V2.3.3 */
#define MX_WIFI_API_VERSION_MAJ                     (2)
#define MX_WIFI_API_VERSION_SUB                     (3)
#define MX_WIFI_API_VERSION_REV                     (3)

/*******************************************************************************
  * MXCHIP Wi-Fi Module Defines
  ******************************************************************************/
/* Common defines. */
#define MX_MAX_IP_LEN                 (16)
#define MX_MAX_SSID_LEN               (32)
#define MX_MAX_KEY_LEN                (64)
#define MX_MAX_BSSID_LEN              (6)
#define MX_MAX_DNSNAME_LEN            (253)
#define MX_MAX_IDENTITY_LEN           (32)
#define MX_TLS_SNI_SERVERNAME_SIZE    (128)
#define MC_WIFI_INTERFACE_NUM         (2)
#define MX_WIFI_PING_MAX              (10)
#define MX_WIFI_SCAN_TIMEOUT          (5000)

/**
  * @brief Wi-Fi mode
  */
enum
{
  MC_SOFTAP,  /**< wifi softap mode. */
  MC_STATION  /**< wifi station mode. */
};

typedef uint8_t mwifi_if_t; /**< wifi mode. ref to @ref mwifi_if_t */

/**
  * @brief Wi-Fi scan mode
  */
enum
{
  MC_SCAN_PASSIVE = 0,  /**< wifi passive scan mode. */
  MC_SCAN_ACTIVE  = 1   /**< wifi active scan mode.  */
};

typedef uint8_t mc_wifi_scan_mode_t; /**< wifi scan mode. ref to @ref mc_wifi_scan_mode_e */

/**
  * @brief Wi-Fi ip address info
  */
typedef struct
{
  char localip[MX_MAX_IP_LEN];  /**< local ip address */
  char netmask[MX_MAX_IP_LEN];  /**< netmask */
  char gateway[MX_MAX_IP_LEN];  /**< gateway ip address */
  char dnserver[MX_MAX_IP_LEN]; /**< dns server ip address */
} mwifi_ip_attr_t; /**< wifi ip address info. */

typedef uint8_t mwifi_security_t; /**< wifi connection security type. */

/**
  * @brief Wi-Fi station connection attributes
  */
typedef struct
{
  uint8_t          bssid[6]; /**< bssid of access-point */
  uint8_t          channel;  /**< channel of access-point */
  mwifi_security_t security; /**< security of access-point */
} mwifi_connect_attr_t;

/**
  * @brief Wi-Fi EAP type
  */
typedef enum
{
  EAP_TYPE_TLS  = 13, /* RFC 2716 */
  EAP_TYPE_TTLS = 21, /* RFC 5281 */
  EAP_TYPE_PEAP = 25  /* draft-josefsson-pppext-eap-tls-eap-06.txt */
} EapType;

/**
  * @brief Wi-Fi EAP info
  */
typedef struct
{
  uint8_t     eap_type;    /* support: EAP_TYPE_PEAP, EAP_TYPE_TTLS, EAP_TYPE_TLS */
  const char *rootca;      /* The EAP server rootca. NULL for don't check the server's certificate */
  const char *client_cert; /* client cert, only need this if the server need check the client's certificate,
                              such as EAP_TYPE_TLS mode. */
  const char *client_key;  /* client private key. DO NOT support encrypted key. */
} mwifi_eap_attr_t;

/*******************************************************************************
  * STM32Cube Driver API Defines
  ******************************************************************************/
/**
  * @brief API operation status code
  */
typedef enum
{
  MX_WIFI_STATUS_OK           = (0),     /**< status code success. */
  MX_WIFI_STATUS_ERROR        = (-1),    /**< status code common error. */
  MX_WIFI_STATUS_TIMEOUT      = (-2),    /**< status code timeout. */
  MX_WIFI_STATUS_IO_ERROR     = (-3),    /**< status code I/O error. */
  MX_WIFI_STATUS_PARAM_ERROR  = (-4)     /**< status code bad argument error. */
} MX_WIFI_STATUS_T;

/* macro */
#define MX_WIFI_MAC_SIZE            (6)     /**< max length of MAC address. */
#define MX_WIFI_SCAN_BUF_SIZE       (1000)  /**< max size of scan buffer. */

#define MIN(A, B)  ( ((A) < (B)) ? (A) : (B) )  /**< helper function: get minimum. */


/**
  * @brief Security settings for wifi network
  */
typedef enum
{
  MX_WIFI_SEC_NONE,       /**< Open system. */
  MX_WIFI_SEC_WEP,        /**< Wired Equivalent Privacy. WEP security. */
  MX_WIFI_SEC_WPA_TKIP,   /**< WPA /w TKIP */
  MX_WIFI_SEC_WPA_AES,    /**< WPA /w AES */
  MX_WIFI_SEC_WPA2_TKIP,  /**< WPA2 /w TKIP */
  MX_WIFI_SEC_WPA2_AES,   /**< WPA2 /w AES */
  MX_WIFI_SEC_WPA2_MIXED, /**< WPA2 /w AES or TKIP */
  MX_WIFI_SEC_AUTO        /**< It is used when calling @ref mwifi_connect,
                               MXOS read security type from scan result. */
} MX_WIFI_SecurityType_t;

typedef int8_t (*IO_Init_Func)(uint16_t mode);                             /**< I/O interface init function. */
typedef int8_t (*IO_DeInit_Func)(void);                                    /**< I/O interface deinit function. */
typedef void (*IO_Delay_Func)(uint32_t ms);                                /**< I/O interface delay function. */
typedef uint16_t (*IO_Send_Func)(uint8_t *data, uint16_t len);             /**< I/O interface send function. */
typedef uint16_t (*IO_Receive_Func)(uint8_t *buffer, uint16_t buff_size);  /**< I/O interface receive function. */

/**
  * @brief Wi-Fi low level I/O interface operation handles
  */
typedef struct
{
  IO_Init_Func IO_Init;           /**< I/O interface init function. */
  IO_DeInit_Func IO_DeInit;       /**< I/O interface deinit function. */
  IO_Delay_Func IO_Delay;         /**< I/O interface delay function. */
  IO_Send_Func IO_Send;           /**< I/O interface send function. */
  IO_Receive_Func IO_Receive;     /**< I/O interface receive function. */
} MX_WIFI_IO_t;

/**
  * @brief Wi-Fi system info
  */
typedef struct
{
  /* FW info */
  uint8_t Product_Name[MX_WIFI_PRODUCT_NAME_SIZE];/**< product name. */
  uint8_t Product_ID[MX_WIFI_PRODUCT_ID_SIZE];    /**< product ID. */
  uint8_t FW_Rev[MX_WIFI_FW_REV_SIZE];            /**< firmware version. */

  /* Wi-Fi MAC address */
  uint8_t MAC[MX_WIFI_MAC_SIZE];                  /**< Wi-Fi MAC address. */
  uint8_t apMAC[MX_WIFI_MAC_SIZE];                /**< Wi-Fi softap MAC address. */
} MX_WIFI_SystemInfo_t;

/**
  * @brief Wi-Fi station info
  */
typedef struct
{
  /* WiFi station setting. */
  uint8_t SSID[MX_WIFI_MAX_SSID_NAME_SIZE + 1];   /**< Wi-Fi station SSID. */
  uint8_t pswd[MX_WIFI_MAX_PSWD_NAME_SIZE + 1];   /**< Wi-Fi station passwd. */
  MX_WIFI_SecurityType_t Security;                /**< Wi-Fi station security. */
  uint8_t DHCP_IsEnabled;                         /**< Wi-Fi station DHCP. */
  /* Status. */
  int8_t IsConnected;                             /**< Wi-Fi station connection status. */
  /* IPv4 addresses. */
  uint8_t IP_Addr[4];                             /**< Wi-Fi station IP address. */
  uint8_t IP_Mask[4];                             /**< Wi-Fi station IP mask. */
  uint8_t Gateway_Addr[4];                        /**< Wi-Fi station gateway. */
  uint8_t DNS1[4];                                /**< Wi-Fi station DNS server. */
  /* IPv6 addresses. */
  int32_t IP6_state[3];                           /**< Wi-Fi station IPv6 address state. */
  uint8_t IP6_Addr[3][16];                        /**< Wi-Fi station IPv6 address. */
  uint8_t IP6_Mask[16];                           /**< Wi-Fi station IPv6 mask. */
  uint8_t Gateway6_Addr[16];                      /**< Wi-Fi station IPv6 gateway. */
  uint8_t IP6_DNS1[16];                           /**< Wi-Fi station IPv6 DNS server. */
} MX_WIFI_Network_t;

/**
  * @brief Wi-Fi softAP info
  */
typedef struct
{
  char SSID[MX_WIFI_MAX_SSID_NAME_SIZE + 1];      /**< Wi-Fi softAP SSID. */
  char pswd[MX_WIFI_MAX_PSWD_NAME_SIZE + 1];      /**< Wi-Fi softAP password. */
  /* MX_WIFI_SecurityType_t Security;   OPEN: no passwd,  WPA2: has passwd, NOT SUPPORTED NOW */
  uint8_t channel;                                /**< Wi-Fi softAP Channel. */
  mwifi_ip_attr_t ip;                             /**< Wi-Fi softAP IP settings. */
} MX_WIFI_APSettings_t;

/** Prototype of Wi-Fi status changed callback function. */
/**
  * @param cate wifi status:  MC_STATION, MC_SOFTAP
  * @param event wifi events: MWIFI_EVENT_STA_DOWN,
  *                           MWIFI_EVENT_STA_UP,
  *                           WIFI_EVENT_STA_GOT_IP,
  *                           MWIFI_EVENT_AP_UP,
  *                           MWIFI_EVENT_AP_DOWN
  * @param arg user argument passed by @MX_WIFI_RegisterStatusCallback
  */
typedef void (*mx_wifi_status_callback_t)(uint8_t cate, uint8_t event, void *arg);

/**
  * @brief FOTA status
  */
typedef enum
{
  MX_WIFI_FOTA_SUCCESS,
  MX_WIFI_FOTA_FAILED
} mx_wifi_fota_status_e;

/**
  * @brief Prototype of FOTA status callback function
  * @param status FOTA status
  * @param user_args arguments set by user when call MX_WIFI_FOTA_start
  * @return none
  */
typedef void (*mx_wifi_fota_status_cb_t)(mx_wifi_fota_status_e status, uint32_t user_args);

/**
  * @brief Prototype of netlink input callback function for network bypass mode
  * @param pbuf network data buffer
  * @param user_args arguments set by user when call MX_WIFI_Network_bypass_mode_set
  * @return none
  */
typedef void (*mx_wifi_netlink_input_cb_t)(mx_buf_t *pbuf, void *user_args);

/**
  * @brief Wi-Fi runtime info
  */
typedef struct
{
  uint32_t Timeout;       /**< Wi-Fi cmd timeout in ms. */

  mx_wifi_status_callback_t status_cb[MC_WIFI_INTERFACE_NUM];  /**< Wi-Fi status callback. */
  void *callback_arg[MC_WIFI_INTERFACE_NUM];                   /**< Wi-Fi status callback argument. */

  mx_wifi_fota_status_cb_t fota_status_cb;      /**< FOTA status callback. */
  uint32_t fota_user_args;                      /**< FOTA status callback argument. */

  mx_wifi_netlink_input_cb_t netlink_input_cb;  /**< netlink input callback. */
  void *netlink_user_args;                      /**< netlink input callback argument. */

  uint8_t scan_result[MX_WIFI_SCAN_BUF_SIZE + 1]; /**< Wi-Fi scan result buffer. */
  uint8_t scan_number;                          /**< Num of Wi-Fi scan result to get. */

  uint8_t interfaces;                           /**< Num of Wi-Fi interfaces inited, 2 if STA+AP inited. */
} MX_WIFI_Runtime_t;

/**
  * @brief Wi-Fi Wi-Fi object handle
  */
typedef struct
{
  /* HW IO */
  MX_WIFI_IO_t fops;                  /**< Wi-Fi low level I/O operation handles. */

  /* system info */
  MX_WIFI_SystemInfo_t SysInfo;       /**< Wi-Fi system info. */

  /* network info */
  MX_WIFI_Network_t NetSettings;      /**< Wi-Fi station info. */
  MX_WIFI_APSettings_t APSettings;    /**< Wi-Fi softAP info. */

  /* run time data */
  MX_WIFI_Runtime_t Runtime;          /**< Wi-Fi runtime info. */

  /* wifi obj lock */
  LOCK_DECLARE(lockcmd);
} MX_WIFIObject_t;


/* Exported functions --------------------------------------------------------*/

/**
  * @defgroup MX_WIFI_IO Driver_I/O
  * @brief Driver I/O interface setting API
  * @{ **
  */


/**
  * @brief Register low level IO interface, UART or SPI
  * @param Obj wifi object handle.
  * @param IO_Init IO init function for low level data transfer, UART or SPI
  * @param IO_DeInit IO de-init function
  * @param IO_Delay IO delay function in ms
  * @param IO_Send IO send data function
  * @param IO_Receive IO receive data function
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
MX_WIFI_STATUS_T MX_WIFI_RegisterBusIO(MX_WIFIObject_t *Obj,
                                       IO_Init_Func IO_Init, IO_DeInit_Func IO_DeInit, IO_Delay_Func IO_Delay,
                                       IO_Send_Func IO_Send, IO_Receive_Func IO_Receive);
/**
  * @brief Reset wifi module by hardware IO. RESET IO set in low level IO configuration.
  * @param Obj wifi object handle.
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
MX_WIFI_STATUS_T MX_WIFI_HardResetModule(MX_WIFIObject_t *Obj);

/**
  * @} **
  */

/**
  * @defgroup MX_WIFI_INIT Driver_init
  * @brief Driver init API
  * @{ **
  */

/**
  * @brief Wi-Fi module init type
  */
typedef enum
{
  MX_WIFI_INIT = 0,   /**< Wi-Fi module init(not reboot). */
  MX_WIFI_RESET = 1   /**< Wi-Fi module reset(reboot). */
} MX_WIFI_InitMode_t;

/**
  * @brief  Initialize WIFI module and get module firmware version and Wi-Fi MAC info.
  * @param  Obj: pointer to module handle
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
MX_WIFI_STATUS_T MX_WIFI_Init(MX_WIFIObject_t *Obj);

/**
  * @brief  DeInitialize WIFI module.
  * @param  Obj: pointer to module handle
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
MX_WIFI_STATUS_T MX_WIFI_DeInit(MX_WIFIObject_t *Obj);

/**
  * @brief  Change default Timeout for wifi cmd.
  * @param  Obj: pointer to module handle
  * @param  Timeout: Timeout in mS
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
MX_WIFI_STATUS_T MX_WIFI_SetTimeout(MX_WIFIObject_t *Obj, uint32_t Timeout);

/**
  * @brief  Yield data from Wi-Fi module.
  * @param  Obj: pointer to module handle
  * @param  timeout: yield timeout in ms
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  * @note   This will be called periodically
  */
MX_WIFI_STATUS_T MX_WIFI_IO_YIELD(MX_WIFIObject_t *Obj, uint32_t timeout);

/**
  * @} **
  */

/**
  * @defgroup MX_WIFI_NETWORK_BYPASS_MODE Network bypass mode
  * @brief Network bypass mode API
  * @{ **
 */

/**
  * @brief Network bypass interface index
  */
enum mwifi_if_index_e
{
  STATION_IDX = 0,
  SOFTAP_IDX  = 1
};


#if (MX_WIFI_NETWORK_BYPASS_MODE == 1)
/**
  * @brief  Set network bypass mode
  * @param  Obj: pointer to module handle
  * @param  enable: 0=disable, 1=enable
  * @param  netlink_input_callbck: input data callback function
  * @param  user_args: user arguments for callback function
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
MX_WIFI_STATUS_T MX_WIFI_Network_bypass_mode_set(MX_WIFIObject_t *Obj, int32_t enable,
                                                 mx_wifi_netlink_input_cb_t netlink_input_callbck, void *user_args);

/**
  * @brief  Network bypass mode data output
  * @param  Obj: pointer to module handle
  * @param  data: pbuf payload
  * @param  len:  payload len
  * @param  interface: STATION_IDX, SOFTAP_IDX
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
MX_WIFI_STATUS_T MX_WIFI_Network_bypass_netlink_output(MX_WIFIObject_t *Obj, void *data,
                                                       int32_t len,
                                                       int32_t interface);
#endif /* MX_WIFI_NETWORK_BYPASS_MODE */


/**
  * @} **
  */

/**
  * @brief Wi-Fi events
  */
enum mwifi_event_e
{
  MWIFI_EVENT_NONE        = 0x00, /**< invalid event */
  MWIFI_EVENT_STA_DOWN    = 0x01, /**< Wi-Fi station down event. */
  MWIFI_EVENT_STA_UP      = 0x02, /**< Wi-Fi station up event. */
  MWIFI_EVENT_STA_GOT_IP  = 0X03, /**< Wi-Fi station got ip event. */
  MWIFI_EVENT_AP_DOWN     = 0x04, /**< Wi-Fi softap down event. */
  MWIFI_EVENT_AP_UP       = 0x05  /**< Wi-Fi softap up event. */
};

typedef uint8_t mwifi_status_t;   /**< wifi status event. ref to @ref mwifi_event_e */

/**
  * @defgroup MX_WIFI_BASIC Station
  * @brief station mode API
  * @{ **
  */

/**
  * @brief Wi-Fi station link info
  */
#pragma pack(1)
typedef struct
{
  uint8_t is_connected;             /**< Wi-Fi station connection status. */
  char    ssid[MX_MAX_SSID_LEN];    /**< Wi-Fi connection AP ssid to connect. */
  uint8_t bssid[MX_MAX_BSSID_LEN];  /**< Wi-Fi connection AP bssid. */
  uint8_t security;                 /**< Wi-Fi connection security type. */
  uint8_t channel;                  /**< Wi-Fi connection channel. */
  int32_t rssi;                     /**< Wi-Fi connection rssi. */
} mc_wifi_link_info_t;
#pragma pack()


/**
  * @brief  Reset the module by Software.
  * @param  Obj: pointer to module handle
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
MX_WIFI_STATUS_T MX_WIFI_ResetModule(MX_WIFIObject_t *Obj);


/**
  * @brief  Reset To factory defaults (do nothing since no data saved in the module now).
  * @param  Obj: pointer to module handle
  * @note   NOT USED NOW
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
MX_WIFI_STATUS_T MX_WIFI_ResetToFactoryDefault(MX_WIFIObject_t *Obj);


/**
  * @brief  Get the firmware version string of the wifi module.
  * @param  Obj: pointer to module handle
  * @param  version: buffer pointer to receive the version string (e.g "V2.3.0").
  * @param  size: length of the buffer, max size 24Bytes.
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
MX_WIFI_STATUS_T MX_WIFI_GetVersion(MX_WIFIObject_t *Obj, uint8_t *version, uint32_t size);


/**
  * @brief  Get the MAC address of the wifi station.
  * @param  Obj: pointer to module handle
  * @param  Mac: pointer to buffer to receive the MAC address array, size 6 bytes.
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
MX_WIFI_STATUS_T MX_WIFI_GetMACAddress(MX_WIFIObject_t *Obj, uint8_t *Mac);


/**
  * @brief  Get the WiFi softap MAC address
  * @param  Obj: pointer to module handle
  * @param  Mac: buffer to receive the softAP MAC address (6 bytes)
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
MX_WIFI_STATUS_T MX_WIFI_GetsoftapMACAddress(MX_WIFIObject_t *Obj, uint8_t *Mac);


/**
  * @brief  wifi scan in blocking mode
  * @param  Obj: pointer to module handle
  * @param  ScanMode: scan mode, active or passive scan.
  * @param  SSID: ssid for active scan (scan specified AP), not used (set NULL) if do passive scan (scan all APs)
  * @param  Len:  ssid len of the AP to scan, not used (set 0) if do passive scan
  * @note   This API just start scan,  use @MX_WIFI_Get_scan_result to get the scan result.
  * @code   Example:
  *         Active scan:
  *            MX_WIFI_Scan(pWifiObj, MC_SCAN_ACTIVE, "ssid_ap", 7);
  *         Passive scan:
  *            MX_WIFI_Scan(pWifiObj, MC_SCAN_PASSIVE, NULL, 0);
  *
  *         Get scan result:
  *            mwifi_ap_info_t mx_aps[MX_WIFI_MAX_DETECTED_AP];  (array to store scan AP info)
  *            int32_t ap_num;
  *            ap_num = MX_WIFI_Get_scan_result(pWifiObj, (uint8_t*)&(mx_aps[0]), MX_WIFI_MAX_DETECTED_AP);
  *            if(ap_num > 0)
  *            {
  *              for(int32_t i = 0; i < ap_num; i++)
  *              {
  *                ( mx_aps[i].ssid )
  *                ( mx_aps[i].rssi )
  *              }
  *            }
  * @endcode
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
MX_WIFI_STATUS_T MX_WIFI_Scan(MX_WIFIObject_t *Obj, mc_wifi_scan_mode_t ScanMode, char *SSID, int32_t Len);


/**
  * @brief  wifi get scan result, call this after MX_WIFI_Scan.
  * @param  Obj: pointer to module handle
  * @param  Results: scan result buffer, contains mwifi_ap_info_t * number, ordered by time the AP found.
  * @param  Number: max ap number to get, max 10
  * @retval return the real ap number got, 0 ~ max (10).
  * @note   must be called after @MX_WIFI_Scan
  */
int8_t MX_WIFI_Get_scan_result(MX_WIFIObject_t *Obj, uint8_t *Results, uint8_t Number);


/**
  * @brief  Register wifi status changed callback with station interface only.
  * @param  Obj: pointer to module handle
  * @param  Cb: wifi status callback function
  * @param  arg: argument pass to callback
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
MX_WIFI_STATUS_T MX_WIFI_RegisterStatusCallback(MX_WIFIObject_t *Obj,
                                                mx_wifi_status_callback_t Cb, void *arg);


/**
  * @brief  Register wifi status changed callback with the specific wifi interface (STA/AP), recommended.
  * @param  Obj: pointer to module handle
  * @param  Cb: wifi status callback function
  * @param  Arg: argument pass to callback
  * @param  Interface: wifi interface STATION or SOFTAP
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
MX_WIFI_STATUS_T MX_WIFI_RegisterStatusCallback_if(MX_WIFIObject_t *Obj,
                                                   mx_wifi_status_callback_t Cb,
                                                   void *Arg,
                                                   mwifi_if_t Interface);


/**
  * @brief  UnRegister wifi status changed callback
  * @param  Obj: pointer to module handle
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  * @note   work with API MX_WIFI_RegisterStatusCallback
  */
MX_WIFI_STATUS_T MX_WIFI_UnRegisterStatusCallback(MX_WIFIObject_t *Obj);


/**
  * @brief  UnRegister wifi status changed callback with the specific wifi interface
  * @param  Obj: pointer to module handle
  * @param  Interface: wifi interface STATION or SOFTAP
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  * @note   work with API MX_WIFI_RegisterStatusCallback_if
  */
MX_WIFI_STATUS_T MX_WIFI_UnRegisterStatusCallback_if(MX_WIFIObject_t *Obj, mwifi_if_t Interface);


/**
  * @brief  Join an Access point, connect in asynchronous mode,
  *         wifi status will be returned from a callback set by MX_WIFI_RegisterStatusCallback.
  * @param  Obj: pointer to module handle.
  * @param  SSID: the access point id, max size 32 bytes.
  * @param  Password: the Access point password, max size 64 bytes.
  * @param  SecType: Security type (NOT USE NOW because the module will connect with the security
  *                  type get from AP by scan).
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  * @note   Set wifi status callback by MX_WIFI_RegisterStatusCallback or
  *         MX_WIFI_RegisterStatusCallback_if before this API.
  */
MX_WIFI_STATUS_T MX_WIFI_Connect(MX_WIFIObject_t *Obj, const mx_char_t *SSID,
                                 const mx_char_t *Password, MX_WIFI_SecurityType_t SecType);

/**
  * @brief Join an Access point, connect in asynchronous mode,
            wifi status will be returned from a callback set by MX_WIFI_RegisterStatusCallback or
             MX_WIFI_RegisterStatusCallback_if.
  * @param Obj: pointer to module handle
  * @param SSID: the access point id, max size 32 bytes.
  * @param Password: the Access point password, max size 64 bytes.
  * @param Attr: set extral attributes of Access-Point to connect, set NULL if use auto mode.
  * @param IP: set static ip to connect, set NULL if use DHCP mode to get ip automatically.
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  * @note   Set wifi status callback by MX_WIFI_RegisterStatusCallback or
  *         MX_WIFI_RegisterStatusCallback_if before this API.
  */
MX_WIFI_STATUS_T MX_WIFI_Connect_Adv(MX_WIFIObject_t *Obj, const mx_char_t *SSID, const mx_char_t *Password,
                                     mwifi_connect_attr_t *Attr, mwifi_ip_attr_t *IP);

/**
  * @brief  Join an Access point with WPA-Enterprise.
  *         Connect in asynchronous mode, wifi status will be returned from a callback set by
  *         MX_WIFI_RegisterStatusCallback or MX_WIFI_RegisterStatusCallback_if.
  * @param  Obj: pointer to module handle
  * @param  SSID: the access point ID.
  * @param  Identity: client identity.
  * @param  Password: client password.
  * @param  Attr: extral attributes of EAP method. NULL for default mode EAP-PEAP.
  * @param  IP: Station IP settings, NULL for DHCP mode.
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  * @note   Set wifi status callback by MX_WIFI_RegisterStatusCallback or
  *         MX_WIFI_RegisterStatusCallback_if before this API.
  */
MX_WIFI_STATUS_T MX_WIFI_EAP_Connect(MX_WIFIObject_t *Obj, const char *SSID,
                                     const char *Identity, const char *Password,
                                     mwifi_eap_attr_t *Attr, mwifi_ip_attr_t *IP);

/**
  * @brief  Disconnect from a station network.
  * @param  Obj: pointer to module handle
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
MX_WIFI_STATUS_T MX_WIFI_Disconnect(MX_WIFIObject_t *Obj);

/**
  * @brief  Join an Access point with WPS (PUSH-BUTTON only) mode.
  *         Connect in asynchronous mode, wifi status will be returned from a callback set by
  *         MX_WIFI_RegisterStatusCallback.
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  * @note   Set wifi status callback by MX_WIFI_RegisterStatusCallback or
  *         MX_WIFI_RegisterStatusCallback_if before this API.
  */
MX_WIFI_STATUS_T MX_WIFI_WPS_Connect(MX_WIFIObject_t *Obj);

/**
  * @brief  Stop WPS connect.
  * @param  Obj: pointer to module handle
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
MX_WIFI_STATUS_T MX_WIFI_WPS_Stop(MX_WIFIObject_t *Obj);

/**
  * @brief  Check whether the module is connected to an access point.
  * @param  Obj: pointer to module handle
  * @retval link status 1: connected, otherwise not connected.
  */
int8_t MX_WIFI_IsConnected(MX_WIFIObject_t *Obj);

/**
  * @brief  Get the local IPv4 address of the wifi module.
  * @param  Obj: pointer to module handle
  * @param  IpAddr: pointer to buffer to receive the IP address array (4 bytes).
  * @param  WifiMode: wifi interface (station or softap).
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
MX_WIFI_STATUS_T MX_WIFI_GetIPAddress(MX_WIFIObject_t *Obj, uint8_t *IpAddr, mwifi_if_t WifiMode);

/**
  * @brief  Get the local IPv6 address of the wifi module.
  * @param  Obj: pointer to module handle
  * @param  IpAddr6: buf to the IPv6 address array (16 bytes).
  * @param  AddrSlot: index of the IPv6 address (index: 0/1/2).
  * @param  WifiMode: wifi interface (station or softap).
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
MX_WIFI_STATUS_T MX_WIFI_GetIP6Address(MX_WIFIObject_t *Obj, uint8_t *IpAddr6, int32_t AddrSlot, mwifi_if_t WifiMode);

/**
  * @brief  Get the local IPv6 address state of the wifi module.
  * @param  Obj: pointer to module handle
  * @param  AddrSlot: index of the IPv6 address (index: 0/1/2).
  * @param  WifiMode: wifi interface (station or softap).
  * @retval IPV6 address State, error if < 0, error code @ref mx_wifi_status_e
  */
int32_t MX_WIFI_GetIP6AddressState(MX_WIFIObject_t *Obj, int32_t AddrSlot, mwifi_if_t WifiMode);


/**
  * @} **
  */

/**
  * @defgroup MX_WIFI_SOFTAP SoftAP
  * @brief softAP mode API
  * @{ **
  */

/**
  * @brief Wi-Fi softAP info
  */

#pragma pack(1)
typedef struct
{
  int32_t rssi;                 /**< Signal strength of the AP */
  char ssid[33];                /**< SSID of the AP */
  uint8_t bssid[6];             /**< BSSID of the AP */
  int32_t channel;              /**< Channel of the AP */
  mwifi_security_t security;    /**< security of the AP */
} mwifi_ap_info_t;
#pragma pack()

/**
  * @brief  Start softAP (miniAP) mode
  *         Asynchronous mode, wifi status will be returned from a callback set by MX_WIFI_RegisterStatusCallback.
  * @param  Obj: pointer to module handle
  * @param  ApSettings: softAP settings.
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
MX_WIFI_STATUS_T MX_WIFI_StartAP(MX_WIFIObject_t *Obj, MX_WIFI_APSettings_t *ApSettings);

/**
  * @brief  Stop softAP (miniAP) mode
  *         Asynchronous mode, wifi status will be returned from a callback set by MX_WIFI_RegisterStatusCallback.
  * @param  Obj: pointer to module handle
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
MX_WIFI_STATUS_T MX_WIFI_StopAP(MX_WIFIObject_t *Obj);

/**
  * @} **
  */

#if (MX_WIFI_NETWORK_BYPASS_MODE == 0)

/**
  * @defgroup WIFI_SOCKET Socket
  * @brief socket related API
  * @{ **
  */


/**
  * @brief  Create a socket.
  * @param  Obj: pointer to module handle
  * @param  Domain: socket domain
  * @param  Type: socket type
  * @param  Protocol: socket protocol
  * @retval Socket file descriptor, return < 0 if failed.
  */
int32_t MX_WIFI_Socket_create(MX_WIFIObject_t *Obj, int32_t Domain, int32_t Type, int32_t Protocol);


/**
  * @brief  Set option for a socket
  * @param  Obj: pointer to module handle
  * @param  SockFd: socket fd
  * @param  Level: option level
  * @param  OptName: option to set
  * @param  OptValue: value buffer for the option
  * @param  OptLen: length of the option value
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
int32_t MX_WIFI_Socket_setsockopt(MX_WIFIObject_t *Obj, int32_t SockFd, int32_t Level,
                                  int32_t OptName, const void *OptValue, int32_t OptLen);


/**
  * @brief  Get option of a socket.
  * @param  Obj: pointer to module handle
  * @param  SockFd: socket fd
  * @param  Level: option level
  * @param  OptName: option to set
  * @param  OptValue: buffer pointer of value of the option
  * @param  OptLen: buffer pointer of length of the option value
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
int32_t MX_WIFI_Socket_getsockopt(MX_WIFIObject_t *Obj, int32_t SockFd, int32_t Level,
                                  int32_t OptName, void *OptValue, uint32_t *OptLen);


/**
  * @brief  Bind a socket.
  * @param  Obj: pointer to module handle
  * @param  SockFd: socket fd
  * @param  Addr: socket address
  * @param  AddrLen: address length
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
int32_t MX_WIFI_Socket_bind(MX_WIFIObject_t *Obj, int32_t SockFd,
                            const struct mx_sockaddr *Addr, int32_t AddrLen);

/**
  * @brief  Listen a socket.
  * @param  Obj: pointer to module handle
  * @param  sockfd: socket fd
  * @param  backlog: max number to queued.
  * @retval Operation Status.
  */
int32_t MX_WIFI_Socket_listen(MX_WIFIObject_t *Obj, int32_t sockfd, int32_t backlog);

/**
  * @brief  Accept a socket.
  * @param  Obj: pointer to module handle
  * @param  SockFd: socket fd
  * @param  Addr: client address
  * @param  addrlen: length of client address
  * @retval AddrLen client socket fd, return < 0 if failed. error code @ref mx_wifi_status_e
  */
int32_t MX_WIFI_Socket_accept(MX_WIFIObject_t *Obj, int32_t SockFd,
                              struct mx_sockaddr *Addr, uint32_t *AddrLen);

/**
  * @brief  Socket connect.
  * @param  Obj: pointer to module handle
  * @param  SockFd: socket fd
  * @param  Addr: client address
  * @param  AddrLen: length of client address
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
int32_t MX_WIFI_Socket_connect(MX_WIFIObject_t *Obj, int32_t SockFd, const struct mx_sockaddr *Addr, int32_t AddrLen);

/**
  * @brief  Socket shutdown.
  * @param  Obj: pointer to module handle
  * @param  SockFd: socket fd
  * @param  Mode: shutdown mode:
  *               0    Stop receiving data for this socket;
  *               1    Stop trying to transmit data from this socket
  *               2    Stop all transmit from this socket
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
int32_t MX_WIFI_Socket_shutdown(MX_WIFIObject_t *Obj, int32_t SockFd, int32_t Mode);

/**
  * @brief  Socket close.
  * @param  Obj: pointer to module handle
  * @param  SockFd: socket fd
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
int32_t MX_WIFI_Socket_close(MX_WIFIObject_t *Obj, int32_t SockFd);

/**
  * @brief  Socket send.
  * @param  Obj: pointer to module handle
  * @param  SockFd: socket fd
  * @param  Buf: send data buffer
  * @param  Len: length of send data
  * @param  flags: zero for MXOS
  * @retval Number of bytes sent, return < 0 if failed, error code @ref mx_wifi_status_e
  */
int32_t MX_WIFI_Socket_send(MX_WIFIObject_t *Obj, int32_t SockFd, const uint8_t *Buf,
                            int32_t Len, int32_t flags);

/**
  * @brief  Socket recv.
  * @param  Obj: pointer to module handle
  * @param  SockFd: socket fd
  * @param  Buf: recv buffer
  * @param  Len: length of recv buffer
  * @param  flags: zero for MXOS
  * @retval Number of bytes received, return < 0 if failed, error code @ref mx_wifi_status_e
  */
int32_t MX_WIFI_Socket_recv(MX_WIFIObject_t *Obj, int32_t SockFd, uint8_t *Buf,
                            int32_t Len, int32_t flags);

/**
  * @brief  Socket sendto.
  * @param  Obj: pointer to module handle
  * @param  SockFd: socket fd
  * @param  Buf: send data buffer
  * @param  Len: length of send data
  * @param  Flags: zero for MXOS
  * @param  ToAddr: address to send
  * @param  toaddrlen: length of address to send
  * @retval Number of bytes sent, return < 0 if failed. error code @ref mx_wifi_status_e
  */
int32_t MX_WIFI_Socket_sendto(MX_WIFIObject_t *Obj, int32_t SockFd,
                              const uint8_t *Buf, int32_t Len, int32_t Flags,
                              struct mx_sockaddr *ToAddr, int32_t ToAddrLen);

/**
  * @brief  Socket recvfrom.
  * @param  Obj: pointer to module handle
  * @param  SockFd: socket fd
  * @param  Buf: recv buffer
  * @param  Len: length of recv buffer
  * @param  Flags: zero for MXOS
  * @param  FromAddr: address of the data source
  * @param  FromAddrLen: length of address of the data source
  * @retval Number of bytes received, return < 0 if failed. error code @ref mx_wifi_status_e
  */
int32_t MX_WIFI_Socket_recvfrom(MX_WIFIObject_t *Obj, int32_t SockFd, uint8_t *Buf,
                                int32_t Len, int32_t Flags,
                                struct mx_sockaddr *FromAddr, uint32_t *FromAddrLen);

/**
  * @brief  Gethostbyname, only for IPv4 address.
  * @param  Obj: pointer to module handle
  * @param  Addr: address of the host
  * @param  Name: hostname
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
int32_t MX_WIFI_Socket_gethostbyname(MX_WIFIObject_t *Obj, struct mx_sockaddr *Addr, const mx_char_t *Name);

/**
  * @brief  Ping a host, only for IPv4 address.
  * @param  Obj: pointer to module handle
  * @param  hostname: ping hostname or IPv4 string
  * @param  count: ping max count
  * @param  delay: ping delay in millisecond
  * @param  response: response time array of ping result, max size 10.
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
int32_t MX_WIFI_Socket_ping(MX_WIFIObject_t *Obj, const char *hostname, int32_t count, int32_t delay,
                            int32_t response[]);

/**
  * @brief  Ping a host, only for IPv6 address.
  * @param  Obj: pointer to module handle
  * @param  hostname: ping hostname or ipv6 string
  * @param  count: ping max count
  * @param  delay: ping delay in millisecond
  * @param  response: response time array of ping result, max size 10.
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
int32_t MX_WIFI_Socket_ping6(MX_WIFIObject_t *Obj, const mx_char_t *hostname, int32_t count, int32_t delay,
                             int32_t response[]);

/**
  * @brief  Get IPv4/v6 address info by nodename.
  * @param  Obj: pointer to module handle
  * @param  NodeName: descriptive name or address string of the host
  * @param  ServerName: not used, set to NULL
  * @param  Hints: structure containing input values that set socktype and protocol
  * @param  Res: buffer to store the result (set to NULL on failure)
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
int32_t MX_WIFI_Socket_getaddrinfo(MX_WIFIObject_t *Obj, const char *NodeName, const char *ServerName,
                                   const struct mx_addrinfo *Hints, struct mx_addrinfo *Res);

/**
  * @brief  Monitor multiple file descriptors for sockets
  * @attention  Never doing operations in different threads
  * @param  Obj: pointer to module handle
  * @param  nfds: is the highest-numbered file descriptor in any of the three
  *         sets, plus 1
  * @param  readfds: A file descriptor sets will be watched to see if characters
  *         become available for reading
  * @param  writefds: A file descriptor sets will be watched to see if a write
  *         will not block.
  * @param  exceptfds: A file descriptor sets will be watched for exceptions.
  * @param  timeout: The timeout argument specifies the interval that select()
  *         should block waiting for a file descriptor to become ready.
  *         If timeout is NULL (no timeout), select() can block until API timeout.
  * @retval On success, return the number of file descriptors contained in the
  *         three returned descriptor sets (that is, the total number of bits
  *         that are set in readfds, writefds, exceptfds) which may be zero if
  *         the timeout expires before anything interesting happens.  On error,
  *         -1 is returned, the file descriptor sets are unmodified, and timeout
  *         becomes undefined.
  */
int32_t MX_WIFI_Socket_select(MX_WIFIObject_t *Obj, int32_t nfds,
                              mx_fd_set *readfds, mx_fd_set *writefds,
                              mx_fd_set *exceptfds, struct mx_timeval *timeout);


/**
  * @brief  socket getpeername.
  * @param  Obj: pointer to module handle
  * @param  SockFd: socket fd
  * @param  Addr: address buffer
  * @param  AddrLen: size of address buffer
  * @retval get address of peer socket, return < 0 if failed, error code @ref mx_wifi_status_e
  */
int32_t MX_WIFI_Socket_getpeername(MX_WIFIObject_t *Obj, int32_t SockFd, struct mx_sockaddr *Addr, uint32_t *AddrLen);


/**
  * @brief  socket getsockname.
  * @param  Obj: pointer to module handle
  * @param  SockFd: socket fd
  * @param  Addr: address buffer
  * @param  AddrLen: size of address buffer
  * @retval get address of local socket, return < 0 if failed, error code @ref mx_wifi_status_e
  */
int32_t MX_WIFI_Socket_getsockname(MX_WIFIObject_t *Obj, int32_t SockFd, struct mx_sockaddr *Addr, uint32_t *AddrLen);


/**
  * @} **
  */

/**
  * @defgroup MX_WIFI_MDNS mDNS
  * @brief mDNS related API
  * @{ **
  */

/** Maximum length of labels
  *
  * A label is one segment of a DNS name. For example, "foo" is a label in the
  * name "foo.local.". RFC 1035 requires that labels do not exceed 63 bytes.
  */
#define MDNS_MAX_LABEL_LEN  63  /* defined by the standard */

/** Maximum length of names
  *
  * A name is a list of labels such as "My Webserver.foo.local" or
  * mydevice.local. RFC 1035 requires that names do not exceed 255 bytes.
  */
#define MDNS_MAX_NAME_LEN 255 /* defined by the standard : 255 */

/** Maximum length of key/value pair
  *
  * TXT records associated with a service are populated with key/value pairs.
  * These key/value pairs must not exceed this length.
  */
#define MDNS_MAX_KEYVAL_LEN 255 /* defined by the standard : 255 */

/** protocol values for the proto member of the mdns_service descriptor */
/** TCP Protocol */
#define MDNS_PROTO_TCP 0
/** UDP Protocol */
#define MDNS_PROTO_UDP 1

/** Maximum no. of services allowed to be announced on a single interface. */
#define MAX_MDNS_LST 5 /* Maximum no. of services */

/* MDNS Error Codes */
#define ERR_MDNS_BASE              -36650  /**< Starting error code for all mdns errors. */
#define ERR_MDNS_INVAL             -36651  /**< invalid argument */
#define ERR_MDNS_BADSRC            -36652  /**< bad service descriptor */
#define ERR_MDNS_TOOBIG            -36653  /**< not enough room for everything */
#define ERR_MDNS_NOIMPL            -36654  /**< unimplemented feature */
#define ERR_MDNS_NOMEM             -36655  /**< insufficient memory */
#define ERR_MDNS_INUSE             -36656  /**< requested resource is in use */
#define ERR_MDNS_NORESP            -36657  /**< requested resource is in use */
#define ERR_MDNS_FSOC              -36658  /**< failed to create socket for mdns */
#define ERR_MDNS_FREUSE            -36659  /**< failed to reuse multicast socket */
#define ERR_MDNS_FBINDTODEVICE     -36660  /**< failed to bind mdns socket to device */
#define ERR_MDNS_FBIND             -36661  /**< failed to bind mdns socket */
#define ERR_MDNS_FMCAST_JOIN       -36662  /**< failed to join multicast socket */
#define ERR_MDNS_FMCAST_SET        -36663  /**< failed to set multicast socket */
#define ERR_MDNS_FQUERY_SOC        -36664  /**< failed to create query socket */
#define ERR_MDNS_FQUERY_THREAD     -36665  /**< failed to create mdns thread */
#define ERR_MDNS_END               -36670  /**< Last generic error code (inclusive) */

/** mDNS Interface State
  * mDNS interface state can be changed by using mdns_iface_state_change()
  * function. For details about when to use the enum please refer to
  * documentation for mdns_iface_state_change(). */
enum iface_state
{
  /** UP the interface and announce services
    * mDNS will probe and announce all services announced via
    * mdns_announce_service() and/or mdns_announce_service_arr().
    * mDNS will go through entire probing sequence explained in above
    * functions. Interface state can be changed to UP, if its DOWN.
    */
  UP = 0,
  /** DOWN the interface and de-announce services
    * mDNS sends good bye packet with ttl=0 so that mDNS clients can remove
    * the services from their mDNS cache table.
    */
  DOWN,
  /** Forcefully re-announce services
    * This state should be used after services are already
    * announced and force announcement is needed due to some reason.
    * mDNS will not perform probing sequence, as it does in case of UP, and
    * will directly re-announce services.
    */
  REANNOUNCE
};

/**
  * @brief  mDNS service info
  */
#pragma pack(1)
struct mc_mdns_service
{
  /** Name of MDNS service  */
  char servname[MDNS_MAX_LABEL_LEN + 1];
  /** Type of MDNS service */
  char servtype[MDNS_MAX_LABEL_LEN + 1];
  /** Domain for MDNS service */
  char domain[MDNS_MAX_LABEL_LEN + 1];
  /** Port number  */
  uint16_t port;
  /** Protocol used */
  int32_t proto;
  /** Key value pairs for TXT records*/
  char keyvals[MDNS_MAX_KEYVAL_LEN + 1];
  /** IP Address of device */
  uint32_t ipaddr;
  /** IPv6 Address of device */
  uint32_t ip6addr[4];

  /** separator for txt record */
  char separator;  /* user set this for keyvals */
};
#pragma pack()

/**
  * @brief  start mDNS service.
  * @param  Obj: pointer to module handle
  * @param  domain: domain of service
  * @param  name: hostname
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
int32_t MX_WIFI_MDNS_start(MX_WIFIObject_t *Obj, const mx_char_t *domain, mx_char_t *hostname);

/**
  * @brief  stop mDNS service.
  * @param  Obj: pointer to module handle
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
int32_t MX_WIFI_MDNS_stop(MX_WIFIObject_t *Obj);

/**
  * @brief  announce a service.
  * @param  Obj: pointer to module handle
  * @param  service: service to announce
  * @param  interface: wifi interface
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
int32_t MX_WIFI_MDNS_announce_service(MX_WIFIObject_t *Obj, struct mc_mdns_service *service, mwifi_if_t interface);

/**
  * @brief  deannounce a service.
  * @param  Obj: pointer to module handle
  * @param  service: service to deannounce
  * @param  interface: wifi interface
  * @retval Operation Status.
  */
int32_t MX_WIFI_MDNS_deannounce_service(MX_WIFIObject_t *Obj, struct mc_mdns_service *service, mwifi_if_t interface);

/**
  * @brief  deannounce all services.
  * @param  Obj: pointer to module handle
  * @param  interface: wifi interface
  * @retval Operation Status.
  */
int32_t MX_WIFI_MDNS_deannounce_service_all(MX_WIFIObject_t *Obj, mwifi_if_t interface);

/**
  * @brief  Send interface state change event to mdns
  * @param  Obj: pointer to module handle
  * @param  interface: wifi interface
  * @param  state: state event, valid interface state from \ref iface_state
  * @retval Operation Status.
  */
int32_t MX_WIFI_MDNS_iface_state_change(MX_WIFIObject_t *Obj, mwifi_if_t interface, enum iface_state state);

/**
  * @brief  Set new host name, use mdns_iface_state_change (interface, REANNOUNCE) to announce
  *         the new host name.
  * @param  Obj: pointer to module handle
  * @param  hostname: new hostname
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
int32_t MX_WIFI_MDNS_set_hostname(MX_WIFIObject_t *Obj, char *hostname);

/**
  * @brief  sets the TXT record field for a given mDNS service.
  * @param  Obj: pointer to module handle
  * @param  Service: mDNS service
  * @param  KeyVals: new txt record string
  * @param  separator: the separator used to separate individual key value pairs
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
int32_t MX_WIFI_MDNS_set_txt_rec(MX_WIFIObject_t *Obj, struct mc_mdns_service *Service, mx_char_t *KeyVals,
                                 mx_char_t separator);

/**
  * @} **
  */

/**
  * @defgroup MX_WIFI_TLS TLS
  * @brief TLS related API
  * @{ **
  */

/**
  * @brief mxchip TLS handler type
  */
typedef void *mtls_t;

/**
  * @brief mxchip TLS version
  */
enum
{
  SSL_V3_MODE   = 1, /**< SSL V3   */
  TLS_V1_0_MODE = 2, /**< TLS V1.0 */
  TLS_V1_1_MODE = 3, /**< TLS V1.1 */
  TLS_V1_2_MODE = 4  /**< TLS V1.2 */
};

typedef uint8_t mtls_ver_t;

/**
  * @brief  set the TLS protocol version.
  * @param  Obj: pointer to module handle
  * @param  version: TLS protocol version
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  * @note   This function should be called before TLS is ready to function (before
  *         mtls_connect and mtls_accept is called by application ).
  */
int32_t MX_WIFI_TLS_set_ver(MX_WIFIObject_t *Obj, mtls_ver_t version);

/**
  * @brief   TLS set client certificate
  * @param   Obj: pointer to module handle
  * @param   client_cert: Point to buffer of client cert.
  * @param   cert_len: length of the client cert.
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
int32_t MX_WIFI_TLS_set_clientCertificate(MX_WIFIObject_t *Obj, uint8_t *client_cert, uint16_t cert_len);

/**
  * @brief   TLS set client private key
  * @param   Obj: pointer to module handle
  * @param   client_private_key: Point to buffer of client private key.
  * @param   key_len: length of the client cert.
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
int32_t MX_WIFI_TLS_set_clientPrivateKey(MX_WIFIObject_t *Obj, uint8_t *client_private_key, uint16_t key_len);

/**
  * @brief   TLS client create a TLS connection.
  * @param   Obj: pointer to module handle
  * @details This function is called on the client side and initiates an TLS/TLS handshake with a
  *              server. When this function is called, the underlying communication channel has already
  *              been set up. This function is called after TCP 3-way handshake finished.
  * @param   domain: Specifies a communication domain
  * @param   type: Specifies the communication semantics.
  * @param   protocol: specifies a particular protocol to be used with the socket.
  * @param   Addr: Point to the target address to be connected
  * @param   AddrLen: containing the size of the buffer pointed to by addr
  * @param   ca: pointer to the CA certificate string, used to verify server's certificate.
  * @param   calen: the string length of ca. 0=do not verify server's certificate.
  * @retval  return the TLS context pointer on success or NULL for fail.
  */
int32_t MX_WIFI_TLS_connect(MX_WIFIObject_t *Obj, int32_t domain, int32_t type, int32_t protocol,
                            const struct mx_sockaddr *Addr, int32_t AddrLen, mx_char_t *ca, int32_t calen);

/**
  * @brief   TLS client create a TLS connection with SNI.
  * @param   Obj: pointer to module handle
  * @details This function is called on the client side and initiates an TLS/TLS handshake with a
  *              server. When this function is called, the underlying communication channel has already
  *              been set up. This function is called after TCP 3-way handshake finished.
  * @param   sni_servername: Specifies the SNI servername
  * @param   sni_servername_len: Specifies the SNI servername length, max size MX_TLS_SNI_SERVERNAME_SIZE
  * @param   addr: Point to the target address to be connected
  * @param   addrlen: containing the size of the buffer pointed to by addr
  * @param   ca: pointer to the CA certificate string, used to verify server's certificate.
  * @param   calen: the string length of ca. 0=do not verify server's certificate.
  * @retval  return the TLS context pointer on success or NULL for fail.
  */
int32_t MX_WIFI_TLS_connect_sni(MX_WIFIObject_t *Obj, const mx_char_t *sni_servername, int32_t sni_servername_len,
                                const struct mx_sockaddr *addr, int32_t addrlen, mx_char_t *ca, int32_t calen);

/**
  * @brief   TLS send data
  * @param   Obj: pointer to module handle
  * @param   tls: Point to the TLS context.
  * @param   Data: Point to data to send.
  * @param   Len: data length.
  * @retval  On success, return the number of bytes sent.  On error,
  *          error code (< 0) is returned, ref to @ref mx_wifi_status_e
  */
int32_t MX_WIFI_TLS_send(MX_WIFIObject_t *Obj, mtls_t tls, const void *Data, int32_t Len);

/**
  * @brief   TLS redeive data
  * @param   Obj: pointer to module handle
  * @param   tls: Point to the TLS context.
  * @param   Data: Point to buffer to receive TLS data.
  * @param   Len: max receive buffer length.
  * @retval  On success, return the number of bytes received.  On error,
  *          error code (< 0) is returned, ref to @ref mx_wifi_status_e
  */
int32_t MX_WIFI_TLS_recv(MX_WIFIObject_t *Obj, mtls_t tls, void *Data, int32_t Len);

/**
  * @brief   Close the TLS session, release resource.
  * @param   Obj: pointer to module handle
  * @param   tls: Point to the TLS context.
  * @return  status code
  * @retval  MX_WIFI_STATUS_OK success
  * @retval  others failure, error code @ref mx_wifi_status_e.
  */
int32_t MX_WIFI_TLS_close(MX_WIFIObject_t *Obj, mtls_t tls);

/**
  * @brief   Set TLS nonblock mode.
  * @param   Obj: pointer to module handle
  * @param   tls: Point to the TLS context.
  * @param   nonblock: true - nonblock, flase - block
  * @return  status code
  * @retval  MX_WIFI_STATUS_OK success
  * @retval  others failure, error code @ref mx_wifi_status_e.
  */
int32_t MX_WIFI_TLS_set_nonblock(MX_WIFIObject_t *Obj, mtls_t tls, int32_t nonblock);

/**
  * @} **
  */


/**
  * @defgroup MX_WIFI_WEBSERVER Webserver
  * @brief Webserver related API
  * @{ **
  */


/**
  * @brief  Start webserver.
  * @param  Obj: pointer to module handle
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
int32_t MX_WIFI_Webserver_start(MX_WIFIObject_t *Obj);

/**
  * @brief  Stop webserver.
  * @param  Obj: pointer to module handle
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
int32_t MX_WIFI_Webserver_stop(MX_WIFIObject_t *Obj);


/**
  * @} **
  */
#endif /* MX_WIFI_NETWORK_BYPASS_MODE */


/**
  * @defgroup MX_WIFI_FOTA FOTA
  * @brief FOTA related API
  * @{ **
  */


/**
  * @brief   Start FOTA.
  * @param   Obj: pointer to module handle
  * @param   Url: HTTP/HTTPS url of bin file to update
  * @param   Md5: Message Digest 5 string (32 Bytes) of bin file to update
  * @param   FotaStatusCallback: callback function for OTA status
  * @retval  Operation Status.
  */
int32_t MX_WIFI_FOTA_start(MX_WIFIObject_t *Obj, const char *Url, const char *Md5,
                           mx_wifi_fota_status_cb_t FotaStatusCallback, uint32_t UserArgs);

/**
  * @} **
  */



/**
  * @defgroup MX_WIFI_STATION_POWERSAVE
  * @brief Wi-Fi station powersave API
  * @{ **
  */


/**
  * @brief  Set powersave onoff for wifi station mode.
  * @param  Obj: pointer to module handle
  * @param  ps_onoff: 0 -- powersave off, 1 -- powersave on
  * @return status code
  * @retval MX_WIFI_STATUS_OK success
  * @retval others failure, error code @ref mx_wifi_status_e.
  */
int32_t MX_WIFI_station_powersave(MX_WIFIObject_t *Obj, int32_t ps_onoff);


/**
  * @} **
  */


/**
  * @} **
  */



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MX_WIFI_H */
