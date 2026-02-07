/*
 * This file is part of INAV.
 *
 * INAV is free software. You can redistribute this software
 * and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * INAV is distributed in the hope that they will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>

#include "platform.h"

#if defined(SITL_BUILD)

#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/tcp.h>

#include "common/utils.h"

#include "drivers/serial.h"
#include "drivers/serial_websocket.h"
#include "target/SITL/serial_proxy.h"

// WebSocket GUID for handshake (RFC 6455)
#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

static const struct serialPortVTable wsVTable[];
static wsPort_t wsPorts[SERIAL_PORT_COUNT];
uint16_t wsBasePort = WS_BASE_PORT_DEFAULT;

// ============================================================================
// SHA-1 Implementation (minimal, for WebSocket handshake only)
// ============================================================================

typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    uint8_t buffer[64];
} SHA1_CTX;

#define SHA1_ROL(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

static void sha1_transform(uint32_t state[5], const uint8_t buffer[64])
{
    uint32_t a, b, c, d, e;
    uint32_t w[80];
    int i;

    // Prepare message schedule
    for (i = 0; i < 16; i++) {
        w[i] = ((uint32_t)buffer[i * 4] << 24) |
               ((uint32_t)buffer[i * 4 + 1] << 16) |
               ((uint32_t)buffer[i * 4 + 2] << 8) |
               ((uint32_t)buffer[i * 4 + 3]);
    }
    for (i = 16; i < 80; i++) {
        w[i] = SHA1_ROL(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);
    }

    // Initialize working variables
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];

    // Main loop
    for (i = 0; i < 80; i++) {
        uint32_t f, k, temp;

        if (i < 20) {
            f = (b & c) | ((~b) & d);
            k = 0x5A827999;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
        }

        temp = SHA1_ROL(a, 5) + f + e + k + w[i];
        e = d;
        d = c;
        c = SHA1_ROL(b, 30);
        b = a;
        a = temp;
    }

    // Add working variables back
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

static void sha1_init(SHA1_CTX *ctx)
{
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xC3D2E1F0;
    ctx->count[0] = 0;
    ctx->count[1] = 0;
}

static void sha1_update(SHA1_CTX *ctx, const uint8_t *data, size_t len)
{
    size_t i, j;

    j = (ctx->count[0] >> 3) & 63;
    if ((ctx->count[0] += len << 3) < (len << 3))
        ctx->count[1]++;
    ctx->count[1] += (len >> 29);

    if ((j + len) > 63) {
        i = 64 - j;
        memcpy(&ctx->buffer[j], data, i);
        sha1_transform(ctx->state, ctx->buffer);
        for (; i + 63 < len; i += 64) {
            sha1_transform(ctx->state, &data[i]);
        }
        j = 0;
    } else {
        i = 0;
    }

    memcpy(&ctx->buffer[j], &data[i], len - i);
}

static void sha1_final(SHA1_CTX *ctx, uint8_t digest[20])
{
    uint32_t i;
    uint8_t finalcount[8];

    for (i = 0; i < 8; i++) {
        finalcount[i] = (uint8_t)((ctx->count[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8)) & 255);
    }

    sha1_update(ctx, (const uint8_t *)"\200", 1);
    while ((ctx->count[0] & 504) != 448) {
        sha1_update(ctx, (const uint8_t *)"\0", 1);
    }
    sha1_update(ctx, finalcount, 8);

    for (i = 0; i < 20; i++) {
        digest[i] = (uint8_t)((ctx->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
    }
}

// ============================================================================
// Base64 Encoding
// ============================================================================

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void base64_encode(const uint8_t *data, size_t input_length, char *output)
{
    size_t i, j;
    for (i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;
        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;

        output[j++] = base64_chars[(triple >> 18) & 0x3F];
        output[j++] = base64_chars[(triple >> 12) & 0x3F];
        output[j++] = base64_chars[(triple >> 6) & 0x3F];
        output[j++] = base64_chars[triple & 0x3F];
    }

    // Handle padding
    int padding = (3 - (input_length % 3)) % 3;
    for (i = 0; i < padding; i++) {
        output[j - 1 - i] = '=';
    }
    output[j] = '\0';
}

// ============================================================================
// WebSocket Protocol Functions
// ============================================================================

static char* ws_find_header(const char *request, const char *header)
{
    char *line = strstr(request, header);
    if (!line) return NULL;

    line += strlen(header);
    while (*line == ' ' || *line == ':') line++;

    char *end = strstr(line, "\r\n");
    if (!end) return NULL;

    size_t len = end - line;
    char *value = malloc(len + 1);
    memcpy(value, line, len);
    value[len] = '\0';
    return value;
}

static bool ws_handshake(wsPort_t *port)
{
    char buffer[2048];
    ssize_t n = recv(port->clientSocketFd, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) {
        return false;
    }
    buffer[n] = '\0';

    // Verify it's a GET request for WebSocket
    if (strncmp(buffer, "GET ", 4) != 0) {
        fprintf(stderr, "[WEBSOCKET] Not a GET request\n");
        return false;
    }

    // Extract Sec-WebSocket-Key
    char *key = ws_find_header(buffer, "Sec-WebSocket-Key");
    if (!key) {
        fprintf(stderr, "[WEBSOCKET] Missing Sec-WebSocket-Key\n");
        return false;
    }

    // Compute accept key: SHA1(key + GUID) then base64
    char concat[256];
    snprintf(concat, sizeof(concat), "%s%s", key, WS_GUID);
    free(key);

    SHA1_CTX sha;
    uint8_t hash[20];
    sha1_init(&sha);
    sha1_update(&sha, (uint8_t *)concat, strlen(concat));
    sha1_final(&sha, hash);

    char accept_key[29];  // Base64 of 20 bytes = 28 chars + null
    base64_encode(hash, 20, accept_key);

    // Send HTTP 101 Switching Protocols response
    char response[512];
    int len = snprintf(response, sizeof(response),
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: %s\r\n"
        "\r\n",
        accept_key);

    if (send(port->clientSocketFd, response, len, 0) != len) {
        fprintf(stderr, "[WEBSOCKET] Failed to send handshake response\n");
        return false;
    }

    fprintf(stderr, "[WEBSOCKET] Handshake complete for UART%d\n", port->id);
    port->isHandshakeComplete = true;
    return true;
}

static ssize_t ws_decode_frame(wsPort_t *port, const uint8_t *data, size_t len, uint8_t *payload, size_t *payload_len)
{
    if (len < 2) return 0;  // Need at least 2 bytes

    const uint8_t *p = data;
    size_t bytes_consumed = 0;

    // Byte 0: FIN and opcode
    bool fin = (*p & 0x80) != 0;
    uint8_t opcode = *p & 0x0F;
    p++; bytes_consumed++;

    // Byte 1: Mask and payload length
    bool masked = (*p & 0x80) != 0;
    uint64_t payload_length = *p & 0x7F;
    p++; bytes_consumed++;

    // Extended payload length
    if (payload_length == 126) {
        if (len < bytes_consumed + 2) return 0;
        payload_length = ((uint64_t)p[0] << 8) | p[1];
        p += 2;
        bytes_consumed += 2;
    } else if (payload_length == 127) {
        if (len < bytes_consumed + 8) return 0;
        payload_length = 0;
        for (int i = 0; i < 8; i++) {
            payload_length = (payload_length << 8) | p[i];
        }
        p += 8;
        bytes_consumed += 8;
    }

    // Masking key (clients must mask)
    uint8_t mask[4] = {0};
    if (masked) {
        if (len < bytes_consumed + 4) return 0;
        memcpy(mask, p, 4);
        p += 4;
        bytes_consumed += 4;
    }

    // Check if we have full payload
    if (len < bytes_consumed + payload_length) {
        return 0;  // Need more data
    }

    // Handle control frames
    if (opcode == WS_OPCODE_CLOSE) {
        return -1;  // Connection close
    }

    if (opcode == WS_OPCODE_PING) {
        // Respond with PONG
        uint8_t pong_frame[2] = {0x8A, 0x00};  // FIN + PONG opcode, no payload
        send(port->clientSocketFd, pong_frame, 2, 0);
        return bytes_consumed + payload_length;
    }

    // Decode payload (unmask if needed)
    if (opcode == WS_OPCODE_BINARY || opcode == WS_OPCODE_TEXT || opcode == WS_OPCODE_CONTINUATION) {
        for (size_t i = 0; i < payload_length; i++) {
            payload[i] = masked ? (p[i] ^ mask[i % 4]) : p[i];
        }
        *payload_len = payload_length;
    }

    return bytes_consumed + payload_length;
}

static void ws_encode_frame(const uint8_t *payload, size_t len, uint8_t opcode, uint8_t *out, size_t *out_len)
{
    uint8_t *p = out;

    // Byte 0: FIN=1, opcode
    *p++ = 0x80 | (opcode & 0x0F);

    // Byte 1+: Payload length (no mask for server->client)
    if (len < 126) {
        *p++ = len & 0x7F;
    } else if (len < 65536) {
        *p++ = 126;
        *p++ = (len >> 8) & 0xFF;
        *p++ = len & 0xFF;
    } else {
        *p++ = 127;
        for (int i = 7; i >= 0; i--) {
            *p++ = (len >> (i * 8)) & 0xFF;
        }
    }

    // Payload (no masking for server)
    memcpy(p, payload, len);
    p += len;

    *out_len = p - out;
}

// ============================================================================
// Serial Port Functions (mirror serial_tcp.c pattern)
// ============================================================================

static void wsReceiveBytes(wsPort_t *port, const uint8_t* buffer, ssize_t recvSize)
{
    for (ssize_t i = 0; i < recvSize; i++) {
        if (port->serialPort.rxCallback) {
            port->serialPort.rxCallback((uint16_t)buffer[i], port->serialPort.rxCallbackData);
        } else {
            pthread_mutex_lock(&port->receiveMutex);
            port->serialPort.rxBuffer[port->serialPort.rxBufferHead] = buffer[i];
            port->serialPort.rxBufferHead = (port->serialPort.rxBufferHead + 1) % port->serialPort.rxBufferSize;
            pthread_mutex_unlock(&port->receiveMutex);
        }
    }
}

void wsReceiveBytesEx(int portIndex, const uint8_t* buffer, ssize_t recvSize)
{
    wsReceiveBytes(&wsPorts[portIndex], buffer, recvSize);
}

static int wsReceive(wsPort_t *port)
{
    char addrbuf[IPADDRESS_PRINT_BUFLEN];

    if (!port->isClientConnected) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(port->socketFd, &fds);

        if (select(port->socketFd + 1, &fds, NULL, NULL, NULL) < 0) {
            fprintf(stderr, "[WEBSOCKET] Unable to wait for connection.\n");
            return -1;
        }

        socklen_t addrLen = sizeof(struct sockaddr_storage);
        port->clientSocketFd = accept(port->socketFd, (struct sockaddr*)&port->clientAddress, &addrLen);
        if (port->clientSocketFd < 1) {
            fprintf(stderr, "[WEBSOCKET] Can't accept connection.\n");
            return -1;
        }

        char *addrptr = prettyPrintAddress((struct sockaddr *)&port->clientAddress, addrbuf, IPADDRESS_PRINT_BUFLEN);
        if (addrptr != NULL) {
            fprintf(stderr, "[WEBSOCKET] %s connected to UART%d (WebSocket)\n", addrptr, port->id);
        }

        port->isClientConnected = true;
        port->isHandshakeComplete = false;
    }

    // Handle WebSocket handshake
    if (!port->isHandshakeComplete) {
        fprintf(stderr, "[WEBSOCKET] Attempting handshake for UART%d\n", port->id);
        if (!ws_handshake(port)) {
            fprintf(stderr, "[WEBSOCKET] Handshake failed for UART%d\n", port->id);
            close(port->clientSocketFd);
            port->isClientConnected = false;
            return -1;
        }
        fprintf(stderr, "[WEBSOCKET] Client connected and ready on UART%d\n", port->id);
        return 0;
    }

    // Receive WebSocket frame
    uint8_t buffer[WS_BUFFER_SIZE];
    ssize_t recvSize = recv(port->clientSocketFd, buffer, WS_BUFFER_SIZE, 0);

    if (port->isClientConnected && (recvSize == 0 || (recvSize == -1 && errno == ECONNRESET))) {
        char *addrptr = prettyPrintAddress((struct sockaddr *)&port->clientAddress, addrbuf, IPADDRESS_PRINT_BUFLEN);
        if (addrptr != NULL) {
            fprintf(stderr, "[WEBSOCKET] %s disconnected from UART%d\n", addrptr, port->id);
        }
        close(port->clientSocketFd);
        memset(&port->clientAddress, 0, sizeof(port->clientAddress));
        port->isClientConnected = false;
        port->isHandshakeComplete = false;
        return 0;
    }

    if (recvSize < 0) {
        return 0;
    }

    // Decode WebSocket frame
    uint8_t payload[WS_MAX_PACKET_SIZE];
    size_t payload_len = 0;
    ssize_t consumed = ws_decode_frame(port, buffer, recvSize, payload, &payload_len);

    if (consumed < 0) {
        // Close frame received
        close(port->clientSocketFd);
        port->isClientConnected = false;
        port->isHandshakeComplete = false;
        return 0;
    }

    if (consumed > 0 && payload_len > 0) {
        fprintf(stderr, "[WEBSOCKET] UART%d RX %zu bytes: ", port->id, payload_len);
        for (size_t i = 0; i < payload_len && i < 32; i++) {
            fprintf(stderr, "%02x ", payload[i]);
        }
        fprintf(stderr, "\n");
        wsReceiveBytes(port, payload, payload_len);
    }

    return (int)payload_len;
}

static void *wsReceiveThread(void* arg)
{
    wsPort_t *port = (wsPort_t*)arg;
    while(wsReceive(port) >= 0)
        ;
    return NULL;
}

static wsPort_t *wsReConfigure(wsPort_t *port, uint32_t id)
{
    socklen_t sockaddrlen;
    if (port->isInitialized){
        return port;
    }

    if (pthread_mutex_init(&port->receiveMutex, NULL) != 0){
        return NULL;
    }

    uint16_t wsPort = wsBasePort + id;
    if (lookupAddress(NULL, wsPort, SOCK_STREAM, (struct sockaddr*)&port->sockAddress, &sockaddrlen) != 0) {
        return NULL;
    }
    port->socketFd = socket(((struct sockaddr*)&port->sockAddress)->sa_family, SOCK_STREAM, IPPROTO_TCP);

    if (port->socketFd < 0) {
        fprintf(stderr, "[WEBSOCKET] Unable to create socket\n");
        return NULL;
    }

    int err = 0;
#ifdef __CYGWIN__
    if (((struct sockaddr*)&port->sockAddress)->sa_family == AF_INET6) {
        int v6only = 0;
        err = setsockopt(port->socketFd, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only));
        if (err != 0) {
            fprintf(stderr, "[WEBSOCKET] setting V6ONLY=false: %s\n", strerror(errno));
        }
    }
#endif

    int one = 1;
    err = setsockopt(port->socketFd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
    err = setsockopt(port->socketFd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    err = fcntl(port->socketFd, F_SETFL, fcntl(port->socketFd, F_GETFL, 0) | O_NONBLOCK);

    if (err < 0){
        fprintf(stderr, "[WEBSOCKET] Unable to set socket options\n");
        return NULL;
    }

    port->isClientConnected = false;
    port->isInitialized = true;
    port->isHandshakeComplete = false;
    port->id = id;

    if (bind(port->socketFd, (struct sockaddr*)&port->sockAddress, sockaddrlen) < 0) {
        fprintf(stderr, "[WEBSOCKET] Unable to bind socket\n");
        return NULL;
    }

    if (listen(port->socketFd, 100) < 0) {
        fprintf(stderr, "[WEBSOCKET] Unable to listen.\n");
        return NULL;
    }

    char addrbuf[IPADDRESS_PRINT_BUFLEN];
    char *addrptr = prettyPrintAddress((struct sockaddr *)&port->sockAddress, addrbuf, IPADDRESS_PRINT_BUFLEN);
    if (addrptr != NULL) {
        fprintf(stderr, "[WEBSOCKET] Bind WebSocket %s to UART%d\n", addrptr, id);
    }

    return port;
}

serialPort_t *wsOpen(USART_TypeDef *USARTx, serialReceiveCallbackPtr callback, void *rxCallbackData, uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    wsPort_t *port = NULL;

#if defined(USE_UART1) || defined(USE_UART2) || defined(USE_UART3) || defined(USE_UART4) || defined(USE_UART5) || defined(USE_UART6) || defined(USE_UART7) || defined(USE_UART8)
    uint32_t id = (uintptr_t)USARTx;
    if (id <= SERIAL_PORT_COUNT) {
        port = wsReConfigure(&wsPorts[id-1], id);
    }
#endif

    if (port == NULL) {
        return NULL;
    }

    port->serialPort.vTable = wsVTable;
    port->serialPort.rxCallback = callback;
    port->serialPort.rxCallbackData = rxCallbackData;
    port->serialPort.rxBufferHead = port->serialPort.rxBufferTail = 0;
    port->serialPort.rxBufferSize = WS_BUFFER_SIZE;
    port->serialPort.rxBuffer = port->rxBuffer;
    port->serialPort.mode = mode;
    port->serialPort.baudRate = baudRate;
    port->serialPort.options = options;

    int err = pthread_create(&port->receiveThread, NULL, wsReceiveThread, (void*)port);
    if (err < 0){
        fprintf(stderr, "[WEBSOCKET] Unable to create receive thread for UART%d\n", id);
        return NULL;
    }
    return (serialPort_t*)port;
}

uint8_t wsRead(serialPort_t *instance)
{
    uint8_t ch;
    wsPort_t *port = (wsPort_t*)instance;
    pthread_mutex_lock(&port->receiveMutex);

    ch = port->serialPort.rxBuffer[port->serialPort.rxBufferTail];
    port->serialPort.rxBufferTail = (port->serialPort.rxBufferTail + 1) % port->serialPort.rxBufferSize;

    pthread_mutex_unlock(&port->receiveMutex);

    return ch;
}

void wsWriteBuf(serialPort_t *instance, const void *data, int count)
{
    wsPort_t *port = (wsPort_t*)instance;

    if (!port->isClientConnected || !port->isHandshakeComplete) {
        return;
    }

    // Encode as WebSocket binary frame
    uint8_t frame[WS_MAX_PACKET_SIZE + 14];  // +14 for header
    size_t frame_len;
    ws_encode_frame((const uint8_t *)data, count, WS_OPCODE_BINARY, frame, &frame_len);

    fprintf(stderr, "[WEBSOCKET] UART%d TX %d bytes: ", port->id, count);
    const uint8_t *payload = (const uint8_t *)data;
    for (int i = 0; i < count && i < 32; i++) {
        fprintf(stderr, "%02x ", payload[i]);
    }
    fprintf(stderr, "\n");

    send(port->clientSocketFd, frame, frame_len, 0);
}

int getWsPortIndex(const serialPort_t *instance)
{
    for (int i = 0; i < SERIAL_PORT_COUNT; i++) {
        if (&(wsPorts[i].serialPort) == instance) return i;
    }
    return -1;
}

void wsWrite(serialPort_t *instance, uint8_t ch)
{
    wsWriteBuf(instance, (void*)&ch, 1);

    int index = getWsPortIndex(instance);
    if (!serialFCProxy && serialProxyIsConnected() && (index == (serialUartIndex-1))) {
        serialProxyWriteData((unsigned char *)&ch, 1);
    }
}

uint32_t wsTotalRxBytesWaiting(const serialPort_t *instance)
{
    wsPort_t *port = (wsPort_t*)instance;
    uint32_t count;

    pthread_mutex_lock(&port->receiveMutex);

    if (port->serialPort.rxBufferHead >= port->serialPort.rxBufferTail) {
        count = port->serialPort.rxBufferHead - port->serialPort.rxBufferTail;
    } else {
        count = port->serialPort.rxBufferSize + port->serialPort.rxBufferHead - port->serialPort.rxBufferTail;
    }

    pthread_mutex_unlock(&port->receiveMutex);

    return count;
}

uint32_t wsRXBytesFree(int portIndex)
{
    return wsPorts[portIndex].serialPort.rxBufferSize - wsTotalRxBytesWaiting(&wsPorts[portIndex].serialPort);
}

uint32_t wsTotalTxBytesFree(const serialPort_t *instance)
{
    UNUSED(instance);
    return WS_MAX_PACKET_SIZE;
}

bool isWsTransmitBufferEmpty(const serialPort_t *instance)
{
    UNUSED(instance);
    return true;
}

bool wsIsConnected(const serialPort_t *instance)
{
    wsPort_t *port = (wsPort_t*)instance;
    return port->isClientConnected && port->isHandshakeComplete;
}

void wsSetBaudRate(serialPort_t *instance, uint32_t baudRate)
{
    UNUSED(instance);
    UNUSED(baudRate);
}

void wsSetMode(serialPort_t *instance, portMode_t mode)
{
    UNUSED(instance);
    UNUSED(mode);
}

void wsSetOptions(serialPort_t *instance, portOptions_t options)
{
    UNUSED(instance);
    UNUSED(options);
}

static const struct serialPortVTable wsVTable[] = {
    {
        .serialWrite = wsWrite,
        .serialTotalRxWaiting = wsTotalRxBytesWaiting,
        .serialTotalTxFree = wsTotalTxBytesFree,
        .serialRead = wsRead,
        .serialSetBaudRate = wsSetBaudRate,
        .isSerialTransmitBufferEmpty = isWsTransmitBufferEmpty,
        .setMode = wsSetMode,
        .setOptions = wsSetOptions,
        .isConnected = wsIsConnected,
        .writeBuf = wsWriteBuf,
        .beginWrite = NULL,
        .endWrite = NULL,
        .isIdle = NULL,
    }
};

#endif
