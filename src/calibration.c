#include "common.h"
#include "adc.h"
#include "motor.h"
#include "maintimer.h"
#include "config.h"

// управл€ющий сигнал на двигатель при калибровке (~15% от максимума)
#define CALIBRATION_OUTPUT 5000

// нуль-зона дл€ определени€ остановки актуатора (достигли упора)
#define NULLZONE	10

// врем€ (*1мс), в течении которого позици€ актуатора не выходит из NULLZONE
#define STOPTIME	200


static uint8_t DetectStop(uint16_t newPos, uint16_t *lastPos, uint8_t *cnt);
static void WriteConfig(uint16_t x1, uint16_t x2);


// процедура автокалибровки
void Calibration(void)		// длинный метод - нужен рефакторинг
{
	uint16_t pot = 0;
	uint16_t iout = 0;
	uint16_t uin = 0;

	uint16_t x1 = 0;
	uint16_t x2 = 0;
	uint8_t cnt = 0;

	enum {eMoveNeg, eMovePos, eCalc} state = eMoveNeg;

	Motor_Enable(1);

	for (;;)
	{
		WDT_RESET();

		Adc_Get(&pot, &iout, &uin);		// новые значени€ от ј÷ѕ

		if (MainTimer_Tick())	// таймер 1000 √ц
		{
			switch (state)
			{
			case eMoveNeg:		// едем в одну сторону до упора
				Motor_Set(-CALIBRATION_OUTPUT, uin);

				if (DetectStop(pot, &x1, &cnt))
				{
					state = eMovePos;
					cnt = 0;
				}
				break;

			case eMovePos:		// едем в другую сторону до упора
				Motor_Set(CALIBRATION_OUTPUT, uin);

				if (DetectStop(pot, &x2, &cnt))
				{
					state = eCalc;
					cnt = 0;
				}
				break;

			case eCalc:			// пауза дл€ "безопасности", вычисление posMul и posOfs и запись результатов во flash
				Motor_Set(0, uin);		

				if (++cnt >= STOPTIME)
				{
					WriteConfig(x1, x2);
				}
				break;

			default:
				state = eMoveNeg;
			}
		}
	}
}


// определение остановки актутатора (доехал до упора)
static uint8_t DetectStop(uint16_t newPos, uint16_t *lastPos, uint8_t *cnt)
{
	if (newPos > *lastPos + NULLZONE || newPos < *lastPos - NULLZONE)
	{
		*cnt = 0;
		*lastPos = newPos;
	}
	else if (++*cnt >= STOPTIME)
	{
		return 1;
	}

	return 0;
}


// вычисление и сохранение параметров
static void WriteConfig(uint16_t x1, uint16_t x2)
{
	if (x2 > x1)
	{
		config.posOfs = x1;
		config.posMul = (4095UL << 14) / (x2 - x1);
		config.motorReversed = 0;
	}
	else
	{
		config.posOfs = x2;
		config.posMul = (4095UL << 14) / (x1 - x2);
		config.motorReversed = 1;
	}

	config.calibrated = 1;
	Config_Store();
}
