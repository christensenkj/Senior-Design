#ifndef LCDLIB_H_
#define LCDLIB_H_

#include <msp430.h>
#include <string.h>
#include <stdio.h>

// Delay Functions
//#define delay_ms(x)     __delay_cycles((long) x * 1000 * 16)
//#define delay_us(x)     __delay_cycles((long) x * 16)

#define delay_ms(x)     __delay_cycles((long) x * 1000 )
#define delay_us(x)     __delay_cycles((long) x )


// Pins

// RS Pins
#define RS_PORT P5OUT
#define RS_DIR  P5DIR
#define RS_PIN  BIT4    // 6.5

// EN Pins
#define EN_PORT P5OUT
#define EN_DIR  P5DIR
#define EN_PIN  BIT1    // 6.4

// RW Pins
#define RW_PORT P5OUT
#define RW_DIR  P5DIR
#define RW_REN  P5REN
#define RW_PIN  BIT0

// Data Pins
#define DATA_0_PORT P6OUT
#define DATA_0_DIR  P6DIR
#define DATA_0_PIN  BIT4

#define DATA_1_PORT P6OUT
#define DATA_1_DIR  P6DIR
#define DATA_1_PIN  BIT5

#define DATA_2_PORT P6OUT
#define DATA_2_DIR  P6DIR
#define DATA_2_PIN  BIT6

#define DATA_3_PORT P6OUT
#define DATA_3_DIR  P6DIR
#define DATA_3_PIN  BIT7

#define DATA_4_PORT P7OUT
#define DATA_4_DIR  P7DIR
#define DATA_4_PIN  BIT0

#define DATA_5_PORT P7OUT
#define DATA_5_DIR  P7DIR
#define DATA_5_PIN  BIT1

#define DATA_6_PORT P7OUT
#define DATA_6_DIR  P7DIR
#define DATA_6_PIN  BIT2

#define DATA_7_PORT P7OUT
#define DATA_7_DIR  P7DIR
#define DATA_7_PIN  BIT3

// RS(Register Select) LOW and HIGH macros
#define RS_COMMAND_MODE  RS_PORT &= ~RS_PIN
#define RS_DATA_MODE     RS_PORT |= RS_PIN
// Enable LOW and HIGH macros
#define ENABLE_LOW       EN_PORT &= ~EN_PIN
#define ENABLE_HIGH      EN_PORT |= EN_PIN

// Commands
#define CLEAR   0x01

// LCD Row Addresses
#define LCD_ADDRESS_ROW1 0x80
#define LCD_ADDRESS_ROW2 0xC0
#define LCD_ADDRESS_ROW3 0x94
#define LCD_ADDRESS_ROW4 0xD4

// Functions
void lcdInit();                                 // Initialize LCD
void lcdTriggerEN();                                  // Trigger Enable
void lcdWriteData(unsigned char data);          // Send Data (Characters)
void lcdWriteCmd(unsigned char cmd);            // Send Commands
void lcdClear();                                // Clear LCD
void lcdSetText(char *text, int x, int y);     // Write string
void lcdSetInt(int val, int x, int y);          // Write integer
void lcdWrite4Bits(unsigned char val);
void lcdWrite8Bits(unsigned char byte);
void lcdSendByte(unsigned char byte);

#endif /* LCDLIB_H_ */
