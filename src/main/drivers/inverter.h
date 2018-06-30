/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#if defined(INVERTER_PIN_UART1) || defined(INVERTER_PIN_UART2) || defined(INVERTER_PIN_UART3) || defined(INVERTER_PIN_UART4) || defined(INVERTER_PIN_UART5) || defined(INVERTER_PIN_UART6) || \
    defined(INVERTER_PIN_UART1_RX) || defined(INVERTER_PIN_UART2_RX) || defined(INVERTER_PIN_UART3_RX) || defined(INVERTER_PIN_UART4_RX) || defined(INVERTER_PIN_UART5_RX) || defined(INVERTER_PIN_UART6_RX) || \
    defined(INVERTER_PIN_UART1_TX) || defined(INVERTER_PIN_UART2_TX) || defined(INVERTER_PIN_UART3_TX) || defined(INVERTER_PIN_UART4_TX) || defined(INVERTER_PIN_UART5_TX) || defined(INVERTER_PIN_UART6_TX)
#define USE_INVERTER
#endif

void initInverters(void);

void enableInverter(USART_TypeDef *USARTx, bool on);
