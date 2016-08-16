#include "common.h"
#include "adc.h"
#include "motor.h"
#include "maintimer.h"
#include "config.h"

// ����������� ������ �� ��������� ��� ���������� (~15% �� ���������)
#define CALIBRATION_OUTPUT 5000

// ����-���� ��� ����������� ��������� ��������� (�������� �����)
#define NULLZONE	10

// ����� (*1��), � ������� �������� ������� ��������� �� ������� �� NULLZONE
#define STOPTIME	200


static uint8_t detectStop(uint16_t newPos, uint16_t *lastPos, uint8_t *cnt);
static void writeConfig(uint16_t x1, uint16_t x2);


// ��������� ��������������
void calibration(void) {
	uint16_t pot = 0;
	uint16_t iout = 0;
	uint16_t uin = 0;

	uint16_t x1 = 0;
	uint16_t x2 = 0;
	uint8_t cnt = 0;

	enum {eMoveNeg, eMovePos, eCalc} state = eMoveNeg;

	motor_enable(1);

	for (;;) {
		WDT_RESET();

		adc_get(&pot, &iout, &uin);		// ����� �������� �� ���

		if (mainTimer_tick()) {	// ������ 1000 ��
			switch (state) {
			case eMoveNeg:		// ���� � ���� ������� �� �����
				motor_set(-CALIBRATION_OUTPUT, uin);

				if (detectStop(pot, &x1, &cnt)) {
					state = eMovePos;
					cnt = 0;
				}
				break;

			case eMovePos:		// ���� � ������ ������� �� �����
				motor_set(CALIBRATION_OUTPUT, uin);

				if (detectStop(pot, &x2, &cnt)) {
					state = eCalc;
					cnt = 0;
				}
				break;

			case eCalc:			// ����� ��� "������������", ���������� posMul � posOfs � ������ ����������� �� flash
				motor_set(0, uin);

				if (++cnt >= STOPTIME) {
					writeConfig(x1, x2);
				}
				break;

			default:
				state = eMoveNeg;
			}
		}
	}
}


// ����������� ��������� ���������� (������ �� �����)
static uint8_t detectStop(uint16_t newPos, uint16_t *lastPos, uint8_t *cnt) {
	if (newPos > *lastPos + NULLZONE || newPos < *lastPos - NULLZONE) {
		*cnt = 0;
		*lastPos = newPos;
	} else if (++*cnt >= STOPTIME) {
		return 1;
	}

	return 0;
}


// ���������� � ���������� ����������
static void writeConfig(uint16_t x1, uint16_t x2) {
	if (x2 > x1) {
		config.posOfs = x1;
		config.posMul = (4095UL << 14) / (x2 - x1);
		config.motorReversed = 0;
	} else {
		config.posOfs = x2;
		config.posMul = (4095UL << 14) / (x1 - x2);
		config.motorReversed = 1;
	}

	config.calibrated = 1;
	config_store();
}
