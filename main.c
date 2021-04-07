#include <msp430.h> 
#include "shtc3.h"

/**
 * main.c
 */

void start_timerA0(void);
float temp;
float hum;
uint8_t update_status;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	shtc3_i2c_config();
	update_status = 0;
	start_timerA0();
	_enable_interrupts();
	while(1) {
	    if (update_status) {
	        shtc3_i2c();
	        temp = returnTemp();
	        hum = returnRH();
	        update_status = 0;
	    }
	}
}

// configure and start TA0
void start_timerA0() {
    // configure timer A
    TA0CCTL0 = CCIE;                      // CCR0 interrupt enabled
    TA0CTL = TASSEL_1 + MC_1 + ID_3;      // ACLK/8, upmode
    TA0CCR0 =  20480;                     // Interrupt once every 5 second
}

// Timer A0 routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
{
    update_status = 1;
}
