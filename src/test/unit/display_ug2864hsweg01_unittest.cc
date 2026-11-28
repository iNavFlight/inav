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

/*
 * Non-blocking OLED driver (drivers/display_ug2864hsweg01.c).
 *
 * The real driver is linked against a fake bus. Drawing goes into the driver's
 * shadow frame; i2c_OLED_pump() must stream only the changed columns as address
 * runs plus data chunks, survive a refused start and a lost chunk, and the
 * blocking initialisation must blank the whole panel.
 */

#include <cstdint>
#include <cstring>
#include <vector>

#include "gtest/gtest.h"

extern "C" {
    #include "platform.h"

    #include "common/log.h"
    #include "common/time.h"
    #include "drivers/bus.h"
    #include "drivers/display_ug2864hsweg01.h"
}

struct transfer_t {
    uint8_t reg;                    // control byte: 0x00 commands, 0x40 display data
    std::vector<uint8_t> data;
};

static busDevice_t fakeBusDevice;
static std::vector<transfer_t> started;     // accepted busWriteBufStart() calls, data copied at start time
static std::vector<transfer_t> blocking;    // busWrite() / busWriteBuf() calls
static bool fakeBusTaken;                   // another device holds the bus: start calls are refused
static int fakeBusyPolls;                   // busIsBusy() reports the transfer on the bus this many more times
static bool fakeTransferPending;
static bool fakeTransferFails;              // the pending transfer ends with a bus error
static uint8_t fakeStatusByte;              // controller detection: 0x03 SSD1306, 0x08 SH1106
static timeUs_t fakeMicros;

extern "C" {
    timeUs_t micros(void) { return fakeMicros; }
    void _logf(logTopic_e, unsigned, const char *, ...) {}

    busDevice_t *busDeviceInit(busType_e bus, devHardwareType_e hw, uint8_t tag, resourceOwner_e owner)
    {
        EXPECT_EQ(BUSTYPE_I2C, bus);
        EXPECT_EQ(DEVHW_UG2864, hw);
        EXPECT_EQ(0, tag);
        EXPECT_EQ(OWNER_OLED_DISPLAY, owner);
        return &fakeBusDevice;
    }

    bool busRead(const busDevice_t *dev, uint8_t reg, uint8_t *data)
    {
        EXPECT_EQ(&fakeBusDevice, dev);
        EXPECT_EQ(0xFF, reg);
        *data = fakeStatusByte;
        return true;
    }

    bool busWrite(const busDevice_t *dev, uint8_t reg, uint8_t data)
    {
        EXPECT_EQ(&fakeBusDevice, dev);
        blocking.push_back({reg, {data}});
        return true;
    }

    bool busWriteBuf(const busDevice_t *dev, uint8_t reg, const uint8_t *data, uint8_t length)
    {
        EXPECT_EQ(&fakeBusDevice, dev);
        blocking.push_back({reg, std::vector<uint8_t>(data, data + length)});
        return true;
    }

    bool busWriteBufStart(const busDevice_t *dev, uint8_t reg, const uint8_t *data, uint8_t length)
    {
        EXPECT_EQ(&fakeBusDevice, dev);
        EXPECT_FALSE(fakeTransferPending);      // one outstanding transfer per device
        if (fakeBusTaken) {
            return false;
        }
        started.push_back({reg, std::vector<uint8_t>(data, data + length)});
        fakeTransferPending = true;
        return true;
    }

    bool busIsBusy(const busDevice_t *dev, bool *error)
    {
        EXPECT_EQ(&fakeBusDevice, dev);
        *error = false;
        if (!fakeTransferPending) {
            return false;
        }
        if (fakeBusyPolls > 0) {
            fakeBusyPolls--;
            return true;
        }
        fakeTransferPending = false;
        *error = fakeTransferFails;
        fakeTransferFails = false;
        return false;
    }
}

class OledAsync : public ::testing::Test {
protected:
    std::vector<transfer_t> initStarted;
    std::vector<transfer_t> initBlocking;

    void SetUp() override {
        started.clear();
        blocking.clear();
        fakeBusTaken = false;
        fakeBusyPolls = 0;
        fakeTransferPending = false;
        fakeTransferFails = false;
        fakeStatusByte = 0x03;
        fakeMicros = 1000000;

        ASSERT_TRUE(ug2864hsweg01InitI2C());
        initStarted = started;
        initBlocking = blocking;
        started.clear();
        blocking.clear();
    }

    // Pump until the driver has nothing left; the fake bus is instant, so every call completes the previous chunk
    void drain() {
        for (int calls = 0; i2c_OLED_pump(); calls++) {
            ASSERT_LT(calls, 1000);
        }
    }

    void draw(uint8_t col, uint8_t row, char c) {
        i2c_OLED_set_xy(col, row);
        i2c_OLED_send_char(c);
    }

    static std::vector<uint8_t> addressRun(uint8_t page, uint8_t col) {
        return { (uint8_t)(0xB0 + page), (uint8_t)(col & 0x0F), (uint8_t)(0x10 | (col >> 4)) };
    }

    static size_t runsWithControl(const std::vector<transfer_t> &v, uint8_t reg) {
        size_t n = 0;
        for (const transfer_t &t : v) {
            if (t.reg == reg) {
                n++;
            }
        }
        return n;
    }

    static size_t dataBytes(const std::vector<transfer_t> &v) {
        size_t n = 0;
        for (const transfer_t &t : v) {
            if (t.reg == 0x40) {
                n += t.data.size();
            }
        }
        return n;
    }
};

// The 5 font columns of 'A'; the gap column after it is already 0 on a blank frame, so it is never sent
static const std::vector<uint8_t> glyphA = { 0x7E, 0x11, 0x11, 0x11, 0x7E };

TEST_F(OledAsync, InitBlanksThePanelWithBlockingWritesAndIsIdleAfterwards) {
    // Display off on its own, the command table, off/normal/start line around the clear, contrast and on
    ASSERT_EQ(4u, initBlocking.size());
    EXPECT_EQ(0x80, initBlocking[0].reg);
    EXPECT_EQ(std::vector<uint8_t>{ 0xAE }, initBlocking[0].data);
    EXPECT_EQ(0x00, initBlocking[1].reg);
    EXPECT_EQ(22u, initBlocking[1].data.size());
    EXPECT_EQ((std::vector<uint8_t>{ 0xA6, 0xAE, 0x40 }), initBlocking[2].data);
    EXPECT_EQ((std::vector<uint8_t>{ 0x81, 200, 0xAF }), initBlocking[3].data);

    // The whole frame went out through the pump: one address run per page, 1024 zero bytes in chunks of at most 15
    EXPECT_EQ(8u, runsWithControl(initStarted, 0x00));
    EXPECT_EQ(1024u, dataBytes(initStarted));
    EXPECT_EQ(addressRun(0, 0), initStarted[0].data);
    for (const transfer_t &t : initStarted) {
        EXPECT_LE(t.data.size(), 15u);
        if (t.reg == 0x40) {
            for (uint8_t b : t.data) {
                EXPECT_EQ(0, b);
            }
        }
    }

    EXPECT_FALSE(i2c_OLED_isBusy());
    EXPECT_FALSE(i2c_OLED_pump());
    EXPECT_TRUE(started.empty());
}

TEST_F(OledAsync, OnlyTheChangedColumnsAreSent) {
    draw(0, 1, 'A');
    EXPECT_TRUE(i2c_OLED_isBusy());
    drain();
    ASSERT_EQ(2u, started.size());
    EXPECT_EQ(0x00, started[0].reg);
    EXPECT_EQ(addressRun(1, 0), started[0].data);
    EXPECT_EQ(0x40, started[1].reg);
    EXPECT_EQ(glyphA, started[1].data);
    EXPECT_FALSE(i2c_OLED_isBusy());

    // The same character again costs nothing
    started.clear();
    draw(0, 1, 'A');
    EXPECT_FALSE(i2c_OLED_isBusy());
    EXPECT_FALSE(i2c_OLED_pump());
    EXPECT_TRUE(started.empty());

    // A new character after an unchanged one: only its five font columns, addressed where they start
    i2c_OLED_set_xy(0, 1);
    i2c_OLED_send_string("AB");
    drain();
    ASSERT_EQ(2u, started.size());
    EXPECT_EQ(addressRun(1, 6), started[0].data);
    EXPECT_EQ(5u, started[1].data.size());
}

TEST_F(OledAsync, ALineIsSplitIntoChunksOfAtMostFifteenBytes) {
    i2c_OLED_set_line(3);
    i2c_OLED_send_string("XXXXXXXXXXXXXXXXXXXXX");     // 21 characters = 126 columns, the last gap stays blank
    drain();
    ASSERT_EQ(10u, started.size());                   // one address run, eight full chunks and the rest
    EXPECT_EQ(addressRun(3, 0), started[0].data);
    for (size_t i = 1; i < started.size(); i++) {
        EXPECT_EQ(0x40, started[i].reg);
        EXPECT_LE(started[i].data.size(), 15u);
    }
    EXPECT_EQ(125u, dataBytes(started));
    EXPECT_EQ(5u, started.back().data.size());
}

TEST_F(OledAsync, NothingNewIsStartedWhileAChunkIsOnTheBus) {
    fakeBusyPolls = 2;
    draw(0, 0, 'A');
    EXPECT_TRUE(i2c_OLED_pump());
    ASSERT_EQ(1u, started.size());                    // address run started
    EXPECT_TRUE(i2c_OLED_pump());
    EXPECT_EQ(1u, started.size());                    // still on the bus
    EXPECT_TRUE(i2c_OLED_pump());
    EXPECT_EQ(1u, started.size());
    EXPECT_TRUE(i2c_OLED_pump());
    EXPECT_EQ(2u, started.size());                    // address run done, data started
    EXPECT_TRUE(i2c_OLED_isBusy());
    EXPECT_FALSE(i2c_OLED_pump());                    // data done, nothing left
    EXPECT_FALSE(i2c_OLED_isBusy());
}

TEST_F(OledAsync, AStartRefusedByABusyBusIsRetried) {
    fakeBusTaken = true;
    draw(0, 0, 'A');
    EXPECT_TRUE(i2c_OLED_pump());
    EXPECT_TRUE(started.empty());
    EXPECT_TRUE(i2c_OLED_isBusy());
    EXPECT_TRUE(i2c_OLED_pump());
    EXPECT_TRUE(started.empty());

    fakeBusTaken = false;
    EXPECT_TRUE(i2c_OLED_pump());
    ASSERT_EQ(1u, started.size());
    EXPECT_EQ(addressRun(0, 0), started[0].data);
    drain();
    ASSERT_EQ(2u, started.size());
    EXPECT_EQ(glyphA, started[1].data);
}

TEST_F(OledAsync, ALostDataChunkIsSentAgainAfterTheBackoff) {
    draw(0, 2, 'A');
    EXPECT_TRUE(i2c_OLED_pump());                     // address run started
    EXPECT_TRUE(i2c_OLED_pump());                     // address run done, data started
    ASSERT_EQ(2u, started.size());

    fakeTransferFails = true;
    EXPECT_FALSE(i2c_OLED_pump());                    // data lost: columns dirty again, backing off
    EXPECT_EQ(2u, started.size());
    EXPECT_TRUE(i2c_OLED_isBusy());

    fakeMicros += 100000;
    EXPECT_FALSE(i2c_OLED_pump());                    // still backing off
    EXPECT_EQ(2u, started.size());

    fakeMicros += 100000;
    drain();
    ASSERT_EQ(4u, started.size());
    EXPECT_EQ(addressRun(2, 0), started[2].data);     // addressed afresh, the panel's pointer is unknown
    EXPECT_EQ(glyphA, started[3].data);
    EXPECT_FALSE(i2c_OLED_isBusy());
}

TEST_F(OledAsync, ALostAddressRunIsRepeated) {
    draw(0, 2, 'A');
    EXPECT_TRUE(i2c_OLED_pump());                     // address run started
    fakeTransferFails = true;
    EXPECT_FALSE(i2c_OLED_pump());                    // lost, backing off
    fakeMicros += 200000;
    drain();
    ASSERT_EQ(3u, started.size());
    EXPECT_EQ(addressRun(2, 0), started[1].data);
    EXPECT_EQ(glyphA, started[2].data);
}

TEST_F(OledAsync, AColumnChangedWhileOnTheWireIsSentAgain) {
    draw(0, 0, 'A');
    EXPECT_TRUE(i2c_OLED_pump());                     // address run started
    EXPECT_TRUE(i2c_OLED_pump());                     // address run done, data started
    ASSERT_EQ(2u, started.size());

    fakeBusyPolls = 2;
    i2c_OLED_set_xy(0, 0);
    i2c_OLED_send_byte(0x55);                         // first column of the chunk on the wire
    EXPECT_TRUE(i2c_OLED_pump());
    EXPECT_TRUE(i2c_OLED_pump());
    EXPECT_EQ(2u, started.size());
    drain();
    ASSERT_EQ(4u, started.size());
    EXPECT_EQ(addressRun(0, 0), started[2].data);
    EXPECT_EQ(std::vector<uint8_t>{ 0x55 }, started[3].data);
}

TEST_F(OledAsync, ClearSendsTheWholeFrameEvenWhenItWasBlank) {
    i2c_OLED_clear_display_quick();
    EXPECT_TRUE(i2c_OLED_isBusy());
    drain();
    EXPECT_EQ(8u, runsWithControl(started, 0x00));
    EXPECT_EQ(1024u, dataBytes(started));
    EXPECT_FALSE(i2c_OLED_isBusy());
}

TEST_F(OledAsync, Sh1106ColumnsStartTwoColumnsIn) {
    fakeStatusByte = 0x08;
    ASSERT_TRUE(ug2864hsweg01InitI2C());
    EXPECT_EQ(addressRun(0, 2), started[0].data);     // the clear starts at the first visible column
    started.clear();

    draw(1, 0, 'A');                                  // character column 1 = pixel column 6, plus the offset
    drain();
    ASSERT_EQ(2u, started.size());
    EXPECT_EQ(addressRun(0, 8), started[0].data);
}

TEST_F(OledAsync, TheCursorWrapsToTheNextPageAfterTheLastColumn) {
    i2c_OLED_set_line(7);
    for (int i = 0; i < 130; i++) {
        i2c_OLED_send_byte(0xFF);
    }
    drain();
    // Page 0 is streamed first (the two columns that wrapped around), then the whole of page 7
    ASSERT_GE(started.size(), 4u);
    EXPECT_EQ(addressRun(0, 0), started[0].data);
    EXPECT_EQ((std::vector<uint8_t>{ 0xFF, 0xFF }), started[1].data);
    EXPECT_EQ(addressRun(7, 0), started[2].data);
    EXPECT_EQ(130u, dataBytes(started));
}
