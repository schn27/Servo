#include "common.h"
#include "adc.h"
#include "interface.h"
#include "motor.h"
#include "maintimer.h"
#include "regulator.h"
#include "config.h"
#include "properties.h"
#include "arraysize.h"
#include "lowpassfilter.h"
#include "deadzone.h"


#define MAXPOS 32767

int16_t GetPosition(uint16_t pot);
int16_t GetActualSpeed(int16_t newpos);
void UpdateCurrentCommand(void);


// главный цикл
void MainLoop(void)
{
	uint16_t data pot = 0;
	uint16_t data iout = 0;
	uint16_t data uin = 0;

	for (;;)
	{
		WDT_RESET();

		if (Adc_Get(&pot, &iout, &uin))				// новые значени€ от ј÷ѕ?
		{
			int16_t x = (properties.uin < 32767) ? properties.uin : 32767;
			LowPassFilter(&x, uin, 0.07 * 256);
			properties.uin = x;

			x = (properties.iout < 32767) ? properties.iout : 32767;
			LowPassFilter(&x, iout, 0.3 * 256);
			properties.iout = x;

			properties.original_position = GetPosition(pot);
		}

		if (MainTimer_Tick())
		{
			static uint8_t cnt = 0;

			int16_t x;

			Interface_Update();
			Config_CheckModified();

			if (++cnt >= 2)
			{
				cnt = 0;

				UpdateCurrentCommand();
				LowPassFilter(&properties.command, properties.current_command, 0.07 * 256);

				LowPassFilter(&properties.position_ndz, properties.original_position, 0.5 * 256);
				MoveDeadZone(&properties.position, properties.position_ndz, config.deadZone);

				properties.speed = GetActualSpeed(properties.position);

				x = Regulator_Calc();

				Motor_Set(config.motorReversed ? -x : x, properties.uin);
				Motor_Enable((properties.uin > UIN_MIN) /*&& (properties.uin < UIN_MAX)*/ && !Interface_IsFreeMode());	// fix: защита по большому напр€жению убрана
			}
		}
	}
}


// преобразование сигнала с потенциометра в нормализованное положение (-32767..+32767)
static int16_t GetPosition(uint16_t pot)
{
	int32_t data pos = (int32_t)pot - config.posOfs;
	pos = ((pos * config.posMul) >> 10) - 0x8000L;

	if (pos > MAXPOS)
		return MAXPOS;
	else if (pos < -MAXPOS)
		return -MAXPOS;
	else
		return (int16_t)pos;
}



// расчЄт текущей скорости (относительных единиц за 0.02 секунды)
static int16_t GetActualSpeed(int16_t newpos)
{
	static int16_t pos[10] = {0};
	static uint8_t pos_index = 0;

	int16_t data oldpos = pos[pos_index];
	pos[pos_index] = newpos;

	if (++pos_index >= ARRAYSIZE(pos))
		pos_index = 0;

	return ((int32_t)newpos - (int32_t)oldpos);
}


// обновление текущей команды с учЄтом скорости
static void UpdateCurrentCommand(void)
{
	static uint8_t initialized = 0;
	static uint8_t cnt = 0;

	if (!initialized)
	{
		properties.current_command = config.centerOfs;
		initialized = 1;
	}

	if (++cnt >= 10)
	{
		int32_t delta = (int32_t)properties.current_command - (int32_t)properties.original_command;

		if (delta < (int32_t)config.speed && delta > -(int32_t)config.speed)
			properties.current_command = properties.original_command;
		else
			properties.current_command += ((delta < 0) ? config.speed : -config.speed);

		cnt = 0;
	}
}
