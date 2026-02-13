/*
 * canard_sitl_driver.c
 *
 * SITL CAN Driver - SocketCAN implementation for Linux
 * Falls back to stub mode on non-Linux platforms or if socket fails
 *
 * Created: 2026-02-12
 */

#include "platform.h"

#if defined(SITL_BUILD) && defined(USE_DRONECAN)

#include "canard.h"
#include "canard_stm32_driver.h"
#include "common/log.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __linux__
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#endif

// Driver state
typedef enum {
    SITL_CAN_MODE_STUB = 0,
    SITL_CAN_MODE_SOCKETCAN
} sitlCANMode_e;

static bool driver_initialized = false;
static sitlCANMode_e can_mode = SITL_CAN_MODE_STUB;

#ifdef __linux__
static int can_socket = -1;
static char can_interface_name[IFNAMSIZ] = DRONECAN_SITL_INTERFACE;
#endif

// Forward declarations
static int16_t sitlCANInitStub(uint32_t bitrate);
static int16_t sitlCANInitSocketCAN(uint32_t bitrate);
static int16_t sitlCANReceiveStub(CanardCANFrame *const rx_frame);
static int16_t sitlCANReceiveSocketCAN(CanardCANFrame *const rx_frame);
static int16_t sitlCANTransmitStub(const CanardCANFrame* const tx_frame);
static int16_t sitlCANTransmitSocketCAN(const CanardCANFrame* const tx_frame);
static void sitlCANGetStatsStub(canardProtocolStatus_t *pProtocolStat);
static void sitlCANGetStatsSocketCAN(canardProtocolStatus_t *pProtocolStat);

/**
 * @brief Initialize CAN interface with SocketCAN (Linux) or stub fallback
 * @param bitrate CAN bitrate in bps
 * @retval 0 on success, negative on error
 */
int16_t canardSTM32CAN1_Init(uint32_t bitrate) {
    if (driver_initialized) {
        return 0; // Already initialized
    }

#ifdef __linux__
    // Try SocketCAN first
    if (sitlCANInitSocketCAN(bitrate) == 0) {
        can_mode = SITL_CAN_MODE_SOCKETCAN;
        driver_initialized = true;
        LOG_INFO(CAN, "DroneCAN SITL driver initialized (SocketCAN on %s)", can_interface_name);
        return 0;
    }
    // Fall back to stub mode
    LOG_WARNING(CAN, "SocketCAN initialization failed, falling back to stub mode");
#endif

    // Use stub mode (non-Linux or SocketCAN failed)
    can_mode = SITL_CAN_MODE_STUB;
    driver_initialized = true;
    sitlCANInitStub(bitrate);
    return 0;
}

// Stub implementations
static int16_t sitlCANInitStub(uint32_t bitrate) {
    (void)bitrate;
    LOG_DEBUG(CAN, "SITL DroneCAN driver initialized (stub mode)");
    return 0;
}

static int16_t sitlCANReceiveStub(CanardCANFrame *const rx_frame) {
    (void)rx_frame;
    return 0; // No data available in stub mode
}

static int16_t sitlCANTransmitStub(const CanardCANFrame* const tx_frame) {
    (void)tx_frame;
    return 1; // Success (frame "transmitted")
}

static void sitlCANGetStatsStub(canardProtocolStatus_t *pProtocolStat) {
    pProtocolStat->BusOff = 0;
    pProtocolStat->ErrorPassive = 0;
}

#ifdef __linux__
// SocketCAN implementations

/**
 * @brief Initialize SocketCAN interface
 * @param bitrate CAN bitrate in bps (for logging, actual rate set via ip link)
 * @retval 0 on success, negative on error
 */
static int16_t sitlCANInitSocketCAN(uint32_t bitrate) {
    struct sockaddr_can addr;
    struct ifreq ifr;

    // Create CAN socket
    can_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (can_socket < 0) {
        LOG_ERROR(CAN, "Failed to create CAN socket: %s", strerror(errno));
        return -1;
    }

    // Set non-blocking mode
    int flags = fcntl(can_socket, F_GETFL, 0);
    if (fcntl(can_socket, F_SETFL, flags | O_NONBLOCK) < 0) {
        LOG_ERROR(CAN, "Failed to set non-blocking mode: %s", strerror(errno));
        close(can_socket);
        can_socket = -1;
        return -1;
    }

    // Get interface index
    strncpy(ifr.ifr_name, can_interface_name, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    if (ioctl(can_socket, SIOCGIFINDEX, &ifr) < 0) {
        LOG_ERROR(CAN, "Failed to get interface index for %s: %s", can_interface_name, strerror(errno));
        close(can_socket);
        can_socket = -1;
        return -1;
    }

    // Bind to CAN interface
    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(can_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        LOG_ERROR(CAN, "Failed to bind to CAN interface %s: %s", can_interface_name, strerror(errno));
        close(can_socket);
        can_socket = -1;
        return -1;
    }

    LOG_INFO(CAN, "SocketCAN initialized on %s at %lu bps", can_interface_name, (unsigned long)bitrate);
    return 0;
}

/**
 * @brief Convert libcanard frame to Linux CAN frame
 */
static void sitlCANFrameToLinux(const CanardCANFrame *const src, struct can_frame *const dst) {
    memset(dst, 0, sizeof(struct can_frame));

    // Handle extended frame format (EFF) - DroneCAN uses 29-bit IDs
    if (src->id & CANARD_CAN_FRAME_EFF) {
        dst->can_id = (src->id & CANARD_CAN_EXT_ID_MASK) | CAN_EFF_FLAG;
    } else {
        dst->can_id = src->id & CANARD_CAN_STD_ID_MASK;
    }

    // Copy data
    dst->can_dlc = src->data_len;
    if (src->data_len > 0) {
        memcpy(dst->data, src->data, src->data_len);
    }
}

/**
 * @brief Convert Linux CAN frame to libcanard frame
 */
static void sitlCANFrameFromLinux(const struct can_frame *const src, CanardCANFrame *const dst) {
    memset(dst, 0, sizeof(CanardCANFrame));

    // Handle extended frame format
    if (src->can_id & CAN_EFF_FLAG) {
        dst->id = (src->can_id & CANARD_CAN_EXT_ID_MASK) | CANARD_CAN_FRAME_EFF;
    } else {
        dst->id = src->can_id & CANARD_CAN_STD_ID_MASK;
    }

    // Copy data
    dst->data_len = src->can_dlc;
    if (src->can_dlc > 0) {
        memcpy(dst->data, src->data, src->can_dlc);
    }
}

/**
 * @brief Receive a CAN frame via SocketCAN
 * @param rx_frame Pointer to frame structure to fill
 * @retval 0 if no frame available, 1 if frame received, negative on error
 */
static int16_t sitlCANReceiveSocketCAN(CanardCANFrame *const rx_frame) {
    struct can_frame frame;
    ssize_t nbytes;

    if (can_socket < 0) {
        return -1;
    }

    // Non-blocking receive
    nbytes = read(can_socket, &frame, sizeof(struct can_frame));
    if (nbytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0; // No data available
        }
        LOG_ERROR(CAN, "SocketCAN receive error: %s", strerror(errno));
        return -1;
    }

    if (nbytes != sizeof(struct can_frame)) {
        LOG_WARNING(CAN, "Incomplete CAN frame received");
        return 0;
    }

    sitlCANFrameFromLinux(&frame, rx_frame);
    return 1;
}

/**
 * @brief Transmit a CAN frame via SocketCAN
 * @param tx_frame Pointer to frame to transmit
 * @retval 1 on success, 0 if busy, negative on error
 */
static int16_t sitlCANTransmitSocketCAN(const CanardCANFrame* const tx_frame) {
    struct can_frame frame;
    ssize_t nbytes;

    if (can_socket < 0) {
        return -1;
    }

    sitlCANFrameToLinux(tx_frame, &frame);

    nbytes = write(can_socket, &frame, sizeof(struct can_frame));
    if (nbytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0; // Busy, try again later
        }
        LOG_ERROR(CAN, "SocketCAN transmit error: %s", strerror(errno));
        return -1;
    }

    return 1; // Success
}

/**
 * @brief Get CAN protocol status from SocketCAN
 * @param pProtocolStat Pointer to status structure to fill
 */
static void sitlCANGetStatsSocketCAN(canardProtocolStatus_t *pProtocolStat) {
    // SocketCAN doesn't expose bus-off/error-passive directly
    // We could check interface flags via netlink, but for SITL testing
    // we assume the virtual CAN is always healthy
    pProtocolStat->BusOff = 0;
    pProtocolStat->ErrorPassive = 0;
}
#endif // __linux__

/**
 * @brief Receive a CAN frame via SocketCAN or stub
 * @param rx_frame Pointer to frame structure to fill
 * @retval 0 if no frame available, 1 if frame received, negative on error
 */
int16_t canardSTM32Recieve(CanardCANFrame *const rx_frame) {
    if (rx_frame == NULL) {
        return -CANARD_ERROR_INVALID_ARGUMENT;
    }

    if (!driver_initialized) {
        return -CANARD_ERROR_INTERNAL;
    }

#ifdef __linux__
    if (can_mode == SITL_CAN_MODE_SOCKETCAN) {
        return sitlCANReceiveSocketCAN(rx_frame);
    }
#endif

    return sitlCANReceiveStub(rx_frame);
}

/**
 * @brief Transmit a CAN frame via SocketCAN or stub
 * @param tx_frame Pointer to frame to transmit
 * @retval 1 on success, 0 if busy, negative on error
 */
int16_t canardSTM32Transmit(const CanardCANFrame* const tx_frame) {
    if (tx_frame == NULL) {
        return -CANARD_ERROR_INVALID_ARGUMENT;
    }

    if (!driver_initialized) {
        return -CANARD_ERROR_INTERNAL;
    }

#ifdef __linux__
    if (can_mode == SITL_CAN_MODE_SOCKETCAN) {
        return sitlCANTransmitSocketCAN(tx_frame);
    }
#endif

    return sitlCANTransmitStub(tx_frame);
}

/**
 * @brief Get CAN protocol status
 * @param pProtocolStat Pointer to status structure to fill
 */
void canardSTM32GetProtocolStatus(canardProtocolStatus_t *pProtocolStat) {
    if (pProtocolStat == NULL) {
        return;
    }

#ifdef __linux__
    if (can_mode == SITL_CAN_MODE_SOCKETCAN) {
        sitlCANGetStatsSocketCAN(pProtocolStat);
        return;
    }
#endif

    sitlCANGetStatsStub(pProtocolStat);
}

/**
 * @brief Get RX FIFO fill level
 * @retval Number of frames in RX FIFO
 */
int32_t canardSTM32GetRxFifoFillLevel(void) {
#ifdef __linux__
    if (can_mode == SITL_CAN_MODE_SOCKETCAN && can_socket >= 0) {
        int available;
        if (ioctl(can_socket, FIONREAD, &available) == 0) {
            return available / sizeof(struct can_frame);
        }
    }
#endif

    return 0;
}

/**
 * @brief Recover from bus-off condition
 */
void canardSTM32RecoverFromBusOff(void) {
    // For SocketCAN, interface recovery is handled by the kernel
    // For stub, nothing to do
}

/**
 * @brief Get unique ID for this node
 * @param id 16-byte buffer to fill with unique ID
 */
void canardSTM32GetUniqueID(uint8_t id[16]) {
    if (id == NULL) {
        return;
    }

    memset(id, 0, 16);

    // "SITL" marker in first 4 bytes
    id[0] = 'S';
    id[1] = 'I';
    id[2] = 'T';
    id[3] = 'L';

#ifdef __linux__
    // Add process ID for uniqueness between multiple SITL instances
    pid_t pid = getpid();
    id[4] = (pid >> 24) & 0xFF;
    id[5] = (pid >> 16) & 0xFF;
    id[6] = (pid >> 8) & 0xFF;
    id[7] = pid & 0xFF;

    // Add timestamp for additional uniqueness
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
        id[8] = (ts.tv_sec >> 24) & 0xFF;
        id[9] = (ts.tv_sec >> 16) & 0xFF;
        id[10] = (ts.tv_sec >> 8) & 0xFF;
        id[11] = ts.tv_sec & 0xFF;
    }
#endif
}

#endif // SITL_BUILD && USE_DRONECAN
