/**
  ******************************************************************************
  * @file    net_mem.h
  * @author  MCD Application Team
  * @brief   Memory allocator functions
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef NET_MEM_H
#define NET_MEM_H
#include "net_conf.h"
#include "net_types.h"
/* disable Misra rule to enable doxigen comment , A sectio of code appear to have been commented out */
/*cstat -MISRAC2012-Dir-4.4 */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NET_ALLOC_DEBUG

#if !defined NET_ALLOC_MAX_NODE
#define NET_ALLOC_MAX_NODE                              50U
#endif /* NET_ALLOC_MAX_NODE */

#if !defined NET_DISPLAY_TAB
#define NET_DISPLAY_TAB                                 4U
#endif /* NET_DISPLAY_TAB */

#if !defined NET_DISPLAY_WIDTH
#define NET_DISPLAY_WIDTH                               120U
#endif /* NET_DISPLAY_WIDTH */

#if !defined NET_DISPLAY_RESULT_WIDTH
#define NET_DISPLAY_RESULT_WIDTH                        8U
#endif /* NET_DISPLAY_RESULT_WIDTH */

#if !defined NET_DISPLAY_DIRNAME_LEN
#define NET_DISPLAY_DIRNAME_LEN                         30U
#endif /* NET_DISPLAY_DIRNAME_LEN */

#if !defined NET_ALLOC_VERBOSE
#define NET_ALLOC_VERBOSE 0
#endif /*   NET_ALLOC_VERBOSE */

#ifndef NET_LEAKAGE_ARRAY
#define NET_LEAKAGE_ARRAY 300U
#endif /* NET_LEAKAGE_ARRAY */

#ifndef NET_ALLOC_BREAK
#define NET_ALLOC_BREAK 0xFFFFFFFFU
#endif /* NET_ALLOC_BREAK */


#define NET_CALLOC(a,b)  net_calloc_debug(a,b,__FILE__,__LINE__)
#define NET_REALLOC(a,b) net_realloc_debug(a,b,__FILE__,__LINE__)
#define NET_MALLOC(a)    net_malloc_debug(a,__FILE__,__LINE__)
#define NET_FREE(a)      net_free_debug(a)


void *net_calloc_debug(size_t m, size_t n, const char *s, uint32_t line);
void *net_malloc_debug(size_t m, const char *s, uint32_t line);
void *net_realloc_debug(void *p, size_t size, const char_t *filename, uint32_t line);
void  net_free_debug(void *p);

void net_alloc_report(void);


#else /* !NET_ALLOC_DEBUG */

#ifdef NET_USE_RTOS
#define NET_CALLOC  net_calloc
#define NET_REALLOC net_realloc
#define NET_MALLOC  pvPortMalloc
#define NET_FREE    vPortFree

void    *net_calloc(size_t n, size_t m);
void    *net_realloc(void *p, size_t m);

#else
#define NET_CALLOC  calloc
#define NET_REALLOC realloc
#define NET_MALLOC  malloc
#define NET_FREE    free
#endif /* NET_USE_RTOS */


#endif /* NET_ALLOC_DEBUG */



#ifdef __cplusplus
}
#endif

#endif /* NET_MEM_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
