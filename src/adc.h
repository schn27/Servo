#ifndef ADC_H
#define ADC_H

#include <stdint.h>

void adc_init(void);
uint8_t adc_get(uint16_t *pos, uint16_t *iout, uint16_t *uin);

#endif
