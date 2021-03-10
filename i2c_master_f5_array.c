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

#define BUFLEN  512

uint8_t RXData;
uint8_t TXData;
volatile uint32_t data[128];
volatile uint16_t RXDataCtr;
volatile uint16_t TXDataCtr;

void init_buttons(void);
void receive_i2c(void);
void send_i2c(void);

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

  // configure timer A
  TA0CCTL0 = CCIE;                      // CCR0 interrupt enabled
  TA0CTL = TASSEL_1 + MC_1 + ID_3;      // SMCLK/8, upmode
  TA0CCR0 =  20480;                     // Interrupt once every 5 seconds

  P4SEL |= BIT1 + BIT2;                     // Assign I2C pins to USCI_B1
  UCB1CTL1 |= UCSWRST;                      // Enable SW reset
  UCB1CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
  UCB1CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
  UCB1BR0 = 18;                             // fSCL = SMCLK/12 = ~100kHz
  UCB1BR1 = 0;
  UCB1CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
  UCB1IE |= UCRXIE | UCNACKIE;                       // Enable TX interrupt

  RXData = 0x00;
  TXData = 0x00;

  init_buttons();
  __bis_SR_register(GIE);     // Enter LPM0 w/ interrupts

  while (1)
  {
  }
}


void send_i2c() {
    TXDataCtr = 0;
    while (UCB1CTL1 & UCTXSTP);             // Ensure stop condition got sent
    UCB1I2CSA = 0x48;                     // Write the slave address
    UCB1CTL1 |= UCTR;                     // set the chip into receive mode
    UCB1CTL0 &= ~UCSLA10;                // 7-bit addressing mode
    UCB1CTL1 |= UCTXSTT;             // I2C TX, start condition
//    while (UCB1CTL1 & UCTXSTT);
}

void receive_i2c() {
    RXData = 0x00;
    RXDataCtr = 0;
    while (UCB1CTL1 & UCTXSTP);             // Ensure stop condition got sent
    UCB1I2CSA = 0x48;                     // Write the slave address
    UCB1CTL1 &= ~UCTR;                     // set the chip into receive mode
    UCB1CTL0 &= ~UCSLA10;                // 7-bit addressing mode
    UCB1CTL1 |= UCTXSTT;             // I2C TX, start condition
//    while (UCB1CTL1 & UCTXSTT);
}


//#pragma vector=PORT1_VECTOR
//__interrupt void Port_1(void)
//{
//    P1IFG &= ~BIT1;                       // Clear SW interrupt flag
//    receive_i2c();
//}


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
      if (RXDataCtr < (BUFLEN-2))
      {
          RXData = UCB1RXBUF;
          *(((uint8_t*)data) + RXDataCtr) = RXData;
          RXDataCtr++;
      }
      else if (RXDataCtr == (BUFLEN-2))
      {
          RXData = UCB1RXBUF;
          UCB1CTL1 |= UCTXSTP;
          *(((uint8_t*)data) + RXDataCtr) = RXData;
          RXDataCtr++;
      }
      else if (RXDataCtr == (BUFLEN-1))
      {
          RXData = UCB1RXBUF;
          *(((uint8_t*)data) + RXDataCtr) = RXData;
          RXDataCtr++;
      }
      break;
  case 12:                          // Vector 12: TXIFG
      if (TXDataCtr < 1) {
          UCB1TXBUF = TXData;                   // Load TX buffer
          TXData++;
          TXDataCtr++;                          // Decrement TX byte counter
      }
      else {
          UCB1CTL1 |= UCTXSTP;                  // I2C stop condition
          UCB1IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
      }
      break;
  default: break;
  }
}

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
{
    receive_i2c();
}


void init_buttons() {
    P1DIR &= ~BIT1;                       // Set SW pin -> Input
    P1REN |= BIT1;                        // Enable Resistor for SW pin
    P1OUT |= BIT1;                        // Select Pull Up for SW pin
    P1IES &= ~BIT1;                       // Select Interrupt on Rising Edge
    P1IE |= BIT1;                         // Enable Interrupt on SW pin
}
