/**
 *
 * @author schn27
 *
 * ���������� ������� ������� (��������������� Hitec)
 *
 * 1. ������������� �� ��������� ��� ������ ��������� (�� ����� �� �����)
 * 2. ��������� ������� ����������/��������� �� ����������������� ���������
 * 3. �������� ������� ���������
 * 4. �������� ������������ ���
 * 5. �������� ���������� �������
 * 6. ����������� ��� ���������� ������ ��������� �� ������� ����� ���������� ���������� � ������� ����������
 * 7. ������������ ��� ���������� ������ ��� ���������� ������������� ���� (��������)
 * 8. ��� �������� ���������� ������� ���� ������������ (��������) ��������� ���������� ������ ���������
 *
 * ������ ����������� ��� bootloader: ���, ����� �����, ������� ���������� ������� �� 0x800.
 * C�. startup.a51 (CSEG AT 0800H) � ����� ����������� (IV(0x800)) � ������� (CODE(0x800)).
 * Oscillator_Init �� ����������, �.�. ��� �������� � "�������".
 *
 */


#include "common.h"
#include "adc.h"
#include "interface.h"
#include "motor.h"
#include "maintimer.h"
#include "regulator.h"
#include "config.h"


void calibration(void);
void mainLoop(void);

static void init(void);
static void resetSources_init(void);
static void portIO_init(void);
#if 0
static void oscillator_init(void);
#endif

void main(void) {
	init();
	
	if (config.calibrated) {
		mainLoop();
	} else {
		calibration();
	}
}


static void init(void) {
	WDT_DISABLE();

	resetSources_init();
	portIO_init();
#if 0
	oscillator_init();
#endif

	config_init();
	adc_init();
	interface_init();
	regulator_init();
	mainTimer_init();
	motor_init();			// ���������������� � �����, ���� ��������� ���������� �� ������������� (����� ���������� PCA)

	EA = 1;					// ��������� ��� ����������

	PCA0CPL5  = 0xFF;		// WDT timeout = 2.7 ms
	WDT_ENABLE();
}


// ��������� ���������� ������
static void resetSources_init(void) {
    int i = 0;

	VDM0CN = 0x80;		// enabled, level low (level HIGH is recommended in datasheet!)
    for (i = 0; i < 20; i++) {}	// Wait 5us for Vdd Monitor stabilization
	RSTSRC = 0x02;		// Vdd Monitor as reset source
}


/** 
 * ������������ � �����������!
 * ��������� ��� ������ ��� ������� ��������� ����������,
 * ��������� ��������� ���������� "������" ��� ������ � CLKMUL.
 */
#if 0
// ��������� ����������� ���������� � ���������� (24.5 ��� * 2 = 49 ���)
static void oscillator_init(void) {
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


// ��������� ������
static void portIO_init(void) {
	DRIVER_DISABLE();		// ������ ��������� ������: ������� ����� ����������, ������ ��������

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


