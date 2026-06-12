/*
 * canard_stm32_driver.c
 *
 *  Created on: Jul 8, 2024
 *      Author: Roni Kant
 */

#include "common/log.h"
#include "common/time.h"
#include "drivers/io.h"
#include "drivers/nvic.h"
#include "canard.h"
#include "canard_stm32_driver.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_def.h"
#include "stm32f7xx_hal_can.h"

#include <string.h>
#include <stdint.h>

#define RX_BUFFER_SIZE 32

struct Timings {
        uint16_t prescaler;
        uint8_t sjw;
        uint8_t bs1;
        uint8_t bs2;
};

typedef struct {
    CAN_RxHeaderTypeDef header;
    uint8_t data[8];
} RxFrame_t;

static struct RxBuffer_t {
    volatile uint8_t writeIndex;
    volatile uint8_t readIndex;
    RxFrame_t rxMsg[RX_BUFFER_SIZE];
} RxBuffer;

static bool canardSTM32ComputeTimings(const uint32_t target_bitrate, struct Timings*out_timings);
static void canardSTM32GPIO_Init(void);
static int8_t rxBufferPushFrame(struct RxBuffer_t *rxBuf, RxFrame_t *rxMsg);
static int8_t rxBufferPopFrame(struct RxBuffer_t *rxBuf, RxFrame_t *rxMsg);
static uint8_t rxBufferNumMessages(struct RxBuffer_t *rxBuf);

static CAN_HandleTypeDef hcan1;

// ---- Public API -------------------------------------------------------------

/**
  * @brief FDCAN1 Initialization Function
  * @param  hfdcan pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  bitrate desired bitrate to run the CAN network at.
  * @retval ret == 1: OK, ret < 0: CANARD_ERROR, ret == 0: Check hfdcan->ErrorCode
  */
int16_t canardSTM32CAN1_Init(uint32_t bitrate)
{
    struct Timings out_timings;

    __HAL_RCC_CAN1_CLK_ENABLE();

    CAN_FilterTypeDef sFilterConfig;
    sFilterConfig.FilterIdHigh = 0;
    sFilterConfig.FilterIdLow  = 0;
    sFilterConfig.FilterMaskIdHigh = 0;
    sFilterConfig.FilterMaskIdLow = 0;
    sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterActivation = ENABLE;

    hcan1.Instance = CAN1;
    hcan1.Init.Mode = CAN_MODE_NORMAL;
    hcan1.Init.TimeTriggeredMode = DISABLE;
    hcan1.Init.AutoBusOff = ENABLE;
    hcan1.Init.AutoWakeUp = DISABLE;
    hcan1.Init.AutoRetransmission = DISABLE;  // ENABLE fills the TX FIFO on a degraded bus; DroneCAN reliability is handled at the application layer
    hcan1.Init.ReceiveFifoLocked = DISABLE;
    hcan1.Init.TransmitFifoPriority = DISABLE;

    if (!canardSTM32ComputeTimings(bitrate, &out_timings))
    {
        LOG_ERROR(CAN, "Failed to compute CAN timings for bitrate %lu", (unsigned long)bitrate);
        return -CANARD_ERROR_INTERNAL;
    }

    hcan1.Init.Prescaler = out_timings.prescaler;
    hcan1.Init.SyncJumpWidth = (uint32_t)out_timings.sjw << CAN_BTR_SJW_Pos;
    hcan1.Init.TimeSeg1     = (uint32_t)out_timings.bs1 << CAN_BTR_TS1_Pos;
    hcan1.Init.TimeSeg2     = (uint32_t)out_timings.bs2 << CAN_BTR_TS2_Pos;
    LOG_DEBUG(CAN, "Prescaler: %d, SJW: %d, BS1: %d, BS2: %d", out_timings.prescaler, out_timings.sjw, out_timings.bs1, out_timings.bs2);

    canardSTM32GPIO_Init();
    if (HAL_CAN_Init(&hcan1) != HAL_OK)
    {
        LOG_ERROR(CAN, "Failed CAN Init");
        return -CANARD_ERROR_INTERNAL;
    }

    if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK) {
        LOG_ERROR(CAN, "Failed Config Filter");
        return -CANARD_ERROR_INTERNAL;
    }

    if (HAL_CAN_Start(&hcan1) != HAL_OK) {
        LOG_ERROR(CAN, "Failed to Start");
        return -CANARD_ERROR_INTERNAL;
    }

    if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {  // persistent Rx notification
        LOG_ERROR(CAN, "Failed to activate interrupt");
        return -CANARD_ERROR_INTERNAL;
    }

    // Enable interrupt only after all initialization succeeds
    // (if any previous step failed, we return early without enabling IRQ)
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, NVIC_PRIO_CAN, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
    if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK) {
        LOG_ERROR(CAN, "Failed to activate TX interrupt");
        return -CANARD_ERROR_INTERNAL;
    }
    HAL_NVIC_SetPriority(CAN1_TX_IRQn, NVIC_PRIO_CAN, 0);
    HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);

    return CANARD_OK;
}

/**
  * @brief  Process CAN message from RxLocation FIFO into rx_frame
  * @param  rx_frame pointer to a CanardCANFrame structure where the received CAN message will be
  * 		stored.
  * @retval ret == 1: OK, ret < 0: CANARD_ERROR, ret == 0: Check hfdcan->ErrorCode
  */
int16_t canardSTM32Receive(CanardCANFrame *const rx_frame) {
    RxFrame_t canRxFrame;

    if (rx_frame == NULL) {
		return -CANARD_ERROR_INVALID_ARGUMENT;
	}

	if (rxBufferPopFrame(&RxBuffer, &canRxFrame) == 0) {  // Wheres the data?
        rx_frame->id = canRxFrame.header.ExtId;

		// Process ID to canard format
		if (canRxFrame.header.IDE == CAN_ID_EXT) { // canard will only process the message if it is extended ID
            rx_frame->id |= CANARD_CAN_FRAME_EFF;
		}

		if (canRxFrame.header.RTR == CAN_RTR_REMOTE) { // canard won't process the message if it is a remote frame
			rx_frame->id |= CANARD_CAN_FRAME_RTR;
		}

		rx_frame->data_len = canRxFrame.header.DLC;
		memcpy(rx_frame->data, canRxFrame.data, canRxFrame.header.DLC);

		// assume a single interface
		rx_frame->iface_id = 0;
		return 1;
	}
	// Either no CAN msg to be read, or an error that can be read from hfdcan->ErrorCode
	return 0;
}

/**
  * @brief  Process tx_frame CAN message into Tx FIFO/Queue and transmit it
  * @param  tx_frame pointer to a CanardCANFrame structure that contains the CAN message to
  * 		transmit.
  * @retval ret == 1: OK, ret < 0: CANARD_ERROR, ret == 0: Check hfdcan->ErrorCode
  */
int16_t canardSTM32Transmit(const CanardCANFrame* const tx_frame) {
	CAN_TxHeaderTypeDef txHeader = {};
    uint8_t txData[8];
    uint32_t txMailbox;
    uint32_t returnCode;

    if (tx_frame == NULL) {
		return -CANARD_ERROR_INVALID_ARGUMENT;
	}

	if (tx_frame->id & CANARD_CAN_FRAME_ERR) {
		return -CANARD_ERROR_INVALID_ARGUMENT; // unsupported frame format
	}

	// Process canard id to STM FDCAN header format
	if (tx_frame->id & CANARD_CAN_FRAME_EFF) {
		txHeader.IDE = CAN_ID_EXT;
		txHeader.ExtId = tx_frame->id & CANARD_CAN_EXT_ID_MASK;
	} else {
		txHeader.IDE = CAN_ID_STD;
		txHeader.StdId = tx_frame->id & CANARD_CAN_STD_ID_MASK;
	}

	txHeader.DLC = tx_frame->data_len;

	if (tx_frame->id & CANARD_CAN_FRAME_RTR) {
		txHeader.RTR = CAN_RTR_REMOTE;
	} else {
		txHeader.RTR = CAN_RTR_DATA;
	}

	memcpy(txData, tx_frame->data, tx_frame->data_len);

	returnCode = HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, &txMailbox);
    if( returnCode == HAL_OK) {
		return 1;
	}

	// TX failed (mailboxes full or bus error) - return 0 to signal retry needed
	return 0;
}

void canardSTM32GetProtocolStatus(canardProtocolStatus_t *pProtocolStat){
    uint32_t esr = hcan1.Instance->ESR;
    pProtocolStat->BusOff       = __HAL_CAN_GET_FLAG(&hcan1, CAN_FLAG_BOF);
    pProtocolStat->ErrorPassive = __HAL_CAN_GET_FLAG(&hcan1, CAN_FLAG_EPV);
    pProtocolStat->tec          = (uint8_t)((esr >> 16) & 0xFF);
    pProtocolStat->rec          = (uint8_t)((esr >> 24) & 0xFF);
    pProtocolStat->lec          = (uint8_t)((esr >> 4) & 0x07);
}

int32_t canardSTM32GetTxQueueFillLevel(void){
    return 0;
}

int32_t canardSTM32GetRxFifoFillLevel(void){
    return rxBufferNumMessages(&RxBuffer);
}

void canardSTM32RecoverFromBusOff(void){
    // No-op: ABOM (CAN_MCR bit 6) is set in canardSTM32CAN1_Init, so hardware
    // manages the full bus-off recovery sequence automatically. After 128x11
    // recessive bits, hardware cycles INRQ and clears ESR.BOFF without software
    // intervention. See RM0410 ss40.7.6 and CAN_MCR.ABOM, CAN_ESR.BOFF.
}

/*
  get a 16 byte unique ID for this node, this should be based on the CPU unique ID or other unique ID
 */
void canardSTM32GetUniqueID(uint8_t id[16]) {
    uint32_t HALUniqueIDs[3];
    // Make Unique ID out of the 96-bit STM32 UID and fill the rest with 0s
    memset(id, 0, 16);
    HALUniqueIDs[0] = *(uint32_t *)UID_BASE;
    HALUniqueIDs[1] = *(uint32_t *)(UID_BASE + 4);
    HALUniqueIDs[2] = *(uint32_t *)(UID_BASE + 8);
    memcpy(id, HALUniqueIDs, 12);
}

// ---- ISR / HAL callbacks ----------------------------------------------------

void CAN1_RX0_IRQHandler(void) {
      HAL_CAN_IRQHandler(&hcan1);
}

void CAN1_TX_IRQHandler(void) {
    HAL_CAN_IRQHandler(&hcan1);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    RxFrame_t frame;
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &frame.header, frame.data) == HAL_OK) {
        if (rxBufferPushFrame(&RxBuffer, &frame) != 0) {
            LOG_WARNING(CAN, "RX buffer full, frame dropped");
        }
    }
}

// ---- Private helpers --------------------------------------------------------

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
    IOConfigGPIOAF(IOGetByTag(IO_TAG(CAN1_TX)), IOCFG_AF_PP, GPIO_AF9_CAN1);
    IOInit(IOGetByTag(IO_TAG(CAN1_RX)), OWNER_DRONECAN, RESOURCE_CAN_RX, 0);
    IOConfigGPIOAF(IOGetByTag(IO_TAG(CAN1_RX)), IOCFG_AF_PP, GPIO_AF9_CAN1);
#endif


 #ifdef CAN1_STANDBY
    // Initialize the standby or listen only pin.  Set default state to enable CAN.
    // TODO: Tie the pin state to a configuration option so we can turn CAN on and off.

    IOInit(IOGetByTag(IO_TAG(CAN1_STANDBY)), OWNER_DRONECAN, RESOURCE_CAN_STANDBY, 0);
    IOConfigGPIO(IOGetByTag(IO_TAG(CAN1_STANDBY)), IOCFG_OUT_PP);
    IOLo(IOGetByTag(IO_TAG(CAN1_STANDBY)));
#endif
}

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
    const int max_quanta_per_bit = (target_bitrate >= 1000000) ? 10 : 18;
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
    if ((prescaler < 1U) || (prescaler > 1024U)) {
        return false;              // No solution
    }

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

    out_timings->prescaler = (uint16_t)(prescaler);
    out_timings->sjw = 3;  // Register value: hardware SJW = sjw+1 = 4 tq. F7 bxCAN needs wider SJW than H7 FDCAN.
    out_timings->bs1 = (uint8_t)(solution.bs1)-1;  // The HAL does not take care of the 1 bs offset in the register so remove it here like AP does.
    out_timings->bs2 = (uint8_t)(solution.bs2)-1;  // The HAL does not take care of the 1 bs offset in the register so remove it here like AP does.

    return true;
}

static int8_t rxBufferPushFrame(struct RxBuffer_t *rxBuf, RxFrame_t *rxMsg) {
    uint8_t next;
    RxFrame_t *pCurrentRxMsg;

    next = rxBuf->writeIndex + 1;
    if(next >= RX_BUFFER_SIZE){
        next = 0;
    }

    if(next == rxBuf->readIndex) {
        return -1;  // rxBuf is full
    }
    pCurrentRxMsg = &rxBuf->rxMsg[rxBuf->writeIndex];
    memcpy(pCurrentRxMsg, rxMsg, sizeof(RxFrame_t));
    rxBuf->writeIndex = next;
    return 0;
}

static int8_t rxBufferPopFrame(struct RxBuffer_t *rxBuf, RxFrame_t *rxMsg) {
    uint8_t next;
    RxFrame_t *pCurrentRxMsg;

    if(rxBuf->writeIndex == rxBuf->readIndex){
        return -1;  // Nothing to read
    }

    next = rxBuf->readIndex + 1;
    if (next >= RX_BUFFER_SIZE){
        next = 0;
    }
    pCurrentRxMsg = &rxBuf->rxMsg[rxBuf->readIndex];
    memcpy(rxMsg, pCurrentRxMsg, sizeof(RxFrame_t));
    rxBuf->readIndex = next;
    return 0;
}

static uint8_t rxBufferNumMessages(struct RxBuffer_t *rxBuf) {
    if(rxBuf->writeIndex < rxBuf->readIndex)
        return((rxBuf->writeIndex + RX_BUFFER_SIZE) - rxBuf->readIndex);

    return (rxBuf->writeIndex - rxBuf->readIndex);
}
