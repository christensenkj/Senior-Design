#include "LCD.h"
#include "string.h"

#define LOWNIB(x)   P6OUT = (P6OUT & 0xF0) + (x & 0x0F)

void lcdInit() {
    delay_ms(100);
    // Wait for 100ms after power is applied.

    P6DIR = EN + RS + DATA;  // Make pins outputs
    P6OUT = 0x03;  // Start LCD (send 0x03)

    lcdTriggerEN(); // Send 0x03 3 times at 5ms then 100 us
    delay_ms(5);
    lcdTriggerEN();
    delay_ms(5);
    lcdTriggerEN();
    delay_ms(5);

    P6OUT = 0x02; // Switch to 4-bit mode
    lcdTriggerEN();
    delay_ms(5);

    lcdWriteCmd(0x28); // 4-bit, 2 line, 5x8
    lcdWriteCmd(0x08); // Instruction Flow
    lcdWriteCmd(0x06); // Auto-Increment
    lcdWriteCmd(0x0C); // Display On, No blink
    lcdWriteCmd(0x01); // Clear LCD
}

void lcdTriggerEN() {
    P6OUT |= EN;
    P6OUT &= ~EN;
}

void lcdWriteData(unsigned char data) {
    _disable_interrupts();
    P6OUT |= RS; // Set RS to Data
    LOWNIB(data >> 4); // Upper nibble
    lcdTriggerEN();
    LOWNIB(data); // Lower nibble
    lcdTriggerEN();
    delay_us(50); // Delay > 47 us
    _enable_interrupts();
}

void lcdWriteCmd(unsigned char cmd) {
    _disable_interrupts();
    P6OUT &= ~RS; // Set RS to Data
    LOWNIB(cmd >> 4); // Upper nibble
    lcdTriggerEN();
    LOWNIB(cmd); // Lower nibble
    lcdTriggerEN();
    delay_ms(5); // Delay > 1.5ms
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

