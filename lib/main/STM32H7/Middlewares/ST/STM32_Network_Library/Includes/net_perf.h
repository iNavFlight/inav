/**
  ******************************************************************************
  * @file    net_perf.h
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
#ifndef NET_PERF_H
#define NET_PERF_H

/* disable Misra rule to enable doxigen comment , A sectio of code appear to have been commented out */
/*cstat -MISRAC2012-Dir-4.4 */

#ifdef __cplusplus
extern "C" {
#endif

#include "net_conf.h"
#include "net_types.h"


#ifdef NET_USE_RTOS

#if (osCMSIS < 0x20000U)
#define NET_TICK   osKernelSysTick
#else
#define NET_TICK   osKernelGetTickCount
#endif /* osCMSIS */

#else
#define NET_TICK        HAL_GetTick

#endif /* NET_USE_RTOS */



/*cstat -MISRAC2012-Rule-21.1 */
#ifndef __IO
#define __IO volatile
#endif /* __IO */
/*cstat +MISRAC2012-Rule-21.1 */

#define NET_DWT_CONTROL             (*((__IO uint32_t*)0xE0001000U))
#define NET_DWT_CYCCNTENA_BIT       (1UL<<0U)
/*!< DWT Cycle Counter register */
#define NET_DWT_CYCCNT              (*((__IO uint32_t*)0xE0001004U))
/*!< DEMCR: Debug Exception and Monitor Control Register */
#define NET_DEMCR                   (*((__IO uint32_t*)0xE000EDFCU))
/*!< Trace enable bit in DEMCR register */
#define NET_TRCENA_BIT              (1UL<<24U)


static inline uint32_t                     net_get_cycle(void)
{
  /*cstat -MISRAC2012-Rule-11.4 */
  return NET_DWT_CYCCNT;
  /*cstat +MISRAC2012-Rule-11.4 */
}

static inline void                         net_stop_cycle(void)
{
  /*cstat -MISRAC2012-Rule-11.4 */
  NET_DWT_CONTROL &= ~NET_DWT_CYCCNTENA_BIT ;
  /*cstat +MISRAC2012-Rule-11.4 */
}

static inline void                         net_start_cycle(void)
{
  /*cstat -MISRAC2012-Rule-11.4 */
  NET_DWT_CONTROL |= NET_DWT_CYCCNTENA_BIT ;
  /*cstat +MISRAC2012-Rule-11.4 */
}

void            net_perf_start(void);
void            net_perf_report(void);

#ifdef NET_USE_RTOS

#if defined(NET_PERF_TASK) && !defined(NET_FREERTOS_PERF)
#warning "To use NET_PERF_TASK please add followings lines to FreeRTOSConfig.h"
#warning  "           void net_perf_task_in(void);"
#warning  "           void net_perf_task_out(void);"
#warning  "           #define NET_FREERTOS_PERF"
#warning  "           #define traceTASK_SWITCHED_IN net_perf_task_in"
#warning  "           #define traceTASK_SWITCHED_OUT net_perf_task_out"
#endif /* NET_PERF_TASK */


#endif /* NET_USE_RTOS */



#ifdef __cplusplus
}
#endif

#endif /* NET_PERF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
