/*
 * canard_sitl_driver.c
 *
 * SITL CAN Driver - Stub implementation for Phase 2.1
 * Will be extended with SocketCAN in Phase 2.2
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

// Stub driver state (Phase 2.1 - no actual CAN communication)
static bool driver_initialized = false;

/**
 * @brief Initialize CAN interface (stub implementation)
 * @param bitrate CAN bitrate in bps
 * @retval 0 on success, negative on error
 */
int16_t canardSTM32CAN1_Init(uint32_t bitrate) {
    (void)bitrate; // Unused in stub implementation

    if (driver_initialized) {
        return 0; // Already initialized
    }

    LOG_DEBUG(CAN, "SITL DroneCAN driver initialized (stub mode)");
    driver_initialized = true;

    return 0; // Success
}

/**
 * @brief Receive a CAN frame (stub - always returns no data)
 * @param rx_frame Pointer to frame structure to fill
 * @retval 0 if no frame available, 1 if frame received, negative on error
 */
int16_t canardSTM32Recieve(CanardCANFrame *const rx_frame) {
    if (rx_frame == NULL) {
        return -CANARD_ERROR_INVALID_ARGUMENT;
    }

    // Stub: No frames to receive in Phase 2.1
    return 0; // No data available
}

/**
 * @brief Transmit a CAN frame (stub - discards frame)
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

    // Stub: Silently discard frame in Phase 2.1
    // TODO Phase 2.2: Send via SocketCAN

    return 1; // Success (frame "transmitted")
}

/**
 * @brief Get CAN protocol status (stub)
 * @param pProtocolStat Pointer to status structure to fill
 */
void canardSTM32GetProtocolStatus(canardProtocolStatus_t *pProtocolStat) {
    if (pProtocolStat == NULL) {
        return;
    }

    // Stub: Report healthy state
    pProtocolStat->BusOff = 0;
    pProtocolStat->ErrorPassive = 0;
}

/**
 * @brief Get RX FIFO fill level (stub - always empty)
 * @retval Number of frames in RX FIFO
 */
int32_t canardSTM32GetRxFifoFillLevel(void) {
    // Stub: No frames in FIFO
    return 0;
}

/**
 * @brief Recover from bus-off condition (stub - no-op)
 */
void canardSTM32RecoverFromBusOff(void) {
    // Stub: Nothing to recover in Phase 2.1
}

/**
 * @brief Get unique ID for this node (stub - generates pseudo-ID)
 * @param id 16-byte buffer to fill with unique ID
 */
void canardSTM32GetUniqueID(uint8_t id[16]) {
    if (id == NULL) {
        return;
    }

    // Stub: Generate a pseudo-unique ID for SITL
    // Use a simple pattern that identifies this as SITL
    memset(id, 0, 16);

    // "SITL" marker in first 4 bytes
    id[0] = 'S';
    id[1] = 'I';
    id[2] = 'T';
    id[3] = 'L';

    // TODO Phase 2.2: Could use hostname, PID, or other unique identifier
}

#endif // SITL_BUILD && USE_DRONECAN
