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
 * File:    main.c
 */
#include <msp430.h>
#include <DHT11_LIB.h>

unsigned char RH_byte1;
unsigned char RH_byte2;
unsigned char T_byte1;
unsigned char T_byte2;
unsigned char checksum;

unsigned char Packet[5];

void init(void);

unsigned char volatile TOUT;            //REQUIRED for library
unsigned char volatile SECOND_TIMER=0;

#pragma vector = TIMER0_A0_VECTOR
__interrupt void CCR0_ISR(void){
        SECOND_TIMER++;
        TOUT=1;
        TOG (P1OUT,BIT0);
        CLR (TA0CCTL0, CCIFG);
}

void main(void) {
    init();

    while(1){
        //Must wait 1 second initially and between all reads
        if(SECOND_TIMER >= 5){      //5 @ CCR0 = 50000 & div 4

//            //  Simple way to gather all data with one instruction
//            read_Packet(Packet);
//            RH_byte1 =  Packet[0];
//            RH_byte2 =  Packet[1];
//            T_byte1 =   Packet[2];
//            T_byte2 =   Packet[3];
//            checksum =  Packet[4];
//
//            if (check_Checksum(Packet))
//                SET (P4OUT, BIT7);

            // Manual way to gather all data without array
            start_Signal();
            if(check_Response()){
                RH_byte1 = read_Byte();
                RH_byte2 = read_Byte();
                T_byte1 = read_Byte();
                T_byte2 = read_Byte();
                checksum = read_Byte();
            }

            if (checksum == (RH_byte1 + RH_byte2 + T_byte1 + T_byte2))
                SET (P4OUT, BIT7);


            SET (TA0CTL, TACLR);
            SET (TA0CTL, 0x10);
            TA0CCR0 = 50000;     //Initialize the timer to count at 5Hz
            SECOND_TIMER = 0;   //Clear counter
        }
    }
}

void init(){
    WDTCTL = WDTPW + WDTHOLD;   // Stop watchdog timer

    P1OUT &= ~BIT0;
    P4OUT &= ~BIT7;
    P1DIR |= BIT0;
    P4DIR |= BIT7;
    P6SEL &= ~DPIN;

    // configure timer A
    TA0CCR0 =  50000;
    TA0CCTL0 = CCIE;                      // CCR0 interrupt enabled
    TA0CTL = TASSEL_2 + MC_1 + ID_2 + TACLR;      // SMCLK/4, upmode

    _enable_interrupt();                        //Enable global interrupt
}
