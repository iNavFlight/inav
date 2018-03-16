/*
    ChibiOS - (C) 2015-2016 flabbergast <s3+flabbergast@sdfeu.org>

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

#define KL2x_MCUCONF

/*
 * HAL driver system settings.
 */
#if 1
/* High-frequency internal RC, 48MHz, possible USB clock recovery */
#define KINETIS_MCGLITE_MODE        KINETIS_MCGLITE_MODE_HIRC
#define KINETIS_SYSCLK_FREQUENCY    48000000UL
#define KINETIS_CLKDIV1_OUTDIV1     1
#endif

#if 0
/* Low-frequency internal RC, 8 MHz mode */
#define KINETIS_MCGLITE_MODE        KINETIS_MCGLITE_MODE_LIRC8M
#define KINETIS_SYSCLK_FREQUENCY    8000000UL
#define KINETIS_CLKDIV1_OUTDIV1     1
#endif

/*
 * SERIAL driver system settings.
 */
#define KINETIS_SERIAL_USE_UART0              TRUE

/*
 * USB driver settings
 */
#define KINETIS_USB_USE_USB0                  TRUE
/* need to redefine this, since the default is for K20x */
#define KINETIS_USB_USB0_IRQ_PRIORITY         2

/*
 * Kinetis FOPT configuration byte
 */
/* for KL27: */
// #define KINETIS_NV_FOPT_BYTE 0x3D
/* NV_FOPT: bit7-6/BOOTSRC_SEL=0b00 (11=from ROM; 00=from FLASH)
   bit5/FAST_INIT=1, bit4/LPBOOT1=1,
   bit3/RESET_PIN_CFG=1, bit2/NMI_DIS=1,
   bit1/BOOTPIN_OPT=0, bit0/LPBOOT0=1 */
/* BOOTPIN_OPT: 1=boot depends on BOOTSRC_SEL
   0=boot samples BOOTCFG0=NMI pin */
/* Boot sequence, page 88 of manual:
 * - If the NMI/BOOTCFG0 input is high or the NMI function is disabled in FTFA_FOPT, the CPU begins execution at the PC location.
 * - If the NMI/BOOTCFG0 input is low, the NMI function is enabled in FTFA_FOPT, and FTFA_FOPT[BOOTPIN_OPT] = 1, this results in an NMI interrupt. The processor executes an Exception Entry and reads the NMI interrupt handler address from vector-table offset 8. The CPU begins execution at the NMI interrupt handler.
 * - When FTFA_FOPT[BOOTPIN_OPT] = 0, it forces boot from ROM if NMI/BOOTCFG0 pin set to 0.
 *
 * Observed behaviour:
 * - when BOOTPIN_OPT=0, BOOTSRC_SEL still matters:
 *   - if 0b11 (from ROM), it still boots from ROM, even if BOOTCFG0 pin
 *     is high/floating, but leaves ROM and runs user app after
 *     5 seconds delay.
 *   - if 0b00 (from FLASH), reset/powerup jumps to user app unless
 *     BOOTCFG0 pin is asserted.
 * - in any case, reset when in bootloader induces the 5 second delay
 *   before starting the user app.
 * 
 */

#endif /* _MCUCONF_H_ */
