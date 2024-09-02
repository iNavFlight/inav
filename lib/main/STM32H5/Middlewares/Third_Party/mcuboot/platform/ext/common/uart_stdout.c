/*
 * Copyright (c) 2017-2020 ARM Limited
 * Copyright (c) 2023 STMicroelectronics
 *
 * Licensed under the Apace License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apace.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "uart_stdout.h"

#include <assert.h>
#include <stdio.h>
#include "Driver_USART.h"
#include "target_cfg.h"


#define ASSERT_HIGH(X)  assert(X == ARM_DRIVER_OK)

/* Imports USART driver */
#if DOMAIN_NS == 1U
extern ARM_DRIVER_USART NS_DRIVER_STDIO;
#define STDIO_DRIVER    NS_DRIVER_STDIO
#else
extern ARM_DRIVER_USART DRIVER_STDIO;
#define STDIO_DRIVER    DRIVER_STDIO
#endif

int stdio_output_string(const unsigned char *str, uint32_t len)
{
    int32_t ret;

    ret = STDIO_DRIVER.Send(str, len);
    if (ret != ARM_DRIVER_OK) {
        return 0;
    }
    /* Add a busy wait after sending. */
    while (STDIO_DRIVER.GetStatus().tx_busy);

    return STDIO_DRIVER.GetTxCount();
}

/* Redirects printf to STDIO_DRIVER in case of ARMCLANG*/
#if defined(__ARMCC_VERSION)
/* Struct FILE is implemented in stdio.h. Used to redirect printf to
 * STDIO_DRIVER
 */
FILE __stdout;
/* __ARMCC_VERSION is only defined starting from Arm compiler version 6 */
int fputc(int ch, FILE *f)
{
    (void)f;

    /* Send byte to USART */
    (void)stdio_output_string((const unsigned char *)&ch, 1);

    /* Return character written */
    return ch;
}
#elif defined(__GNUC__)
/* Redirects printf to STDIO_DRIVER in case of GNUARM */
int _write(int fd, char *str, int len)
{
    (void)fd;

    /* Send string and return the number of characters written */
    return stdio_output_string((const unsigned char *)str, (uint32_t)len);
}
#elif defined(__ICCARM__)
int putchar(int ch)
{
    /* Send byte to USART */
    (void)stdio_output_string((const unsigned char *)&ch, 1);

    /* Return character written */
    return ch;
}
#endif

void stdio_init(void)
{
    int32_t ret;
    ret = STDIO_DRIVER.Initialize(NULL);
    ASSERT_HIGH(ret);

    ret = STDIO_DRIVER.PowerControl(ARM_POWER_FULL);
    ASSERT_HIGH(ret);

    ret = STDIO_DRIVER.Control(ARM_USART_MODE_ASYNCHRONOUS,
                               DEFAULT_UART_BAUDRATE);
    ASSERT_HIGH(ret);
    (void)ret;

    (void)STDIO_DRIVER.Control(ARM_USART_CONTROL_TX, 1);
}

void stdio_uninit(void)
{
    int32_t ret;

    (void)STDIO_DRIVER.PowerControl(ARM_POWER_OFF);

    ret = STDIO_DRIVER.Uninitialize();
    ASSERT_HIGH(ret);
    (void)ret;
}
