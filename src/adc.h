#ifndef ADC_H
#define ADC_H

#include <stdint.h>

void Adc_Init(void);
uint8_t Adc_Get(uint16_t *pos, uint16_t *iout, uint16_t *uin);

#endif
