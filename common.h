// common.h

#ifndef COMMON_H
#define COMMON_H

#include <c8051F410.h>

// high priority
#define PCA_INTERRUPT_HANDLER interrupt INTERRUPT_PCA0 using 1
#define CALLED_BY_PCA_INTERRUPT_HANDLER using 1

// low priority
#define UART_INTERRUPT_HANDLER interrupt INTERRUPT_UART0 using 2
#define CALLED_BY_UART_INTERRUPT_HANDLER using 2
#define ADC_INTERRUPT_HANDLER interrupt INTERRUPT_ADC0_EOC using 2
#define CALLED_BY_ADC_INTERRUPT_HANDLER using 2

#define INT0_INTERRUPT_HANDLER interrupt INTERRUPT_INT0 using 2
#define CALLED_BY_INT0_INTERRUPT_HANDLER using 2

// clock frequency
#define FOSC	49000000.0

// safe debug with disabled drivers
//#define SAFEDEBUG


// enable/disable drivers
#ifndef SAFEDEBUG
#define DRIVER_ENABLE() P0 |= 0x01
#else
#define DRIVER_ENABLE()
#endif
#define DRIVER_DISABLE() P0 &= ~0x01


// enable/disable rs485_transmitter
#define RS485TX_ENABLE() P0 |= 0x08
#define RS485TX_DISABLE() P0 &= ~0x08


// valid input voltage range
#define UIN_MIN		(5000L * 4095L / 23739L)
#define UIN_MAX		(16000L * 4095L / 23739L)


// software reset
#define RESET() RSTSRC = 0x10

// watchdog control
#define WDT_ENABLE() PCA0MD |= 0x40
#define WDT_DISABLE() PCA0MD &= ~0x40
#define WDT_RESET() PCA0CPH5 = 0x00


#endif
