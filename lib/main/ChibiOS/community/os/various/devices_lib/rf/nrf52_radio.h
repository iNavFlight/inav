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
 * @brief Enhanced ShockBurst (ESB) is a basic protocol supporting two-way data
 *        packet communication including packet buffering, packet acknowledgment
 *        and automatic retransmission of lost packets.
 *
 * ported on: 25/10/2018, by andru
 *
 */

#ifndef NRF52_RADIO_H_
#define NRF52_RADIO_H_

// Hard coded parameters - change if necessary
#ifndef NRF52_MAX_PAYLOAD_LENGTH
#define NRF52_MAX_PAYLOAD_LENGTH            32                  /**< The max size of the payload. Valid values are 1 to 252 */
#endif

#define NRF52_CRC_RESET_VALUE             	0xFFFF              /**< CRC reset value*/

#define NRF52_TX_FIFO_SIZE                  8                   /**< The size of the transmission first in first out buffer. */
#define NRF52_RX_FIFO_SIZE                  8                   /**< The size of the reception first in first out buffer. */

#define NRF52_RADIO_USE_TIMER0            	FALSE               /**< TIMER0 will be used by the module. */
#define NRF52_RADIO_USE_TIMER1            	TRUE                /**< TIMER1 will be used by the module. */
#define NRF52_RADIO_USE_TIMER2            	FALSE               /**< TIMER2 will be used by the module. */
#define NRF52_RADIO_USE_TIMER3            	FALSE               /**< TIMER3 will be used by the module. */
#define NRF52_RADIO_USE_TIMER4            	FALSE               /**< TIMER4 will be used by the module. */

#define NRF52_RADIO_IRQ_PRIORITY			3                   /**< RADIO interrupt priority. */
#define NRF52_RADIO_INTTHD_PRIORITY         (NORMALPRIO+2)      /**< Interrupts handle thread priority. */
#define NRF52_RADIO_EVTTHD_PRIORITY         (NORMALPRIO+1)      /**< Events handle thread priority */

#define NRF52_RADIO_PPI_TIMER_START         10                  /**< The PPI channel used for timer start. */
#define NRF52_RADIO_PPI_TIMER_STOP          11                  /**< The PPI channel used for timer stop. */
#define NRF52_RADIO_PPI_RX_TIMEOUT          12                  /**< The PPI channel used for RX timeout. */
#define NRF52_RADIO_PPI_TX_START            13                  /**< The PPI channel used for starting TX. */


typedef enum {
	NRF52_SUCCESS,                                        /* Call was successful.                  */
	NRF52_INVALID_STATE,                                  /* Module is not initialized.            */
	NRF52_ERROR_BUSY,                                     /* Module was not in idle state.         */
	NRF52_ERROR_NULL,                                     /* Required parameter was NULL.          */
	NRF52_ERROR_INVALID_PARAM,                            /* Required parameter is invalid         */
	NRF52_ERROR_NOT_SUPPORTED,                            /* p_payload->noack was false while selective ack was not enabled. */
	NRF52_ERROR_INVALID_LENGTH,                           /* Payload length was invalid (zero or larger than max allowed).   */
} nrf52_error_t;

// Internal radio module state.
typedef enum {
    NRF52_STATE_UNINIT,                                   /**< Module not initialized. */
    NRF52_STATE_IDLE,                                     /**< Module idle. */
    NRF52_STATE_PTX_TX,                                   /**< Module transmitting without ack. */
    NRF52_STATE_PTX_TX_ACK,                               /**< Module transmitting with ack. */
    NRF52_STATE_PTX_RX_ACK,                               /**< Module transmitting with ack and reception of payload with the ack response. */
    NRF52_STATE_PRX,                                      /**< Module receiving packets without ack. */
    NRF52_STATE_PRX_SEND_ACK,                             /**< Module transmitting ack in RX mode. */
} nrf52_state_t;

/**@brief Events to indicate the last transmission/receiving status. */
typedef enum {
    NRF52_EVENT_TX_SUCCESS  = 0x01,   /**< Event triggered on TX success.     */
    NRF52_EVENT_TX_FAILED   = 0x02,   /**< Event triggered on TX failed.      */
    NRF52_EVENT_RX_RECEIVED = 0x04,   /**< Event triggered on RX Received.    */
} nrf52_event_t;

// Interrupt flags
typedef enum {
	NRF52_INT_TX_SUCCESS_MSK = 0x01,  /**< The flag used to indicate a success since last event. */
	NRF52_INT_TX_FAILED_MSK  = 0x02,  /**< The flag used to indicate a failiure since last event. */
	NRF52_INT_RX_DR_MSK 	 = 0x04,  /**< The flag used to indicate a received packet since last event. */
} nrf52_int_flags_t;

/**Macro to create initializer for a TX data packet.
 *
 * @details This macro generates an initializer. It is more efficient
 *          than setting the individual parameters dynamically.
 *
 * @param[in]   _pipe   The pipe to use for the data packet.
 * @param[in]   ...     Comma separated list of character data to put in the TX buffer.
 *                      Supported values are from 1 to 63 characters.
 *
 * @return  Initializer that sets up pipe, length and the byte array for content of the TX data.
 */
#define NRF52_CREATE_PAYLOAD(_pipe, ...)                                                  \
        {.pipe = _pipe, .length = NUM_VA_ARGS(__VA_ARGS__), .data = {__VA_ARGS__}};         \
        STATIC_ASSERT(NUM_VA_ARGS(__VA_ARGS__) > 0 && NUM_VA_ARGS(__VA_ARGS__) <= 63)

/**@brief Enhanced ShockBurst protocol. */
typedef enum {
    NRF52_PROTOCOL_ESB,      /*< Enhanced ShockBurst with fixed payload length.                                            */
    NRF52_PROTOCOL_ESB_DPL   /*< Enhanced ShockBurst with dynamic payload length.                                          */
} nrf52_protocol_t;

/**@brief Enhanced ShockBurst mode. */
typedef enum {
    NRF52_MODE_PTX,          /*< Primary transmitter mode. */
    NRF52_MODE_PRX           /*< Primary receiver mode.    */
} nrf52_mode_t;

/**@brief Enhanced ShockBurst bitrate mode. */
typedef enum {
    NRF52_BITRATE_2MBPS     = RADIO_MODE_MODE_Nrf_2Mbit,      /**< 2Mbit radio mode.                                             */
    NRF52_BITRATE_1MBPS     = RADIO_MODE_MODE_Nrf_1Mbit,      /**< 1Mbit radio mode.                                             */
} nrf52_bitrate_t;

/**@brief Enhanced ShockBurst CRC modes. */
typedef enum {
    NRF52_CRC_16BIT         = RADIO_CRCCNF_LEN_Two,                   /**< Use two byte CRC. */
    NRF52_CRC_8BIT          = RADIO_CRCCNF_LEN_One,                   /**< Use one byte CRC. */
    NRF52_CRC_OFF           = RADIO_CRCCNF_LEN_Disabled               /**< Disable CRC.      */
} nrf52_crc_t;

/**@brief Enhanced ShockBurst radio transmission power modes. */
typedef enum {
    NRF52_TX_POWER_4DBM     = RADIO_TXPOWER_TXPOWER_Pos4dBm,  /**< 4 dBm radio transmit power.   */
    NRF52_TX_POWER_0DBM     = RADIO_TXPOWER_TXPOWER_0dBm,     /**< 0 dBm radio transmit power.   */
    NRF52_TX_POWER_NEG4DBM  = RADIO_TXPOWER_TXPOWER_Neg4dBm,  /**< -4 dBm radio transmit power.  */
    NRF52_TX_POWER_NEG8DBM  = RADIO_TXPOWER_TXPOWER_Neg8dBm,  /**< -8 dBm radio transmit power.  */
    NRF52_TX_POWER_NEG12DBM = RADIO_TXPOWER_TXPOWER_Neg12dBm, /**< -12 dBm radio transmit power. */
    NRF52_TX_POWER_NEG16DBM = RADIO_TXPOWER_TXPOWER_Neg16dBm, /**< -16 dBm radio transmit power. */
    NRF52_TX_POWER_NEG20DBM = RADIO_TXPOWER_TXPOWER_Neg20dBm, /**< -20 dBm radio transmit power. */
    NRF52_TX_POWER_NEG30DBM = RADIO_TXPOWER_TXPOWER_Neg30dBm  /**< -30 dBm radio transmit power. */
} nrf52_tx_power_t;

/**@brief Enhanced ShockBurst transmission modes. */
typedef enum {
    NRF52_TXMODE_AUTO,        /*< Automatic TX mode - When the TX fifo is non-empty and the radio is idle packets will be sent automatically. */
    NRF52_TXMODE_MANUAL,      /*< Manual TX mode - Packets will not be sent until radio_start_tx() is called. Can be used to ensure consistent packet timing. */
    NRF52_TXMODE_MANUAL_START /*< Manual start TX mode - Packets will not be sent until radio_start_tx() is called, but transmission will continue automatically until the TX fifo is empty. */
} nrf52_tx_mode_t;

/**@brief Enhanced ShockBurst addresses.
 *
 * @details The module is able to transmit packets with the TX address stored in tx_address.
            The module can also receive packets from peers with up to eight different tx_addresses
            stored in esb_addr_p0 - esb_addr_p7. esb_addr_p0 can have 5 arbitrary bytes
            independent of the other addresses. esb_addr_p1 - esb_addr_p7 will share the
            same four byte base address found in the last four bytes of esb_addr_p1.
            They have an independent prefix byte found in esb_addr_p1[0] and esb_addr_p2 -
            esb_addr_p7.
*/
typedef struct {
    uint8_t base_addr_p0[4];        /**< Base address for pipe 0 encoded in big endian. */
    uint8_t base_addr_p1[4];        /**< Base address for pipe 1-7 encoded in big endian. */
    uint8_t pipe_prefixes[8];       /**< Address prefix for pipe P0 to P7. */
    uint8_t num_pipes;              /**< Number of pipes available. */
    uint8_t addr_length;            /**< Length of address including prefix */
    uint8_t rx_pipes;		        /**< Bitfield for enabled RX pipes. */
    uint8_t rf_channel;             /**< Which channel is to be used. Must be in range 0 and 125 to be valid. */
} nrf52_address_t;

/**@brief Enhanced ShockBurst payload.
 *
 * @note The payload is used both for transmission and receive with ack and payload.
*/
typedef struct
{
    uint8_t length;                              /**< Length of the packet. Should be equal or less than NRF_ESB_MAX_PAYLOAD_LENGTH. */
    uint8_t pipe;                                /**< Pipe used for this payload. */
    int8_t  rssi;                                /**< RSSI for received packet. */
    uint8_t noack;                               /**< Flag indicating that this packet will not be acknowledged. */
    uint8_t pid;                                 /**< PID assigned during communication. */
    uint8_t data[NRF52_MAX_PAYLOAD_LENGTH];      /**< The payload data. */
} nrf52_payload_t;

/**@brief Retransmit attempts delay and counter. */
typedef struct {
    uint16_t              delay;                  /**< The delay between each retransmission of unacked packets. */
    uint16_t              count;                  /**< The number of retransmissions attempts before transmission fail. */
} nrf52_retransmit_t;

/**@brief Main nrf_esb configuration struct. */
typedef struct {
    nrf52_protocol_t      protocol;               /**< Enhanced ShockBurst protocol. */
    nrf52_mode_t          mode;                   /**< Enhanced ShockBurst default RX or TX mode. */

    // General RF parameters
    nrf52_bitrate_t       bitrate;                /**< Enhanced ShockBurst bitrate mode. */
    nrf52_crc_t           crc;                    /**< Enhanced ShockBurst CRC mode. */
    nrf52_tx_power_t      tx_power;   		      /**< Enhanced ShockBurst radio transmission power mode.*/

    // Control settings
    nrf52_tx_mode_t       tx_mode;                /**< Enhanced ShockBurst transmit mode. */

    bool                  selective_auto_ack;     /**< Enable or disable selective auto acknowledgement. */

    nrf52_retransmit_t    retransmit;             /**< Packet retransmit parameters */

    uint8_t               payload_length;         /**< Enhanced ShockBurst static payload length */

    nrf52_address_t    	  address;                /**< Address parameters structure */
} nrf52_config_t;

typedef struct {
  /**
   * @brief NRF52 radio peripheral.
   */
  NRF_RADIO_Type          *radio;
  /**
   * @brief NRF52 timer peripheral.
   */
  NRF_TIMER_Type          *timer;
  /**
   * @brief Driver state.
   */
  nrf52_state_t           state;
  /**
   * @brief RF parameters.
   */
  nrf52_config_t          config;
  /**
   * @brief Interrupts flag.
   */
  nrf52_int_flags_t	      flags;
  /**
   * @brief TX attempt number.
   */
  uint16_t                tx_attempt;
  /**
   * @brief TX retransmits remaining.
   */
  uint16_t                tx_remaining;
  /**
   * @brief Radio events source.
   */
  event_source_t eventsrc;
} RFDriver;

extern RFDriver RFD1;

nrf52_error_t radio_init(nrf52_config_t const *config);
nrf52_error_t radio_disable(void);
nrf52_error_t radio_write_payload(nrf52_payload_t const * p_payload);
nrf52_error_t radio_read_rx_payload(nrf52_payload_t * p_payload);
nrf52_error_t radio_start_tx(void);
nrf52_error_t radio_start_rx(void);
nrf52_error_t radio_stop_rx(void);
nrf52_error_t radio_flush_tx(void);
nrf52_error_t radio_flush_rx(void);
nrf52_error_t radio_pop_tx(void);
nrf52_error_t radio_set_base_address_0(uint8_t const * p_addr);
nrf52_error_t radio_set_base_address_1(uint8_t const * p_addr);
nrf52_error_t radio_set_prefixes(uint8_t const * p_prefixes, uint8_t num_pipes);
nrf52_error_t radio_set_prefix(uint8_t pipe, uint8_t prefix);

#endif /* NRF52_RADIO_H_ */
