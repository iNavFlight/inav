 /**
  *
  * Portions COPYRIGHT 2018 STMicroelectronics, All Rights Reserved
  * Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
  *
  ******************************************************************************
  * @file    timing_alt_template.c
  * @author  MCD Application Team
  * @brief   mbedtls alternate timing functions implementation.
  *          mbedtls timing API is implemented using the CMSIS-RTOS v1/v2 API
  *          this file has to be renamed to timing_alt.c and copied under
  *          the project tree.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Apache 2.0 license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  * https://opensource.org/licenses/Apache-2.0
  *
  ******************************************************************************
  */

#include "timing_alt.h"

#if defined(MBEDTLS_TIMING_ALT)

/* include the appropriate header file */
#include "stm32<xxxxx>_hal.h"
#include "cmsis_os.h"

struct _hr_time
{
    uint32_t elapsed_time;
};
volatile int mbedtls_timing_alarmed = 0;

static uint8_t timer_created = 0;
#if (osCMSIS < 0x20000)
static osTimerId timer;
#else
static osTimerId_t timer;
#endif
static void osTimerCallback(void const *argument)
{
  UNUSED(argument);
  mbedtls_timing_alarmed = 1;
  osTimerStop(timer);
}

unsigned long mbedtls_timing_hardclock( void )
{
   /* retrieve the CPU cycles using the Cortex-M DWT->CYCCNT register
    * avaialable only starting from CM3
    */

#if (__CORTEX_M >= 0x03U)
    static int dwt_started = 0;
    if( dwt_started == 0 )
    {
      dwt_started = 1;
      /* Enable Tracing */
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
#if (__CORTEX_M == 0x07U)
        /* in Cortex M7, the trace needs to be unlocked
         * via the DWT->LAR register with 0xC5ACCE55 value
         */
        DWT->LAR = 0xC5ACCE55;
#endif
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

      /* Reset counter */
      DWT->CYCCNT = 0;
    }

    return (unsigned long)DWT->CYCCNT;
#else
    return 0;
#endif
}

unsigned long mbedtls_timing_get_timer( struct mbedtls_timing_hr_time *val, int reset )
{
  unsigned long delta;
  uint32_t offset;
  struct _hr_time *t = (struct _hr_time *) val;

  offset = osKernelSysTick();

  if( reset )
  {
    t->elapsed_time = offset;
    return( 0 );
  }

  delta = offset - t->elapsed_time;
  return( delta );

}

void mbedtls_set_alarm( int seconds )
{
  if (timer_created == 0)
  {
#if (osCMSIS < 0x20000)
    osTimerDef(Timer, osTimerCallback);
    timer = osTimerCreate(osTimer(Timer), osTimerOnce, NULL);
#else
    timer = osTimerNew((osTimerFunc_t)osTimerCallback, osTimerOnce, NULL, NULL);
#endif
    timer_created = 1;
  }

  mbedtls_timing_alarmed = 0;
  osTimerStart(timer, seconds * 1000);
}

void mbedtls_timing_set_delay( void *data, uint32_t int_ms, uint32_t fin_ms )
{
  mbedtls_timing_delay_context *ctx = (mbedtls_timing_delay_context *) data;

  ctx->int_ms = int_ms;
  ctx->fin_ms = fin_ms;

  if( fin_ms != 0 )
    mbedtls_timing_get_timer( &ctx->timer, 1 );
}

int mbedtls_timing_get_delay( void *data )
{
  mbedtls_timing_delay_context *ctx = (mbedtls_timing_delay_context *) data;
  unsigned long elapsed_ms;

  if( ctx->fin_ms == 0 )
    return( -1 );

  elapsed_ms = mbedtls_timing_get_timer( &ctx->timer, 0 );

  if( elapsed_ms >= ctx->fin_ms )
    return( 2 );

  if( elapsed_ms >= ctx->int_ms )
    return( 1 );

  return( 0 );
}
#endif /* MBEDTLS_TIMING_ALT */
