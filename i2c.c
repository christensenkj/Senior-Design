/*
 * i2c.c
 *
 *  Created on: Mar 13, 2021
 *      Author: karston
 */
#include <msp430.h>
#include <stdint.h>
#include "i2c.h"

// i2c variables
uint8_t TXData;
uint8_t RXData;
volatile uint16_t RXDataCtr;
volatile uint16_t TXDataCtr;

// buffer to store outlet data
volatile int16_t data[256];

//status flags
uint8_t screen_state;
uint8_t button_state;
uint8_t display_status;
uint8_t toggle_status;
uint8_t update_status;
uint8_t refresh_screen_status;

extern uint8_t outlet_num_abs;

uint8_t outlet_statuses[6] = {0,0,0,0,0,0};

void i2c_init(void) {
    I2C_REN |= (I2C_SDA_PIN + I2C_SCL_PIN);
    I2C_OUT |= (I2C_SDA_PIN + I2C_SCL_PIN);
    I2C_SEL |= (I2C_SDA_PIN + I2C_SCL_PIN);   // Assign I2C pins to USCI_B1
    UCB0CTL1 |= UCSWRST;                      // Enable SW reset
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
    UCB0BR0 = 255;                            // fSCL = SMCLK/255 = ~100kHz
//    UCB0BR0 = 16;                            // fSCL = SMCLK/255 = ~100kHz
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
    UCB0IE |= UCRXIE | UCNACKIE |UCTXIE;      // Enable TX interrupt
    TXData = 0x00;
}

void i2c_send_toggle(uint8_t i2_addr, uint8_t outlet_num) {
    // Set TXData
    TXData = outlet_num;
    // initialize data counter to 0
    TXDataCtr = 0;
    // Ensure stop condition was sent
    while (UCB0CTL1 & UCTXSTP);
    // Slave address
    UCB0I2CSA = i2_addr;
    // Transmit mode
    UCB0CTL1 |= UCTR;
    // 7-bit addressing mode
    UCB0CTL0 &= ~UCSLA10;
    // send start bit
    UCB0CTL1 |= UCTXSTT;
}

void i2c_receive_outlet(uint8_t i2_addr) {
    // set RX data vessel to 0
    RXData = 0x00;
    // initialize RX data counter to 0
    RXDataCtr = 0;
    // Ensure stop condition was sent
    while (UCB0CTL1 & UCTXSTP);
    // Slave address
    UCB0I2CSA = i2_addr;
    // Receive mode
    UCB0CTL1 &= ~UCTR;
    // 7 bit addressing mode
    UCB0CTL0 &= ~UCSLA10;
    // Send start bit
    UCB0CTL1 |= UCTXSTT;
}


// I2C routine
#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void) {
  switch(__even_in_range(UCB0IV,12)) {
  // No interrupts
  case  0: break;
  // ALIFG
  case  2: break;
  // NACKIFG
  case  4:
      // Send stop condition
      UCB0CTL1 |= UCTXSTP;
      // Clear RX flag and TX flag
      UCB0IFG &= ~(UCRXIFG + UCTXIFG);
      // reset i2c status
      toggle_status = 0;
      update_status = 0;
      break;
  // Start interrupts STTIFG
  case  6: break;
  // Stop interrupt STPIFG
  case  8: break;
  // RX interrupt: RXIFG
  case 10:
      // If not the last or second to last expected data packet
      if (RXDataCtr < (BUFLEN-2)) {
          // Read the RX buffer
          RXData = UCB0RXBUF;
          // Shift the local global array and put the RX buffer into it
          *(((uint8_t*)data) + RXDataCtr) = RXData;
          // Increment the number of bytes received counter
          RXDataCtr++;
      }
      // If second to last byte, send a stop STP along with reading the buffer
      else if (RXDataCtr == (BUFLEN-2)) {
          RXData = UCB0RXBUF;
          UCB0CTL1 |= UCTXSTP;
          *(((uint8_t*)data) + RXDataCtr) = RXData;
          RXDataCtr++;
      }
      // If last byte, just receive the buffer
      else if (RXDataCtr == (BUFLEN-1)) {
          RXData = UCB0RXBUF;
          *(((uint8_t*)data) + RXDataCtr) = RXData;
          RXDataCtr++;
          UCB0IFG &= ~UCRXIFG;
          // reset i2c status
          update_status = 0;
      }
      break;
  // Transmit interrupt TXIFG
  case 12:

      if (TXDataCtr < 1) {
          // Load data into TX buffer
          UCB0TXBUF = TXData;
          // increment the bytes sent counter
          TXDataCtr++;
          // toggle outlet status
          outlet_statuses[outlet_num_abs-1] = !outlet_statuses[outlet_num_abs-1];
      }
      // If the byte was sent, send a stop condition
      else {
          // Load data into TX buffer
          UCB0CTL1 |= UCTXSTP;
          UCB0IFG &= ~UCTXIFG;
          TXDataCtr = 0;
          // reset i2c status
          toggle_status = 0;
      }
      break;
  default: break;
  }
}
