#include <LCD_new.h>
#include "string.h"

void lcdInit() {
//    delay_ms(100);
    // Wait for 100ms after power is applied.
    // Initialize pins
    RS_DIR |= RS_PIN;
    EN_DIR |= EN_PIN;
    RW_DIR |= RW_PIN;
    RS_PORT &= ~RS_PIN;
    EN_PORT &= ~EN_PIN;
    RW_PORT &= ~RW_PIN;

    DATA_0_DIR |= DATA_0_PIN;
    DATA_1_DIR |= DATA_1_PIN;
    DATA_2_DIR |= DATA_2_PIN;
    DATA_3_DIR |= DATA_3_PIN;
    DATA_0_PORT &= ~DATA_0_PIN;
    DATA_1_PORT &= ~DATA_1_PIN;
    DATA_2_PORT &= ~DATA_2_PIN;
    DATA_3_PORT &= ~DATA_3_PIN;

    DATA_4_DIR |= DATA_4_PIN;
    DATA_5_DIR |= DATA_5_PIN;
    DATA_6_DIR |= DATA_6_PIN;
    DATA_7_DIR |= DATA_7_PIN;
    DATA_4_PORT &= ~DATA_4_PIN;
    DATA_5_PORT &= ~DATA_5_PIN;
    DATA_6_PORT &= ~DATA_6_PIN;
    DATA_7_PORT &= ~DATA_7_PIN;

    delay_ms(100);


//    lcdTriggerEN();
//    delay_ms(100);
//    lcdTriggerEN();
    // Reset the LCD
    P7OUT = 0x03;
    delay_ms(30);
    lcdTriggerEN();
    delay_ms(10);
    lcdTriggerEN();
    delay_ms(10);
    lcdTriggerEN();
    delay_ms(10);
    P7OUT = 0x02;
    lcdTriggerEN();
//    delay_ms(5);

//    delay_ms(100);
//    // Reset the LCD
//    P7OUT = 0x03;
//    lcdTriggerEN();
//    delay_ms(5);
//    lcdTriggerEN();
//    delay_ms(5);
//    lcdTriggerEN();
//    delay_ms(5);
//    P7OUT = 0x02;
//    lcdTriggerEN();
//    delay_ms(5);
//

    // Send commands to initialize LCD
    lcdWriteCmd(0x28); // 4-bit, 2 line, 5x8
//    delay_ms(10);
    lcdWriteCmd(0x08); // Instruction Flow
//    delay_ms(10);
    lcdWriteCmd(0x0C); // Display On, No blink
//    delay_ms(10);
//    lcdWriteCmd(0x01); // Clear LCD
//    delay_ms(10);
    lcdWriteCmd(0x06); // Auto-Increment
//    delay_ms(10);
}

void lcdTriggerEN() {
    ENABLE_HIGH;
    delay_ms(1);
    ENABLE_LOW;
}

void lcdWrite8Bits(unsigned char byte)
{
    if(byte & 0x80)
        DATA_7_PORT |=  DATA_7_PIN;
    else
        DATA_7_PORT &= ~DATA_7_PIN;
    if(byte & 0x40)
        DATA_6_PORT |=  DATA_6_PIN;
    else
        DATA_6_PORT &= ~DATA_6_PIN;
    if(byte & 0x20)
        DATA_5_PORT |=  DATA_5_PIN;
    else
        DATA_5_PORT &= ~DATA_5_PIN;
    if(byte & 0x10)
        DATA_4_PORT |=  DATA_4_PIN;
    else
        DATA_4_PORT &= ~DATA_4_PIN;
    if(byte & 0x08)
        DATA_3_PORT |=  DATA_3_PIN;
    else
        DATA_3_PORT &= ~DATA_3_PIN;
    if(byte & 0x04)
        DATA_2_PORT |=  DATA_2_PIN;
    else
        DATA_2_PORT &= ~DATA_2_PIN;
    if(byte & 0x02)
        DATA_1_PORT |=  DATA_1_PIN;
    else
        DATA_1_PORT &= ~DATA_1_PIN;
    if(byte & 0x01)
        DATA_0_PORT |=  DATA_0_PIN;
    else
        DATA_0_PORT &= ~DATA_0_PIN;
    lcdTriggerEN();
}


void lcdWrite4Bits(unsigned char val)
{
    if(val & 0x08)
        DATA_7_PORT |=  DATA_7_PIN;
    else
        DATA_7_PORT &=  ~DATA_7_PIN;
    if(val & 0x04)
        DATA_6_PORT |=  DATA_6_PIN;
    else
        DATA_6_PORT &=  ~DATA_6_PIN;
    if(val & 0x02)
        DATA_5_PORT |=  DATA_5_PIN;
    else
        DATA_5_PORT &=  ~DATA_5_PIN;
    if(val & 0x01)
        DATA_4_PORT |=  DATA_4_PIN;
    else
        DATA_4_PORT &=  ~DATA_4_PIN;
//    P7OUT = val;
    lcdTriggerEN();
}

void lcdSendByte(unsigned char byte)
{
//    lcdWrite8Bits(byte);
    lcdWrite4Bits(byte >> 4);
    lcdWrite4Bits(byte);
//    delay_ms(5);
}

void lcdWriteData(unsigned char data) {
    _disable_interrupts();
    RS_DATA_MODE;
    lcdSendByte(data);
    _enable_interrupts();
}

void lcdWriteCmd(unsigned char cmd) {
    _disable_interrupts();
    RS_COMMAND_MODE;
    lcdSendByte(cmd);
    _enable_interrupts();
}



void lcdSetText(char* text, int x, int y) {
    int i = 0;
    int cmd = 0x80;
    switch (x){
    case 0:
        break;
    case 1:
        cmd |= 0x40; // Set LCD for second line write
        break;
    case 2:
        cmd |= 0x14; // Set LCD for third line write
        break;
    case 3:
        cmd |= 0x54; // Set LCD for fourth line write
        break;
    default:
        break;
    }
    cmd += y;
    lcdWriteCmd(cmd);

    while (text[i] != '\0') {
        lcdWriteData(text[i]);
        i++;
    }
}

void lcdSetInt(int val, int x, int y){
    char number_string[16];
    sprintf(number_string, "%d", val); // Convert the integer to character string
    lcdSetText(number_string, x, y);
}

void lcdClear() {
    lcdWriteCmd(CLEAR);
    delay_ms(5);
}
