/**
  ******************************************************************************
  * @file    net_address.c
  * @author  MCD Application Team
  * @brief   Implements network address conversion routines
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

#define NET_IN_RANGE(c, lo, up)  (((c) >= (lo)) && ((c) <= (up)))
#define NET_IS_PRINT(c)          (NET_IN_RANGE(c, 0x20, 0x7f))
#define NET_ISDIGIT(c)           (NET_IN_RANGE(c, '0', '9'))
#define NET_ISXDIGIT(c)          (NET_ISDIGIT(c) || NET_IN_RANGE(c, 'a', 'f') || NET_IN_RANGE(c, 'A', 'F'))
#define NET_ISLOWER(c)           (NET_IN_RANGE(c, 'a', 'z'))
#define NET_ISSPACE(c)           (((c) == ' ')\
                                  || ((c) == '\f') || ((c) == '\n') || ((c) == '\r') || ((c) == '\t') || ((c) == '\v'))
/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
#if !defined(NET_USE_LWIP_DEFINITIONS)
char_t *net_ntoa_r(const net_ip_addr_t *addr, char_t *buf, int32_t buflen)
{
  uint32_t NET_S_ADDR;
  uint8_t     val;
  char_t inv[3];
  uint8_t *ap;
  uint8_t rem;
  uint8_t i;
  int32_t len = 0;
  char_t *buf_ret;

  NET_S_ADDR = addr->addr;

  ap = (uint8_t *)&NET_S_ADDR;
  for (uint8_t n = 0; n < (uint8_t) 4; n++)
  {
    i = 0;
    val = ap[n];
    do
    {
      rem = val % 10U;
      val /=  10U;
      inv[i] = (char_t)'0' + rem;
      i++;
    } while (val != 0U);

    while (i != 0U)
    {
      i--;
      if (len < buflen)
      {
        buf[len] = inv[i];
        len++;
      }
    }

    if ((n < 3U) && (len < buflen))
    {
      buf[len] = (char_t) '.';
      len++;
    }
  }

  if (len < buflen)
  {
    buf[len] = (char_t) '\0';
    buf_ret = buf;

  }
  else
  {
    buf_ret = NULL;
  }

  return buf_ret;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
char_t *net_ntoa(const net_ip_addr_t *addr)
{
  static char_t str[16];
  return net_ntoa_r(addr, str, 16);
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
int32_t net_aton(const char_t *ptr, net_ip_addr_t *addr)
{
  uint32_t val = 0;
  uint32_t base;
  char_t c0;
  const char_t      *cp =  ptr;
  uint32_t parts[4];
  uint32_t *pp = parts;
  int32_t ret = 1;
  int32_t done;

  c0 = *cp;
  done = 0;
  for (;;)
  {
    /*
     * Collect number up to ``.''.
     * Values are specified as for C:
     * 0x=hex, 0=octal, 1-9=decimal.
     */
    if (done == 1)
    {
      break;
    }

    if (!NET_ISDIGIT(c0))
    {
      ret = 0;
      done = 1;
    }
    else
    {
      val = 0;
      base = 10;
      if (c0 == '0')
      {
        ++cp;
        c0 = (char_t) * cp;
        if ((c0 == (char_t) 'x') || (c0 == (char_t) 'X'))
        {
          base = 16;
          ++cp;
          c0 = (char_t) * cp;
        }
        else
        {
          base = 8;
        }
      }

      for (;;)
      {
        if (NET_ISDIGIT(c0))
        {
          val = (val * base) + (uint32_t)c0 - (uint32_t) '0';
          ++cp;
          c0 = (char_t) * cp;
        }
        else if ((base == 16U) && NET_ISXDIGIT(c0))
        {
          val = (val << 4) | ((uint32_t)c0 + 10U - (uint32_t)(NET_ISLOWER(c0) ? 'a' : 'A'));
          ++cp;
          c0 = (char_t) * cp;
        }
        else
        {
          break;
        }
      }

      if (c0 == '.')
      {
        /*
         * Internet format:
         *  a.b.c.d
         *  a.b.c   (with c treated as 16 bits)
         *  a.b (with b treated as 24 bits)
         */
        if (pp >= (parts + 3))
        {
          ret = 0;
          done = 1;
        }
        else
        {
          *pp = val;
          pp++;
          ++cp;
          c0 = (char_t) * cp;
        }
      }
      else
      {
        done = 1;
      }
    }
  }
  /*
   * Check for trailing characters.
   */
  if ((c0 != (char_t)'\0') && (NET_ISSPACE((c0)) == false))
  {
    ret = 0;
  }
  else

    /*
     * Concoct the address according to
     * the number of parts specified.
     */
  {
    switch (pp - parts + 1)
    {

      case 0:
        ret = 0;      /* initial nondigit */
        break;

      case 1:             /* a -- 32 bits */
        break;

      case 2:             /* a.b -- 8.24 bits */
        if (val > 0xffffffUL)
        {
          ret = 0;
        }
        val |= parts[0] << 24;
        break;

      case 3:             /* a.b.c -- 8.8.16 bits */
        if (val > 0xffffU)
        {
          ret = 0;
          break;
        }
        val |= (parts[0] << 24) | (parts[1] << 16);
        break;

      case 4:             /* a.b.c.d -- 8.8.8.8 bits */
        if (val > 0xffU)
        {
          ret = 0;
          break;
        }
        val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
        break;
      default:
        ret = 0;
        break;
    }
  }

  if (ret == 1)
  {
    if (addr != NULL)
    {
      addr->addr = NET_HTONL(val);
    }
  }
  return ret;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
int32_t net_aton_r(const char_t *cp)
{
  net_ip_addr_t val;
  int32_t       ret;
  val.addr = 0;
  if (net_aton(cp, &val) != 0)
  {
    ret = (int32_t) val.addr;
  }
  else
  {
    ret = 0;
  }
  return (ret);
}

#endif /* NET_USE_LWIP_DEFINITIONS */


uint16_t    net_get_port(net_sockaddr_t      *addr)
{
  /*cstat -MISRAC2012-Rule-11.3 -MISRAC2012-Rule-11.8 */
  return (NET_NTOHS(((net_sockaddr_in_t *)addr)->sin_port));
  /*cstat +MISRAC2012-Rule-11.3 +MISRAC2012-Rule-11.8 +MISRAC2012-Rule-10.8 Cast */
}
void    net_set_port(net_sockaddr_t      *addr, uint16_t port)
{
  /*cstat -MISRAC2012-Rule-11.3 Cast */
  ((net_sockaddr_in_t *)addr)->sin_port        = NET_HTONS(port);
  /*cstat +MISRAC2012-Rule-11.3 Cast */
}

net_ip_addr_t    net_get_ip_addr(net_sockaddr_t      *addr)
{
  net_ip_addr_t ipaddr;
  uint32_t              addrv;
  /*cstat -MISRAC2012-Rule-11.3 Cast */
  addrv = ((net_sockaddr_in_t *)addr)->sin_addr.s_addr;
  /*cstat +MISRAC2012-Rule-11.3 Cast */
  NET_COPY(ipaddr, addrv);
  return ipaddr;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
