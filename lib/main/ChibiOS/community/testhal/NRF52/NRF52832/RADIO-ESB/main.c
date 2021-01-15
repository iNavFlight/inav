#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "nrf52_radio.h"

static SerialConfig serial_config = {
    .speed   = 38400,
    .tx_pad  = UART_TX,
    .rx_pad  = UART_RX,
#if NRF5_SERIAL_USE_HWFLOWCTRL	== TRUE
    .rts_pad = UART_RTS,
    .cts_pad = UART_CTS,
#endif
};

static THD_WORKING_AREA(waLEDThread, 64);
static THD_FUNCTION(LEDThread, arg) {
    (void)arg;
    
    chRegSetThreadName("blinker");
    palSetPadMode(IOPORT1, LED1, PAL_MODE_OUTPUT_PUSHPULL);

    while (1) {
      palTogglePad(IOPORT1, LED1);
      chThdSleepMilliseconds(500);
    }
}

static nrf52_config_t radiocfg = {
	.protocol = NRF52_PROTOCOL_ESB_DPL,
	.mode = NRF52_MODE_PRX,
	.bitrate = NRF52_BITRATE_1MBPS,
	.crc = NRF52_CRC_8BIT,
	.tx_power = NRF52_TX_POWER_0DBM,
	.tx_mode = NRF52_TXMODE_MANUAL_START,
	.selective_auto_ack = false,
	.retransmit = { 1000, 3 },
	.payload_length = 0,
	.address = {
		.base_addr_p0 = { 0xF3, 0xF3, 0xF3, 0x01 },
		.base_addr_p1 = { 0x3F, 0x3F, 0x3F, 0x01 },
		.pipe_prefixes = { 0xF3, 0x3F, },
		.num_pipes = 2,
		.addr_length = 5,
		.rx_pipes = 1 << 0,
		.rf_channel = 1,
	},
};

static uint16_t cnt, fail_pkt, good_pkt;
static nrf52_payload_t tx_payload = {
	.pipe = 1,
};
static nrf52_payload_t rx_payload;

static THD_WORKING_AREA(waRadioThread, 256);
static THD_FUNCTION(RadioThread, arg) {
    (void)arg;

    event_listener_t el;
    chEvtRegisterMask(&RFD1.eventsrc, &el, EVENT_MASK(0));

    chRegSetThreadName("radio");

    while (1) {
    	chEvtWaitAny(EVENT_MASK(0));
    	eventflags_t flags = chEvtGetAndClearFlags(&el);
    	if (flags & NRF52_EVENT_TX_SUCCESS) {
        	radio_start_rx();
    		good_pkt++;
    	}
    	if (flags & NRF52_EVENT_TX_FAILED) {
        	radio_start_rx();
    		fail_pkt++;
    	}
    	if (flags & NRF52_EVENT_RX_RECEIVED) {
    		memset(rx_payload.data, 0, 32);
    		radio_read_rx_payload(&rx_payload);
    	}
    }
}

/**@brief Function for application main entry.
 */
int main(void) {
    
    halInit();
    chSysInit();

    sdStart(&SD1, &serial_config);

    chThdCreateStatic(waLEDThread, sizeof(waLEDThread), NORMALPRIO, LEDThread, NULL);
    chThdCreateStatic(waRadioThread, sizeof(waRadioThread), NORMALPRIO, RadioThread, NULL);

    radio_init(&radiocfg);
    radio_flush_tx();
    radio_flush_rx();
    radio_start_rx();

    cnt = good_pkt = fail_pkt = 0;

    while (true) {
    	memset(tx_payload.data, 0, 32);
    	sprintf((char*)tx_payload.data, "counter value=%d" , cnt++);
    	tx_payload.length = strlen((char *)tx_payload.data);
    	radio_stop_rx();
        radio_write_payload(&tx_payload);
    	radio_start_tx();
    	chprintf((BaseSequentialStream *)&SD1, "packets: good=%d, fail=%d, sent=%s\r\n", good_pkt, fail_pkt, tx_payload.data);
    	chThdSleepMilliseconds(500);
    	if (strlen((char*) rx_payload.data)) {
    		chprintf((BaseSequentialStream *)&SD1, "rssi=%d, received=%s\r\n",	rx_payload.rssi, rx_payload.data);
    		rx_payload.data[0] = 0;
    	}
    }
}

