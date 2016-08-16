#include <stdint.h>
#include "common.h"

#define ADCCHN_UIN	0x0B
#define ADCCHN_POS	0x0C
#define ADCCHN_IOUT	0x0D

sfr16 ADC0 = 0xBD;		// not present in c8051f410.h

static uint8_t data ready = 0;

static uint16_t data posAcc = 0;
static uint16_t data ioutAcc = 0;
static uint16_t data uinAcc = 0;

#define ACC_BITS	3

#define ADC_SAMPLE_RATE 13000.0
#define RELOAD_VALUE (65536 - (uint16_t)(FOSC / ADC_SAMPLE_RATE))


// init ADC and timer3
void Adc_Init(void)
{
    REF0CN = 0x13;		// Vref = 2.2 V, Internal Analog Bias Generator on, Internal Reference Buffer enabled

    ADC0CF = 0x80;		// SAR clock = 2.88 MHz
    ADC0CN = 0x81;		// enabled, conversion initiated on overflow of Timer 3
	ADC0MX = ADCCHN_POS;

	CKCON = (CKCON & ~0xC0) | 0x40;		// Timer3 clock = System clock
	TMR3CN = 0x04;		// Timer3 enabled (16 bit)
	TMR3RLH = RELOAD_VALUE >> 8;
	TMR3RLL = RELOAD_VALUE & 0xFF;

	EIE1 |= 0x08;		// enable ADC interrupt
}


// get current values (0..4095)
uint8_t Adc_Get(uint16_t *pos, uint16_t *iout, uint16_t *uin)
{
	if (!ready)
		return 0;

	*pos = posAcc >> ACC_BITS;
	*iout = ioutAcc >> ACC_BITS;
	*uin = uinAcc >> ACC_BITS;

	posAcc = 0;
	ioutAcc = 0;
	uinAcc = 0;
	ready = 0;

	return 1;
}


// ADC interrupt handler
void Adc_ISR(void) ADC_INTERRUPT_HANDLER
{
	static uint8_t accCounter = 0;

	AD0INT = 0;		// clear interrupt flag

	if (ready)
		return;		// wait for Adc_Get

	if (ADC0MX == ADCCHN_POS)
	{
		posAcc += ADC0;
		ADC0MX = ADCCHN_IOUT;
	}
	else if (ADC0MX == ADCCHN_IOUT)
	{
		ioutAcc += ADC0;
		ADC0MX = ADCCHN_UIN;
	}
	else if (ADC0MX == ADCCHN_UIN)
	{
		uinAcc += ADC0;
		ADC0MX = ADCCHN_POS;

		if (++accCounter >= (1 << ACC_BITS))
		{
			accCounter = 0;
			ready = 1;	// all channels have been read
		}
	}
	else
		ADC0MX = ADCCHN_POS;
}
