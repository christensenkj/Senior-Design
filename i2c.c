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
volatile int32_t data[256];

//status flags
uint8_t screen_state;
uint8_t outlet_status;
uint8_t button_state;
uint8_t display_status;
uint8_t toggle_status;
uint8_t update_status;
uint8_t refresh_screen_status;


volatile uint8_t thTXDataCtr;
volatile uint8_t *thTXData;
volatile uint8_t thRXDataCtr;
volatile uint8_t *thRXData;
volatile uint8_t thBuff[6];
//volatile uint8_t RXReady;
volatile uint8_t SHTC3_ready;
volatile uint8_t th_flag;
volatile uint32_t RH;
volatile uint32_t TP;
uint32_t th_addr = 0x70;
float Humid;
float Temp;

// flags to keep track of which I2C interrupt code to run
uint8_t i2c_mode;
enum I2C_MODE {
    OUTLET_MODE,
    SHTC3_MODE,
};

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
    SHTC3_ready = 0;
    RH = 0x00;
}

void i2c_send_toggle(uint8_t i2_addr, uint8_t outlet_num) {
    i2c_mode = OUTLET_MODE;
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
    i2c_mode = OUTLET_MODE;
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

void shtc3_i2c(void)
{
    i2c_mode = SHTC3_MODE;
    UCB0IE |= UCTXIE + UCRXIE;              // Enable TX and RX interrupts on I2C
    UCB0I2CSA = th_addr;
    Wakeup();
    Measure();
    UCB0IFG &= ~UCNACKIFG;                  // Clear Nack interrupt flag
    UCB0IE |= UCNACKIE;                     // Enable Nack interrupts
    Read();
    UCB0IE &= ~UCNACKIE;
    Sleep();
    Interpret();
}

void Wakeup(void)
{
    UCB0CTL1 |= UCTR;                       // Set as transmitter
    while (UCB0CTL1 & UCTXSTP);
    thTXDataCtr = 2;                          // Load TX Byte Counter
    uint8_t wakeup_command[] = {0x35,0x17}; // Wakeup command for SHTC3
    thTXData = (uint8_t *)wakeup_command;     // Grab the pointer for the command
    th_flag = 1;
    UCB0CTL1 |= UCTXSTT;                    // TX start
    while(th_flag);
    while (UCB0CTL1 & UCTXSTP);             // Ensure 1st stop sent before continuing
    __delay_cycles(100);                    // Waking up takes time.
}

void Measure(void)
{
    UCB0CTL1 |= UCTR;                       // Set as transmitter
    thTXDataCtr = 2;                          // Load TX Byte counter
    uint8_t measure_command[] = {0x58,0xE0};// Measure command for low power mode and no clock stretching
    thTXData = (uint8_t *)measure_command;    // Grab the pointer for the command
    th_flag = 1;
    UCB0CTL1 |= UCTXSTT;                    // TX start
    while(th_flag);
    while (UCB0CTL1 & UCTXSTP);             // Ensure 2nd stop sent before continuing
}

void Read(void)
{
    UCB0CTL1 &= ~UCTR;                      // Set as receiver
    thRXDataCtr = 6;                          // Load RX Byte counter
    thRXData = (uint8_t *)thBuff;             // Set pointer to the buffer
    //RXReady = 0;                          // Initiate the flag for polling
    th_flag = 1;
    UCB0CTL1 |= UCTXSTT;
    while(th_flag);
    while (UCB0CTL1 & UCTXSTP);
}

void Sleep(void)
{
    UCB0CTL1 |= UCTR;                       // Set as transmitter
    thTXDataCtr = 2;                          // Load TX Byte counter
    uint8_t sleep_command[] = {0xB0,0x98};  // Sleep command
    thTXData = (uint8_t *)sleep_command;      // Grab pointer to the command
    th_flag = 1;
    UCB0CTL1 |= UCTXSTT;                    // TX Start
    while(th_flag);
    while (UCB0CTL1 & UCTXSTP);             // Ensure 4th stop condition sent
}

void Interpret(void)
{
    RH = (thBuff[5] << 8) | thBuff[4];
    RH = RH & 0x0000FFFF;
    TP = (thBuff[2] << 8) | thBuff[1];
    TP = TP & 0x0000FFFF;
    //RH = RH << 8;
    //RH = RH | RXBuff[1];
    //int RelHum = (int)RH;
    Humid = (float)(100*((float)RH/65536));
    Temp = (float)(32 + 1.8*(-45 + 175*((float)TP/65536)));
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
      th_flag = 0;
      break;
  // Start interrupts STTIFG
  case  6: break;
  // Stop interrupt STPIFG
  case  8: break;
  // RX interrupt: RXIFG
  case 10:

      if (i2c_mode == OUTLET_MODE) {

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

      }

      else if (i2c_mode == SHTC3_MODE) {

          thRXDataCtr--;                            // Decrement RX Byte Counter
          if (thRXDataCtr)
          {
              *(thRXData + thRXDataCtr) = UCB0RXBUF;  // Move RX Buffer data into local buffer
              if (thRXDataCtr == 1)                 // 1 byte left
              {
                  UCB0CTL1 |= UCTXSTP;            // I2C stop condition
                  th_flag = 0;
              }
          }
          else
          {
              *thRXData = UCB0RXBUF;                // Move last byte into the local buffer
          }

      }

      break;
  // Transmit interrupt TXIFG
  case 12:

      if (i2c_mode == OUTLET_MODE) {

          if (TXDataCtr < 1) {
              // Load data into TX buffer
              UCB0TXBUF = TXData;
              // increment the bytes sent counter
              TXDataCtr++;
          }
          // If the byte was sent, send a stop condition
          else {
              // Load data into TX buffer
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
      }

      else if (i2c_mode == SHTC3_MODE) {

          if (thTXDataCtr)
          {
              UCB0TXBUF = *(thTXData++);            // Load TX Buffer
              thTXDataCtr--;                        // Decrement TX byte counter
          }
          else
          {
              UCB0CTL1 |= UCTXSTP;                // Send stop condition
              th_flag = 0;
              UCB0IFG &= ~UCTXIFG;                // Clear TX interrupt flag
          }
      }
      break;
  default: break;
  }
}
