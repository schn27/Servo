// rs485.c

#include <common.h>
#include <rs485.h>

#define TIMEOUT	0.005
#define TIMER2_RELOAD_VALUE	(65536L - (int)((FOSC / 12.0) * TIMEOUT))


void rs485_enable_tx(char enable);
void rs485_timeout_reset(void);
char rs485_timeout(void);


void rs485_init(void)
{
	TMOD = (TMOD & ~0xF0) | 0x20;	// set Timer1 mode2: 8-bit counter/timer with auto-reload
	CKCON |= 0x08;					// Timer1 clock = system clock
	TH1 = 43;						// UART baud rate = (49000000 / (256 - 43)) / 2 = 115023
	TR1 = 1;						// enable Timer1

	SCON0 = 0x30;	// 8bit, check for valid stop bit, reception enabled

	rs485_enable_tx(0);
}


uint8_t rs485_put(const uint8_t *buf, uint8_t bufsize)
{
	uint8_t cnt = 0;
	rs485_enable_tx(1);

	while (cnt < bufsize)
	{
		rs485_timeout_reset();

		SBUF0 = buf[cnt++];

		while (!TI0 && !rs485_timeout()) continue;

		if (rs485_timeout())
			break;

		TI0 = 0;
	}

	rs485_enable_tx(0);
	return cnt;
}


uint8_t rs485_get(uint8_t *buf, uint8_t bufsize)
{
	uint8_t cnt = 0;

	while (cnt < bufsize)
	{
		rs485_timeout_reset();

		while (!RI0 && !rs485_timeout()) continue;

		if (rs485_timeout())
			break;

		RI0 = 0;
		buf[cnt++] = SBUF0;
	}

	return cnt;
}


static void rs485_enable_tx(char enable)
{
	if (enable)
	{
		REN0 = 0;	// disable reception
		RS485TX_ENABLE();
	}
	else
	{
		RS485TX_DISABLE();
		REN0 = 1;	// enable reception
	}
}


static void rs485_timeout_reset(void)
{
	CKCON &= ~0x30;		// Timer2 clock defined by the T2XCLK bit in TMR2CN
	TMR2RLL = TIMER2_RELOAD_VALUE & 0xFF;
	TMR2RLH = TIMER2_RELOAD_VALUE / 256;

	TMR2L = TMR2RLL;
	TMR2H = TMR2RLH;

    TMR2CN = 0x04;	// Timer 2 enabled
	TF2H = 0;	// сброс флага переполнения таймера
}

static char rs485_timeout(void)
{
	return TF2H;
}


