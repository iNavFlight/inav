/*
 * canard_stm32_driver.c
 *
 *  Created on: Jul 8, 2024
 *      Author: Roni Kant
 */

#include "common/log.h"
#include "common/time.h"
#include "drivers/io.h"
#include "canard.h"
#include "canard_stm32_driver.h"

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_def.h"
#include "stm32h7xx_hal_fdcan.h"

#include <string.h>
#include <stdint.h>

/* --- Internal types and state --- */

struct Timings {
        uint16_t prescaler;
        uint8_t sjw;
        uint8_t bs1;
        uint8_t bs2;
};

static FDCAN_HandleTypeDef hfdcan1;

/* --- Initialization --- */

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void canardSTM32GPIO_Init(void)
{
   // Set up the Rx and Tx pins for CAN1 and if present, the standby or listen only pin.
#if defined(CAN1_TX) && defined(CAN1_RX)
    IOInit(IOGetByTag(IO_TAG(CAN1_TX)), OWNER_DRONECAN, RESOURCE_CAN_TX, 0);
    IOConfigGPIOAF(IOGetByTag(IO_TAG(CAN1_TX)), IOCFG_AF_PP, GPIO_AF9_FDCAN1);
    IOInit(IOGetByTag(IO_TAG(CAN1_RX)), OWNER_DRONECAN, RESOURCE_CAN_RX, 0);
    IOConfigGPIOAF(IOGetByTag(IO_TAG(CAN1_RX)), IOCFG_AF_PP, GPIO_AF9_FDCAN1);
#endif


 #ifdef CAN1_STANDBY
    // Initialize the standby or listen only pin.  Set default state to enable CAN.
    // TODO: Tie the pin state to a configuration option so we can turn CAN on and off.

    IOInit(IOGetByTag(IO_TAG(CAN1_STANDBY)), OWNER_DRONECAN, RESOURCE_CAN_STANDBY, 0);
    IOConfigGPIO(IOGetByTag(IO_TAG(CAN1_STANDBY)), IOCFG_OUT_PP);  // Do any boards use pullups, external/internal?
    IOLo(IOGetByTag(IO_TAG(CAN1_STANDBY)));
#endif
}

/**
  * @brief  Compute FDCAN bit-timing parameters (prescaler, SJW, BS1, BS2) for a
  *         given target bitrate, targeting an ~87.5% sample point location.
  * @param  target_bitrate  Desired CAN bus bitrate in bits per second.
  * @param  out_timings     Pointer to a Timings struct to receive the computed values.
  * @retval true on success, false if no valid timing solution exists for the bitrate.
  */
static bool canardSTM32ComputeTimings(const uint32_t target_bitrate, struct Timings *out_timings)
{
    if (target_bitrate < 1) {
        return false;
    }

    /*
     * Hardware configuration
     */
    const uint32_t pclk = HAL_RCC_GetPCLK1Freq();

    static const int MaxBS1 = 16;
    static const int MaxBS2 = 8;

    /*
     * Ref. "Automatic Baudrate Detection in CANopen Networks", U. Koppe, MicroControl GmbH & Co. KG
     *      CAN in Automation, 2003
     *
     * According to the source, optimal quanta per bit are:
     *   Bitrate        Optimal Maximum
     *   1000 kbps      8       10
     *   500  kbps      16      17
     *   250  kbps      16      17
     *   125  kbps      16      17
     */
    const int max_quanta_per_bit = (target_bitrate >= 1000000) ? 10 : 17;  // TODO: F7 uses 18 here — revisit both alongside SJW tuning
    LOG_DEBUG(CAN, "Baudrate: %lu", target_bitrate);
    LOG_DEBUG(CAN, "Max Quanta per bit: %i", max_quanta_per_bit);

    static const int MaxSamplePointLocation = 900;

    /*
     * Computing (prescaler * BS):
     *   BITRATE = 1 / (PRESCALER * (1 / PCLK) * (1 + BS1 + BS2))       -- See the Reference Manual
     *   BITRATE = PCLK / (PRESCALER * (1 + BS1 + BS2))                 -- Simplified
     * let:
     *   BS = 1 + BS1 + BS2                                             -- Number of time quanta per bit
     *   PRESCALER_BS = PRESCALER * BS
     * ==>
     *   PRESCALER_BS = PCLK / BITRATE
     */
    const uint32_t prescaler_bs = pclk / target_bitrate;
    LOG_DEBUG(CAN, "Prescaler BS product: %lu", prescaler_bs);
     /*
     * Searching for such prescaler value so that the number of quanta per bit is highest.
     */
    uint8_t bs1_bs2_sum = (uint8_t)(max_quanta_per_bit - 1);

    while ((prescaler_bs % (1 + bs1_bs2_sum)) != 0) {
        if (bs1_bs2_sum <= 2) {
            return false;          // No solution
        }
        bs1_bs2_sum--;
    }

    const uint32_t prescaler = prescaler_bs / (1 + bs1_bs2_sum);
    if ((prescaler < 1U) || (prescaler > 512U)) {  // FDCAN NBTP.NBRP is 9-bit: valid range 1-512
        return false;              // No solution
    }
    LOG_DEBUG(CAN, "Prescaler: %lu", prescaler);

      /*
     * Now we have a constraint: (BS1 + BS2) == bs1_bs2_sum.
     * We need to find the values so that the sample point is as close as possible to the optimal value.
     *
     *   Solve[(1 + bs1)/(1 + bs1 + bs2) == 7/8, bs2]  (* Where 7/8 is 0.875, the recommended sample point location *)
     *   {{bs2 -> (1 + bs1)/7}}
     *
     * Hence:
     *   bs2 = (1 + bs1) / 7
     *   bs1 = (7 * bs1_bs2_sum - 1) / 8
     *
     * Sample point location can be computed as follows:
     *   Sample point location = (1 + bs1) / (1 + bs1 + bs2)
     *
     * Since the optimal solution is so close to the maximum, we prepare two solutions, and then pick the best one:
     *   - With rounding to nearest
     *   - With rounding to zero
     */
    struct BsPair {
        uint8_t bs1;
        uint8_t bs2;
        uint16_t sample_point_permill;
    } solution;

    // First attempt with rounding to nearest
    solution.bs1 = (uint8_t)(((7 * bs1_bs2_sum - 1) + 4) / 8);
    solution.bs2 = (uint8_t)(bs1_bs2_sum - solution.bs1);
    solution.sample_point_permill = (uint16_t)(1000 * (1 + solution.bs1) / (1 + solution.bs1 + solution.bs2));

    if (solution.sample_point_permill > MaxSamplePointLocation) {
        // Second attempt with rounding to zero
        solution.bs1 = (uint8_t)((7 * bs1_bs2_sum - 1) / 8);
        solution.bs2 = (uint8_t)(bs1_bs2_sum - solution.bs1);
        solution.sample_point_permill = (uint16_t)(1000 * (1 + solution.bs1) / (1 + solution.bs1 + solution.bs2));
    }
     /*
     * Final validation
     * Helpful Python:
     * def sample_point_from_btr(x):
     *     assert 0b0011110010000000111111000000000 & x == 0
     *     ts2,ts1,brp = (x>>20)&7, (x>>16)&15, x&511
     *     return (1+ts1+1)/(1+ts1+1+ts2+1)
     *
     */
    if ((target_bitrate != (pclk / (prescaler * (1 + solution.bs1 + solution.bs2)))) || !((solution.bs1 >= 1) && (solution.bs1 <= MaxBS1) && (solution.bs2 >= 1) && (solution.bs2 <= MaxBS2)))
    {
        return false;
    }

    LOG_DEBUG(CAN, "Timings: quanta/bit: %d, sample point location: %f%%",
          (int)(1 + solution.bs1 + solution.bs2), (double)(solution.sample_point_permill) / (double)(10.0));

    out_timings->prescaler = (uint16_t)(prescaler);
    out_timings->sjw = 8;                        // Not happy with this value, but 1MBPs with unshielded cable?
    out_timings->bs1 = (uint8_t)(solution.bs1);  // The HAL takes care of the 1 bs offset in the register so don't remove it here like AP does.
    out_timings->bs2 = (uint8_t)(solution.bs2);  // The HAL takes care of the 1 bs offset in the register so don't remove it here like AP does.

    return true;
}

/**
  * @brief CAN1 Initialization Function
  * @param  bitrate desired bitrate to run the CAN network at.
  * @retval CANARD_OK (0) on success, negative CANARD_ERROR on failure.
  */
int16_t canardSTM32CAN1_Init(uint32_t bitrate)
{
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    struct Timings out_timings;

    FDCAN_FilterTypeDef sFilterConfig;
    sFilterConfig.IdType = FDCAN_EXTENDED_ID;
    sFilterConfig.FilterIndex = 0;
    sFilterConfig.FilterType = FDCAN_FILTER_DUAL;
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    sFilterConfig.FilterID1 = 0x0;
    sFilterConfig.FilterID2 = 0x1FFFFFFFU;  // FILTER_DUAL matches two IDs, but ConfigGlobalFilter below
                                             // accepts all non-matching frames — this filter is a no-op
    hfdcan1.Instance = FDCAN1;
    hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;  // Initialize in CAN2.0 mode not CAN_FD
    hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
    hfdcan1.Init.AutoRetransmission = DISABLE;
    hfdcan1.Init.TransmitPause = DISABLE;
    hfdcan1.Init.ProtocolException = DISABLE;

    if (!canardSTM32ComputeTimings(bitrate, &out_timings))
    {
        LOG_ERROR(CAN, "Failed to compute CAN timings for bitrate %lu", (unsigned long)bitrate);
        return -CANARD_ERROR_INTERNAL;
    }

    hfdcan1.Init.NominalPrescaler = out_timings.prescaler;
    hfdcan1.Init.NominalSyncJumpWidth = out_timings.sjw;
    hfdcan1.Init.NominalTimeSeg1 = out_timings.bs1;
    hfdcan1.Init.NominalTimeSeg2 = out_timings.bs2;
    LOG_DEBUG(CAN, "Prescaler: %d, SJW: %d, BS1: %d, BS2: %d", out_timings.prescaler, out_timings.sjw, out_timings.bs1, out_timings.bs2);

    hfdcan1.Init.RxFifo0ElmtsNbr = 30;
    hfdcan1.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
    hfdcan1.Init.RxBuffersNbr = 1;
    hfdcan1.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
    hfdcan1.Init.StdFiltersNbr = 0;
    hfdcan1.Init.ExtFiltersNbr = 1;
    hfdcan1.Init.TxFifoQueueElmtsNbr = 32;
    hfdcan1.Init.TxEventsNbr = 0;
    hfdcan1.Init.TxBuffersNbr = 5;
    hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
    hfdcan1.Init.TxElmtSize = FDCAN_DATA_BYTES_8;
    LOG_DEBUG(CAN, "In CAN Init");

    /** Initializes the peripherals clock
    */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      LOG_DEBUG(CAN, "Unable to configure peripheral clock");
      return -CANARD_ERROR_INTERNAL;
    }

    /* FDCAN1 clock enable */
    __HAL_RCC_FDCAN_CLK_ENABLE();

    canardSTM32GPIO_Init();  // Set up the pins for CAN and optional listen only mode

    if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
    {
        LOG_ERROR(CAN, "Failed CAN Init");
        return -CANARD_ERROR_INTERNAL;
    }
    if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK) {
        LOG_ERROR(CAN, "Failed Config Filter");
        return -CANARD_ERROR_INTERNAL;
    }
    if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK) {
        LOG_ERROR(CAN, "Failed to config FDCAN filter");
        return -CANARD_ERROR_INTERNAL;
    }

    if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) {
        LOG_ERROR(CAN, "Failed to Start");
        return -CANARD_ERROR_INTERNAL;
    }
    return CANARD_OK;
}

/* --- RX --- */

/**
  * @brief  Process CAN message from RxLocation FIFO into rx_frame
  * @param  hfdcan pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  RxLocation Location of the received message to be read.
  *         This parameter can be a value of @arg FDCAN_Rx_location.
  * @param  rx_frame pointer to a CanardCANFrame structure where the received CAN message will be
  * 		stored.
  * @retval ret == 1: OK, ret < 0: CANARD_ERROR, ret == 0: Check hfdcan->ErrorCode
  */
int16_t canardSTM32Recieve(CanardCANFrame *const rx_frame) {
	if (rx_frame == NULL) {
		return -CANARD_ERROR_INVALID_ARGUMENT;
	}

	FDCAN_RxHeaderTypeDef RxHeader;
	uint8_t RxData[8];

	if (HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK) {

		// Process ID to canard format
		rx_frame->id = RxHeader.Identifier;

		if (RxHeader.IdType == FDCAN_EXTENDED_ID) { // canard will only process the message if it is extended ID
			rx_frame->id |= CANARD_CAN_FRAME_EFF;
		}

		if (RxHeader.RxFrameType == FDCAN_REMOTE_FRAME) { // canard won't process the message if it is a remote frame
			rx_frame->id |= CANARD_CAN_FRAME_RTR;
		}

		rx_frame->data_len = RxHeader.DataLength;
		memcpy(rx_frame->data, RxData, RxHeader.DataLength);

		// assume a single interface
		rx_frame->iface_id = 0;

		return 1;
	}

	// Either no CAN msg to be read, or an error that can be read from hfdcan->ErrorCode
	return 0;
}

/* --- TX --- */

/**
  * @brief  Process tx_frame CAN message into Tx FIFO/Queue and transmit it
  * @param  tx_frame pointer to a CanardCANFrame structure that contains the CAN message to
  * 		transmit.
  * @retval ret == 1: OK, ret < 0: CANARD_ERROR, ret == 0: Check hfdcan->ErrorCode
  */
int16_t canardSTM32Transmit(const CanardCANFrame* const tx_frame) {
	if (tx_frame == NULL) {
		return -CANARD_ERROR_INVALID_ARGUMENT;
	}

	if (tx_frame->id & CANARD_CAN_FRAME_ERR) {
		return -CANARD_ERROR_INVALID_ARGUMENT; // unsupported frame format
	}

	FDCAN_TxHeaderTypeDef TxHeader;
	uint8_t TxData[8];

	// Process canard id to STM FDCAN header format
	if (tx_frame->id & CANARD_CAN_FRAME_EFF) {
		TxHeader.IdType = FDCAN_EXTENDED_ID;
		TxHeader.Identifier = tx_frame->id & CANARD_CAN_EXT_ID_MASK;
	} else {
		TxHeader.IdType = FDCAN_STANDARD_ID;
		TxHeader.Identifier = tx_frame->id & CANARD_CAN_STD_ID_MASK;
	}

	TxHeader.DataLength = tx_frame->data_len;

	if (tx_frame->id & CANARD_CAN_FRAME_RTR) {
		TxHeader.TxFrameType = FDCAN_REMOTE_FRAME;
	} else {
		TxHeader.TxFrameType = FDCAN_DATA_FRAME;
	}

	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE; // node is not in error-passive state; sender sets ESI to active
	TxHeader.BitRateSwitch = FDCAN_BRS_OFF; // Disabling FDCAN (using CAN 2.0)
	TxHeader.FDFormat = FDCAN_CLASSIC_CAN; // Disabling FDCAN (using CAN 2.0)
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS; // no TX event FIFO allocated; event tracking not needed
	TxHeader.MessageMarker = 0; // only meaningful when TX events are enabled; unused here
    memcpy(TxData, tx_frame->data, TxHeader.DataLength);

	if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData) == HAL_OK) {
		return 1;
	}

	LOG_DEBUG(CAN, "Failed at adding message with id: %lu to Tx Queue", TxHeader.Identifier);

	// This might be for many reasons including the Tx Fifo being full, the error can be read from hfdcan->ErrorCode
	return 0;
}

/* --- Diagnostics --- */

/**
  * @brief  Read FDCAN protocol status (bus-off, error passive) into a
  *         canardProtocolStatus_t struct.
  * @param  pProtocolStat  Pointer to the struct to populate with current status.
  */
void canardSTM32GetProtocolStatus(canardProtocolStatus_t *pProtocolStat){
	FDCAN_ProtocolStatusTypeDef protocolStatus = {};

    memset(pProtocolStat, 0, sizeof(*pProtocolStat));
    HAL_FDCAN_GetProtocolStatus(&hfdcan1, &protocolStatus);
    pProtocolStat->BusOff = protocolStatus.BusOff;
    pProtocolStat->ErrorPassive = protocolStatus.ErrorPassive;
    LOG_DEBUG(CAN, "BusOff: %lu", protocolStatus.BusOff);
    LOG_DEBUG(CAN, "ErrorPassive: %lu", protocolStatus.ErrorPassive);
}

/**
  * @brief  Return the current number of frames pending in the software TX queue.
  *         The H7 FDCAN driver transmits directly into the hardware TX FIFO with no
  *         intermediate software queue, so this always returns 0.
  * @retval 0 (no software TX queue on H7).
  */
int32_t canardSTM32GetTxQueueFillLevel(void) {
    return 0;
}

/**
  * @brief  Return the current number of frames waiting in the FDCAN RX FIFO0.
  * @retval Number of frames available to read.
  */
int32_t canardSTM32GetRxFifoFillLevel(void){
    return (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0));
}

/**
  * @brief  Recover the FDCAN peripheral from a bus-off condition by clearing
  *         the INIT bit in CCCR, which restarts the CAN participation.
  */
void canardSTM32RecoverFromBusOff(void){
    CLEAR_BIT(hfdcan1.Instance->CCCR, FDCAN_CCCR_INIT);  // Clear INIT bit to recover from Bus-Off
}

/**
  * @brief  Populate a 16-byte unique node ID from the STM32 96-bit factory UID.
  *         The 12 UID bytes are copied into the first 12 bytes; the remainder is zero-padded.
  * @param  id  Output buffer of exactly 16 bytes to receive the unique ID.
  */
void canardSTM32GetUniqueID(uint8_t id[16]) {
    uint32_t HALUniqueIDs[3];
    // Make Unique ID out of the 96-bit STM32 UID and fill the rest with 0s
    memset(id, 0, 16);
    HALUniqueIDs[0] = HAL_GetUIDw0();
    HALUniqueIDs[1] = HAL_GetUIDw1();
    HALUniqueIDs[2] = HAL_GetUIDw2();
    memcpy(id, HALUniqueIDs, 12);
}
