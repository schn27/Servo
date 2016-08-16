// regulator.c - вычисление управл€ющего воздействи€ на двигатель 

#include <common.h>
#include <regulator.h>
#include <pid.h>
#include <config.h>
#include <properties.h>
#include <convvalues.h>
#include <deadzone.h>


/////////////////////////////////////////////
#define FPPOS	8
#define KP1		(18.0 * (1 << FPPOS))
#define KI1		(0.0 * (1 << FPPOS))
#define KD1		(-20.0 * (1 << FPPOS))

#define OUTMIN	-32767
#define OUTMAX	32767
/////////////////////////////////////////////


// ѕ»ƒ-регул€тор
static PID pidPos2Pwm;


int16_t ApplyIoutLimit(int16_t value);


// инициализаци€ регул€тора
void Regulator_Init(void)
{
	PID_Init(&pidPos2Pwm);

	pidPos2Pwm.fixedPointPos = FPPOS;
	pidPos2Pwm.kP = KP1/*config.kP1*/;
	pidPos2Pwm.kI = KI1/*config.kI1*/;
	pidPos2Pwm.kD = KD1/*config.kD1*/;
	pidPos2Pwm.outMin = OUTMIN;
	pidPos2Pwm.outMax = OUTMAX;
}



// вычисление нового управл€ющего воздействи€
int16_t Regulator_Calc(void)
{
	pidPos2Pwm.ref = properties.command;
	pidPos2Pwm.fdb = GetValueFromDeadZone(properties.position, properties.command, config.deadZone);
	PID_Calc(&pidPos2Pwm);

	return ApplyIoutLimit(pidPos2Pwm.out);
}


// ограничение тока путЄм уменьшени€ управл€ющего воздействи€
static int16_t ApplyIoutLimit(int16_t value)
{
	static int8_t k = 64;

	if (config.ioutMax == 0)
		return 0;

	if (properties.iout > conv_current_from_mA(config.ioutMax))
	{
		if (k > 0)
			--k;
	}
	else
	{
		if (k < 64)
			++k;
	}
		
	return ((int32_t)value * k) >> 6;
}


