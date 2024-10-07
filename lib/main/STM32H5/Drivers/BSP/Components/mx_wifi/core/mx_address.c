/**
  ******************************************************************************
  * @file    mx_address.c
  * @author  MCD Application Team
  * @brief   Implements network address conversion routines
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "mx_address.h"
#include "mx_wifi.h"

#include <stdlib.h>
#include <stdbool.h>

#define BYTE_IN_RANGE(C, LO, UP)  (((C) >= (LO)) && ((C) <= (UP)))
#define BYTE_IS_PRINT(C)          BYTE_IN_RANGE((C), 0x20, 0x7f)
#define BYTE_ISDIGIT(C)           BYTE_IN_RANGE((C), '0', '9')
#define BYTE_ISXDIGIT(C)          (BYTE_ISDIGIT(C) || BYTE_IN_RANGE((C), 'a', 'f') || BYTE_IN_RANGE((C), 'A', 'F'))
#define BYTE_ISLOWER(C)           BYTE_IN_RANGE((C), 'a', 'z')
#define BYTE_ISSPACE(C) \
  (((C) == ' ') || ((C) == '\f') || ((C) == '\n') || ((C) == '\r') || ((C) == '\t') || ((C) == '\v'))

#define MX_NET_HTONL(A)   ((((uint32_t)(A) & 0xff000000U) >> 24) | \
                           (((uint32_t)(A) & 0x00ff0000U) >>  8) | \
                           (((uint32_t)(A) & 0x0000ff00U) <<  8) | \
                           (((uint32_t)(A) & 0x000000ffU) << 24))

#if !defined(MX_USE_LWIP_DEFINITIONS)
static int32_t mx_aton(const mx_char_t *ptr, mx_ip_addr_t *addr);


/**
  * @brief  Convert IP address from string to int32
  * @param  ptr: IP string buffer
  * @param  addr: IP address structure
  * @retval status 1 success, otherwise failed
  */
static int32_t mx_aton(const mx_char_t *ptr, mx_ip_addr_t *addr)
{
  uint32_t val = 0;
  uint32_t base;
  mx_char_t c0;
  const mx_char_t *cp = ptr;
  uint32_t parts[4];
  uint32_t *pp = parts;
  int32_t ret = 1;
  int32_t done = 0;

  c0 = *cp;

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

    if (!BYTE_ISDIGIT(c0))
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
        c0 = (mx_char_t) * cp;
        if ((c0 == (mx_char_t)'x') || (c0 == (mx_char_t)'X'))
        {
          base = 16;
          ++cp;
          c0 = (mx_char_t) * cp;
        }
        else
        {
          base = 8;
        }
      }

      for (;;)
      {
        if (BYTE_ISDIGIT(c0))
        {
          val = (val * base) + (uint32_t)c0 - (uint32_t)'0';
          ++cp;
          c0 = (mx_char_t) * cp;
        }
        else if ((base == 16U) && BYTE_ISXDIGIT(c0))
        {
          val = (val << 4) | ((uint32_t)c0 + 10U - (BYTE_ISLOWER(c0) ? (uint32_t)'a' : (uint32_t)'A'));
          ++cp;
          c0 = (mx_char_t) * cp;
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
          c0 = (mx_char_t) * cp;
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
  if ((c0 != (mx_char_t)'\0') && (BYTE_ISSPACE((c0)) == false))
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
        ret = 0;          /* initial non digit */
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
      addr->addr = MX_NET_HTONL(val);
    }
  }
  return ret;
}


int32_t mx_aton_r(const mx_char_t *cp)
{
  mx_ip_addr_t val = {0};
  int32_t ret = 0;

  if (mx_aton(cp, &val) != 0)
  {
    ret = (int32_t) val.addr;
  }

  return ret;
}


mx_char_t *mx_ntoa(const mx_ip_addr_t *addr)
{
  int32_t len = 0;
  static mx_char_t buf[MX_MAX_IP_LEN];
  const int32_t buf_size = (int32_t)sizeof(buf);

  const uint32_t ip_addr = addr->addr;
  const uint8_t *const ap = (const uint8_t *)&ip_addr;

  for (uint8_t n = 0; n < (uint8_t) 4; n++)
  {
    mx_char_t inv[3];
    uint8_t i = 0;
    uint8_t val = ap[n];
    do
    {
      const uint8_t rem = val % 10U;
      val /=  10U;
      inv[i] = (mx_char_t)'0' + rem;
      i++;
    } while (val != 0U);

    while (i != 0U)
    {
      i--;
      if (len < buf_size)
      {
        buf[len] = inv[i];
        len++;
      }
    }

    if ((n < 3U) && (len < buf_size))
    {
      buf[len] = (mx_char_t) '.';
      len++;
    }
  }

  MX_ASSERT(len < buf_size);

  buf[len] = (mx_char_t) '\0';

  return buf;
}

#endif /* MX_USE_LWIP_DEFINITIONS */
