#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

// 1.2 is SDA
// 1.3 is SCL

uint8_t RXData;
uint8_t TXData;
uint8_t TXByteCtr;

bool toggle_led;

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;
  PM5CTL0 &= ~LOCKLPM5;

  P1DIR |= BIT0;
  P1OUT &= ~BIT0;

  UCB0CTL1 |= UCSWRST;                // Make sure USCI_B is in Reset before Config
  P1SEL0 |= BIT2 + BIT3;        // Select the Pins for I2C mode
  UCB0CTLW0 |= UCMODE_3 + UCSYNC;     // Set to I2C Slave Mode
  UCB0CTLW0 &= ~UCMST;                // Set to Slave Mode;
  UCB0I2COA0 |= 0x48 + UCOAEN;      // Set own Address 0x15
  UCB0CTL1 &= ~UCSWRST;               // USCI_B in operation

  UCB0IFG = UCB0IFG & ~UCSTTIFG & ~UCRXIFG0 & ~UCSTPIFG;  // Clear interrupt flags for Start, RX, and Stop
  UCB0IE |= UCSTTIE | UCRXIE0 | UCSTPIE | UCTXIE0;                  // Enable Start, RX, and Stop interrupts

  toggle_led = false;
  TXByteCtr = 1;
  TXData = 1;

  __bis_SR_register(GIE);     // Enter LPM0 w/ interrupts

  while (1)
  {
      if (toggle_led) {
          P1OUT ^= BIT0;
          toggle_led = false;
      }
  }
}


#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void) {
    switch(__even_in_range(UCB0IV,0x1e)) {
        case 0x00: // Vector 0: No interrupts
            break;
        case 0x02:  // Vector 2: ALIFG
            break;
        case 0x04: // Vector 4: NACKIFG
            break;
        case 0x06: // Vector 6: STTIFG
            break;
        case 0x08:  // Vector 8: STPIFG
            TXByteCtr = 1;
            TXData++;
            break;
        case 0x0a:  // Vector 10: RXIFG3
            break;
        case 0x0c:  // Vector 12: TXIFG3
            break;
        case 0x0e:  // Vector 14: RXIFG2
            break;
        case 0x10:  // Vector 16: TXIFG2
            break;
        case 0x12:  // Vector 18: RXIFG1
            break;
        case 0x14:  // Vector 20: TXIFG1
            break;
        case 0x16: // Vector 22: RXIFG0
            RXData = UCB0RXBUF;
            toggle_led = !toggle_led;
            UCB0IFG &= ~UCRXIFG0;
            break;
        case 0x18:  // Vector 24: TXIFG0
            UCB0TXBUF = TXData;
            break;
        case 0x1a:  // Vector 26: BCNTIFG
            break;
        case 0x1c:  // Vector 28: clock low time-out
            break;
        case 0x1e:  // Vector 30: 9th bit
            break;
        default: break;
    }
}

//#pragma vector=PORT4_VECTOR
//__interrupt void Port_4(void)
//{
//    P4IFG &= ~BIT5;                       // Clear SW interrupt flag
//}
