/*
 * i2c.c
 *
 *  Created on: Mar 13, 2021
 *      Author: karston
 */
#include <msp430.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "i2c.h"

// i2c variables
uint8_t TXData;
uint8_t RXData;
volatile int32_t data[BUFLEN/4];
volatile float float_data[BUFLEN/8];
volatile uint16_t RXDataCtr;
volatile uint16_t TXDataCtr;
uint8_t screen_state;
uint8_t outlet_status;
uint8_t button_state;
uint8_t display_status;
uint8_t toggle_status;
uint8_t update_status;
uint8_t refresh_screen_status;

// Structs
struct outlet_struct {
   float v_rms;
   float i_rms;
   float p_apparent;
} outlet_info;

struct th_struct {
   float temp;
   float hum;
} th_info;

// Define strings for displays and website
char res_v[5];
char res_i[5];
char res_p[5];
char res_t[5];
char res_h[5];

// Initialize i2c
//void i2c_init(void) {
//    I2C_REN |= (I2C_SDA_PIN + I2C_SCL_PIN);
//    I2C_OUT |= (I2C_SDA_PIN + I2C_SCL_PIN);
//    I2C_SEL |= (I2C_SDA_PIN + I2C_SCL_PIN);   // Assign I2C pins to USCI_B1
//    UCB0CTL1 |= UCSWRST;                      // Enable SW reset
//    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
//    UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
//    UCB0BR0 = 255;                            // fSCL = SMCLK/255 = ~100kHz
//    UCB0BR1 = 0;
//    UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
//    UCB0IE |= UCRXIE | UCNACKIE |UCTXIE;      // Enable TX interrupt
//    TXData = 0x00;
//}
void i2c_init(void) {
    I2C_REN |= (I2C_SDA_PIN + I2C_SCL_PIN);
    I2C_OUT |= (I2C_SDA_PIN + I2C_SCL_PIN);
    I2C_SEL |= (I2C_SDA_PIN + I2C_SCL_PIN);   // Assign I2C pins to USCI_B1
    UCB0CTL1 |= UCSWRST;                      // Enable SW reset
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
    UCB0BR0 = 255;                            // fSCL = SMCLK/255 = ~100kHz
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
    UCB0IE |= UCRXIE | UCNACKIE |UCTXIE;      // Enable TX interrupt
    TXData = 0x00;
}

//void i2c_send_toggle() {
//    // Set TXData
//    TXData = 'T';
//    // initialize data counter to 0
//    TXDataCtr = 0;
//    // Ensure stop condition was sent
//    while (UCB0CTL1 & UCTXSTP);
//    // Slave address
//    UCB0I2CSA = 0x33;
//    // Transmit mode
//    UCB0CTL1 |= UCTR;
//    // 7-bit addressing mode
//    UCB0CTL0 &= ~UCSLA10;
//    // send stop bit prematurely
//    UCB0CTL1 |= UCTXSTP;
//    // send start bit
//    UCB0CTL1 |= UCTXSTT;
//    while (UCB0CTL1 & UCTXSTT);
//}
void i2c_send_toggle() {
    // Set TXData
    TXData = 'T';
    // initialize data counter to 0
    TXDataCtr = 0;
    // Ensure stop condition was sent
    while (UCB0CTL1 & UCTXSTP);
    // Slave address
    UCB0I2CSA = 0x33;
    // Transmit mode
    UCB0CTL1 |= UCTR;
    // 7-bit addressing mode
    UCB0CTL0 &= ~UCSLA10;
    // send stop bit prematurely
//    UCB0CTL1 |= UCTXSTP;
    // send start bit
    UCB0CTL1 |= UCTXSTT;
//    while (UCB0CTL1 & UCTXSTT);
}

//void i2c_receive_outlet() {
//    // set RX data vessel to 0
//    RXData = 0x00;
//    // initialize RX data counter to 0
//    RXDataCtr = 0;
//    // Ensure stop condition was sent
//    while (UCB0CTL1 & UCTXSTP);
//    // Slave address
//    UCB0I2CSA = 0x33;
//    // Receive mode
//    UCB0CTL1 &= ~UCTR;
//    // 7 bit addressing mode
//    UCB0CTL0 &= ~UCSLA10;
//    // Send start bit
//    UCB0CTL1 |= UCTXSTT;
////    while (UCB0CTL1 & UCTXSTT);
//}
void i2c_receive_outlet() {
    // set RX data vessel to 0
    RXData = 0x00;
    // initialize RX data counter to 0
    RXDataCtr = 0;
    // Ensure stop condition was sent
    while (UCB0CTL1 & UCTXSTP);
    // Slave address
    UCB0I2CSA = 0x33;
    // Receive mode
    UCB0CTL1 &= ~UCTR;
    // 7 bit addressing mode
    UCB0CTL0 &= ~UCSLA10;
    // Send start bit
    UCB0CTL1 |= UCTXSTT;
//    while (UCB0CTL1 & UCTXSTT);
}


// do math on the outlet info
void do_math() {
    float v_rms = 0.0;
    float i_rms = 0.0;
    float p_apparent = 0.0;

    unsigned int i;
    // first calculate v_rms
    for (i = 0; i < BUFLEN/8; i++) {
        if (data[i] == 0x00800000) {
            data[i] = 0x00000000;
        }
        if (data[i+64] == 0x00800000) {
            data[i+64] = 0x00000000;
        }
        float normalized_v = (float)(~data[i] + 1) / (float)(~0x7FFFFF + 1);
        float normalized_i = (float)(~data[i+64] + 1) / (float)(~0x7FFFFF + 1);
        float_data [i] = normalized_i;
        v_rms += normalized_v*normalized_v;
        i_rms += normalized_i*normalized_i;
        p_apparent += normalized_v*normalized_i;
    }
    outlet_info.v_rms = sqrt(v_rms / (BUFLEN/8));
    outlet_info.i_rms = sqrt(17.85 * i_rms / (BUFLEN/8));
    outlet_info.p_apparent = p_apparent / (BUFLEN/8);
}


void get_th_info_i2c() {
    th_info.temp = 00.0;
    th_info.hum = 00.00;
}

void format_strings() {
    sprintf(res_v, "%.2f", outlet_info.v_rms);
    sprintf(res_i, "%.2f", outlet_info.i_rms);
    sprintf(res_p, "%.2f", outlet_info.p_apparent);
    sprintf(res_t, "%.2f", th_info.temp);
    sprintf(res_h, "%.2f", th_info.hum);
}


//// I2C routine
//#pragma vector = USCI_B1_VECTOR
//__interrupt void USCI_B1_ISR(void)
//{
//  switch(__even_in_range(UCB0IV,12))
//  {
//  // No interrupts
//  case  0: break;
//  // ALIFG
//  case  2: break;
//  // NACKIFG
//  case  4:
//      // Send stop condition
//      UCB0CTL1 |= UCTXSTP;
//      // Clear RX flag and TX flag
//      UCB0IFG &= ~(UCRXIFG + UCTXIFG);
//      // reset i2c status
//      toggle_status = 0;
//      update_status = 0;
//      break;
//  // Start interrupts STTIFG
//  case  6: break;
//  // Stop interrupt STPIFG
//  case  8: break;
//  // RX interrupt: RXIFG
//  case 10:
//      // If not the last or second to last expected data packet
//      if (RXDataCtr < (BUFLEN-2))
//      {
//          // Read the RX buffer
//          RXData = UCB0RXBUF;
//          // Shift the local global array and put the RX buffer into it
//          *(((uint8_t*)data) + RXDataCtr) = RXData;
//          // Increment the number of bytes received counter
//          RXDataCtr++;
//      }
//      // If second to last byte, send a stop STP along with reading the buffer
//      else if (RXDataCtr == (BUFLEN-2))
//      {
//          RXData = UCB0RXBUF;
//          UCB0CTL1 |= UCTXSTP;
//          *(((uint8_t*)data) + RXDataCtr) = RXData;
//          RXDataCtr++;
//      }
//      // If last byte, just receive the buffer
//      else if (RXDataCtr == (BUFLEN-1))
//      {
//          RXData = UCB0RXBUF;
//          *(((uint8_t*)data) + RXDataCtr) = RXData;
//          RXDataCtr++;
//          UCB0IFG &= ~UCRXIFG;
//          // reset i2c status
//          update_status = 0;
//      }
//      break;
//  // Transmit interrupt TXIFG
//  case 12:
//
//      if (TXDataCtr < 1) {
//          // Load data into TX buffer
//          UCB0TXBUF = TXData;
//          // increment the bytes sent counter
//          TXDataCtr = 0;
//          UCB0IFG &= ~UCTXIFG;
//          // toggle outlet status
//          outlet_status = !outlet_status;
//          // update the LCD screens with the new information
//          refresh_screen_status = 1;
//          // reset i2c status
//          toggle_status = 0;
//          P4DIR |= BIT7;
//          P4OUT ^= BIT7;
//      }
//      // If the byte was sent, send a stop condition
//      else {
//          UCB0CTL1 |= UCTXSTP;
//          UCB0IFG &= ~UCTXIFG;
//          // toggle outlet status
//          outlet_status = !outlet_status;
//          // update the LCD screens with the new information
//          refresh_screen_status = 1;
//          // reset i2c status
//          toggle_status = 0;
//
//      }
//      break;
//  default: break;
//  }
//}

// I2C routine
#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
{
  switch(__even_in_range(UCB0IV,12))
  {
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
      if (RXDataCtr < (BUFLEN-2))
      {
          // Read the RX buffer
          RXData = UCB0RXBUF;
          // Shift the local global array and put the RX buffer into it
          *(((uint8_t*)data) + RXDataCtr) = RXData;
          // Increment the number of bytes received counter
          RXDataCtr++;
      }
      // If second to last byte, send a stop STP along with reading the buffer
      else if (RXDataCtr == (BUFLEN-2))
      {
          RXData = UCB0RXBUF;
          UCB0CTL1 |= UCTXSTP;
          *(((uint8_t*)data) + RXDataCtr) = RXData;
          RXDataCtr++;
      }
      // If last byte, just receive the buffer
      else if (RXDataCtr == (BUFLEN-1))
      {
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
//          outlet_status = !outlet_status;
          // update the LCD screens with the new information
//          refresh_screen_status = 1;
          // reset i2c status
//          toggle_status = 0;
//          P4DIR |= BIT7;
//          P4OUT ^= BIT7;
      }
      // If the byte was sent, send a stop condition
      else {
          // Load data into TX buffer
//          UCB0TXBUF = TXData;
          UCB0CTL1 |= UCTXSTP;
          UCB0IFG &= ~UCTXIFG;
          TXDataCtr = 0;
          // toggle outlet status
          outlet_status = !outlet_status;
          // update the LCD screens with the new information
          refresh_screen_status = 1;
          // reset i2c status
          toggle_status = 0;

      }
      break;
  default: break;
  }
}

