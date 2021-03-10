#include <msp430.h>
#include <stdint.h>

// 4.1 is SDA
// 4.2 is SCL

#define BUF_SIZE    128

uint8_t TXData;
uint8_t RXData;

uint8_t TXAddr;

int8_t TXByteCtr;
int16_t RXByteCtr;

uint32_t circbuf[128];
uint32_t tempval;

void init_buttons(void);
void send_byte_i2c(void);

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  P4SEL |= BIT1 + BIT2;                     // Assign I2C pins to USCI_B1
  UCB1CTL1 |= UCSWRST;                      // Enable SW reset
  UCB1CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
  UCB1CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
  UCB1BR0 = 12;                             // fSCL = SMCLK/12 = ~100kHz
  UCB1BR1 = 0;
  UCB1CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
  UCB1IE |= UCTXIE + UCRXIE;                         // Enable TX interrupt

  TXData = 9;

  init_buttons();
  __bis_SR_register(GIE);     // Enter LPM0 w/ interrupts

  while (1)
  {
  }
}


void send_byte_i2c() {
//    TXByteCtr = 1;                          // Load TX byte counter
    RXByteCtr = 1;
    while (UCB1CTL1 & UCTXSTP);             // Ensure stop condition got sent
    UCB1I2CSA = 0x48;                     // Write the slave address
//    UCB1CTL1 |= UCTR;                     // set the chip into transmit mode
    UCB1CTL1 &= ~UCTR;                     // set the chip into receive mode
    UCB1CTL0 &= ~UCSLA10;                // 7-bit addressing mode
    UCB1CTL1 |= UCTXSTT;             // I2C TX, start condition
    while (UCB1CTL1 & UCTXSTT);
    UCB1CTL1 |= UCTXSTP;                  // I2C stop condition

}


#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    P1IFG &= ~BIT1;
    P1IE &= ~BIT1;
    send_byte_i2c();
    P1IE |= BIT1;
}


//------------------------------------------------------------------------------
// The USCIAB0_ISR is structured such that it can be used to transmit any
// number of bytes by pre-loading TXByteCtr with the byte count.
//-----------------------------------------------------------------------
#pragma vector = USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void)
{
  switch(__even_in_range(UCB1IV,12))
  {
  case  0: break;                           // Vector  0: No interrupts
  case  2: break;                           // Vector  2: ALIFG
  // Vector  4: NACKIFG
  case  4: break;
  case  6: break;                           // Vector  6: STTIFG
  case  8: break;                           // Vector  8: STPIFG

  // RX Interrupt
  case 10:
      if (RXByteCtr == 1) {
          RXData = UCB1RXBUF;
          RXByteCtr--;
      }
      else {
//          UCB1CTL1 |= UCTXSTP;                  // I2C stop condition
          UCB1IFG &= ~UCRXIFG;                  // Clear USCI_B0 TX int flag
      }
      break;

  // TX Interrupt
  case 12:                                  // Vector 12: TXIFG
      if (TXByteCtr == 1) {
          UCB1TXBUF = TXData;                   // Load TX buffer
          TXByteCtr--;                          // Decrement TX byte counter
          TXData++;
      }
      else {
          UCB1CTL1 |= UCTXSTP;                  // I2C stop condition
          UCB1IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
      }
      break;

  default: break;
  }
}

void init_buttons() {
    P1DIR &= ~BIT1;                       // Set SW pin -> Input
    P1REN |= BIT1;                        // Enable Resistor for SW pin
    P1OUT |= BIT1;                        // Select Pull Up for SW pin
    P1IES &= ~BIT1;                       // Select Interrupt on Rising Edge
    P1IE |= BIT1;                         // Enable Interrupt on SW pin
}
