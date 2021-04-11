/*
 *
 * Engineer: Bryce Feigum (bafeigum at gmail)
 * Date: December 22, 2012
 *
 * Version: 1.0 - Initial Release
 * Version: 1.1 - Added GNUv2 License
 *
 * Copyright (C) 2013  Bryce Feigum
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * Project: DHT11_LIB
 * File:    DHT11_LIB.c
 */

#include <msp430.h>
#include <DHT11_LIB.h>
#include <stdint.h>

extern uint8_t update_status;

uint8_t temp;
uint8_t hum;
uint8_t RH_byte2;
uint8_t T_byte2;
uint8_t checksum;
extern unsigned char volatile TOUT;

void init_dht11(){
    WDTCTL = WDTPW + WDTHOLD;   // Stop watchdog timer

    P6SEL &= ~DPIN;

    // configure timer A
//    TA2CCR0 =  50000;
    TA2CCTL0 = CCIE;                      // CCR0 interrupt enabled
    TA2CTL = TASSEL_2 + MC_1 + ID_2 + TACLR;      // SMCLK/4, upmode

//    _enable_interrupt();                        //Enable global interrupt
}

unsigned char read_Byte(){
    TOUT = 0;
    unsigned char num = 0;
    unsigned char i;
    CLR(TA2CCTL0,CCIE);
    for (i=8; i>0; i--){
        while(!(TST(P6IN,DPIN))); //Wait for signal to go high
        SET(TA2CTL,TACLR); //Reset timer
        SET(TA2CTL,0x10); //Reenable timer
        SET(TA2CCTL0,CCIE); //Enable timer interrupt
        while(TST(P6IN,DPIN)); //Wait for signal to go low
        CLR(TA2CTL,0x30); //Halt Timer
        if (TA2R > 13)    //40 @ 1x divider
            num |= 1 << (i-1);
    }
    return num;
}

unsigned char read_Packet(unsigned char * data){
    start_Signal();
    if (check_Response()){
        //Cannot be done with a for loop!
        data[0] = read_Byte();
        data[1] = read_Byte();
        data[2] = read_Byte();
        data[3] = read_Byte();
        data[4] = read_Byte();
        return 1;
    }
    else return 0;
}

unsigned char check_Response(){
    TOUT=0;
    SET(TA2CTL,TACLR);   //Reset timer to 0;
    TA2CCR0 = 25;        //Set timer to overflow in 100uS. 100 @ 1x divider
    SET(TA2CCTL0,CCIE);  //And enable timer interrupt
    while(!(TST(P6IN,DPIN)) && !TOUT);
    if (TOUT)
        return 0;
    else {
        SET(TA2CTL,TACLR);
        SET(TA2CCTL0,CCIE);
        while((TST(P6IN,DPIN)) && !TOUT);
        if(TOUT)
            return 0;
        else{
            CLR(TA2CCTL0,CCIE);  // Disable timer interrupt
            return 1;
        }
    }
}

void start_Signal(){
    SET(P6DIR, DPIN);       // Set Data pin to output direction
    CLR(P6OUT, DPIN);        // Set output to low
    __delay_cycles(25000);  // Low for at least 18ms
    SET(P6OUT, DPIN);
    __delay_cycles(30);     // High for at 20us-40us
    CLR(P6DIR, DPIN);        // Set data pin to input direction
}

//returns true if checksum is correct
unsigned char check_Checksum(unsigned char *data){
    if (data[4] != (data[0] + data[1] + data[2] + data[3])){
        return 0;
    }
    else return 1;
}

void receive_th() {
    P1IE &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
    // send i2c transmission to get the t/h info
    // Manual way to gather all data without array
    start_Signal();
    if(check_Response()){
        hum = read_Byte();
        RH_byte2 = read_Byte();
        temp = read_Byte();
        T_byte2 = read_Byte();
        checksum = read_Byte();
    }
    P1IE |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
    update_status = 0;
}

