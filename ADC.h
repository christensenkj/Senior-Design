/*
 * adc.h
 *
 *  Created on: Feb 12, 2021
 *      Author: Zach Wilson
 */

#ifndef ADC_H_
#define ADC_H_

#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>

enum adc_channels {
    ADC_CHANNEL0,
    ADC_CHANNEL1,
    ADC_CHANNEL2,
    ADC_CHANNEL3
};

enum adc_overflow_ie {
    ADC_OVERFLOW_IE_DIS,
    ADC_OVERFLOW_IE_EN
};

#define SD24IV_2    0x2     // ADC Overflow interrupt
#define SD24IV_4    0x4     // ADC Channel 0 interrupt
#define SD24IV_6    0x6     // ADC Channel 1 interrupt
#define SD24IV_8    0x8     // ADC Channel 2 interrupt
#define SD24IV_A    0xA     // ADC Channel 3 interrupt

typedef struct {
    bool overFlow_en;                   // 1 if enable overflow interrupt to notify of overflowed adc data
    uint8_t adc_channel;                // 0 if channel 0, 1 if channel 1, 2 if channel 2, 3 if channel 3
} ADC_CONFIG_STRUCT;

void adc_config(bool overFlow, uint8_t adcChannel);
void ADC_start(uint8_t adcChannel);

#endif /* ADC_H_ */
