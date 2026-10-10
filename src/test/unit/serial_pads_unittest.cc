/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <string.h>

extern "C" {
#include "platform.h"
#include "common/streambuf.h"
#include "config/feature.h"
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/serial.h"
#include "drivers/serial_uart.h"
#include "drivers/timer.h"
#include "fc/config.h"
#include "io/serial.h"
#include "io/serial_pads.h"
}

#include "gtest/gtest.h"
#include "unittest_macros.h"

// A board with an input first, so pads and timerHardware[] indexes differ: PB0 is S1 and carries motor 1
static const serialPortIdentifier_e UART2 = SERIAL_PORT_USART2;
static const serialPortIdentifier_e UART3 = SERIAL_PORT_USART3;
static const serialPortIdentifier_e UART4 = SERIAL_PORT_USART4;

struct altPin {
    int uart;
    bool tx;
    ioTag_t pin;
};

static const altPin altPins[] = {
    { UART2, true,  IO_TAG(PA2) },
    { UART2, false, IO_TAG(PA0) },
    { UART3, true,  IO_TAG(PB0) },
    { UART4, true,  IO_TAG(PA0) },
    { UART4, true,  IO_TAG(PC10) },
    { UART4, false, IO_TAG(PA1) },
};

static ioTag_t uartPins[SERIAL_PAD_UART_COUNT][SERIAL_PAD_DIRECTION_COUNT];
static bool portUsed[SERIAL_PAD_UART_COUNT];
static uint32_t features;

extern "C" {
timerHardware_t timerHardware[6];
const int timerHardwareCount = 6;

void uartGetPortPins(UARTDevice_e device, serialPortPins_t *pins)
{
    pins->txPin = uartPins[device][SERIAL_PAD_TX];
    pins->rxPin = uartPins[device][SERIAL_PAD_RX];
}

bool uartRoutePin(UARTDevice_e device, bool tx, ioTag_t pin, bool apply)
{
    for (const altPin &alt : altPins) {
        if (alt.uart == device && alt.tx == tx && alt.pin == pin) {
            if (apply) {
                uartPins[device][tx ? SERIAL_PAD_TX : SERIAL_PAD_RX] = pin;
            }
            return true;
        }
    }
    return false;
}

bool doesConfigurationUsePort(serialPortIdentifier_e identifier)
{
    return identifier >= 0 && identifier < SERIAL_PAD_UART_COUNT && portUsed[identifier];
}

const timerHardware_t *timerGetByTag(ioTag_t tag, timerUsageFlag_e flag)
{
    for (int i = 0; tag && i < timerHardwareCount; i++) {
        if (timerHardware[i].tag == tag && (flag == 0 || (timerHardware[i].usageFlags & flag))) {
            return &timerHardware[i];
        }
    }
    return NULL;
}

bool feature(uint32_t mask)
{
    return features & mask;
}

bool serialIsPortAvailable(serialPortIdentifier_e identifier)
{
    return identifier == UART2 || identifier == UART3 || identifier == UART4;
}

uint32_t pwmGetPadFunction(const timerHardware_t *timHw, uint8_t *number)
{
    if (timHw == &timerHardware[1]) {
        *number = 1;
        return TIM_USE_MOTOR;
    }
    *number = 0;
    return 0;
}
}

class SerialPadsTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        const ioTag_t tags[] = { IO_TAG(PB3), IO_TAG(PB0), IO_TAG(PA0), IO_TAG(PA1), IO_TAG(PA2), IO_TAG(PC10) };
        memset(timerHardware, 0, sizeof(timerHardware));
        for (int i = 0; i < timerHardwareCount; i++) {
            timerHardware[i].tag = tags[i];
            timerHardware[i].usageFlags = TIM_USE_OUTPUT_AUTO;
        }
        timerHardware[0].usageFlags = TIM_USE_PPM;

        memset(uartPins, 0, sizeof(uartPins));
        uartPins[UART2][SERIAL_PAD_TX] = IO_TAG(PC6);
        uartPins[UART2][SERIAL_PAD_RX] = IO_TAG(PC7);
        uartPins[UART3][SERIAL_PAD_TX] = IO_TAG(PC10);
        uartPins[UART3][SERIAL_PAD_RX] = IO_TAG(PC11);
        uartPins[UART4][SERIAL_PAD_TX] = IO_TAG(PB9);
        uartPins[UART4][SERIAL_PAD_RX] = IO_TAG(PB8);

        memset(portUsed, 0, sizeof(portUsed));
        features = 0;
        memset(serialPadConfigMutable(), 0, sizeof(serialPadConfig_t));
    }
};

TEST_F(SerialPadsTest, PadsAreNumberedWithoutInputs)
{
    EXPECT_EQ(1, serialPadFind(IO_TAG(PB0)));
    EXPECT_EQ(2, serialPadFind(IO_TAG(PA0)));
    EXPECT_EQ(5, serialPadFind(IO_TAG(PC10)));
    EXPECT_EQ(0, serialPadFind(IO_TAG(PB3)));
    EXPECT_EQ(0, serialPadFind(IO_TAG(NONE)));
}

TEST_F(SerialPadsTest, OnlyAPortInUseMoves)
{
    serialPadConfigMutable()->pin[UART4][SERIAL_PAD_TX] = IO_TAG(PA0);

    serialPadsInit();
    EXPECT_EQ(IO_TAG(PB9), uartPins[UART4][SERIAL_PAD_TX]);
    EXPECT_FALSE(serialPadIsRouted(IO_TAG(PA0)));

    portUsed[UART4] = true;
    serialPadsInit();
    EXPECT_EQ(IO_TAG(PA0), uartPins[UART4][SERIAL_PAD_TX]);
    EXPECT_EQ(IO_TAG(PB8), uartPins[UART4][SERIAL_PAD_RX]);
    EXPECT_TRUE(serialPadIsRouted(IO_TAG(PA0)));
    EXPECT_FALSE(serialPadIsRouted(IO_TAG(PA1)));
}

TEST_F(SerialPadsTest, APinAnotherPortHoldsStays)
{
    serialPadConfigMutable()->pin[UART4][SERIAL_PAD_TX] = IO_TAG(PC10);
    portUsed[UART3] = true;
    portUsed[UART4] = true;

    serialPadsInit();
    EXPECT_EQ(IO_TAG(PB9), uartPins[UART4][SERIAL_PAD_TX]);
    EXPECT_FALSE(serialPadIsRouted(IO_TAG(PC10)));
}

TEST_F(SerialPadsTest, OutputsKeepTheirNumbering)
{
    serialPadConfigMutable()->pin[UART4][SERIAL_PAD_TX] = IO_TAG(PA0);
    portUsed[UART4] = true;
    serialPadsInit();

    serialPortPins_t pins;
    serialPadGetOwnPins(UART4, &pins);
    EXPECT_EQ(IO_TAG(PB9), pins.txPin);
    EXPECT_EQ(IO_TAG(PB8), pins.rxPin);
}

TEST_F(SerialPadsTest, OnlyPadsTheUartReachesAreValid)
{
    portUsed[UART3] = true;
    serialPadsInit();

    EXPECT_TRUE(serialPadIsValid(UART4, SERIAL_PAD_TX, 0));
    EXPECT_TRUE(serialPadIsValid(UART4, SERIAL_PAD_TX, 2));
    EXPECT_FALSE(serialPadIsValid(UART4, SERIAL_PAD_RX, 2));
    EXPECT_FALSE(serialPadIsValid(UART4, SERIAL_PAD_TX, 6));
    EXPECT_FALSE(serialPadIsValid((serialPortIdentifier_e)SERIAL_PAD_UART_COUNT, SERIAL_PAD_TX, 0));
    // S5 is UART3's own TX, free only while UART3 has no function
    EXPECT_FALSE(serialPadIsValid(UART4, SERIAL_PAD_TX, 5));
    portUsed[UART3] = false;
    EXPECT_TRUE(serialPadIsValid(UART4, SERIAL_PAD_TX, 5));
}

TEST_F(SerialPadsTest, SoftSerialKeepsItsPin)
{
    // SOFTSERIAL_1_TX_PIN is PA2 (S4) in this test's build
    serialPadConfigMutable()->pin[UART2][SERIAL_PAD_TX] = IO_TAG(PA2);
    portUsed[UART2] = true;
    features = FEATURE_SOFTSERIAL;

    serialPadsInit();
    EXPECT_EQ(IO_TAG(PC6), uartPins[UART2][SERIAL_PAD_TX]);
    EXPECT_FALSE(serialPadIsValid(UART2, SERIAL_PAD_TX, 4));

    features = 0;
    serialPadsInit();
    EXPECT_EQ(IO_TAG(PA2), uartPins[UART2][SERIAL_PAD_TX]);
}

TEST_F(SerialPadsTest, ChoosingAPadTakesItFromAnotherPort)
{
    serialPadsInit();
    serialPadSet(UART4, SERIAL_PAD_TX, 2);
    EXPECT_EQ(IO_TAG(PA0), serialPadConfig()->pin[UART4][SERIAL_PAD_TX]);

    serialPadSet(UART2, SERIAL_PAD_RX, 2);
    EXPECT_EQ(IO_TAG(PA0), serialPadConfig()->pin[UART2][SERIAL_PAD_RX]);
    EXPECT_EQ(IO_TAG(NONE), serialPadConfig()->pin[UART4][SERIAL_PAD_TX]);

    serialPadSet(UART2, SERIAL_PAD_RX, 0);
    EXPECT_EQ(IO_TAG(NONE), serialPadConfig()->pin[UART2][SERIAL_PAD_RX]);
}

TEST_F(SerialPadsTest, ListOffersEachReachablePad)
{
    serialPadConfigMutable()->pin[UART4][SERIAL_PAD_TX] = IO_TAG(PA0);
    portUsed[UART3] = true;
    serialPadsInit();

    uint8_t buffer[64];
    sbuf_t dst;
    sbufInit(&dst, buffer, buffer + sizeof(buffer));
    serialPadsWriteList(&dst);

    // identifier, direction, pad, chosen, usage (1 motor), motor or servo number; S5 is UART3's own TX
    const uint8_t expected[] = {
        5,
        UART2, SERIAL_PAD_TX, 4, 0, 0, 0,
        UART2, SERIAL_PAD_RX, 2, 0, 0, 0,
        UART3, SERIAL_PAD_TX, 1, 0, 1, 1,
        UART4, SERIAL_PAD_TX, 2, 1, 0, 0,
        UART4, SERIAL_PAD_RX, 3, 0, 0, 0,
    };
    ASSERT_EQ(sizeof(expected), (size_t)(dst.ptr - buffer));
    EXPECT_EQ(0, memcmp(expected, buffer, sizeof(expected)));
}
