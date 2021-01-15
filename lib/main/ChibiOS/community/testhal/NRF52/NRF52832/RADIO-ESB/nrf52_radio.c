/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 * Enhanced ShockBurst proprietary protocol to ChibiOS port
 *
 *  ported on: 25/10/2018, by andru
 *
 */

#include <stdint.h>
#include <string.h>

#include "ch.h"
#include "hal.h"

#include "nrf52_radio.h"


#define BIT_MASK_UINT_8(x) 					  (0xFF >> (8 - (x)))
#define NRF52_PIPE_COUNT 					  9

#define RADIO_SHORTS_COMMON ( RADIO_SHORTS_READY_START_Msk | RADIO_SHORTS_END_DISABLE_Msk | \
            RADIO_SHORTS_ADDRESS_RSSISTART_Msk | RADIO_SHORTS_DISABLED_RSSISTOP_Msk )

// Constant parameters
#define RX_WAIT_FOR_ACK_TIMEOUT_US_2MBPS      (48)        /**< 2MBit RX wait for ack timeout value. Smallest reliable value - 43 */
#define RX_WAIT_FOR_ACK_TIMEOUT_US_1MBPS      (64)        /**< 1MBit RX wait for ack timeout value. Smallest reliable value - 59 */

#define NRF52_ADDR_UPDATE_MASK_BASE0          (1 << 0)    /*< Mask value to signal updating BASE0 radio address. */
#define NRF52_ADDR_UPDATE_MASK_BASE1          (1 << 1)    /*< Mask value to signal updating BASE1 radio address. */
#define NRF52_ADDR_UPDATE_MASK_PREFIX         (1 << 2)    /*< Mask value to signal updating radio prefixes */

#define NRF52_PID_RESET_VALUE                 0xFF        /**< Invalid PID value which is guaranteed to not collide with any valid PID value. */
#define NRF52_PID_MAX                         3           /**< Maximum value for PID. */
#define NRF52_CRC_RESET_VALUE                 0xFFFF      /**< CRC reset value*/

#ifndef NRF52_RADIO_USE_TIMER0
#define NRF52_RADIO_USE_TIMER0                FALSE
#endif

#ifndef NRF52_RADIO_USE_TIMER1
#define NRF52_RADIO_USE_TIMER1                FALSE
#endif

#ifndef NRF52_RADIO_USE_TIMER2
#define NRF52_RADIO_USE_TIMER2                FALSE
#endif

#ifndef NRF52_RADIO_USE_TIMER3
#define NRF52_RADIO_USE_TIMER3                FALSE
#endif

#ifndef NRF52_RADIO_USE_TIMER4
#define NRF52_RADIO_USE_TIMER4                FALSE
#endif

#ifndef NRF52_RADIO_IRQ_PRIORITY
#define NRF52_RADIO_IRQ_PRIORITY			  3                   /**< RADIO interrupt priority. */
#endif

#ifndef NRF52_RADIO_PPI_TIMER_START
#error "PPI channel NRF52_RADIO_PPI_TIMER_START need to be defined"
#endif

#ifndef NRF52_RADIO_PPI_TIMER_STOP
#error "PPI channel NRF52_RADIO_PPI_TIMER_STOP need to be defined"
#endif

#ifndef NRF52_RADIO_PPI_RX_TIMEOUT
#error "PPI channel NRF52_RADIO_PPI_RX_TIMEOUT need to be defined"
#endif

#ifndef NRF52_RADIO_PPI_TX_START
#error "PPI channel NRF52_RADIO_PPI_TX_START need to be defined"
#endif

#if (NRF52_RADIO_USE_TIMER0 == FALSE) && (NRF52_RADIO_USE_TIMER1 == FALSE) && \
	(NRF52_RADIO_USE_TIMER2 == FALSE) && (NRF52_RADIO_USE_TIMER3 == FALSE) && \
	(NRF52_RADIO_USE_TIMER4 == FALSE)
#error "At least one hardware TIMER must be defined"
#endif

#ifndef NRF52_RADIO_INTTHD_PRIORITY
#error "Interrupt handle thread priority need to be defined"
#endif

#ifndef NRF52_RADIO_EVTTHD_PRIORITY
#error "Event thread priority need to be defined"
#endif

#define VERIFY_PAYLOAD_LENGTH(p)                            \
do                                                          \
{                                                           \
    if(p->length == 0 ||                                    \
       p->length > NRF52_MAX_PAYLOAD_LENGTH ||              \
       (RFD1.config.protocol == NRF52_PROTOCOL_ESB &&       \
        p->length > RFD1.config.payload_length))            \
    {                                                       \
        return NRF52_ERROR_INVALID_LENGTH;                  \
    }                                                       \
}while(0)

//Structure holding pipe info PID and CRC and ack payload.
typedef struct
{
    uint16_t            m_crc;
    uint8_t             m_pid;
    uint8_t             m_ack_payload;
} pipe_info_t;

// First in first out queue of payloads to be transmitted.
typedef struct
{
    nrf52_payload_t *   p_payload[NRF52_TX_FIFO_SIZE];      /**< Pointer to the actual queue. */
    uint32_t            entry_point;                        /**< Current start of queue. */
    uint32_t            exit_point;                         /**< Current end of queue. */
    uint32_t            count;                              /**< Current number of elements in the queue. */
} nrf52_payload_tx_fifo_t;

// First in first out queue of received payloads.
typedef struct
{
    nrf52_payload_t *   p_payload[NRF52_RX_FIFO_SIZE];      /**< Pointer to the actual queue. */
    uint32_t            entry_point;                        /**< Current start of queue. */
    uint32_t            exit_point;                         /**< Current end of queue. */
    uint32_t            count;                              /**< Current number of elements in the queue. */
} nrf52_payload_rx_fifo_t;

// These function pointers are changed dynamically, depending on protocol configuration and state.
//static void (*on_radio_end)(RFDriver *rfp) = NULL;
static void (*set_rf_payload_format)(RFDriver *rfp, uint32_t payload_length) = NULL;

// The following functions are assigned to the function pointers above.
static void on_radio_disabled_tx_noack(RFDriver *rfp);
static void on_radio_disabled_tx(RFDriver *rfp);
static void on_radio_disabled_tx_wait_for_ack(RFDriver *rfp);
static void on_radio_disabled_rx(RFDriver *rfp);
static void on_radio_disabled_rx_ack(RFDriver *rfp);

static volatile uint16_t wait_for_ack_timeout_us;
static nrf52_payload_t * p_current_payload;

// TX FIFO
static nrf52_payload_t            tx_fifo_payload[NRF52_TX_FIFO_SIZE];
static nrf52_payload_tx_fifo_t    tx_fifo;

// RX FIFO
static nrf52_payload_t            rx_fifo_payload[NRF52_RX_FIFO_SIZE];
static nrf52_payload_rx_fifo_t    rx_fifo;

// Payload buffers
static uint8_t                    tx_payload_buffer[NRF52_MAX_PAYLOAD_LENGTH + 2];
static uint8_t                    rx_payload_buffer[NRF52_MAX_PAYLOAD_LENGTH + 2];

static uint8_t                    pids[NRF52_PIPE_COUNT];
static pipe_info_t                rx_pipe_info[NRF52_PIPE_COUNT];

 // disable and events semaphores.
static binary_semaphore_t disable_sem;
static binary_semaphore_t events_sem;

RFDriver RFD1;

// Function to do bytewise bit-swap on a unsigned 32 bit value
static uint32_t bytewise_bit_swap(uint8_t const * p_inp) {
    uint32_t inp = (*(uint32_t*)p_inp);

    return __REV((uint32_t)__RBIT(inp)); //lint -esym(628, __rev) -esym(526, __rev) -esym(628, __rbit) -esym(526, __rbit) */
}

// Internal function to convert base addresses from nRF24L type addressing to nRF52 type addressing
static uint32_t addr_conv(uint8_t const* p_addr) {
    return __REV(bytewise_bit_swap(p_addr)); //lint -esym(628, __rev) -esym(526, __rev) */
}

static thread_t *rfEvtThread_p;
static THD_WORKING_AREA(waRFEvtThread, 64);
static THD_FUNCTION(rfEvtThread, arg) {
    (void)arg;

    chRegSetThreadName("rfevent");

    while (!chThdShouldTerminateX()) {
    	chBSemWait(&events_sem);

    	nrf52_int_flags_t interrupts = RFD1.flags;
        RFD1.flags = 0;

        if (interrupts & NRF52_INT_TX_SUCCESS_MSK) {
            chEvtBroadcastFlags(&RFD1.eventsrc, (eventflags_t) NRF52_EVENT_TX_SUCCESS);
        }
        if (interrupts & NRF52_INT_TX_FAILED_MSK) {
        	chEvtBroadcastFlags(&RFD1.eventsrc, (eventflags_t) NRF52_EVENT_TX_FAILED);
        }
        if (interrupts & NRF52_INT_RX_DR_MSK) {
        	chEvtBroadcastFlags(&RFD1.eventsrc, (eventflags_t) NRF52_EVENT_RX_RECEIVED);
        }
    }
    chThdExit((msg_t) 0);
}

static thread_t *rfIntThread_p;
static THD_WORKING_AREA(waRFIntThread, 64);
static THD_FUNCTION(rfIntThread, arg) {
    (void)arg;

    chRegSetThreadName("rfint");

    while (!chThdShouldTerminateX()) {
    	chBSemWait(&disable_sem);
    	switch (RFD1.state) {
    	  case NRF52_STATE_PTX_TX:
    		  on_radio_disabled_tx_noack(&RFD1);
    		  break;
    	  case NRF52_STATE_PTX_TX_ACK:
    		  on_radio_disabled_tx(&RFD1);
    		  break;
    	  case NRF52_STATE_PTX_RX_ACK:
    		  on_radio_disabled_tx_wait_for_ack(&RFD1);
    		  break;
    	  case NRF52_STATE_PRX:
    		  on_radio_disabled_rx(&RFD1);
    		  break;
    	  case NRF52_STATE_PRX_SEND_ACK:
    		  on_radio_disabled_rx_ack(&RFD1);
    		  break;
    	  default:
    		  break;
    	}
    }
	chThdExit((msg_t) 0);
}

static void serve_radio_interrupt(RFDriver *rfp) {
	(void) rfp;
    if ((NRF_RADIO->INTENSET & RADIO_INTENSET_READY_Msk) && NRF_RADIO->EVENTS_READY) {
        NRF_RADIO->EVENTS_READY = 0;
        (void) NRF_RADIO->EVENTS_READY;
    }
    if ((NRF_RADIO->INTENSET & RADIO_INTENSET_DISABLED_Msk) && NRF_RADIO->EVENTS_DISABLED) {
        NRF_RADIO->EVENTS_DISABLED = 0;
        (void) NRF_RADIO->EVENTS_DISABLED;
        chSysLockFromISR();
       	chBSemSignalI(&disable_sem);
       	chSysUnlockFromISR();
    }
}

/**
 * @brief   RADIO events interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector44) {

  OSAL_IRQ_PROLOGUE();

  serve_radio_interrupt(&RFD1);

  OSAL_IRQ_EPILOGUE();
}

static void set_rf_payload_format_esb_dpl(RFDriver *rfp, uint32_t payload_length) {
	(void)payload_length;
#if (NRF52_MAX_PAYLOAD_LENGTH <= 32)
    // Using 6 bits for length
    NRF_RADIO->PCNF0 = (0 << RADIO_PCNF0_S0LEN_Pos) |
                       (6 << RADIO_PCNF0_LFLEN_Pos) |
                       (3 << RADIO_PCNF0_S1LEN_Pos) ;
#else
    // Using 8 bits for length
    NRF_RADIO->PCNF0 = (0 << RADIO_PCNF0_S0LEN_Pos) |
                       (8 << RADIO_PCNF0_LFLEN_Pos) |
                       (3 << RADIO_PCNF0_S1LEN_Pos) ;
#endif
    NRF_RADIO->PCNF1 = (RADIO_PCNF1_WHITEEN_Disabled    << RADIO_PCNF1_WHITEEN_Pos) |
                       (RADIO_PCNF1_ENDIAN_Big          << RADIO_PCNF1_ENDIAN_Pos)  |
                       ((rfp->config.address.addr_length - 1)  << RADIO_PCNF1_BALEN_Pos)   |
                       (0                               << RADIO_PCNF1_STATLEN_Pos) |
                       (NRF52_MAX_PAYLOAD_LENGTH        << RADIO_PCNF1_MAXLEN_Pos);
}

static void set_rf_payload_format_esb(RFDriver *rfp, uint32_t payload_length) {
    NRF_RADIO->PCNF0 = (1 << RADIO_PCNF0_S0LEN_Pos) |
                       (0 << RADIO_PCNF0_LFLEN_Pos) |
                       (1 << RADIO_PCNF0_S1LEN_Pos);

    NRF_RADIO->PCNF1 = (RADIO_PCNF1_WHITEEN_Disabled    << RADIO_PCNF1_WHITEEN_Pos) |
                       (RADIO_PCNF1_ENDIAN_Big          << RADIO_PCNF1_ENDIAN_Pos)  |
                       ((rfp->config.address.addr_length - 1)  << RADIO_PCNF1_BALEN_Pos)   |
                       (payload_length                  << RADIO_PCNF1_STATLEN_Pos) |
                       (payload_length                  << RADIO_PCNF1_MAXLEN_Pos);
}

/* Set BASE0 and BASE1 addresses & prefixes registers
 *   NRF52 { prefixes[0], base0_addr[0], base0_addr[1], base0_addr[2], base0_addr[3] } ==
 *   NRF24 { addr[0], addr[1], addr[2], addr[3], addr[4] }
 */
static void set_addresses(RFDriver *rfp, uint8_t update_mask) {
    if (update_mask & NRF52_ADDR_UPDATE_MASK_BASE0) {
        NRF_RADIO->BASE0 = addr_conv(rfp->config.address.base_addr_p0);
        NRF_RADIO->DAB[0] = addr_conv(rfp->config.address.base_addr_p0);
    }

    if (update_mask & NRF52_ADDR_UPDATE_MASK_BASE1) {
        NRF_RADIO->BASE1 = addr_conv(rfp->config.address.base_addr_p1);
        NRF_RADIO->DAB[1] = addr_conv(rfp->config.address.base_addr_p1);
    }

    if (update_mask & NRF52_ADDR_UPDATE_MASK_PREFIX) {
        NRF_RADIO->PREFIX0 = bytewise_bit_swap(&rfp->config.address.pipe_prefixes[0]);
        NRF_RADIO->DAP[0] = bytewise_bit_swap(&rfp->config.address.pipe_prefixes[0]);
        NRF_RADIO->PREFIX1 = bytewise_bit_swap(&rfp->config.address.pipe_prefixes[4]);
        NRF_RADIO->DAP[1] = bytewise_bit_swap(&rfp->config.address.pipe_prefixes[4]);
    }
}

static void set_tx_power(RFDriver *rfp) {
    NRF_RADIO->TXPOWER = rfp->config.tx_power << RADIO_TXPOWER_TXPOWER_Pos;
}

static void set_bitrate(RFDriver *rfp) {
    NRF_RADIO->MODE = rfp->config.bitrate << RADIO_MODE_MODE_Pos;

    switch (rfp->config.bitrate) {
        case NRF52_BITRATE_2MBPS:
            wait_for_ack_timeout_us = RX_WAIT_FOR_ACK_TIMEOUT_US_2MBPS;
            break;
        case NRF52_BITRATE_1MBPS:
            wait_for_ack_timeout_us = RX_WAIT_FOR_ACK_TIMEOUT_US_1MBPS;
            break;
    }
}

static void set_protocol(RFDriver *rfp) {
    switch (rfp->config.protocol) {
        case NRF52_PROTOCOL_ESB_DPL:
            set_rf_payload_format = set_rf_payload_format_esb_dpl;
            break;
        case NRF52_PROTOCOL_ESB:
            set_rf_payload_format = set_rf_payload_format_esb;
            break;
    }
}

static void set_crc(RFDriver *rfp) {
    NRF_RADIO->CRCCNF = rfp->config.crc << RADIO_CRCCNF_LEN_Pos;

    if (rfp->config.crc == RADIO_CRCCNF_LEN_Two)
    {
        NRF_RADIO->CRCINIT = 0xFFFFUL;      // Initial value
        NRF_RADIO->CRCPOLY = 0x11021UL;     // CRC poly: x^16+x^12^x^5+1
    }
    else if (rfp->config.crc == RADIO_CRCCNF_LEN_One)
    {
        NRF_RADIO->CRCINIT = 0xFFUL;        // Initial value
        NRF_RADIO->CRCPOLY = 0x107UL;       // CRC poly: x^8+x^2^x^1+1
    }
}

static void ppi_init(RFDriver *rfp) {
    NRF_PPI->CH[NRF52_RADIO_PPI_TIMER_START].EEP = (uint32_t)&NRF_RADIO->EVENTS_READY;
    NRF_PPI->CH[NRF52_RADIO_PPI_TIMER_START].TEP = (uint32_t)&rfp->timer->TASKS_START;

    NRF_PPI->CH[NRF52_RADIO_PPI_TIMER_STOP].EEP  = (uint32_t)&NRF_RADIO->EVENTS_ADDRESS;
    NRF_PPI->CH[NRF52_RADIO_PPI_TIMER_STOP].TEP  = (uint32_t)&rfp->timer->TASKS_STOP;

    NRF_PPI->CH[NRF52_RADIO_PPI_RX_TIMEOUT].EEP  = (uint32_t)&rfp->timer->EVENTS_COMPARE[0];
    NRF_PPI->CH[NRF52_RADIO_PPI_RX_TIMEOUT].TEP  = (uint32_t)&NRF_RADIO->TASKS_DISABLE;

    NRF_PPI->CH[NRF52_RADIO_PPI_TX_START].EEP    = (uint32_t)&rfp->timer->EVENTS_COMPARE[1];
    NRF_PPI->CH[NRF52_RADIO_PPI_TX_START].TEP    = (uint32_t)&NRF_RADIO->TASKS_TXEN;
}

static void set_parameters(RFDriver *rfp) {
    set_tx_power(rfp);
    set_bitrate(rfp);
    set_protocol(rfp);
    set_crc(rfp);
    set_rf_payload_format(rfp, rfp->config.payload_length);
}

static void reset_fifo(void) {
    tx_fifo.entry_point = 0;
    tx_fifo.exit_point  = 0;
    tx_fifo.count       = 0;

    rx_fifo.entry_point = 0;
    rx_fifo.exit_point  = 0;
    rx_fifo.count       = 0;
}

static void init_fifo(void) {
	uint8_t i;
    reset_fifo();

    for (i = 0; i < NRF52_TX_FIFO_SIZE; i++) {
        tx_fifo.p_payload[i] = &tx_fifo_payload[i];
    }

    for (i = 0; i < NRF52_RX_FIFO_SIZE; i++) {
        rx_fifo.p_payload[i] = &rx_fifo_payload[i];
    }
}

static void tx_fifo_remove_last(void) {
    if (tx_fifo.count > 0) {
        nvicDisableVector(RADIO_IRQn);

        tx_fifo.count--;
        if (++tx_fifo.exit_point >= NRF52_TX_FIFO_SIZE) {
            tx_fifo.exit_point = 0;
        }

        nvicEnableVector(RADIO_IRQn, NRF52_RADIO_IRQ_PRIORITY);
    }
}

/** @brief  Function to push the content of the rx_buffer to the RX FIFO.
 *
 *  The module will point the register NRF_RADIO->PACKETPTR to a buffer for receiving packets.
 *  After receiving a packet the module will call this function to copy the received data to
 *  the RX FIFO.
 *
 *  @param  pipe Pipe number to set for the packet.
 *  @param  pid  Packet ID.
 *
 *  @retval true   Operation successful.
 *  @retval false  Operation failed.
 */
static bool rx_fifo_push_rfbuf(RFDriver *rfp, uint8_t pipe, uint8_t pid) {
    if (rx_fifo.count < NRF52_RX_FIFO_SIZE) {
        if (rfp->config.protocol == NRF52_PROTOCOL_ESB_DPL) {
            if (rx_payload_buffer[0] > NRF52_MAX_PAYLOAD_LENGTH) {
                return false;
            }

            rx_fifo.p_payload[rx_fifo.entry_point]->length = rx_payload_buffer[0];
        }
        else if (rfp->state == NRF52_STATE_PTX_RX_ACK) {
            // Received packet is an acknowledgment
            rx_fifo.p_payload[rx_fifo.entry_point]->length = 0;
        }
        else {
            rx_fifo.p_payload[rx_fifo.entry_point]->length = rfp->config.payload_length;
        }

        memcpy(rx_fifo.p_payload[rx_fifo.entry_point]->data, &rx_payload_buffer[2],
               rx_fifo.p_payload[rx_fifo.entry_point]->length);

        rx_fifo.p_payload[rx_fifo.entry_point]->pipe = pipe;
        rx_fifo.p_payload[rx_fifo.entry_point]->rssi = NRF_RADIO->RSSISAMPLE;
        rx_fifo.p_payload[rx_fifo.entry_point]->pid = pid;
        if (++rx_fifo.entry_point >= NRF52_RX_FIFO_SIZE) {
            rx_fifo.entry_point = 0;
        }
        rx_fifo.count++;

        return true;
    }

    return false;
}

static void timer_init(RFDriver *rfp) {
    // Configure the system timer with a 1 MHz base frequency
    rfp->timer->PRESCALER = 4;
    rfp->timer->BITMODE   = TIMER_BITMODE_BITMODE_16Bit;
    rfp->timer->SHORTS    = TIMER_SHORTS_COMPARE1_CLEAR_Msk | TIMER_SHORTS_COMPARE1_STOP_Msk;
}

static void start_tx_transaction(RFDriver *rfp) {
    bool ack;

    rfp->tx_attempt = 1;
    rfp->tx_remaining = rfp->config.retransmit.count;

    // Prepare the payload
    p_current_payload = tx_fifo.p_payload[tx_fifo.exit_point];

    // Handling ack if noack is set to false or if selctive auto ack is turned turned off
    ack = !p_current_payload->noack || !rfp->config.selective_auto_ack;

    switch (rfp->config.protocol) {
        case NRF52_PROTOCOL_ESB:
            set_rf_payload_format(rfp, p_current_payload->length);
            tx_payload_buffer[0] = p_current_payload->pid;
            tx_payload_buffer[1] = 0;
            memcpy(&tx_payload_buffer[2], p_current_payload->data, p_current_payload->length);

            NRF_RADIO->SHORTS   = RADIO_SHORTS_COMMON | RADIO_SHORTS_DISABLED_RXEN_Msk;
            NRF_RADIO->INTENSET = RADIO_INTENSET_DISABLED_Msk | RADIO_INTENSET_READY_Msk;

            // Configure the retransmit counter
            rfp->tx_remaining = rfp->config.retransmit.count;
            rfp->state = NRF52_STATE_PTX_TX_ACK;
            break;

        case NRF52_PROTOCOL_ESB_DPL:
            tx_payload_buffer[0] = p_current_payload->length;
            tx_payload_buffer[1] = p_current_payload->pid << 1;
            tx_payload_buffer[1] |= ack ? 0x00 : 0x01;
            memcpy(&tx_payload_buffer[2], p_current_payload->data, p_current_payload->length);

            if (ack) {
                NRF_RADIO->SHORTS   = RADIO_SHORTS_COMMON | RADIO_SHORTS_DISABLED_RXEN_Msk;
                NRF_RADIO->INTENSET = RADIO_INTENSET_DISABLED_Msk | RADIO_INTENSET_READY_Msk;

                // Configure the retransmit counter
                rfp->tx_remaining = rfp->config.retransmit.count;
                rfp->state = NRF52_STATE_PTX_TX_ACK;
            }
            else {
                NRF_RADIO->SHORTS   = RADIO_SHORTS_COMMON;
                NRF_RADIO->INTENSET = RADIO_INTENSET_DISABLED_Msk;
                rfp->state = NRF52_STATE_PTX_TX;
            }
            break;
    }

    NRF_RADIO->TXADDRESS    = p_current_payload->pipe;
    NRF_RADIO->RXADDRESSES  = 1 << p_current_payload->pipe;

    NRF_RADIO->FREQUENCY    = rfp->config.address.rf_channel;
    NRF_RADIO->PACKETPTR    = (uint32_t)tx_payload_buffer;

    NRF_RADIO->EVENTS_READY = 0;
    NRF_RADIO->EVENTS_DISABLED = 0;
    (void)NRF_RADIO->EVENTS_READY;
    (void)NRF_RADIO->EVENTS_DISABLED;

    nvicClearPending(RADIO_IRQn);
    nvicEnableVector(RADIO_IRQn, NRF52_RADIO_IRQ_PRIORITY);

    NRF_RADIO->TASKS_TXEN  = 1;
}

static void on_radio_disabled_tx_noack(RFDriver *rfp) {
    rfp->flags |= NRF52_INT_TX_SUCCESS_MSK;
    tx_fifo_remove_last();

	chBSemSignal(&events_sem);

	if (tx_fifo.count == 0) {
        rfp->state = NRF52_STATE_IDLE;
    }
    else {
        start_tx_transaction(rfp);
    }
}

static void on_radio_disabled_tx(RFDriver *rfp) {
    // Remove the DISABLED -> RXEN shortcut, to make sure the radio stays
    // disabled after the RX window
    NRF_RADIO->SHORTS = RADIO_SHORTS_COMMON;

    // Make sure the timer is started the next time the radio is ready,
    // and that it will disable the radio automatically if no packet is
    // received by the time defined in m_wait_for_ack_timeout_us
    rfp->timer->CC[0]    = wait_for_ack_timeout_us + 130;
    rfp->timer->CC[1]    = rfp->config.retransmit.delay - 130;
    rfp->timer->TASKS_CLEAR = 1;
    rfp->timer->EVENTS_COMPARE[0] = 0;
    rfp->timer->EVENTS_COMPARE[1] = 0;
    (void)rfp->timer->EVENTS_COMPARE[0];
    (void)rfp->timer->EVENTS_COMPARE[1];

    NRF_PPI->CHENSET = (1 << NRF52_RADIO_PPI_TIMER_START) |
                       (1 << NRF52_RADIO_PPI_RX_TIMEOUT) |
                       (1 << NRF52_RADIO_PPI_TIMER_STOP);
    NRF_PPI->CHENCLR = (1 << NRF52_RADIO_PPI_TX_START);

    NRF_RADIO->EVENTS_END = 0;
    (void)NRF_RADIO->EVENTS_END;

    if (rfp->config.protocol == NRF52_PROTOCOL_ESB) {
        set_rf_payload_format(rfp, 0);
    }

    NRF_RADIO->PACKETPTR = (uint32_t)rx_payload_buffer;
    rfp->state = NRF52_STATE_PTX_RX_ACK;
}

static void on_radio_disabled_tx_wait_for_ack(RFDriver *rfp) {
    // This marks the completion of a TX_RX sequence (TX with ACK)

    // Make sure the timer will not deactivate the radio if a packet is received
    NRF_PPI->CHENCLR = (1 << NRF52_RADIO_PPI_TIMER_START) |
                       (1 << NRF52_RADIO_PPI_RX_TIMEOUT)  |
                       (1 << NRF52_RADIO_PPI_TIMER_STOP);

    // If the radio has received a packet and the CRC status is OK
    if (NRF_RADIO->EVENTS_END && NRF_RADIO->CRCSTATUS != 0) {
        rfp->timer->TASKS_STOP = 1;
        NRF_PPI->CHENCLR = (1 << NRF52_RADIO_PPI_TX_START);
        rfp->flags |= NRF52_INT_TX_SUCCESS_MSK;
        rfp->tx_attempt++;// = rfp->config.retransmit.count - rfp->tx_remaining + 1;

        tx_fifo_remove_last();

        if (rfp->config.protocol != NRF52_PROTOCOL_ESB && rx_payload_buffer[0] > 0) {
            if (rx_fifo_push_rfbuf(rfp, (uint8_t)NRF_RADIO->TXADDRESS, 0)) {
                rfp->flags |= NRF52_INT_RX_DR_MSK;
            }
        }

    	chBSemSignal(&events_sem);

        if ((tx_fifo.count == 0) || (rfp->config.tx_mode == NRF52_TXMODE_MANUAL)) {
            rfp->state = NRF52_STATE_IDLE;
        }
        else {
            start_tx_transaction(rfp);
        }
    }
    else {
        if (rfp->tx_remaining-- == 0) {
            rfp->timer->TASKS_STOP = 1;
            NRF_PPI->CHENCLR = (1 << NRF52_RADIO_PPI_TX_START);
            // All retransmits are expended, and the TX operation is suspended
            rfp->tx_attempt = rfp->config.retransmit.count + 1;
            rfp->flags |= NRF52_INT_TX_FAILED_MSK;

            chBSemSignal(&events_sem);

            rfp->state = NRF52_STATE_IDLE;
        }
        else {
            // There are still have more retransmits left, TX mode should be
            // entered again as soon as the system timer reaches CC[1].
            NRF_RADIO->SHORTS = RADIO_SHORTS_COMMON | RADIO_SHORTS_DISABLED_RXEN_Msk;
            set_rf_payload_format(rfp, p_current_payload->length);
            NRF_RADIO->PACKETPTR = (uint32_t)tx_payload_buffer;
            rfp->state = NRF52_STATE_PTX_TX_ACK;
            rfp->timer->TASKS_START = 1;
            NRF_PPI->CHENSET = (1 << NRF52_RADIO_PPI_TX_START);
            if (rfp->timer->EVENTS_COMPARE[1])
                NRF_RADIO->TASKS_TXEN = 1;
        }
    }
}

static void clear_events_restart_rx(RFDriver *rfp) {
    NRF_RADIO->SHORTS = RADIO_SHORTS_COMMON;
    set_rf_payload_format(rfp, rfp->config.payload_length);
    NRF_RADIO->PACKETPTR = (uint32_t)rx_payload_buffer;

    NRF_RADIO->INTENCLR = RADIO_INTENCLR_DISABLED_Msk;
    NRF_RADIO->EVENTS_DISABLED = 0;
    (void) NRF_RADIO->EVENTS_DISABLED;

    NRF_RADIO->TASKS_DISABLE = 1;
    while (NRF_RADIO->EVENTS_DISABLED == 0);

    NRF_RADIO->EVENTS_DISABLED = 0;
    (void) NRF_RADIO->EVENTS_DISABLED;
    NRF_RADIO->INTENSET = RADIO_INTENSET_DISABLED_Msk;

    NRF_RADIO->SHORTS = RADIO_SHORTS_COMMON | RADIO_SHORTS_DISABLED_TXEN_Msk;
    NRF_RADIO->TASKS_RXEN = 1;
}

static void on_radio_disabled_rx(RFDriver *rfp) {
    bool            ack                = false;
    bool            retransmit_payload = false;
    bool            send_rx_event      = true;
    pipe_info_t *   p_pipe_info;

    if (NRF_RADIO->CRCSTATUS == 0) {
        clear_events_restart_rx(rfp);
        return;
    }

    if(rx_fifo.count >= NRF52_RX_FIFO_SIZE) {
        clear_events_restart_rx(rfp);
        return;
    }

    p_pipe_info = &rx_pipe_info[NRF_RADIO->RXMATCH];
    if (NRF_RADIO->RXCRC           == p_pipe_info->m_crc &&
       (rx_payload_buffer[1] >> 1) == p_pipe_info->m_pid  ) {
        retransmit_payload = true;
        send_rx_event = false;
    }

    p_pipe_info->m_pid = rx_payload_buffer[1] >> 1;
    p_pipe_info->m_crc = NRF_RADIO->RXCRC;

    if(rfp->config.selective_auto_ack == false || ((rx_payload_buffer[1] & 0x01) == 0))
        ack = true;

    if(ack) {
        NRF_RADIO->SHORTS = RADIO_SHORTS_COMMON | RADIO_SHORTS_DISABLED_RXEN_Msk;

        switch(rfp->config.protocol) {
            case NRF52_PROTOCOL_ESB_DPL:
                {
                    if (tx_fifo.count > 0 &&
                        (tx_fifo.p_payload[tx_fifo.exit_point]->pipe == NRF_RADIO->RXMATCH))
                    {
                        // Pipe stays in ACK with payload until TX fifo is empty
                        // Do not report TX success on first ack payload or retransmit
                        if (p_pipe_info->m_ack_payload != 0 && !retransmit_payload) {
                            if(++tx_fifo.exit_point >= NRF52_TX_FIFO_SIZE) {
                                tx_fifo.exit_point = 0;
                            }

                            tx_fifo.count--;

                            // ACK payloads also require TX_DS
                            // (page 40 of the 'nRF24LE1_Product_Specification_rev1_6.pdf').
                            rfp->flags |= NRF52_INT_TX_SUCCESS_MSK;
                        }

                        p_pipe_info->m_ack_payload = 1;

                        p_current_payload = tx_fifo.p_payload[tx_fifo.exit_point];

                        set_rf_payload_format(rfp, p_current_payload->length);
                        tx_payload_buffer[0] = p_current_payload->length;
                        memcpy(&tx_payload_buffer[2],
                               p_current_payload->data,
                               p_current_payload->length);
                    }
                    else {
                        p_pipe_info->m_ack_payload = 0;
                        set_rf_payload_format(rfp, 0);
                        tx_payload_buffer[0] = 0;
                    }

                    tx_payload_buffer[1] = rx_payload_buffer[1];
                }
                break;

            case NRF52_PROTOCOL_ESB:
                {
                    set_rf_payload_format(rfp, 0);
                    tx_payload_buffer[0] = rx_payload_buffer[0];
                    tx_payload_buffer[1] = 0;
                }
                break;
        }

        rfp->state = NRF52_STATE_PRX_SEND_ACK;
        NRF_RADIO->TXADDRESS = NRF_RADIO->RXMATCH;
        NRF_RADIO->PACKETPTR = (uint32_t)tx_payload_buffer;
    }
    else {
        clear_events_restart_rx(rfp);
    }

    if (send_rx_event) {
        // Push the new packet to the RX buffer and trigger a received event if the operation was
        // successful.
        if (rx_fifo_push_rfbuf(rfp, NRF_RADIO->RXMATCH, p_pipe_info->m_pid)) {
            rfp->flags |= NRF52_INT_RX_DR_MSK;
            chBSemSignal(&events_sem);
        }
    }
}

static void on_radio_disabled_rx_ack(RFDriver *rfp) {
    NRF_RADIO->SHORTS = RADIO_SHORTS_COMMON | RADIO_SHORTS_DISABLED_TXEN_Msk;
    set_rf_payload_format(rfp, rfp->config.payload_length);

    NRF_RADIO->PACKETPTR = (uint32_t)rx_payload_buffer;

    rfp->state = NRF52_STATE_PRX;
}

nrf52_error_t radio_disable(void) {
    RFD1.state = NRF52_STATE_IDLE;

    // Clear PPI
    NRF_PPI->CHENCLR = (1 << NRF52_RADIO_PPI_TIMER_START) |
                       (1 << NRF52_RADIO_PPI_TIMER_STOP)  |
                       (1 << NRF52_RADIO_PPI_RX_TIMEOUT);

    reset_fifo();

    memset(rx_pipe_info, 0, sizeof(rx_pipe_info));
    memset(pids, 0, sizeof(pids));

    // Disable the radio
    NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos |
                        RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos;

    nvicDisableVector(RADIO_IRQn);

    // Terminate interrupts handle thread
    chThdTerminate(rfIntThread_p);
    chBSemSignal(&disable_sem);
    chThdWait(rfIntThread_p);

    // Terminate events handle thread
    chThdTerminate(rfEvtThread_p);
    RFD1.flags = 0;
    chBSemSignal(&events_sem);
    chThdWait(rfEvtThread_p);

    RFD1.state = NRF52_STATE_UNINIT;

    return NRF52_SUCCESS;
}

//
nrf52_error_t radio_init(nrf52_config_t const *config) {
	osalDbgAssert(config != NULL,
		"config must be defined");
	osalDbgAssert(&config->address != NULL,
		"address must be defined");
	osalDbgAssert(NRF52_RADIO_IRQ_PRIORITY <= 7,
		"wrong radio irq priority");

    if (RFD1.state != NRF52_STATE_UNINIT) {
    	nrf52_error_t err = radio_disable();
        if (err != NRF52_SUCCESS)
            return err;
    }

    RFD1.radio = NRF_RADIO;
	RFD1.config = *config;
    RFD1.flags    = 0;

    init_fifo();

#if NRF52_RADIO_USE_TIMER0
    RFD1.timer = NRF_TIMER0;
#endif
#if NRF52_RADIO_USE_TIMER1
    RFD1.timer = NRF_TIMER1;
#endif
#if NRF52_RADIO_USE_TIMER2
    RFD1.timer = NRF_TIMER2;
#endif
#if NRF52_RADIO_USE_TIMER3
    RFD1.timer = NRF_TIMER3;
#endif
#if NRF52_RADIO_USE_TIMER4
    RFD1.timer = NRF_TIMER4;
#endif

    set_parameters(&RFD1);

    set_addresses(&RFD1, NRF52_ADDR_UPDATE_MASK_BASE0);
    set_addresses(&RFD1, NRF52_ADDR_UPDATE_MASK_BASE1);
    set_addresses(&RFD1, NRF52_ADDR_UPDATE_MASK_PREFIX);

    ppi_init(&RFD1);
    timer_init(&RFD1);

    chBSemObjectInit(&disable_sem, TRUE);
    chBSemObjectInit(&events_sem, TRUE);

    chEvtObjectInit(&RFD1.eventsrc);

    // interrupt handle thread
    rfIntThread_p = chThdCreateStatic(waRFIntThread, sizeof(waRFIntThread),
    		NRF52_RADIO_INTTHD_PRIORITY, rfIntThread, NULL);

    // events handle thread
    rfEvtThread_p = chThdCreateStatic(waRFEvtThread, sizeof(waRFEvtThread),
    		NRF52_RADIO_EVTTHD_PRIORITY, rfEvtThread, NULL);

    nvicEnableVector(RADIO_IRQn, NRF52_RADIO_IRQ_PRIORITY);

    RFD1.state = NRF52_STATE_IDLE;

    return NRF52_SUCCESS;
}

nrf52_error_t radio_write_payload(nrf52_payload_t const * p_payload) {
    if (RFD1.state == NRF52_STATE_UNINIT)
    	return NRF52_INVALID_STATE;
    if(p_payload == NULL)
    	return NRF52_ERROR_NULL;
    VERIFY_PAYLOAD_LENGTH(p_payload);
    if (tx_fifo.count >= NRF52_TX_FIFO_SIZE)
    	return NRF52_ERROR_INVALID_LENGTH;

    if (RFD1.config.mode == NRF52_MODE_PTX &&
        p_payload->noack && !RFD1.config.selective_auto_ack )
    {
        return NRF52_ERROR_NOT_SUPPORTED;
    }

    nvicDisableVector(RADIO_IRQn);

    memcpy(tx_fifo.p_payload[tx_fifo.entry_point], p_payload, sizeof(nrf52_payload_t));

    pids[p_payload->pipe] = (pids[p_payload->pipe] + 1) % (NRF52_PID_MAX + 1);
    tx_fifo.p_payload[tx_fifo.entry_point]->pid = pids[p_payload->pipe];

    if (++tx_fifo.entry_point >= NRF52_TX_FIFO_SIZE) {
        tx_fifo.entry_point = 0;
    }

    tx_fifo.count++;

    nvicEnableVector(RADIO_IRQn, NRF52_RADIO_IRQ_PRIORITY);

    if (RFD1.config.mode == NRF52_MODE_PTX &&
        RFD1.config.tx_mode == NRF52_TXMODE_AUTO &&
        RFD1.state == NRF52_STATE_IDLE)
    {
        start_tx_transaction(&RFD1);
    }

    return NRF52_SUCCESS;
}

nrf52_error_t radio_read_rx_payload(nrf52_payload_t * p_payload) {
    if (RFD1.state == NRF52_STATE_UNINIT)
    	return NRF52_INVALID_STATE;
    if (p_payload == NULL)
    	return NRF52_ERROR_NULL;

    if (rx_fifo.count == 0) {
        return NRF52_ERROR_INVALID_LENGTH;
    }

    nvicDisableVector(RADIO_IRQn);

    p_payload->length = rx_fifo.p_payload[rx_fifo.exit_point]->length;
    p_payload->pipe   = rx_fifo.p_payload[rx_fifo.exit_point]->pipe;
    p_payload->rssi   = rx_fifo.p_payload[rx_fifo.exit_point]->rssi;
    p_payload->pid    = rx_fifo.p_payload[rx_fifo.exit_point]->pid;
    memcpy(p_payload->data, rx_fifo.p_payload[rx_fifo.exit_point]->data, p_payload->length);

    if (++rx_fifo.exit_point >= NRF52_RX_FIFO_SIZE) {
        rx_fifo.exit_point = 0;
    }

    rx_fifo.count--;

    nvicEnableVector(RADIO_IRQn, NRF52_RADIO_IRQ_PRIORITY);

    return NRF52_SUCCESS;
}

nrf52_error_t radio_start_tx(void) {
    if (RFD1.state != NRF52_STATE_IDLE)
    	return NRF52_ERROR_BUSY;

    if (tx_fifo.count == 0) {
        return NRF52_ERROR_INVALID_LENGTH;
    }

    start_tx_transaction(&RFD1);

    return NRF52_SUCCESS;
}

nrf52_error_t radio_start_rx(void) {
    if (RFD1.state != NRF52_STATE_IDLE)
    	return NRF52_ERROR_BUSY;

    NRF_RADIO->INTENCLR = 0xFFFFFFFF;
    NRF_RADIO->EVENTS_DISABLED = 0;
    (void) NRF_RADIO->EVENTS_DISABLED;

    NRF_RADIO->SHORTS      = RADIO_SHORTS_COMMON | RADIO_SHORTS_DISABLED_TXEN_Msk;
    NRF_RADIO->INTENSET    = RADIO_INTENSET_DISABLED_Msk;
    RFD1.state             = NRF52_STATE_PRX;

    NRF_RADIO->RXADDRESSES  = RFD1.config.address.rx_pipes;
    NRF_RADIO->FREQUENCY    = RFD1.config.address.rf_channel;
    NRF_RADIO->PACKETPTR    = (uint32_t)rx_payload_buffer;

    nvicClearPending(RADIO_IRQn);
    nvicEnableVector(RADIO_IRQn, NRF52_RADIO_IRQ_PRIORITY);

    NRF_RADIO->EVENTS_ADDRESS = 0;
    NRF_RADIO->EVENTS_PAYLOAD = 0;
    NRF_RADIO->EVENTS_DISABLED = 0;
    (void) NRF_RADIO->EVENTS_ADDRESS;
    (void) NRF_RADIO->EVENTS_PAYLOAD;
    (void) NRF_RADIO->EVENTS_DISABLED;

    NRF_RADIO->TASKS_RXEN  = 1;

    return NRF52_SUCCESS;
}

nrf52_error_t radio_stop_rx(void) {
    if (RFD1.state != NRF52_STATE_PRX) {
        return NRF52_INVALID_STATE;
    }

    NRF_RADIO->SHORTS = 0;
    NRF_RADIO->INTENCLR = 0xFFFFFFFF;
    NRF_RADIO->EVENTS_DISABLED = 0;
    (void) NRF_RADIO->EVENTS_DISABLED;
    NRF_RADIO->TASKS_DISABLE = 1;
    while (NRF_RADIO->EVENTS_DISABLED == 0);
    RFD1.state = NRF52_STATE_IDLE;

    return NRF52_SUCCESS;
}

nrf52_error_t radio_flush_tx(void) {
    if (RFD1.state == NRF52_STATE_UNINIT)
    	return NRF52_INVALID_STATE;

    nvicDisableVector(RADIO_IRQn);

    tx_fifo.count = 0;
    tx_fifo.entry_point = 0;
    tx_fifo.exit_point = 0;

    nvicEnableVector(RADIO_IRQn, NRF52_RADIO_IRQ_PRIORITY);

    return NRF52_SUCCESS;
}

nrf52_error_t radio_pop_tx(void) {
    if (RFD1.state == NRF52_STATE_UNINIT)
    	return NRF52_INVALID_STATE;
    if (tx_fifo.count == 0)
    	return NRF52_ERROR_INVALID_LENGTH;

    nvicDisableVector(RADIO_IRQn);

    if (++tx_fifo.entry_point >= NRF52_TX_FIFO_SIZE) {
        tx_fifo.entry_point = 0;
    }
    tx_fifo.count--;

    nvicEnableVector(RADIO_IRQn, NRF52_RADIO_IRQ_PRIORITY);

    return NRF52_SUCCESS;
}

nrf52_error_t radio_flush_rx(void) {
    if (RFD1.state == NRF52_STATE_UNINIT)
    	return NRF52_INVALID_STATE;

    nvicDisableVector(RADIO_IRQn);

    rx_fifo.count = 0;
    rx_fifo.entry_point = 0;
    rx_fifo.exit_point = 0;

    memset(rx_pipe_info, 0, sizeof(rx_pipe_info));

    nvicEnableVector(RADIO_IRQn, NRF52_RADIO_IRQ_PRIORITY);

    return NRF52_SUCCESS;
}

nrf52_error_t radio_set_base_address_0(uint8_t const * p_addr) {
    if (RFD1.state != NRF52_STATE_IDLE)
    	return NRF52_ERROR_BUSY;
    if (p_addr == NULL)
        return NRF52_ERROR_NULL;

    memcpy(RFD1.config.address.base_addr_p0, p_addr, 4);
    set_addresses(&RFD1, NRF52_ADDR_UPDATE_MASK_BASE0);

    return NRF52_SUCCESS;
}

nrf52_error_t radio_set_base_address_1(uint8_t const * p_addr) {
    if (RFD1.state != NRF52_STATE_IDLE)
    	return NRF52_ERROR_BUSY;
    if (p_addr == NULL)
        return NRF52_ERROR_NULL;

    memcpy(RFD1.config.address.base_addr_p1, p_addr, 4);
    set_addresses(&RFD1, NRF52_ADDR_UPDATE_MASK_BASE1);

    return NRF52_SUCCESS;
}

nrf52_error_t radio_set_prefixes(uint8_t const * p_prefixes, uint8_t num_pipes) {
    if (RFD1.state != NRF52_STATE_IDLE)
    	return NRF52_ERROR_BUSY;
    if (p_prefixes == NULL)
        return NRF52_ERROR_NULL;
    if (num_pipes > 8)
    	return NRF52_ERROR_INVALID_PARAM;

    memcpy(RFD1.config.address.pipe_prefixes, p_prefixes, num_pipes);
    RFD1.config.address.num_pipes = num_pipes;
    RFD1.config.address.rx_pipes = BIT_MASK_UINT_8(num_pipes);

    set_addresses(&RFD1, NRF52_ADDR_UPDATE_MASK_PREFIX);

    return NRF52_SUCCESS;
}

nrf52_error_t radio_set_prefix(uint8_t pipe, uint8_t prefix) {
    if (RFD1.state != NRF52_STATE_IDLE)
    	return NRF52_ERROR_BUSY;
    if (pipe > 8)
    	return NRF52_ERROR_INVALID_PARAM;

    RFD1.config.address.pipe_prefixes[pipe] = prefix;

    NRF_RADIO->PREFIX0 = bytewise_bit_swap(&RFD1.config.address.pipe_prefixes[0]);
    NRF_RADIO->PREFIX1 = bytewise_bit_swap(&RFD1.config.address.pipe_prefixes[4]);

    return NRF52_SUCCESS;
}
