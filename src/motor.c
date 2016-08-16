/*

  верхние ключи: 1 - замкнут, 0 - разомкнут
  нижние ключи: 0 - замкнут, 1 - разомкнут

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


// номинальное напряжение питания (6 В)
#define UIN_NOMINAL	(6000L * 4095L / 23739L)

// ведущий модуль (центры ШИМов)
#define MASTER 4

// из расчёта, что PCA clock = System clock = 49 МГц
#define DEADTIME		10		/* ~0.21 us */
#define PERIOD			2048	/* ~24 кГц */
#define MINDUTYCYCLE	256		/* ~5.0 us */
#define MAXDUTYCYCLE	(PERIOD - MINDUTYCYCLE - DEADTIME)

#define	DIMMINGMAX		255

// макросы чтения/записи регистров PCA0H:PCA0L
#define PCA0_READ(val) do {val = PCA0L; val |= (uint16_t)PCA0H << 8;} while(0)
#define PCA0_WRITE(val) do {PCA0L = (uint8_t)(val & 0xFF); PCA0H = (uint8_t)(val >> 8);} while(0)

// макросы чтения/записи регистров PCA0СPHn:PCA0СPLn (не используются)
#define PCA0CP_READ(n, val) do {val = PCA0CPL##n; val |= (uint16_t)PCA0CPH##n << 8;} while(0)
#define PCA0CP_WRITE(n, val) do {PCA0CPL##n = (uint8_t)(val & 0xFF); PCA0CPH##n = (uint8_t)(val >> 8);} while(0)

// соответствие ключей и модулей PCA
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

// разрешение/запрет прерываний PCA0
#define PCA0INT_ENABLE() EIE1 |= 0x10
#define PCA0INT_DISABLE() EIE1 &= ~0x10


// данные для передачи обработчику прерывания PCA
typedef struct {
	uint8_t direction;	// 0 - питание двигателя через Q1-Q4, 1 - через Q2-Q3
	uint16_t pduty;		// длительность рабочего цикла ШИМ в отсчётах таймера PCA (разгон)
	uint16_t nduty;		// длительность рабочего цикла ШИМ в отсчётах таймера PCA (торможение)
	uint8_t dimming;	// количество разрешённых импульсов ШИМ в периоде из DIMMINGPERIOD импульсов
	uint8_t isNew;		// 1 = данные обновлены (можно считывать в прерывании), 0 = данные считаны
} controlData_t;

static controlData_t data controlData = {0};


// 1 - управление двигателем разрешено, 0 - запрещено
static uint8_t data motorEnabled = 0;


static void bridgeStateControl(uint8_t phase, uint8_t portState);
static uint8_t getRandom(void);


// инициализация ШИМ управления мостом (драйвера должны быть под запретом EN = 0)
void motor_init(void) {
    PCA0MD   = 0x08;	// WDT disabled, PCA clock = System clock
    PCA0CPM0 = 0x4C;	// HSO
    PCA0CPM1 = 0x4C;	// HSO
    PCA0CPM2 = 0x4C;	// HSO
    PCA0CPM3 = 0x4C;	// HSO
    PCA0CPM4 = 0x49; 	// SW timer + interrupt

	// программирование срабатывания модулей для перевода ключей из состояния 1 в 0
	// в 1 входит при инициализации модуля как HSO
	PCA0_WRITE(0x0000);
	SET_Q1(0x10);
	SET_Q2(0x10 + DEADTIME);
	SET_Q3(0x10);
	SET_Q4(0x10 + DEADTIME);
	SET_MASTER(PERIOD / 2);

    EIP1 |= 0x10;		// высокий приоритет прерывания PCA0

	PCA0INT_ENABLE();

    PCA0CN = 0x40;		// counter/timer enabled
}

// преобразование нормированного управляющего воздействия в длительность рабочего цикла ШИМ
// максимальный ШИМ: value = +32767, value = -32767
// нулевой ШИМ: value = 0
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
			// при выходе на минимальный ШИМ дальнейшая регулировка ведётся диммингом
			controlData.dimming = (controlData.pduty < DIMMINGMAX) ? controlData.pduty : DIMMINGMAX;
			controlData.pduty = MINDUTYCYCLE;

			if (controlData.nduty < controlData.pduty + DEADTIME) {
				controlData.nduty = controlData.pduty + DEADTIME;
			}
		}
	}

	controlData.isNew = 1;
}


// разрешение управления двигателем, при запрете нижние ключи открываются, верхние закрываются
void motor_enable(uint8_t enable) {
	motorEnabled = enable;
}


// обработчик прерывания от PCA (от ведущего модуля)	
void motor_isr(void) PCA_INTERRUPT_HANDLER {
	// локальная копия controlData
	static uint8_t direction = 0;
	static uint16_t pduty = 0;
	static uint16_t nduty = 0;
	static uint8_t dimming = DIMMINGMAX;

	// текущая фаза ШИМ
	// 0 - оба верхних разомкнуты (0), оба нижних замкнуты (0)
	// 1 - управляемый верхний замкнут (1), диагональный нижний замкнут (0),
	//     пассивный верхний разомкнут (0), пассивный нижний разомкнут (1) 
	static uint8_t data phase = 0;

	// активный цикл (используется при димминге)
	static uint8_t data activeCycle = 0;

	// время следующего прерывания
	static uint16_t data masterTime = PERIOD;

	// время изменения состояния ключей Q1..4 
	static uint16_t data q1Time = PERIOD / 2 - 1;
	static uint16_t data q2Time = PERIOD / 2 - 1;
	static uint16_t data q3Time = PERIOD / 2 - 1;
	static uint16_t data q4Time = PERIOD / 2 - 1;

	// считывание состояния портов P0, P1 для контроля правильности управления мостом
	uint8_t data portCtrl = (P0 & 0x06) | (P1 & 0xC0);	// выделены только значащие биты

	// программирование PCA
	SET_Q1(q1Time);
	SET_Q2(q2Time);
	SET_Q3(q3Time);
	SET_Q4(q4Time);
	SET_MASTER(masterTime);

	// сброс флага прерывания
	PCA0CN &= ~(1 << MASTER);

	q1Time = q2Time = q3Time = q4Time = masterTime - 1;

	// контроль состояния выводов управления мостом
	bridgeStateControl(phase, portCtrl);

	// расчёт времён на полупериод вперёд (поэтому на phase == 1 считается передний фронт)
	if (phase == 1) {
		// безопасное копирование заданной ширины ШИМ
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

		masterTime += PERIOD - PERIOD / 2;		// на случай если PERIOD нечётный
		phase = 1;
	}
}



// детектирование расхождения фактического состояния выводов управления мостом с желаемым
static void bridgeStateControl(uint8_t phase, uint8_t portState) CALLED_BY_PCA_INTERRUPT_HANDLER {
	uint8_t error = 
		(portState != 0x00) && 
		((phase == 0) || (portState != 0xC0 && portState != 0x06));

	if (error) {
		DRIVER_DISABLE();
		RESET();
	}
}



// генератор псевдослучайной последовательности (Linear Feedback Shift Register)
static uint8_t getRandom(void) CALLED_BY_PCA_INTERRUPT_HANDLER {
	static uint16_t data lfsr = 0xA1A1;
	uint8_t lsb = lfsr & 1;

	lfsr >>= 1;

	if (lsb) {
		lfsr ^= 0xA1A1;
	}

	return (lfsr & 0xFF) ? lfsr & 0xFF : 1;
}
