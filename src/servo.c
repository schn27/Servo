// servo.c - инициализация и главный цикл

/**
 *
 * @author Alexandr Malikov
 *
 * Контроллер рулевой машинки (модифицированый Hitec)
 *
 * 1. Калиброваться по положению при первом включении (от упора до упора)
 * 2. Принимать команды управления/настройки по последовательному протоколу
 * 3. Измерять текущее положение
 * 4. Измерять потребляемый ток
 * 5. Измерять напряжение питания
 * 6. Формировать ШИМ управление мостом двигателя по разнице между внутренним задатчиком и текущим положением
 * 7. Ограничивать ШИМ управление мостом при превышении максимального тока (параметр)
 * 8. При снижении напряжения питания ниже минимального (параметр) запрещать управление мостом двигателя
 *
 * Проект адаптирован под bootloader: код, точка входа, вектора прерываний смещены на 0x800.
 * Cм. startup.a51 (CSEG AT 0800H) и опции компилятора (IV(0x800)) и линкера (CODE(0x800)).
 * Oscillator_Init не вызывается, т.к. это приводит к "падению".
 *
 */


#include <common.h>
#include <adc.h>
#include <interface.h>
#include <motor.h>
#include <maintimer.h>
#include <regulator.h>
#include <config.h>


void Calibration(void);
void MainLoop(void);

static void Init(void);
static void ResetSources_Init(void);
static void Oscillator_Init(void);
static void Port_IO_Init(void);


void main(void)
{
	Init();
	
	if (config.calibrated)
		MainLoop();
	else
		Calibration();
}


// инициализация
static void Init(void)
{
	WDT_DISABLE();

	ResetSources_Init();
	Port_IO_Init();
#if 0
	Oscillator_Init();
#endif

	Config_Init();
	Adc_Init();
	Interface_Init();
	Regulator_Init();
	MainTimer_Init();
	Motor_Init();			// инициализировать в конце, либо разрешать прерывания до инициализации (иначе развалится PCA)

	EA = 1;					// разрешить все прерывания

	PCA0CPL5  = 0xFF;		// WDT timeout = 2.7 ms
	WDT_ENABLE();
}


// настройка источников сброса
static void ResetSources_Init(void)
{
    int i = 0;

	VDM0CN = 0x80;		// enabled, level low (level HIGH is recommended in datasheet!)
    for (i = 0; i < 20; i++) {}	// Wait 5us for Vdd Monitor stabilization
	RSTSRC = 0x02;		// Vdd Monitor as reset source
}


/** 
 * Несовместимо с загрузчиком!
 * Загрузчик при старте уже произвёл настройку умножителя,
 * повторная настройка умножителя "падает" при записи в CLKMUL.
 */
#if 0
// настройка внутреннего генератора и умножителя (24.5 МГц * 2 = 49 МГц)
static void Oscillator_Init(void)
{
    int i = 0;

    PFE0CN &= ~0x20;
    FLSCL = 0x10;
    PFE0CN |= 0x20;

    CLKMUL = 0x80;
    for (i = 0; i < 20; i++) {}	// Wait 5us for initialization

    CLKMUL |= 0xC0;
    while ((CLKMUL & 0x20) == 0) {}

    CLKSEL = 0x02;

    OSCICN = 0x87;
}
#endif


// настройка портов
static void Port_IO_Init(void)
{
	DRIVER_DISABLE();		// запрет драйверов ключей: верхние ключи разомкнуты, нижние замкнуты

    // P0.0  -  Skipped,     Push-Pull,  Digital
    // P0.1  -  CEX0 (PCA),  Push-Pull,  Digital
    // P0.2  -  CEX1 (PCA),  Push-Pull,  Digital
    // P0.3  -  Skipped,     Push-Pull,  Digital
    // P0.4  -  TX0 (UART0), Push-Pull,  Digital
    // P0.5  -  RX0 (UART0), Push-Pull,  Digital
    // P0.6  -  Skipped,     Open-Drain, Digital
    // P0.7  -  Skipped,     Open-Drain, Digital

    // P1.0  -  Skipped,     Open-Drain, Digital
    // P1.1  -  Skipped,     Open-Drain, Digital
    // P1.2  -  Skipped,     Open-Drain, Analog
    // P1.3  -  Skipped,     Open-Drain, Analog
    // P1.4  -  Skipped,     Open-Drain, Analog
    // P1.5  -  Skipped,     Open-Drain, Analog
    // P1.6  -  CEX2 (PCA),  Push-Pull,  Digital
    // P1.7  -  CEX3 (PCA),  Push-Pull,  Digital

    // P2.0  -  Skipped,     Open-Drain, Digital
    // P2.1  -  Skipped,     Open-Drain, Digital
    // P2.2  -  Skipped,     Open-Drain, Digital
    // P2.3  -  Skipped,     Open-Drain, Digital
    // P2.4  -  Skipped,     Open-Drain, Digital
    // P2.5  -  Skipped,     Open-Drain, Digital
    // P2.6  -  Skipped,     Open-Drain, Digital
    // P2.7  -  Skipped,     Open-Drain, Digital

    P1MDIN    = 0xC3;
    P0MDOUT   = 0x3F;
    P1MDOUT   = 0xC0;
    P0SKIP    = 0xC9;
    P1SKIP    = 0x3F;
    P2SKIP    = 0xFF;
    XBR0      = 0x01;
    XBR1      = 0xC4;
}


