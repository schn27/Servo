#include "common.h"

// ������� ������� 1000 ��
#define FREQ	1000.0

// ������ 2 ����� ������������� � �������� ������ ����������
#define TIMER2_RELOAD_VALUE	(65536L - (int)((FOSC / 12.0) / FREQ))

void mainTimer_init(void) {
	CKCON &= ~0x30;		// Timer2 clock defined by the T2XCLK bit in TMR2CN
	TMR2RLL = TIMER2_RELOAD_VALUE & 0xFF;
	TMR2RLH = TIMER2_RELOAD_VALUE / 256;

    TMR2CN = 0x04;	// Timer 2 enabled
}


char mainTimer_tick(void) {
	if (TF2H) {
		TF2H = 0;	// ����� ����� ������������ �������
		return 1;
	}

	return 0;
}
