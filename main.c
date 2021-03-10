#include <msp430.h>
#include "w5500.h"
#include "msp_config.h"
#include "msp_server.h"

// server network characteristics
const uint8_t sourceIP[4] = {192, 168, 1, 254}; // local IP of msp430
const uint8_t gatewayIP[4] = {192, 168, 1, 1}; // gateway IP
const uint8_t subnetMask[4] = {255, 255, 255, 0}; // subnet mask

// flag for timer interrupt
uint8_t connection_time;

void send_update(uint8_t sn, uint16_t port);
void start_timerA0(void);
void stop_timerA0(void);
void stop_watchdog(void);

int main(void) {
    // stop the watchdog timer
    stop_watchdog();

    // configure the msp430 clock
    if (msp_config_clock() != 0) {
        // error code
       while(1);
    }

    //configure the pins
    if (msp_config_spi_pins() != 0) {
        // error code
        while(1);
    }

    // configure usci controller for SPI
    if (msp_config_usci() != 0) {
        // error code
        while(1);
    }

    // reset the w5500
    reset_w5500();
    // configure the w5500
    w5500_config(sourceIP, gatewayIP, subnetMask);

    // configure and start the timerA0
    start_timerA0();
    P4DIR |= BIT7;
    P4OUT |= BIT7;

    // flag to indicate timer interrupt
    connection_time = 0;

    // enable interrupts globally
    _enable_interrupts();

    // main while loop
    while(1) {

        // handle timer interrupts
        if (connection_time) {
            P4DIR |= BIT7;
            P4OUT ^= BIT7;
            send_update(0, 200);
            connection_time = 0;
        }
    }
}

// stop the watchdog timer
void stop_watchdog() {
    // stop the watchdog timer
     WDTCTL = WDTPW | WDTHOLD;
}

// configure and start TA0
void start_timerA0() {
    // configure timer A
    TA0CCTL0 = CCIE;                      // CCR0 interrupt enabled
    TA0CTL = TASSEL_1 + MC_1 + ID_3;      // SMCLK/8, upmode
    TA0CCR0 =  20480/10;                     // Interrupt once every half second
}

// stop TA0
void stop_timerA0() {
    TA0CCR0 =  0;                     // Interrupt once every 5 seconds
}

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
{
    _disable_interrupts();
    connection_time = 1;
    _enable_interrupts();
}
