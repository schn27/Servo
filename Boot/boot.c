// boot.c

/**
 *
 * @author Alexandr Malikov
 *
 * Загрузчик по RS-485 для контроллера рулевой машинки (модифицированый Hitec)
 *
 */

#include <common.h>
#include <config.h>


void MainLoop(void);

void Init(void);
void ResetSources_Init(void);
void Oscillator_Init(void);
void Port_IO_Init(void);
void Voltage_Reference_Init(void);


void main(void)
{
	Init();
	MainLoop();
}


// инициализация
static void Init(void)
{
	WDT_DISABLE();

	ResetSources_Init();
	Port_IO_Init();
	Oscillator_Init();
	Voltage_Reference_Init();
	Config_Init();
}


// настройка источников сброса
static void ResetSources_Init(void)
{
    int i = 0;

	VDM0CN = 0xA0;		// enabled, level high
    for (i = 0; i < 20; i++) {}	// Wait 5us for Vdd Monitor stabilization
	RSTSRC = 0x02;		// Missing Clock Detector, Vdd Monitor
}



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


// настройка портов
static void Port_IO_Init(void)
{
    // P0.0  -  Skipped,     Open-Drain, Digital
    // P0.1  -  Skipped,     Open-Drain, Digital
    // P0.2  -  Skipped,     Open-Drain, Digital
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
    // P1.6  -  Skipped,     Open-Drain, Digital
    // P1.7  -  Skipped,     Open-Drain, Digital

    // P2.0  -  Skipped,     Open-Drain, Digital
    // P2.1  -  Skipped,     Open-Drain, Digital
    // P2.2  -  Skipped,     Open-Drain, Digital
    // P2.3  -  Skipped,     Open-Drain, Digital
    // P2.4  -  Skipped,     Open-Drain, Digital
    // P2.5  -  Skipped,     Open-Drain, Digital
    // P2.6  -  Skipped,     Open-Drain, Digital
    // P2.7  -  Skipped,     Open-Drain, Digital

    P1MDIN    = 0xC3;
    P0MDOUT   = 0x38;
    P0SKIP    = 0xCF;
    P1SKIP    = 0xFF;
    P2SKIP    = 0xFF;
    XBR0      = 0x01;
    XBR1      = 0xC0;
}


static void Voltage_Reference_Init(void)
{
    REF0CN    = 0x13;
}



