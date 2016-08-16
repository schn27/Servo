/*

  ������� �����: 1 - �������, 0 - ���������
  ������ �����: 0 - �������, 1 - ���������

  (+) ---+-----------+
         |           |
	  Q1  /           / Q3
         |           |
		 +---( M )---+
      __ |           |  __
  	  Q2  /           / Q4
         |           |
  (-) ---+-----------+

*/

#include <stdint.h>
#include "common.h"
#include "version.h"


// ����������� ���������� ������� (6 �)
#define UIN_NOMINAL	(6000L * 4095L / 23739L)

// ������� ������ (������ �����)
#define MASTER 4

// �� �������, ��� PCA clock = System clock = 49 ���
#define DEADTIME		10		/* ~0.21 us */
#define PERIOD			2048	/* ~24 ��� */
#define MINDUTYCYCLE	256		/* ~5.0 us */
#define MAXDUTYCYCLE	(PERIOD - MINDUTYCYCLE - DEADTIME)

#define	DIMMINGMAX		255

// ������� ������/������ ��������� PCA0H:PCA0L
#define PCA0_READ(val) do {val = PCA0L; val |= (uint16_t)PCA0H << 8;} while(0)
#define PCA0_WRITE(val) do {PCA0L = (uint8_t)(val & 0xFF); PCA0H = (uint8_t)(val >> 8);} while(0)

// ������� ������/������ ��������� PCA0�PHn:PCA0�PLn (�� ������������)
#define PCA0CP_READ(n, val) do {val = PCA0CPL##n; val |= (uint16_t)PCA0CPH##n << 8;} while(0)
#define PCA0CP_WRITE(n, val) do {PCA0CPL##n = (uint8_t)(val & 0xFF); PCA0CPH##n = (uint8_t)(val >> 8);} while(0)

// ������������ ������ � ������� PCA
#if BOARD_REV == 1
#define SET_Q1(val) do {PCA0CPL1 = (uint8_t)(val & 0xFF); PCA0CPH1 = (uint8_t)(val >> 8);} while(0)
#define SET_Q2(val) do {PCA0CPL0 = (uint8_t)(val & 0xFF); PCA0CPH0 = (uint8_t)(val >> 8);} while(0)
#define SET_Q3(val) do {PCA0CPL2 = (uint8_t)(val & 0xFF); PCA0CPH2 = (uint8_t)(val >> 8);} while(0)
#define SET_Q4(val) do {PCA0CPL3 = (uint8_t)(val & 0xFF); PCA0CPH3 = (uint8_t)(val >> 8);} while(0)
#elif BOARD_REV == 2
#define SET_Q1(val) do {PCA0CPL0 = (uint8_t)(val & 0xFF); PCA0CPH0 = (uint8_t)(val >> 8);} while(0)
#define SET_Q2(val) do {PCA0CPL1 = (uint8_t)(val & 0xFF); PCA0CPH1 = (uint8_t)(val >> 8);} while(0)
#define SET_Q3(val) do {PCA0CPL3 = (uint8_t)(val & 0xFF); PCA0CPH3 = (uint8_t)(val >> 8);} while(0)
#define SET_Q4(val) do {PCA0CPL2 = (uint8_t)(val & 0xFF); PCA0CPH2 = (uint8_t)(val >> 8);} while(0)
#else
#error "Unknown board revision"
#endif

#define SET_MASTER(val) do {PCA0CPL4 = (uint8_t)(val & 0xFF); PCA0CPH4 = (uint8_t)(val >> 8);} while(0)

// ����������/������ ���������� PCA0
#define PCA0INT_ENABLE() EIE1 |= 0x10
#define PCA0INT_DISABLE() EIE1 &= ~0x10


// ������ ��� �������� ����������� ���������� PCA
typedef struct {
	uint8_t direction;	// 0 - ������� ��������� ����� Q1-Q4, 1 - ����� Q2-Q3
	uint16_t pduty;		// ������������ �������� ����� ��� � �������� ������� PCA (������)
	uint16_t nduty;		// ������������ �������� ����� ��� � �������� ������� PCA (����������)
	uint8_t dimming;	// ���������� ����������� ��������� ��� � ������� �� DIMMINGPERIOD ���������
	uint8_t isNew;		// 1 = ������ ��������� (����� ��������� � ����������), 0 = ������ �������
} controlData_t;

static controlData_t data controlData = {0};


// 1 - ���������� ���������� ���������, 0 - ���������
static uint8_t data motorEnabled = 0;


static void bridgeStateControl(uint8_t phase, uint8_t portState);
static uint8_t getRandom(void);


// ������������� ��� ���������� ������ (�������� ������ ���� ��� �������� EN = 0)
void motor_init(void) {
    PCA0MD   = 0x08;	// WDT disabled, PCA clock = System clock
    PCA0CPM0 = 0x4C;	// HSO
    PCA0CPM1 = 0x4C;	// HSO
    PCA0CPM2 = 0x4C;	// HSO
    PCA0CPM3 = 0x4C;	// HSO
    PCA0CPM4 = 0x49; 	// SW timer + interrupt

	// ���������������� ������������ ������� ��� �������� ������ �� ��������� 1 � 0
	// � 1 ������ ��� ������������� ������ ��� HSO
	PCA0_WRITE(0x0000);
	SET_Q1(0x10);
	SET_Q2(0x10 + DEADTIME);
	SET_Q3(0x10);
	SET_Q4(0x10 + DEADTIME);
	SET_MASTER(PERIOD / 2);

    EIP1 |= 0x10;		// ������� ��������� ���������� PCA0

	PCA0INT_ENABLE();

    PCA0CN = 0x40;		// counter/timer enabled
}

// �������������� �������������� ������������ ����������� � ������������ �������� ����� ���
// ������������ ���: value = +32767, value = -32767
// ������� ���: value = 0
void motor_set(int16_t value, uint16_t uin) {
	uint16_t correction = 0;

	if (controlData.isNew) {
		return;
	}

	controlData.direction = 0;

	if (value < 0) {
		controlData.direction = 1;
		value = -value;
	}

	value /= (32768 / PERIOD);

	if (value > MAXDUTYCYCLE) {
		value = MAXDUTYCYCLE;
	}

	if (uin > UIN_NOMINAL) {
		correction = ((uint32_t)value * (uin - UIN_NOMINAL) / uin);
	}

	controlData.dimming = DIMMINGMAX;

	if (value <= correction) {
		controlData.pduty = controlData.nduty = 0;
	} else {
		controlData.pduty = value - correction;
		controlData.nduty = value + DEADTIME;

		if (controlData.pduty < MINDUTYCYCLE) {
			// ��� ������ �� ����������� ��� ���������� ����������� ������ ���������
			controlData.dimming = (controlData.pduty < DIMMINGMAX) ? controlData.pduty : DIMMINGMAX;
			controlData.pduty = MINDUTYCYCLE;

			if (controlData.nduty < controlData.pduty + DEADTIME) {
				controlData.nduty = controlData.pduty + DEADTIME;
			}
		}
	}

	controlData.isNew = 1;
}


// ���������� ���������� ����������, ��� ������� ������ ����� �����������, ������� �����������
void motor_enable(uint8_t enable) {
	motorEnabled = enable;
}


// ���������� ���������� �� PCA (�� �������� ������)	
void motor_isr(void) PCA_INTERRUPT_HANDLER {
	// ��������� ����� controlData
	static uint8_t direction = 0;
	static uint16_t pduty = 0;
	static uint16_t nduty = 0;
	static uint8_t dimming = DIMMINGMAX;

	// ������� ���� ���
	// 0 - ��� ������� ���������� (0), ��� ������ �������� (0)
	// 1 - ����������� ������� ������� (1), ������������ ������ ������� (0),
	//     ��������� ������� ��������� (0), ��������� ������ ��������� (1) 
	static uint8_t data phase = 0;

	// �������� ���� (������������ ��� ��������)
	static uint8_t data activeCycle = 0;

	// ����� ���������� ����������
	static uint16_t data masterTime = PERIOD;

	// ����� ��������� ��������� ������ Q1..4 
	static uint16_t data q1Time = PERIOD / 2 - 1;
	static uint16_t data q2Time = PERIOD / 2 - 1;
	static uint16_t data q3Time = PERIOD / 2 - 1;
	static uint16_t data q4Time = PERIOD / 2 - 1;

	// ���������� ��������� ������ P0, P1 ��� �������� ������������ ���������� ������
	uint8_t data portCtrl = (P0 & 0x06) | (P1 & 0xC0);	// �������� ������ �������� ����

	// ���������������� PCA
	SET_Q1(q1Time);
	SET_Q2(q2Time);
	SET_Q3(q3Time);
	SET_Q4(q4Time);
	SET_MASTER(masterTime);

	// ����� ����� ����������
	PCA0CN &= ~(1 << MASTER);

	q1Time = q2Time = q3Time = q4Time = masterTime - 1;

	// �������� ��������� ������� ���������� ������
	bridgeStateControl(phase, portCtrl);

	// ������ ����� �� ���������� ����� (������� �� phase == 1 ��������� �������� �����)
	if (phase == 1) {
		// ���������� ����������� �������� ������ ���
		if (controlData.isNew) {
			direction = controlData.direction;
			pduty = controlData.pduty;
			nduty = controlData.nduty;
			dimming = controlData.dimming;
			controlData.isNew = 0;
		}

		if (motorEnabled) {
			DRIVER_ENABLE();
		} else {
			DRIVER_DISABLE();
		}

		activeCycle = (motorEnabled && pduty && (getRandom() <= dimming));

		if (activeCycle) {
			if (direction == 0) {
				q1Time = masterTime + (PERIOD - pduty) / 2;	
				q2Time = masterTime + (PERIOD - nduty) / 2;
			} else {
				q3Time = masterTime + (PERIOD - pduty) / 2;	
				q4Time = masterTime + (PERIOD - nduty) / 2;
			}
		}

		masterTime += PERIOD / 2;
		phase = 0;
	} else {
		if (activeCycle) {
			if (direction == 0) {
				q1Time = masterTime + pduty / 2;
				q2Time = masterTime + nduty / 2;
			} else {
				q3Time = masterTime + pduty / 2;	
				q4Time = masterTime + nduty / 2;
			}
		}

		masterTime += PERIOD - PERIOD / 2;		// �� ������ ���� PERIOD ��������
		phase = 1;
	}
}



// �������������� ����������� ������������ ��������� ������� ���������� ������ � ��������
static void bridgeStateControl(uint8_t phase, uint8_t portState) CALLED_BY_PCA_INTERRUPT_HANDLER {
	uint8_t error = 
		(portState != 0x00) && 
		((phase == 0) || (portState != 0xC0 && portState != 0x06));

	if (error) {
		DRIVER_DISABLE();
		RESET();
	}
}



// ��������� ��������������� ������������������ (Linear Feedback Shift Register)
static uint8_t getRandom(void) CALLED_BY_PCA_INTERRUPT_HANDLER {
	static uint16_t data lfsr = 0xA1A1;
	uint8_t lsb = lfsr & 1;

	lfsr >>= 1;

	if (lsb) {
		lfsr ^= 0xA1A1;
	}

	return (lfsr & 0xFF) ? lfsr & 0xFF : 1;
}
