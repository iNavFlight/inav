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

#include <cstdint>
#include <cstring>

extern "C" {
#include "common/utils.h"
#include "drivers/serial.h"
}

#include "gtest/gtest.h"

static uint32_t testRxWaiting(const serialPort_t *instance)
{
    if (instance->rxBufferHead >= instance->rxBufferTail) {
        return instance->rxBufferHead - instance->rxBufferTail;
    }
    return instance->rxBufferSize + instance->rxBufferHead - instance->rxBufferTail;
}

static serialPortVTable testVTable;
static int callbackCount;

static void testRxCallback(uint16_t data, void *rxCallbackData)
{
    UNUSED(data);
    UNUSED(rxCallbackData);
    callbackCount++;
}

class SerialInjectTest : public ::testing::Test {
protected:
    uint8_t buffer[8];
    serialPort_t port;

    void SetUp() override
    {
        memset(&testVTable, 0, sizeof(testVTable));
        testVTable.serialTotalRxWaiting = testRxWaiting;
        memset(buffer, 0, sizeof(buffer));
        memset(&port, 0, sizeof(port));
        port.vTable = &testVTable;
        port.mode = MODE_RXTX;
        port.rxBuffer = buffer;
        port.rxBufferSize = sizeof(buffer);
        callbackCount = 0;
    }
};

TEST_F(SerialInjectTest, FillsRxBuffer)
{
    const uint8_t data[] = { 1, 2, 3 };
    EXPECT_TRUE(serialInjectRxBuf(&port, data, sizeof(data)));
    EXPECT_EQ(port.rxBufferHead, 3u);
    EXPECT_EQ(memcmp(buffer, data, sizeof(data)), 0);
}

TEST_F(SerialInjectTest, WrapsAround)
{
    port.rxBufferHead = 6;
    port.rxBufferTail = 6;
    const uint8_t data[] = { 1, 2, 3, 4 };
    EXPECT_TRUE(serialInjectRxBuf(&port, data, sizeof(data)));
    EXPECT_EQ(port.rxBufferHead, 2u);
    EXPECT_EQ(buffer[6], 1);
    EXPECT_EQ(buffer[7], 2);
    EXPECT_EQ(buffer[0], 3);
    EXPECT_EQ(buffer[1], 4);
}

TEST_F(SerialInjectTest, AllOrNothing)
{
    // the 8-byte ring holds 7 bytes; with 5 waiting there is room for 2
    port.rxBufferHead = 5;
    const uint8_t data[] = { 1, 2, 3 };
    EXPECT_FALSE(serialInjectRxBuf(&port, data, 3));
    EXPECT_EQ(port.rxBufferHead, 5u);
    EXPECT_TRUE(serialInjectRxBuf(&port, data, 2));
    EXPECT_EQ(port.rxBufferHead, 7u);
    EXPECT_FALSE(serialInjectRxBuf(&port, data, 1));
}

TEST_F(SerialInjectTest, RefusesPortWithRxCallback)
{
    // serial RC receivers parse in interrupt context and must not be fed from a task
    port.rxCallback = testRxCallback;
    const uint8_t data[] = { 9, 8, 7 };
    EXPECT_FALSE(serialInjectRxBuf(&port, data, sizeof(data)));
    EXPECT_EQ(callbackCount, 0);
    EXPECT_EQ(port.rxBufferHead, 0u);
}

TEST_F(SerialInjectTest, RefusesPortThatDoesNotReceive)
{
    port.mode = MODE_TX;
    const uint8_t data[] = { 1 };
    EXPECT_FALSE(serialInjectRxBuf(&port, data, sizeof(data)));
    EXPECT_EQ(port.rxBufferHead, 0u);
}

TEST_F(SerialInjectTest, RefusesPortWithoutRxBuffer)
{
    // like the USB VCP: no software RX buffer and no callback
    port.rxBuffer = NULL;
    port.rxBufferSize = 0;
    const uint8_t data[] = { 1 };
    EXPECT_FALSE(serialInjectRxBuf(&port, data, sizeof(data)));
    EXPECT_FALSE(serialInjectRxBuf(&port, data, 0));
}

TEST_F(SerialInjectTest, EmptyDataIsAProbe)
{
    EXPECT_TRUE(serialInjectRxBuf(&port, NULL, 0));
    EXPECT_EQ(port.rxBufferHead, 0u);
}
