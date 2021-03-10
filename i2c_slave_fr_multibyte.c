#include <msp430.h>
#include <stdint.h>

volatile uint8_t TXData;
volatile uint8_t TXDataCtr;

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;
  PM5CTL0 &= ~LOCKLPM5;

  UCB0CTL1 |= UCSWRST;                // Make sure USCI_B is in Reset before Config
  P1SEL0 |= BIT2 + BIT3;        // Select the Pins for I2C mode
  UCB0CTLW0 |= UCMODE_3 + UCSYNC;     // Set to I2C Slave Mode
  UCB0CTLW0 &= ~UCMST;                // Set to Slave Mode;
  UCB0I2COA0 |= 0x48 + UCOAEN;      // Set own Address 0x15
  UCB0CTL1 &= ~UCSWRST;               // USCI_B in operation

  UCB0IFG = UCB0IFG & ~UCTXIFG0 & ~UCSTPIFG;  // Clear interrupt flags for TX and Stop
  UCB0IE |= UCTXIE0 | UCSTPIE;                  // Enable TX and Stop interrupts

  TXDataCtr = 0x03;
  TXData = 0x11;

  __bis_SR_register(GIE);     // Enter LPM0 w/ interrupts

  while (1)
  {
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
            TXData = 0x11;
            TXDataCtr = 3;
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
            break;
        case 0x18:  // Vector 24: TXIFG0
            if (TXDataCtr == 3) {
                UCB0TXBUF = TXData;
                TXDataCtr--;
            }
            else if (TXDataCtr == 2) {
                TXData = 0x22;
                UCB0TXBUF = TXData;
                TXDataCtr--;
            }
            else if (TXDataCtr == 1) {
                TXData = 0x33;
                UCB0TXBUF = TXData;
                TXDataCtr--;
            }
            else {
                TXData = 0x44;
                UCB0TXBUF = TXData;
            }
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
