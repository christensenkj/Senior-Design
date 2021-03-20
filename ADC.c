/*
 * adc.c
 *
 *  Created on: Feb 12, 2021
 *      Author: Zach Wilson
 */

#include "ADC.h"

#define BUFLEN      512
static uint8_t buf_ptr_0;
static uint8_t buf_ptr_1;
static uint8_t buf_ptr_2;
static uint8_t buf_ptr_3;

static void internal_config_adc(ADC_CONFIG_STRUCT *config);
static void write_ADC_buffer0(void);
static void write_ADC_buffer1(void);
static void write_ADC_buffer2(void);
static void write_ADC_buffer3(void);
static void shift_circ_buf0(uint32_t *buf, unsigned int size);
static void shift_circ_buf1(uint32_t *buf, unsigned int size);
static void shift_circ_buf2(uint32_t *buf, unsigned int size);
static void shift_circ_buf3(uint32_t *buf, unsigned int size);

uint32_t circularBuffer[BUFLEN/4];


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
        buf_ptr_0 = 64;
    } else if (Channel == ADC_CHANNEL1) {
        SD24CCTL1 |= SD24SNGL + SD24OSR_256 + SD24DF + SD24IE;   // Single conversion, Over-sampling ratio 64, 2s compliment format, enable data interrupt for SD24 (auto clear)
        SD24INCTL1 |= SD24GAIN_1 + SD24INCH_0;                               // Gain of 1
        buf_ptr_1 = 0;
    } else if (Channel == ADC_CHANNEL2) {
        SD24CCTL2 |= SD24SNGL + SD24OSR_256 + SD24DF + SD24IE;   // Single conversion, Over-sampling ratio 64, 2s compliment format, enable data interrupt for SD24 (auto clear)
        SD24INCTL2 |= SD24GAIN_1 + SD24INCH_0;                               // Gain of 1
        buf_ptr_2 = 0;
    } else if (Channel == ADC_CHANNEL3) {
        SD24CCTL3 |= SD24SNGL + SD24OSR_32 + SD24DF + SD24IE;   // Single conversion, Over-sampling ratio 64, 2s compliment format, enable data interrupt for SD24 (auto clear)
        SD24INCTL3 |= SD24GAIN_1 + SD24INCH_0;                               // Gain of 1
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
//        SD24CCTL0 &= ~SD24SC;
        break;
    case (SD24IV_6):
        write_ADC_buffer1();
//        SD24CCTL1 &= ~SD24SC;
        break;
    case (SD24IV_8):
        write_ADC_buffer2();
//        SD24CCTL2 &= ~SD24SC;
        break;
    case (SD24IV_A):
        write_ADC_buffer3();
//        SD24CCTL3 &= ~SD24SC;
        break;
    default:
        break;
    }
    TA0CCTL0 |= CCIE;
}

void write_ADC_buffer0() {
//    SD24CCTL0 &= ~SD24LSBACC;
//    uint32_t tempRead_upper = SD24MEM0 & 0xFF00;
//    SD24CCTL0 |= SD24LSBACC;
    uint32_t tempRead_lower = SD24MEM0;
//    int32_t tempRead = (tempRead_upper << 8) | tempRead_lower;
    //shift_circ_buf0(circularBuffer, BUFLEN/4);
//    circularBuffer[64] = tempRead;
    circularBuffer[buf_ptr_0] = tempRead_lower;
    buf_ptr_0++;
    if (buf_ptr_0 == 128) {
        buf_ptr_0 = 64;
    }
}

void write_ADC_buffer1() {
    SD24CCTL1 &= ~SD24LSBACC;
    uint32_t tempRead_upper = SD24MEM1 & 0xFF00;
    SD24CCTL1 |= SD24LSBACC;
    uint32_t tempRead_lower = SD24MEM1;
    int32_t tempRead = (tempRead_upper << 8) | tempRead_lower;
    shift_circ_buf1(circularBuffer, BUFLEN/4);
    circularBuffer[64] = tempRead;
}

void write_ADC_buffer2() {
    SD24CCTL2 &= ~SD24LSBACC;
    uint32_t tempRead_upper = SD24MEM2 & 0xFF00;
    SD24CCTL2 |= SD24LSBACC;
    uint32_t tempRead_lower = SD24MEM2;
    int32_t tempRead = (tempRead_upper << 8) | tempRead_lower;
    shift_circ_buf2(circularBuffer, BUFLEN/4);
    circularBuffer[0] = tempRead;
}

void write_ADC_buffer3() {
//    SD24CCTL3 &= ~SD24LSBACC;
//    uint32_t tempRead_upper = SD24MEM3 & 0xFF00;
//    SD24CCTL3 |= SD24LSBACC;
    uint32_t tempRead_lower = SD24MEM3;
//    int32_t tempRead = (tempRead_upper << 8) | tempRead_lower;
//    shift_circ_buf3(circularBuffer, BUFLEN/4);
//    circularBuffer[0] = tempRead;
    circularBuffer[buf_ptr_3] = tempRead_lower;
    buf_ptr_3++;
    if (buf_ptr_3 == 64) {
        buf_ptr_0 = 0;
    }
}

void shift_circ_buf0(uint32_t *buf, unsigned int size) {
    unsigned int i;
    for (i=(size-1); i>(size/2); i--) {
        buf[i] = buf[i-1];
    }
}

void shift_circ_buf1(uint32_t *buf, unsigned int size) {
    unsigned int i;
    for (i=(size-1); i>(size/2); i--) {
        buf[i] = buf[i-1];
    }
}

void shift_circ_buf2(uint32_t *buf, unsigned int size) {
    unsigned int i;
    for (i=(size-1); i>0; i--) {
        buf[i] = buf[i-1];
    }
}

void shift_circ_buf3(uint32_t *buf, unsigned int size) {
    unsigned int i;
    for (i=(size/2)-1; i>0; i--) {
        buf[i] = buf[i-1];
    }
}

