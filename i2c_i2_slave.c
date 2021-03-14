#include <msp430.h> 
#include <stdint.h>
#include <stdbool.h>
#include <ADC.h>

// yellow: ground
// red: sda
// black: scl

#define BUFLEN  512
extern uint32_t circularBuffer[BUFLEN/4];

volatile uint8_t TXData;
volatile uint8_t RXData;
volatile uint16_t TXDataCtr;
volatile uint16_t RXDataCtr;
uint8_t toggle_status;
uint8_t outlet_status;

void toggle_outlet(void);
void config_relay_pins(void);
void config_timerA0(void);
/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	UCB0CTL1 |= UCSWRST; // put eUSCI_B in reset state
	UCB0CTLW0 |= UCMODE_3; // I2C slave mode
	UCB0CTLW1 = UCASTP_2; // autom. STOP assertion
	UCB0TBCNT = BUFLEN; // TX 8 bytes of data
	UCB0I2COA0 = 0x0033 + UCOAEN; // address slave is 12hex
	P1SEL0 |= BIT6 | BIT7; // configure I2C pins (device specific)

	/* Zach's reset code */
//	UCB0CTLW1 |= UCCLTO_1;  // Selects appx. 28ms as the max time
	/********************/


	UCB0CTL1 &= ~UCSWRST; // eUSCI_B in operational state
//	UCB0IFG &= ~UCCLTOIFG;  // Clear the IFG for Clock timeout (Zach)
//    UCB0IE |= UCRXIE0 |  UCTXIE | UCSTPIE | UCCLTOIE;                  // Enable Start, RX, and Stop interrupts (Zach: and Timeout)
    UCB0IE |= UCRXIE0 |  UCTXIE | UCSTPIE;                  // Enable Start, RX, and Stop interrupts (Zach: and Timeout)

    TXDataCtr = 0;
    toggle_status = 0;
    outlet_status = 1;

    // configure timer A0
    config_timerA0();

    config_relay_pins();

    adc_config(ADC_OVERFLOW_IE_DIS,ADC_CHANNEL3);               // Configure ADC Channel 3 (Voltage) and disable overflow interrupt
    adc_config(ADC_OVERFLOW_IE_DIS,ADC_CHANNEL0);               // Configure ADC Channel 3 (Voltage) and disable overflow interrupt

    __bis_SR_register(GIE);     // Enter LPM0 w/ interrupts

	while(1) {
        if (toggle_status) {
            toggle_outlet();
            toggle_status = 0;
        }
	}
}

void config_timerA0() {
    TA0CCTL0 = CCIE;                      // CCR0 interrupt enabled
    TA0CTL = TASSEL_2 + MC_1 + ID_0;      // SMCLK/8, upmode
    TA0CCR0 =  2880;                     // 7680 Hz
}

void toggle_outlet() {
    outlet_status = !outlet_status;
    // Toggle outlet 1
    P1OUT ^= BIT3;
}

void config_relay_pins() {
    P1DIR |= BIT3 + BIT4 + BIT5;
    P1OUT &= ~(BIT3 + BIT4 + BIT5);
}


#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void) {
    SD24CCTL0 &= ~SD24IE;
    SD24CCTL1 &= ~SD24IE;
    SD24CCTL2 &= ~SD24IE;
    SD24CCTL3 &= ~SD24IE;
    TA0CCTL0 &= ~CCIE;
    switch(__even_in_range(UCB0IV,0x1e)) {
        case 0x00: // Vector 0: No interrupts break;
        case 0x02:// Vector 2: ALIFG break;
        case 0x04:// Vector 4: NACKIFG break;
        case 0x06:  // Vector 6: STTIFG break;
            break;
        case 0x08: // Vector 8: STPIFG break;
            TXDataCtr = 0;
            TA0CCTL0 |= CCIE;
            SD24CCTL0 |= SD24IE;
            SD24CCTL1 |= SD24IE;
            SD24CCTL2 |= SD24IE;
            SD24CCTL3 |= SD24IE;
            UCB0IFG &= ~(UCTXIFG + UCRXIFG);
            break;
        case 0x0a: // Vector 10: RXIFG3 break;
        case 0x0c: // Vector 14: TXIFG3 break;
        case 0x0e:  // Vector 16: RXIFG2 break;
        case 0x10: // Vector 18: TXIFG2 break;
        case 0x12: // Vector 20: RXIFG1 break;
        case 0x14:  // Vector 22: TXIFG1 break;
        case 0x16: // Vector 22: RXIFG0
            RXData = UCB0RXBUF;
            if (RXData == 'T') {
                toggle_status = 1;
            }
            break;
        case 0x18:  // Vector 24: TXIFG0
            if (TXDataCtr < BUFLEN) {
                TXData = *(((uint8_t*)circularBuffer) + TXDataCtr);
                UCB0TXBUF = TXData;
                TXDataCtr++;
            }
            else {
                TXDataCtr++;
                TXData = 0xFF;
                UCB0TXBUF = TXData;
            }
            break;
        case 0x1a:  // Vector 28: BCNTIFG break;
        case 0x1c: // Vector 30: clock low timeout break;
//            // Interrupt handler will reset variables used within I2C, then reset the I2C line itself
//            // Example Data -------------------------- //
//            TXDataCtr = 0;
//            toggle_status = 0;
//            // --------------------------------------- //
//            UCB0CTL1 |= UCSWRST;    // Reset I2C step 1: Disable
//            UCB0CTLW0 |= UCMODE_3; // I2C slave mode
//            UCB0CTLW1 = UCASTP_2; // autom. STOP assertion
//            UCB0TBCNT = BUFLEN; // TX 8 bytes of data
//            UCB0I2COA0 = 0x0033 + UCOAEN; // address slave is 12hex
//            P1SEL0 |= BIT6 | BIT7; // configure I2C pins (device specific)
//
//            /* Zach's reset code */
//            UCB0CTLW1 |= UCCLTO_3;  // Selects appx. 28ms as the max time
//            /********************/
//            UCB0CTL1 &= ~UCSWRST;   // Reset I2C step 2: Re-Enable
//            UCB0IE |= UCRXIE0 |  UCTXIE | UCSTPIE | UCCLTOIE;                  // Enable Start, RX, and Stop interrupts (Zach: and Timeout)
            break;
        case 0x1e: // Vector 32: 9th bit break;
        default: break;
    }
}

// Timer A1 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
{
    ADC_start(ADC_CHANNEL3);
    ADC_start(ADC_CHANNEL0);
//    ADC_start(ADC_CHANNEL2);
//    ADC_start(ADC_CHANNEL3);
}
