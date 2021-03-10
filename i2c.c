/*
 * i2c.c
 *
 *  Created on: Mar 2, 2021
 *      Author: karston
 */

#include <msp430.h>
#include <stdint.h>
#include "i2c.h"

uint8_t RXData;
uint8_t TXData;
volatile uint32_t data[128];
volatile uint16_t RXDataCtr;
volatile uint16_t TXDataCtr;

void init_i2c() {
    P4SEL |= BIT1 + BIT2;                     // Assign I2C pins to USCI_B1
    UCB1CTL1 |= UCSWRST;                      // Enable SW reset
    UCB1CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
    UCB1CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
    UCB1BR0 = 18;                             // fSCL = SMCLK/12 = ~100kHz
    UCB1BR1 = 0;
    UCB1CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
    UCB1IE |= UCRXIE | UCNACKIE;                       // Enable TX interrupt
    return;
}

void request_data_i2c() {
    RXData = 0x00;
    RXDataCtr = 0;
    while (UCB1CTL1 & UCTXSTP);             // Ensure stop condition got sent
    UCB1I2CSA = 0x48;                     // Write the slave address
    UCB1CTL1 &= ~UCTR;                     // set the chip into receive mode
    UCB1CTL0 &= ~UCSLA10;                // 7-bit addressing mode
    UCB1CTL1 |= UCTXSTT;             // I2C TX, start condition
//    while (UCB1CTL1 & UCTXSTT);
}

void send_command_i2c() {
    TXData = 'T';
    TXDataCtr = 1;
    while (UCB1CTL1 & UCTXSTP);             // Ensure stop condition got sent
    UCB1I2CSA = 0x48;                     // Write the slave address
    UCB1CTL1 |= UCTR;                     // set the chip into receive mode
    UCB1CTL0 &= ~UCSLA10;                // 7-bit addressing mode
    UCB1CTL1 |= UCTXSTT;             // I2C TX, start condition
//    while (UCB1CTL1 & UCTXSTT);
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
      if (TXDataCtr == 1) {
          UCB1TXBUF = TXData;                   // Load TX buffer
          TXDataCtr--;                          // Decrement TX byte counter
      }
      else {
          UCB1CTL1 |= UCTXSTP;                  // I2C stop condition
          UCB1IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
      }
      break;
  default: break;
  }
}


