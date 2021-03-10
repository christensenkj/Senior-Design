
/*
 * main.c
 *
 *  Created on: Feb 12, 2021
 *      Author: Zach Wilson
 */

#include "ADC.h"

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;                                   // Stop watchdog timer

    // configure timer A0
    TA0CCTL0 = CCIE;                      // CCR0 interrupt enabled
    TA0CTL = TASSEL_2 + MC_1 + ID_0;      // SMCLK/8, upmode
    TA0CCR0 =  1440;                     // 7680 Hz

    adc_config(ADC_OVERFLOW_IE_DIS,ADC_CHANNEL0);               // Configure ADC Channel 3 (Voltage) and disable overflow interrupt
    __bis_SR_register(GIE);
    while(1);
}

// Timer A1 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
{
    ADC_start(ADC_CHANNEL0);
}

