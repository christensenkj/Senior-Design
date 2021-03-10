//  MSP430F552x Demo - USCI_B0 I2C Master TX single bytes to MSP430 Slave
//
//  Description: This demo connects two MSP430's via the I2C bus. The master
//  transmits to the slave. This is the master code. It continuously
//  transmits 00h, 01h, ..., 0ffh and demonstrates how to implement an I2C
//  master transmitter sending a single byte using the USCI_B0 TX interrupt.
//  ACLK = n/a, MCLK = SMCLK = BRCLK = default DCO = ~1.045MHz
//
// ***to be used with "MSP430F55xx_uscib0_i2c_07.c" ***
//
//                                /|\  /|\
//                MSP430F5529     10k  10k     MSP430F5529
//                   slave         |    |         master
//             -----------------   |    |   -----------------
//           -|XIN  P3.0/UCB0SDA|<-|----+->|P3.0/UCB0SDA  XIN|-
//            |                 |  |       |                 |
//           -|XOUT             |  |       |             XOUT|-
//            |     P3.1/UCB0SCL|<-+------>|P3.1/UCB0SCL     |
//            |                 |          |                 |
//
//   Bhargavi Nisarga
//   Texas Instruments Inc.
//   April 2009
//   Built with CCSv4 and IAR Embedded Workbench Version: 4.21
//******************************************************************************
//The code from TI company, use reference volt to pull-up SDA and SCL.
#include <msp430.h>
#include <stdint.h>

volatile uint32_t RXData;
volatile uint8_t RXDataCtr;
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
  UCB1IE |= UCRXIE | UCNACKIE;                       // Enable TX interrupt

  RXData = 0x00000000;


  init_buttons();
  __bis_SR_register(GIE);     // Enter LPM0 w/ interrupts

  while (1)
  {
  }
}


void send_byte_i2c() {
    RXData = 0x00000000;
    RXDataCtr = 0x03;
    while (UCB1CTL1 & UCTXSTP);             // Ensure stop condition got sent
    UCB1I2CSA = 0x48;                     // Write the slave address
    UCB1CTL1 &= ~UCTR;                     // set the chip into receive mode
    UCB1CTL0 &= ~UCSLA10;                // 7-bit addressing mode
    UCB1CTL1 |= UCTXSTT;             // I2C TX, start condition
    while (UCB1CTL1 & UCTXSTT);
}


#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    P1IFG &= ~BIT1;                       // Clear SW interrupt flag
    send_byte_i2c();
}


//------------------------------------------------------------------------------
// The USCIAB0_ISR is structured such that it can be used to recieve any
// number of bytes by pre-loading RXByteCtr with the byte count.
//-----------------------------------------------------------------------
#pragma vector = USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void)
{
  switch(__even_in_range(UCB1IV,12))
  {
  case  0: break;                           // Vector  0: No interrupts
  case  2: break;                           // Vector  2: ALIFG
  case  4:                                  // Vector  4: NACKIFG
      UCB1CTL1 |= UCTXSTP;                  // I2C stop condition
      UCB1IFG &= ~UCRXIFG;                  // Clear USCI_B0 RX int flag
      break;
  case  6: break;                           // Vector  6: STTIFG
  case  8: break;                           // Vector  8: STPIFG
  case 10:                                  // Vector 10: RXIFG
      if (RXDataCtr == 3)
      {
          RXData |= UCB1RXBUF;
          RXData = RXData << 8;
          RXDataCtr--;
      }
      else if (RXDataCtr == 2)
      {
          RXData |= UCB1RXBUF;
          RXData = RXData << 8;
          RXDataCtr--;
      }
      else if (RXDataCtr == 1)
      {
          RXData |= UCB1RXBUF;
          UCB1CTL1 |= UCTXSTP;
          RXData = RXData << 8;
          RXDataCtr--;
      }
      else
      {
          RXData |= UCB1RXBUF;
          RXDataCtr--;
      }
      break;
  case 12: break;                            // Vector 12: TXIFG
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
