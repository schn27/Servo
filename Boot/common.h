// common.h

#ifndef COMMON_H
#define COMMON_H

#include <c8051F410.h>

// clock frequency
#define FOSC	49000000.0

// enable/disable rs485_transmitter
#define RS485TX_ENABLE() P0 |= 0x08
#define RS485TX_DISABLE() P0 &= ~0x08

// software reset
#define RESET() RSTSRC = 0x10

// watchdog control
#define WDT_ENABLE() PCA0MD |= 0x40
#define WDT_DISABLE() PCA0MD &= ~0x40
#define WDT_RESET() PCA0CPH5 = 0x00


#endif
