/*
 * adc.c
 *
 *  Created on: Feb 12, 2021
 *      Author: Zach Wilson
 */

#include "ADC.h"

uint8_t buf_ptr_0;
uint8_t buf_ptr_1;
uint8_t buf_ptr_2;
uint8_t buf_ptr_3;

extern uint8_t buf0;
extern uint8_t buf1;
extern uint8_t buf2;
extern uint8_t buf3;

uint16_t cal_offset_i = 0x80;
uint16_t cal_offset_v = 0x48;

static void internal_config_adc(ADC_CONFIG_STRUCT *config);
static void write_ADC_buffer0(void);
static void write_ADC_buffer1(void);
static void write_ADC_buffer2(void);
static void write_ADC_buffer3(void);

int16_t circularBuffer[BUFELEM];


void adc_config(bool overFlow, uint8_t adcChannel) {
    ADC_CONFIG_STRUCT configure_adc;
    configure_adc.overFlow_en = overFlow;                   // Enables the overflow interrupt
    configure_adc.adc_channel = adcChannel;                 // Configuring channel 0
    internal_config_adc(&configure_adc);
}

void internal_config_adc(ADC_CONFIG_STRUCT *config) {
    // From structure configure
    bool overFlow = config->overFlow_en;
    uint8_t Channel = config->adc_channel;
    circularBuffer[0] = 0;

    // Control Reg
    if (overFlow) {
        SD24CTL |= SD24OVIE;    // Enable overflow interrupt, must be cleared by software
    }
    // Choose Channel
    if (Channel == ADC_CHANNEL0)  {
        SD24CCTL0 |= SD24SNGL + SD24OSR_32 + SD24DF + SD24IE;   // Single conversion, Over-sampling ratio 64, 2s compliment format, enable data interrupt for SD24 (auto clear)
        SD24INCTL0 |= SD24GAIN_1 + SD24INCH_0;                               // Gain of 1
        buf_ptr_0 = 32;
    } else if (Channel == ADC_CHANNEL1) {
        SD24CCTL1 |= SD24SNGL + SD24OSR_32 + SD24DF + SD24IE;   // Single conversion, Over-sampling ratio 64, 2s compliment format, enable data interrupt for SD24 (auto clear)
        SD24INCTL1 |= SD24GAIN_1 + SD24INCH_0;                               // Gain of 1
        buf_ptr_1 = 64;
    } else if (Channel == ADC_CHANNEL2) {
        SD24CCTL2 |= SD24SNGL + SD24OSR_32 + SD24DF + SD24IE;   // Single conversion, Over-sampling ratio 64, 2s compliment format, enable data interrupt for SD24 (auto clear)
        SD24INCTL2 |= SD24GAIN_1 + SD24INCH_0;                               // Gain of 1
        buf_ptr_2 = 96;
    } else if (Channel == ADC_CHANNEL3) {
        SD24CCTL3 |= SD24SNGL + SD24OSR_32 + SD24DF + SD24IE;   // Single conversion, Over-sampling ratio 64, 2s compliment format, enable data interrupt for SD24 (auto clear)
        buf_ptr_3 = 0;
    } else {
        while(1);
    }
}

void ADC_start(uint8_t adcChannel) {
    switch (adcChannel) {
        case (ADC_CHANNEL0):
            SD24CCTL0 |= SD24SC;    // Start ADC Conversion on Channel 0
            break;
        case (ADC_CHANNEL1):
            SD24CCTL1 |= SD24SC;    // Start ADC Conversion on Channel 1
            break;
        case (ADC_CHANNEL2):
            SD24CCTL2 |= SD24SC;    // Start ADC Conversion on Channel 2
            break;
        case (ADC_CHANNEL3):
            SD24CCTL3 |= SD24SC;    // Start ADC Conversion on Channel 3
            break;
        default:
            break;
    }
}

#pragma vector = SD24_VECTOR
__interrupt void SD24_ISR(void) {
    TA0CCTL0 &= ~CCIE;
    switch (SD24IV) {
    case (SD24IV_4):
        write_ADC_buffer0();
        break;
    case (SD24IV_6):
        write_ADC_buffer1();
        break;
    case (SD24IV_8):
        write_ADC_buffer2();
        break;
    case (SD24IV_A):
        write_ADC_buffer3();
        break;
    default:
        break;
    }
    TA0CCTL0 |= CCIE;
}

void write_ADC_buffer0() {
    int16_t tempRead_lower = SD24MEM0 - cal_offset_i;
    circularBuffer[buf_ptr_0] = tempRead_lower;
    buf_ptr_0++;
    if (buf_ptr_0 == 64) {
        buf_ptr_0 = 32;
    }
    buf0 = buf_ptr_0;
}

void write_ADC_buffer1() {
    int16_t tempRead_lower = SD24MEM1 - cal_offset_i;
    circularBuffer[buf_ptr_1] = tempRead_lower;
    buf_ptr_1++;
    if (buf_ptr_1 == 96) {
        buf_ptr_1 = 64;
    }
    buf1 = buf_ptr_1;
}

void write_ADC_buffer2() {
    int16_t tempRead_lower = SD24MEM2 - cal_offset_i;
    circularBuffer[buf_ptr_2] = tempRead_lower;
    buf_ptr_2++;
    if (buf_ptr_2 == 128) {
        buf_ptr_2 = 96;
    }
    buf2 = buf_ptr_2;
}

void write_ADC_buffer3() {
    int16_t tempRead_lower = SD24MEM3 - cal_offset_v;
    circularBuffer[buf_ptr_3] = tempRead_lower;
    buf_ptr_3++;
    if (buf_ptr_3 == 32) {
        buf_ptr_3 = 0;
    }
    buf3 = buf_ptr_3;
}
