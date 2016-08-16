#include "common.h"
#include "obuffer.h"
#include "arraysize.h"

static uint8_t rxbuf[32];
static obuffer_t data rx;

static uint8_t txbuf[16];
static obuffer_t data tx;

void rs485_enable_tx(char enable);


void rs485_init(void) {
	obuffer_init(rx, rxbuf, ARRAYSIZE(rxbuf));
	obuffer_init(tx, txbuf, ARRAYSIZE(txbuf));

	TMOD = (TMOD & ~0xF0) | 0x20;	// set Timer1 mode2: 8-bit counter/timer with auto-reload
	CKCON |= 0x08;					// Timer1 clock = system clock
	TH1 = 43;						// UART baud rate = (49000000 / (256 - 43)) / 2 = 115023
	TR1 = 1;						// enable Timer1

	SCON0 = 0x30;	// 8bit, check for valid stop bit, reception enabled
	ES0 = 1;		// enable UART interrupt

	rs485_enable_tx(0);
}


char rs485_put(uint8_t value) {
	char res = 0;

	ES0 = 0;		// disable UART interrupt

	res = obuffer_put(tx, value);

	// if transmition is disabled (i.e. reception is enabled)
	// enable transmition and send the first byte
	if (REN0) {
		uint8_t t;
		if (obuffer_get(tx, t)) {
			rs485_enable_tx(1);
			SBUF0 = t;
		}
	}

	ES0 = 1;		// enable UART interrupt

	return res;
}


char rs485_get(uint8_t *value) {
	char res = 0;
	uint8_t t = 0;

	ES0 = 0;		// disable UART interrupt
	res = obuffer_get(rx, t);
	ES0 = 1;		// enable UART interrupt

	*value = t;
	return res;
}


void rs485_isr(void) UART_INTERRUPT_HANDLER {
	// byte received?
	if (RI0) {
		RI0 = 0;
		obuffer_put(rx, SBUF0);
	}

	// byte sent?
	if (TI0) {
		uint8_t t;
		TI0 = 0;
		if (obuffer_get(tx, t)) {
			SBUF0 = t;
		} else {
			rs485_enable_tx(0);
		}
	}
}


static void rs485_enable_tx(char enable) {
	if (enable) {
		REN0 = 0;	// disable reception
		RS485TX_ENABLE();
	} else {
		RS485TX_DISABLE();
		REN0 = 1;	// enable reception
	}
}
