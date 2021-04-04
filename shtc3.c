/*
 * shtc3.c
 *
 *  Created on: Apr 1, 2021
 *      Author: Zach Wilson
 */

#include "shtc3.h"

volatile uint8_t TXByteCtr;
volatile uint8_t *TXData;
volatile uint8_t RXByteCtr;
volatile uint8_t *RXData;
volatile uint8_t RXBuff[6];
volatile uint8_t I2C_flag;
volatile uint32_t RH;
volatile uint32_t TP;
volatile float Humid;
volatile float Temp;
volatile struct RET_STRUCT ret_struct;

// Configuration function for initializing the I2C communication on the USCIB0 bus system
void shtc3_i2c_config(void){
    UCB0CTL1 |= UCSWRST;                    // Put I2C into reset mode
    UCB0CTL0 |= UCMST + UCSYNC + UCMODE_3;  // I2C master mode; Synchronous; I2C Mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;          // Set SMCLK for I2C
    UCB0BR0 = 255;                          // fSCL = SMCLK/255 = appx 100kHz
    UCB0BR1 = 0;
    UCB0I2CSA = 0x70;                       // SHTC3 address
    UCB0CTL1 &= ~UCSWRST;                   // Take I2C out of Reset
}

// CALL FUNCTION: This is the function to initiate conversation with SHTC3 over USCIB0
// Scheduled, then called once the timer finishes the interrupt routine,
void shtc3_i2c(void)
{
    UCB0IE |= UCTXIE + UCRXIE;              // Enable TX and RX interrupts on I2C
    Wakeup();
    Measure();
    UCB0IFG &= ~UCNACKIFG;                  // Clear Nack interrupt flag
    UCB0IE |= UCNACKIE;                     // Enable Nack interrupts
    Read();
    UCB0IE &= ~UCNACKIE;
    Sleep();
    Interpret();
    P1OUT &= ~BIT0;                         // Reset the LED to prove out of I2C
}

// Wakeup the SHTC3 from sleep mode
void Wakeup(void)
{
    UCB0CTL1 |= UCTR;                       // Set as transmitter
    while (UCB0CTL1 & UCTXSTP);
    TXByteCtr = 2;                          // Load TX Byte Counter
    uint8_t wakeup_command[] = {0x35,0x17}; // Wakeup command for SHTC3
    TXData = (uint8_t *)wakeup_command;     // Grab the pointer for the command
    I2C_flag = 1;
    UCB0CTL1 |= UCTXSTT;                    // TX start
    while(I2C_flag);
    while (UCB0CTL1 & UCTXSTP);             // Ensure 1st stop sent before continuing
    __delay_cycles(100);                    // Waking up takes time. Let the baby arise
}

// Initiate the process of measuring humidity and temperature respectively
void Measure(void)
{
    UCB0CTL1 |= UCTR;                       // Set as transmitter
    TXByteCtr = 2;                          // Load TX Byte counter
    uint8_t measure_command[] = {0x58,0xE0};// Measure command for low power mode and no clock stretching
    TXData = (uint8_t *)measure_command;    // Grab the pointer for the command
    I2C_flag = 1;
    UCB0CTL1 |= UCTXSTT;                    // TX start
    while(I2C_flag);
    while (UCB0CTL1 & UCTXSTP);             // Ensure 2nd stop sent before continuing
}

// Start reading the humidity and temperature values from the I2C line
void Read(void)
{
    UCB0CTL1 &= ~UCTR;                      // Set as receiver
    RXByteCtr = 6;                          // Load RX Byte counter
    RXData = (uint8_t *)RXBuff;             // Set pointer to the buffer
    I2C_flag = 1;
    UCB0CTL1 |= UCTXSTT;
    while(I2C_flag);
    while (UCB0CTL1 & UCTXSTP);
}

// Put the SHTC3 in sleep mode
void Sleep(void)
{
    UCB0CTL1 |= UCTR;                       // Set as transmitter
    TXByteCtr = 2;                          // Load TX Byte counter
    uint8_t sleep_command[] = {0xB0,0x98};  // Sleep command
    TXData = (uint8_t *)sleep_command;      // Grab pointer to the command
    I2C_flag = 1;
    UCB0CTL1 |= UCTXSTT;                    // TX Start
    while(I2C_flag);
    while (UCB0CTL1 & UCTXSTP);             // Ensure 4th stop condition sent
}

// Do the maths for figuring out relative humidity, and temperature in fahrenheit
void Interpret(void)
{
    RH = (RXBuff[5] << 8) | RXBuff[4];
    RH = RH & 0x0000FFFF;
    TP = (RXBuff[2] << 8) | RXBuff[1];
    TP = TP & 0x0000FFFF;
    Humid = (float)(100*((float)RH/65536));
    Temp = (float)(32 + 1.8*(-45 + 175*((float)TP/65536)));
    ret_struct.RH = Humid;
    ret_struct.Temp = Temp;
}

// Return the computed value of relative humidity
float returnRH(void) {
    return ret_struct.RH;
}

// Return the computed value of temperature
float returnTemp(void) {
    return ret_struct.Temp;
}

// USCIB0 ISR
#pragma vector = USCI_B0_VECTOR
__interrupt void I2C_ISR (void)
{
    switch(__even_in_range(UCB0IV,12))
    {
    case 0: break;                          // Vector 0: No interrupts
    case 2: break;                          // Vector 2: ALIFG
    case 4:                                 // Vector 4: NACKIFG
        UCB0IFG &= ~UCNACKIFG;                  // Clear the interrupt flag
        UCB0CTL1 |= UCTXSTT;                    // Start condition
        break;
    case 6: break;                          // Vector 6: STTIFG
    case 8: break;                          // Vector 8: STPIFG
    case 10:                                // Vector 10: RXIFG
        RXByteCtr--;                            // Decrement RX Byte Counter
        if (RXByteCtr)
        {
            *(RXData + RXByteCtr) = UCB0RXBUF;  // Move RX Buffer data into local buffer
            if (RXByteCtr == 1)                 // 1 byte left
            {
                UCB0CTL1 |= UCTXSTP;            // I2C stop condition
                I2C_flag = 0;
            }
        }
        else
        {
            *RXData = UCB0RXBUF;                // Move last byte into the local buffer
        }
        break;
    case 12:                                // Vector 12: TXIFG
        if (TXByteCtr)
        {
            UCB0TXBUF = *(TXData++);            // Load TX Buffer
            TXByteCtr--;                        // Decrement TX byte counter
        }
        else
        {
            UCB0CTL1 |= UCTXSTP;                // Send stop condition
            I2C_flag = 0;
            UCB0IFG &= ~UCTXIFG;                // Clear TX interrupt flag
        }
        break;
    default: break;
    }
}
