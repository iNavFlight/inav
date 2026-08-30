/**
  ******************************************************************************
  * @file    net_mbedtls.c
  * @author  MCD Application Team
  * @brief   Network abstraction at transport layer level. mbedTLS implementation.
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
#include "net_connect.h"
#include "net_internals.h"
#ifdef NET_MBEDTLS_HOST_SUPPORT

#if (osCMSIS >= 0x20000U)
#define OSSEMAPHOREWAIT         osSemaphoreAcquire
#else
#define OSSEMAPHOREWAIT         osSemaphoreWait
#endif /* osCMSIS */

int32_t  mbedtls_rng_raw(void *data, uchar_t *output, size_t len);

extern struct __RNG_HandleTypeDef hrng;

/* Private defines -----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void mbedtls_free_resource(net_socket_t *sock);
static int32_t  mbedtls_net_recv(void *ctx, uchar_t *buf, size_t len, uint32_t timeout);
static int32_t  mbedtls_net_send(void *ctx, const uchar_t *buf, size_t len);

#ifdef NET_USE_RTOS
extern void *pxCurrentTCB;


#ifdef  MBEDTLS_THREADING_ALT


void mutex_init(mbedtls_threading_mutex_t   *mutex);
void mutex_free(mbedtls_threading_mutex_t   *mutex);
int32_t mutex_lock(mbedtls_threading_mutex_t *mutex);
int32_t mutex_unlock(mbedtls_threading_mutex_t *mutex);

void mutex_init(mbedtls_threading_mutex_t   *mutex)
{
#if (osCMSIS >= 0x20000U)
  mutex->id = osSemaphoreNew(1, 1, NULL);
#else
  mutex->id = osSemaphoreCreate(&mutex->def, 1);
#endif /* osCMSIS */
}

void mutex_free(mbedtls_threading_mutex_t   *mutex)
{
  (void) osSemaphoreDelete(mutex->id);
}

#define WAITFOREVER     0xFFFFFFFFU     /* redefined for  MISRA checks */

int32_t mutex_lock(mbedtls_threading_mutex_t *mutex)
{
  BaseType_t ret;
  ret = (BaseType_t)  OSSEMAPHOREWAIT(mutex->id, WAITFOREVER);
  return (ret >= 0) ? 0 : -1;
}

int32_t mutex_unlock(mbedtls_threading_mutex_t *mutex)
{
  BaseType_t ret;
  ret = (BaseType_t) osSemaphoreRelease(mutex->id);
  return (ret >= 0) ? 0 : -1;
}

#endif /* MBEDTLS_THREADING_ALT */
#endif /* NET_USE_RTOS */

void net_tls_init(void)
{
#ifdef MBEDTLS_THREADING_ALT
  mbedtls_threading_set_alt(mutex_init, mutex_free, mutex_lock, mutex_unlock);
#endif /* MBEDTLS_THREADING_ALT */
}

void net_tls_destroy(void)
{
#ifdef MBEDTLS_THREADING_ALT
  mbedtls_threading_free_alt();
#endif /* MBEDTLS_THREADING_ALT */
}

/* Functions Definition ------------------------------------------------------*/
bool net_mbedtls_check_tlsdata(net_socket_t *sock)
{
  bool ret = true;
  void  *p;
  if (NULL == sock->tlsData)
  {
    /*cstat -MISRAC2012-Rule-21.3 -MISRAC2012-Dir-4.12 */
    p = NET_MALLOC(sizeof(net_tls_data_t));
    /*cstat +MISRAC2012-Rule-21.3 +MISRAC2012-Dir-4.12 */
    if (p == NULL)
    {
      NET_DBG_ERROR("Error during setting option.\n");
      ret = false;
    }
    else
    {
      /*cstat -MISRAC2012-Rule-11.5 */
      sock->tlsData = p;
      /*cstat +MISRAC2012-Rule-11.5 */
      (void) memset(sock->tlsData, 0, sizeof(net_tls_data_t));
      sock->tlsData->tls_srv_verification = true;
    }
  }
  return ret;
}


static void DebugPrint(void *ctx,
                       int32_t level,
                       const char_t *file,
                       int32_t line,
                       const char_t *str)

{
  /* Unused parameters. */
  (void)ctx;
  (void) level;
#ifdef NET_USE_RTOS
  NET_PRINT_WO_CR("%p => %s:%04ld: %s\n", pxCurrentTCB, file, (int32_t) line, str);
#else
  NET_PRINT_WO_CR("%s:%04ld: %s\n", file, (uint32_t) line, str);
#endif /* NET_USE_RTOS */
}


void net_mbedtls_set_read_timeout(net_socket_t *sock)
{
  net_tls_data_t *tlsData = sock->tlsData;
  if (tlsData != NULL)
  {
    mbedtls_ssl_conf_read_timeout(&tlsData->conf, (uint32_t) sock->read_timeout);
  }
}



static  void *net_wrapper_calloc(size_t  n, size_t m)
{
  /*cstat -MISRAC2012-Dir-4.12 -MISRAC2012-Rule-21.3 */
  return NET_CALLOC(n, m);
  /*cstat +MISRAC2012-Dir-4.12 +MISRAC2012-Rule-21.3 */
}

/*cstat -MISRAC2012-Dir-4.6_b */
typedef void (*mbedtls_debug_func_t)(void *, int, const char *, int, const char *);
typedef int (*mbedtls_rng_func_t)(void *, unsigned char *, size_t);
/*cstat +MISRAC2012-Dir-4.6_b */


static void net_wrapper_free(void *p)
{
  /*cstat -MISRAC2012-Rule-21.3 */
  NET_FREE(p);
  /*cstat +MISRAC2012-Rule-21.3 */
}

uint32_t        NET_TICK(void);

int32_t net_mbedtls_start(net_socket_t *sock)
{
  int32_t       ret = NET_OK;
  net_tls_data_t *tlsData = sock->tlsData;
  uint32_t      start_tick;

  (void)   mbedtls_platform_set_calloc_free(net_wrapper_calloc, net_wrapper_free);
  mbedtls_ssl_init(&tlsData->ssl);
  mbedtls_ssl_config_init(&tlsData->conf);
  /*cstat -MISRAC2012-Rule-11.1 */
  mbedtls_ssl_conf_dbg(&tlsData->conf, (mbedtls_debug_func_t) DebugPrint, NULL);
  /*cstat +MISRAC2012-Rule-11.1 */

  mbedtls_debug_set_threshold(NET_MBEDTLS_DEBUG_LEVEL);
  mbedtls_x509_crt_init(&tlsData->cacert);
  if (tlsData->tls_dev_cert != NULL)
  {
    mbedtls_x509_crt_init(&tlsData->clicert);
  }
  if (tlsData->tls_dev_key != NULL)
  {
    mbedtls_pk_init(&tlsData->pkey);
  }

  /* Root CA */
  if (tlsData->tls_ca_certs != NULL)
  {
    ret = mbedtls_x509_crt_parse(&tlsData->cacert, (uchar_t const *) tlsData->tls_ca_certs,
                                 strlen((char_t const *) tlsData->tls_ca_certs) + 1U);

    if (ret != 0)
    {
      NET_DBG_ERROR(" failed\n  !  mbedtls_x509_crt_parse returned 0x%lx while parsing root cert\n", ret);
      mbedtls_free_resource(sock);
      ret =  NET_ERROR_MBEDTLS_CRT_PARSE;
    }
  }
  else
  {
    /*no root so cannot check server identification ?
     tlsData->tls_srv_verification = */
  }


  /* Client cert. and key */
  if ((ret == NET_OK) && (tlsData->tls_dev_cert != NULL) && (tlsData->tls_dev_key != NULL))
  {
    ret = mbedtls_x509_crt_parse(&tlsData->clicert, (uchar_t const *) tlsData->tls_dev_cert,
                                 strlen((char_t const *)tlsData->tls_dev_cert) + 1U);
    if (ret != 0)
    {
      NET_DBG_ERROR(" failed\n  !  mbedtls_x509_crt_parse returned -0x%lx while parsing device cert\n", -ret);
      mbedtls_free_resource(sock);
      ret = NET_ERROR_MBEDTLS_CRT_PARSE;
    }
    else
    {
      ret = mbedtls_pk_parse_key(&tlsData->pkey, (uchar_t const *)tlsData->tls_dev_key,
                                 strlen((char_t const *)tlsData->tls_dev_key) + 1U,
                                 (uchar_t const *)tlsData->tls_dev_pwd, tlsData->tls_dev_pwd_len);
      if (ret != 0)
      {
        NET_DBG_ERROR(" failed\n  !  mbedtls_pk_parse_key returned -0x%lx while parsing private key\n\n", -ret);
        mbedtls_free_resource(sock);
        ret = NET_ERROR_MBEDTLS_KEY_PARSE;
      }
    }
  }

  /* TLS Connection */
  if (ret == NET_OK)
  {
    ret = mbedtls_ssl_config_defaults(&tlsData->conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM,
                                      MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != 0)
    {
      NET_DBG_ERROR(" failed\n  ! mbedtls_ssl_config_defaults returned -0x%lx\n\n", -ret);
      mbedtls_free_resource(sock);
      ret = NET_ERROR_MBEDTLS_CONFIG;
    }
  }

  if (ret == NET_OK)
  {
    /* Allow the user to select a TLS profile? */
    if (tlsData->tls_cert_prof != NULL)
    {
      mbedtls_ssl_conf_cert_profile(&tlsData->conf, tlsData->tls_cert_prof);
    }

    /* Only for debug
     * mbedtls_ssl_conf_verify(&(tlsDataParams->conf), _iot_tls_verify_cert, NULL); */
    if (tlsData->tls_srv_verification == true)
    {
      mbedtls_ssl_conf_authmode(&tlsData->conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    }
    else
    {
      mbedtls_ssl_conf_authmode(&tlsData->conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    }

    /* no verification because no certificat */
    /*
    if (tlsData->tls_ca_certs == NULL)
    {
      mbedtls_ssl_conf_authmode(&tlsData->conf, MBEDTLS_SSL_VERIFY_NONE);
    }
    */
    /*cstat -MISRAC2012-Rule-11.1 */
    mbedtls_ssl_conf_rng(&tlsData->conf, (mbedtls_rng_func_t) mbedtls_rng_raw, &hrng);
    /*cstat +MISRAC2012-Rule-11.1 */
    mbedtls_ssl_conf_ca_chain(&tlsData->conf, &tlsData->cacert, NULL);

    if ((tlsData->tls_dev_cert != NULL) && (tlsData->tls_dev_key != NULL))
    {
      ret = mbedtls_ssl_conf_own_cert(&tlsData->conf, &tlsData->clicert, &tlsData->pkey);
      if (ret != 0)
      {
        NET_DBG_ERROR(" failed\n  ! mbedtls_ssl_conf_own_cert returned -0x%lx\n\n", -ret);
        mbedtls_free_resource(sock);
        ret = NET_ERROR_MBEDTLS_CONFIG;
      }
    }
  }
  if (ret == NET_OK)
  {
    ret = mbedtls_ssl_setup(&tlsData->ssl, &tlsData->conf);
    if (ret != 0)
    {
      NET_DBG_ERROR(" failed\n  ! mbedtls_ssl_setup returned -0x%lx\n\n", -ret);
      mbedtls_free_resource(sock);
      ret = NET_ERROR_MBEDTLS_SSL_SETUP;
    }
  }

  if ((ret == NET_OK) && (tlsData->tls_srv_name != NULL))
  {
    ret = mbedtls_ssl_set_hostname(&tlsData->ssl, (char_t const *)tlsData->tls_srv_name);
    if (ret  != 0)
    {
      NET_DBG_ERROR(" failed\n  ! mbedtls_ssl_set_hostname returned %ld\n\n", ret);
      mbedtls_free_resource(sock);
      ret = NET_ERROR_MBEDTLS_SET_HOSTNAME;
    }
  }

  if (ret == NET_OK)
  {
    /*cstat -MISRAC2012-Rule-11.1 */
    mbedtls_ssl_set_bio(&tlsData->ssl,  sock, (mbedtls_ssl_send_t *) mbedtls_net_send, NULL,
                        (mbedtls_ssl_recv_timeout_t *) mbedtls_net_recv);
    /*cstat +MISRAC2012-Rule-11.1 */
    mbedtls_ssl_conf_read_timeout(&tlsData->conf, (uint32_t)sock->read_timeout);


    NET_DBG_INFO("\n\nSSL state connect : %d ", sock->tlsData->ssl.state);


    NET_DBG_INFO("\n\nSSL state connect : %d ", sock->tlsData->ssl.state);
    NET_DBG_INFO("  . Performing the SSL/TLS handshake...");

    ret = mbedtls_ssl_handshake(&tlsData->ssl);
    start_tick = NET_TICK();
    while (ret != 0)
    {
      uint32_t  elapsed_tick = NET_TICK() - start_tick;

      if (elapsed_tick > NET_MBEDTLS_CONNECT_TIMEOUT)
      {
        mbedtls_free_resource(sock);
        ret = NET_ERROR_MBEDTLS_CONNECT;
        break;
      }

      if ((ret != MBEDTLS_ERR_SSL_WANT_READ) && (ret != MBEDTLS_ERR_SSL_WANT_WRITE))
      {
        tlsData->flags = mbedtls_ssl_get_verify_result(&tlsData->ssl);
        if (tlsData->flags != 0U)
        {
          char_t vrfy_buf[512];
          (void) mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", tlsData->flags);
          if (tlsData->tls_srv_verification == true)
          {
            NET_DBG_ERROR("Server verification:\n%s\n", vrfy_buf);
          }
          else
          {
            NET_DBG_INFO("Server verification:\n%s\n", vrfy_buf);
          }
        }
        NET_DBG_ERROR(" failed\n  ! mbedtls_ssl_handshake returned -0x%lx\n", -ret);

        mbedtls_free_resource(sock);
        ret = (ret == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED) ? NET_ERROR_MBEDTLS_REMOTE_AUTH : NET_ERROR_MBEDTLS_CONNECT;
        /*cstat -MISRAC2012-Rule-15.4 */
        break;
        /*cstat +MISRAC2012-Rule-15.4 */
      }
      ret = mbedtls_ssl_handshake(&tlsData->ssl);
    }

    if (ret == NET_OK)
    {
      int32_t exp;
      NET_DBG_INFO(" ok\n    [ Protocol is %s ]\n    [ Ciphersuite is %s ]\n",
                   mbedtls_ssl_get_version(&sock->tlsData->ssl),
                   mbedtls_ssl_get_ciphersuite(&sock->tlsData->ssl));

      exp = mbedtls_ssl_get_record_expansion(&tlsData->ssl);
      if (exp >= 0)
      {
        NET_DBG_INFO("    [ Record expansion is %d ]\n", exp);
      }
      else
      {
        NET_DBG_INFO("    [ Record expansion is unknown (compression) ]\n");
      }

      NET_DBG_INFO("  . Verifying peer X.509 certificate...");


#ifdef NET_DBG_INFO
#define NET_CERTIFICATE_DISPLAY_LEN     2048U
      if (mbedtls_ssl_get_peer_cert(&sock->tlsData->ssl) != NULL)
      {
        /*cstat -MISRAC2012-Rule-11.5 -MISRAC2012-Rule-21.3 -MISRAC2012-Dir-4.12 */
        char_t        *buf = NET_MALLOC(sizeof(char_t) * NET_CERTIFICATE_DISPLAY_LEN);
        /*cstat +MISRAC2012-Rule-11.5 +MISRAC2012-Rule-21.3 +MISRAC2012-Dir-4.12 */

        if (buf != NULL)
        {
          NET_DBG_INFO("  . Peer certificate information    ...\n");
          (void) mbedtls_x509_crt_info(buf, NET_CERTIFICATE_DISPLAY_LEN - 1U, "      ",
                                       mbedtls_ssl_get_peer_cert(&sock->tlsData->ssl));
          NET_DBG_INFO("%s\n", buf);
          /*cstat -MISRAC2012-Rule-21.3 */
          NET_FREE(buf);
          /*cstat +MISRAC2012-Rule-21.3 */

        }
        else
        {
          NET_DBG_INFO("  . Cannot allocate memory to display certificate information    ...\n");
        }
      }
#endif /* NET_DBG_INFO */
    }
  }
  return ret;

}


int32_t net_mbedtls_sock_recv(net_socket_t *sock, uint8_t *buf, size_t len)
{
  net_tls_data_t *tlsData = sock->tlsData;
  int32_t ret;

  ret = mbedtls_ssl_read(&tlsData->ssl, buf, len);
  if (ret <= 0)
  {
    switch (ret)
    {
      case 0:
        ret = NET_ERROR_DISCONNECTED;
        break;

      case MBEDTLS_ERR_SSL_TIMEOUT: /* In case a blocking read function was passed through mbedtls_ssl_set_bio() */
        ret = NET_TIMEOUT;
        break;

      case MBEDTLS_ERR_SSL_WANT_READ:
        ret = NET_TIMEOUT;
        break;

      default:
        NET_DBG_ERROR(" failed\n  ! mbedtls_ssl_read returned -0x%lx\n\n", -ret);
        ret = NET_ERROR_MBEDTLS;
        break;
    }
  }
  return ret;
}


int32_t net_mbedtls_sock_send(net_socket_t *sock, const uint8_t *buf, size_t len)
{
  int32_t ret;
  net_tls_data_t *tlsData = sock->tlsData;
#if PERF
  stat.mbedtls_send_cycle -= net_get_cycle();
#endif /* PERF */

  ret = mbedtls_ssl_write(&tlsData->ssl, buf, len);
  if (ret == 0)
  {
    ret = NET_ERROR_DISCONNECTED;
  }
  else if (ret < 0)
  {
    NET_DBG_ERROR(" failed\n  ! mbedtls_ssl_write returned -0x%lx\n\n", -ret);
    ret = NET_ERROR_MBEDTLS;
  }
  else
  {
    /* ret > 0 nothing to do , MISRA checks */
  }
#if PERF
  stat.mbedtls_send_cycle += net_get_cycle();
#endif /* PERF */

  return (ret);
}


int32_t net_mbedtls_stop(net_socket_t *sock)
{
  int32_t ret;
  /* Closure notification is required by TLS if the session was not already closed by the remote host. */
  net_tls_data_t *tlsData = sock->tlsData;
  do
  {
    ret = mbedtls_ssl_close_notify(&tlsData->ssl);
  } while ((ret == MBEDTLS_ERR_SSL_WANT_WRITE) || (ret == MBEDTLS_ERR_SSL_WANT_READ));

  /* All other negative return values indicate connection needs to be reset.
   * No further action required since this is disconnect call */

  mbedtls_free_resource(sock);
  return NET_OK;
}


static void mbedtls_free_resource(net_socket_t *sock)
{
  net_tls_data_t *tlsData = sock->tlsData;

  mbedtls_x509_crt_free(&tlsData->clicert);
  mbedtls_pk_free(&tlsData->pkey);
  mbedtls_x509_crt_free(&tlsData->cacert);
  mbedtls_ssl_free(&tlsData->ssl);
  mbedtls_ssl_config_free(&tlsData->conf);
  /*cstat -MISRAC2012-Rule-21.3 */
  NET_FREE(tlsData);
  /*cstat +MISRAC2012-Rule-21.3 */
  sock->tlsData = 0;
  return;
}



/* received interface implementation.*/
static int32_t mbedtls_net_recv(void *ctx, uchar_t *buf, size_t len, uint32_t timeout)
{
  int32_t ret =  NET_OK;
  int32_t flags = 0;
  /*cstat -MISRAC2012-Rule-11.5 */
  net_socket_t  *pSocket = (net_socket_t *) ctx;
  /*cstat +MISRAC2012-Rule-11.5 */

  if (pSocket->read_timeout != (int32_t)timeout)
  {
    ret = pSocket->pnetif->pdrv->psetsockopt(pSocket->ulsocket,
                                             NET_SOL_SOCKET, NET_SO_RCVTIMEO, &timeout, sizeof(uint32_t));
    if (ret == NET_OK)
    {
      pSocket->read_timeout = (int32_t) timeout;
    }
  }

  if (ret == NET_OK)
  {
    if (pSocket->read_timeout == 0)
    {
      flags = (int8_t) NET_MSG_DONTWAIT;
    }
    UNLOCK_SOCK(pSocket->idx);
    ret = pSocket->pnetif->pdrv->precv(pSocket->ulsocket, buf, len, flags);
    LOCK_SOCK(pSocket->idx);

    if (ret <= 0)
    {
      switch (ret)
      {
        case 0:
          ret = MBEDTLS_ERR_SSL_WANT_READ;
          break;

        case NET_TIMEOUT:
          /* According to mbedtls headers, MBEDTLS_ERR_SSL_TIMEOUT should be returned. */
          /* But it saturates the error log with false errors. By contrast,     */
          /*    MBEDTLS_ERR_SSL_WANT_READ does not raise any error. */
          ret =  MBEDTLS_ERR_SSL_WANT_READ;
          break;

        default:
          NET_DBG_ERROR("mbedtls_net_recv() : error %ld in recv() - requestedLen=%d\n", ret, len);
          ret =    MBEDTLS_ERR_SSL_INTERNAL_ERROR;
          break;
      }
    }
  }

  return ret;
}


static int32_t mbedtls_net_send(void *ctx, const uchar_t *buf, size_t len)
{
  /*cstat -MISRAC2012-Rule-11.5 */
  net_socket_t  *pSocket = (net_socket_t *) ctx;
  /*cstat +MISRAC2012-Rule-11.5 */
  int32_t ret;
  int32_t flags = 0;

  if (pSocket->write_timeout == 0)
  {
    flags = (int8_t) NET_MSG_DONTWAIT;
  }
  UNLOCK_SOCK(pSocket->idx);
  /*cstat -MISRAC2012-Rule-11.8 removing const attribute */
  ret = pSocket->pnetif->pdrv->psend(pSocket->ulsocket, (uchar_t *) buf, len, flags);
  /*cstat +MISRAC2012-Rule-11.8 */
  LOCK_SOCK(pSocket->idx);


  if (ret >= 0)
  {
    if (ret == 0)
    {
      ret = MBEDTLS_ERR_SSL_WANT_WRITE;
    }
  }
  else
  {
    NET_DBG_ERROR("mbedtls_net_send(): error %ld in send() - requestedLen=%d\n", ret, len);

    /* TODO: The underlying layers do not allow to distinguish between
    *          MBEDTLS_ERR_SSL_INTERNAL_ERROR,
    *          MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY,
    *          MBEDTLS_ERR_SSL_CONN_EOF.
    *  Most often, the error is due to the closure of the connection by the remote host. */

    ret =  MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY;
  }
  return ret;
}

#endif /* NET_MBEDTLS_HOST_SUPPORT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
