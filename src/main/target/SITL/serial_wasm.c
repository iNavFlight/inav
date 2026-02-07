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
 * WASM Virtual Serial Port
 *
 * Implements a virtual serial port for WebAssembly builds.
 * JavaScript can write bytes via serialWriteByte() and read bytes via serialReadByte().
 * This port integrates with INAV's existing MSP infrastructure - no special handling needed!
 *
 * Architecture:
 *   JavaScript → serialWriteByte() → RX ring buffer → MSP parser → MSP handler → TX ring buffer → serialReadByte() → JavaScript
 *
 * This is just a transport layer, exactly like UART, TCP, UDP, or BLE.
 *
 * Event Notification:
 *   When firmware completes writing a response, serialEndWrite() notifies JavaScript
 *   by calling Module.wasmSerialDataCallback() if set. This is like a hardware interrupt.
 */

#ifdef __EMSCRIPTEN__

#include <emscripten.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"
#include "drivers/serial.h"
#include "io/serial.h"

// JavaScript callback notification (like a hardware interrupt)
// Called when WASM has data ready to send to JavaScript
// JavaScript should set: Module.wasmSerialDataCallback = function() { ... }
EM_JS(void, notifySerialDataAvailable, (), {
    if (Module.wasmSerialDataCallback) {
        Module.wasmSerialDataCallback();
    }
});

// Ring buffer sizes
#define WASM_SERIAL_RX_BUFFER_SIZE 512
#define WASM_SERIAL_TX_BUFFER_SIZE 2048

// Ring buffers (no volatile needed - single-threaded WASM)
static uint8_t wasmSerialRxBuffer[WASM_SERIAL_RX_BUFFER_SIZE];
static uint8_t wasmSerialTxBuffer[WASM_SERIAL_TX_BUFFER_SIZE];

// Serial port structure
static serialPort_t wasmSerialPort;

// Track initialization state
static bool wasmSerialInitialized = false;

// Counters for dropped bytes (buffer overflow) - exposed via serialGetRx/TxDroppedBytes()
static uint32_t wasmSerialTxDroppedBytes = 0;
static uint32_t wasmSerialRxDroppedBytes = 0;

// One-time overflow warning flags (log first occurrence only to avoid console spam)
static bool wasmSerialTxOverflowLogged = false;
static bool wasmSerialRxOverflowLogged = false;

// Forward declarations
static void wasmSerialWrite(serialPort_t *instance, uint8_t ch);
static uint32_t wasmSerialTotalRxWaiting(const serialPort_t *instance);
static uint32_t wasmSerialTotalTxFree(const serialPort_t *instance);
static uint8_t wasmSerialRead(serialPort_t *instance);
static void wasmSerialSetBaudRate(serialPort_t *instance, uint32_t baudRate);
static bool wasmSerialIsTransmitBufferEmpty(const serialPort_t *instance);
static void wasmSerialSetMode(serialPort_t *instance, portMode_t mode);
static void wasmSerialSetOptions(serialPort_t *instance, portOptions_t options);
static void wasmSerialWriteBuf(serialPort_t *instance, const void *data, int count);
static bool wasmSerialIsConnected(const serialPort_t *instance);
static bool wasmSerialIsIdle(serialPort_t *instance);
static void wasmSerialBeginWrite(serialPort_t *instance);
static void wasmSerialEndWrite(serialPort_t *instance);

// Virtual serial port vtable
static const struct serialPortVTable wasmSerialVTable = {
    .serialWrite = wasmSerialWrite,
    .serialTotalRxWaiting = wasmSerialTotalRxWaiting,
    .serialTotalTxFree = wasmSerialTotalTxFree,
    .serialRead = wasmSerialRead,
    .serialSetBaudRate = wasmSerialSetBaudRate,
    .isSerialTransmitBufferEmpty = wasmSerialIsTransmitBufferEmpty,
    .setMode = wasmSerialSetMode,
    .setOptions = wasmSerialSetOptions,
    .writeBuf = wasmSerialWriteBuf,
    .isConnected = wasmSerialIsConnected,
    .isIdle = wasmSerialIsIdle,
    .beginWrite = wasmSerialBeginWrite,
    .endWrite = wasmSerialEndWrite,
};

/**
 * Initialize WASM serial port
 * @return Serial port instance
 */
serialPort_t *wasmSerialInit(void)
{
    if (wasmSerialInitialized) {
        return &wasmSerialPort;  // Already initialized
    }

    wasmSerialPort.vTable = &wasmSerialVTable;
    wasmSerialPort.identifier = SERIAL_PORT_NONE;  // Virtual port
    wasmSerialPort.mode = MODE_RXTX;
    wasmSerialPort.options = SERIAL_NOT_INVERTED;
    wasmSerialPort.baudRate = 115200;  // Nominal baud rate

    wasmSerialPort.rxBuffer = wasmSerialRxBuffer;
    wasmSerialPort.txBuffer = wasmSerialTxBuffer;
    wasmSerialPort.rxBufferSize = WASM_SERIAL_RX_BUFFER_SIZE;
    wasmSerialPort.txBufferSize = WASM_SERIAL_TX_BUFFER_SIZE;
    wasmSerialPort.rxBufferHead = 0;
    wasmSerialPort.rxBufferTail = 0;
    wasmSerialPort.txBufferHead = 0;
    wasmSerialPort.txBufferTail = 0;

    wasmSerialPort.rxCallback = NULL;
    wasmSerialPort.rxCallbackData = NULL;

    wasmSerialInitialized = true;
    return &wasmSerialPort;
}

/**
 * Get WASM serial port instance
 * @return Serial port instance
 */
serialPort_t *wasmSerialGetPort(void)
{
    return &wasmSerialPort;
}

// ============================================================================
// Serial port vtable implementations
// ============================================================================

static void wasmSerialWrite(serialPort_t *instance, uint8_t ch)
{
    uint32_t nextHead = (instance->txBufferHead + 1) % instance->txBufferSize;

    if (nextHead != instance->txBufferTail) {
        instance->txBuffer[instance->txBufferHead] = ch;
        instance->txBufferHead = nextHead;
    } else {
        // Buffer full - drop byte and track for debugging
        wasmSerialTxDroppedBytes++;
        if (!wasmSerialTxOverflowLogged) {
            wasmSerialTxOverflowLogged = true;
            EM_ASM({ console.error('[WASM Serial] TX buffer overflow - firmware sending faster than JS reading'); });
        }
    }
}

static uint32_t wasmSerialTotalRxWaiting(const serialPort_t *instance)
{
    if (instance->rxBufferHead >= instance->rxBufferTail) {
        return instance->rxBufferHead - instance->rxBufferTail;
    } else {
        return instance->rxBufferSize - instance->rxBufferTail + instance->rxBufferHead;
    }
}

static uint32_t wasmSerialTotalTxFree(const serialPort_t *instance)
{
    uint32_t bytesUsed;
    if (instance->txBufferHead >= instance->txBufferTail) {
        bytesUsed = instance->txBufferHead - instance->txBufferTail;
    } else {
        bytesUsed = instance->txBufferSize - instance->txBufferTail + instance->txBufferHead;
    }

    return instance->txBufferSize - bytesUsed - 1;  // -1 to distinguish full from empty
}

static uint8_t wasmSerialRead(serialPort_t *instance)
{
    if (instance->rxBufferHead == instance->rxBufferTail) {
        return 0;  // No data available
    }

    uint8_t ch = instance->rxBuffer[instance->rxBufferTail];
    instance->rxBufferTail = (instance->rxBufferTail + 1) % instance->rxBufferSize;

    return ch;
}

static void wasmSerialSetBaudRate(serialPort_t *instance, uint32_t baudRate)
{
    instance->baudRate = baudRate;  // Nominal only for WASM
}

static bool wasmSerialIsTransmitBufferEmpty(const serialPort_t *instance)
{
    // For WASM, always return true to avoid blocking in waitForSerialPortToFinishTransmitting()
    // JavaScript reads TX buffer asynchronously via interrupt-style callback, so we don't
    // need to wait here - the main loop will yield control and JS will read the bytes
    UNUSED(instance);
    return true;
}

static void wasmSerialSetMode(serialPort_t *instance, portMode_t mode)
{
    instance->mode = mode;
}

static void wasmSerialSetOptions(serialPort_t *instance, portOptions_t options)
{
    instance->options = options;
}

static void wasmSerialWriteBuf(serialPort_t *instance, const void *data, int count)
{
    const uint8_t *bytes = (const uint8_t *)data;
    for (int i = 0; i < count; i++) {
        wasmSerialWrite(instance, bytes[i]);
    }
}

static bool wasmSerialIsConnected(const serialPort_t *instance)
{
    UNUSED(instance);
    return true;  // Always connected for WASM
}

static bool wasmSerialIsIdle(serialPort_t *instance)
{
    return wasmSerialIsTransmitBufferEmpty(instance);
}

static void wasmSerialBeginWrite(serialPort_t *instance)
{
    UNUSED(instance);
    // No-op for WASM
}

static void wasmSerialEndWrite(serialPort_t *instance)
{
    UNUSED(instance);

    // Notify JavaScript that data is available (like a hardware interrupt)
    // This is called after MSP writes a complete response frame
    notifySerialDataAvailable();
}

// ============================================================================
// JavaScript interface functions
// ============================================================================

/**
 * Write a byte to WASM serial RX buffer (from JavaScript)
 * JavaScript calls this to send MSP packet bytes to the firmware
 *
 * @param data Byte to write
 */
EMSCRIPTEN_KEEPALIVE
void serialWriteByte(uint8_t data)
{
    // Ensure serial port is initialized before first use
    if (!wasmSerialInitialized) {
        wasmSerialInit();
    }

    serialPort_t *port = &wasmSerialPort;
    uint32_t nextHead = (port->rxBufferHead + 1) % port->rxBufferSize;

    if (nextHead != port->rxBufferTail) {
        port->rxBuffer[port->rxBufferHead] = data;
        port->rxBufferHead = nextHead;
    } else {
        // Buffer full - drop byte (counter available via serialGetRxDroppedBytes)
        wasmSerialRxDroppedBytes++;
        if (!wasmSerialRxOverflowLogged) {
            wasmSerialRxOverflowLogged = true;
            EM_ASM({ console.error('[WASM Serial] RX buffer overflow - JS sending faster than firmware processing'); });
        }
    }
}

/**
 * Read a byte from WASM serial TX buffer (to JavaScript)
 * JavaScript calls this to receive MSP response bytes from the firmware
 *
 * @return Byte value, or -1 if no data available
 */
EMSCRIPTEN_KEEPALIVE
int serialReadByte(void)
{
    // Ensure serial port is initialized
    if (!wasmSerialInitialized) {
        return -1;  // Not initialized yet, no data
    }

    serialPort_t *port = &wasmSerialPort;

    if (port->txBufferHead == port->txBufferTail) {
        return -1;  // No data available
    }

    uint8_t data = port->txBuffer[port->txBufferTail];
    port->txBufferTail = (port->txBufferTail + 1) % port->txBufferSize;

    return data;
}

/**
 * Check how many bytes are available to read from WASM serial TX buffer
 * JavaScript calls this to check if response data is ready
 *
 * @return Number of bytes available
 */
EMSCRIPTEN_KEEPALIVE
int serialAvailable(void)
{
    // Ensure serial port is initialized
    if (!wasmSerialInitialized) {
        return 0;  // Not initialized yet, no data
    }

    serialPort_t *port = &wasmSerialPort;

    // Calculate bytes used (available to read)
    if (port->txBufferHead >= port->txBufferTail) {
        return port->txBufferHead - port->txBufferTail;
    } else {
        return port->txBufferSize - port->txBufferTail + port->txBufferHead;
    }
}

/**
 * Get count of bytes dropped due to RX buffer overflow
 * JavaScript can call this to detect if the firmware is receiving data faster
 * than it can process.
 */
EMSCRIPTEN_KEEPALIVE
uint32_t serialGetRxDroppedBytes(void)
{
    return wasmSerialRxDroppedBytes;
}

/**
 * Get count of bytes dropped due to TX buffer overflow
 * JavaScript can call this to detect if the firmware is sending data faster
 * than JavaScript is reading.
 */
EMSCRIPTEN_KEEPALIVE
uint32_t serialGetTxDroppedBytes(void)
{
    return wasmSerialTxDroppedBytes;
}

#endif  // __EMSCRIPTEN__
