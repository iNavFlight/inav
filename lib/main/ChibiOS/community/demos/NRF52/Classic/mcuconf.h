/*
    Copyright (C) 2016 Stephane D'Alu

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef _MCUCONF_H_
#define _MCUCONF_H_

/*
 * Board setting
 */


#define NRF5_SOFTDEVICE_THREAD_WA_SIZE 128

#define SHELL_CMD_TEST_ENABLED FALSE
#define SHELL_CMD_ECHO_ENABLED FALSE
#define SHELL_CMD_INFO_ENABLED FALSE




#define NRF5_SOFTDEVICE_LFCLK_SOURCE   NRF_CLOCK_LF_SRC_XTAL
#define NRF5_SOFTDEVICE_LFCLK_ACCURACY NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM


/*
 * HAL driver system settings.
 */
#define NRF5_SERIAL_USE_UART0             TRUE
#define NRF5_SERIAL_USE_HWFLOWCTRL	   TRUE
#define NRF5_RNG_USE_RNG0 		   TRUE
#define NRF5_GPT_USE_TIMER0 		   TRUE

#define NRF5_QEI_USE_QDEC0 TRUE
#define NRF5_QEI_USE_LED   FALSE

#define WDG_USE_TIMEOUT_CALLBACK    TRUE


#endif /* _MCUCONF_H_ */
