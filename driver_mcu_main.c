#include <msp430.h>
#include "w5500.h"
#include "msp_config.h"
#include "msp_server.h"
#include "i2c.h"
#include "math_engine.h"
#include "strings.h"

// server network characteristics
const uint8_t sourceIP[4] = {192, 168, 1, 254}; // local IP of msp430
const uint8_t gatewayIP[4] = {192, 168, 1, 1}; // gateway IP
const uint8_t subnetMask[4] = {255, 255, 255, 0}; // subnet mask

// flag for timer interrupt
uint8_t connection_time_A0;
uint8_t connection_time_A1;

// outlet i2c variables
extern uint8_t screen_state;
extern uint8_t outlet_status;
extern uint8_t button_state;
extern uint8_t display_status;
extern uint8_t toggle_status;
extern uint8_t update_status;
extern uint8_t refresh_screen_status;

// i2c addresses of outlet board mcus
#define NUM_I2_MCU  2
// store the addresses of the outlet board mcus
uint8_t i2_addrs[NUM_I2_MCU] = {0x33, 0x44};
// store an array of the structs that keep track of outlet info for each outlet board mcu
struct outlet_struct outlet_infos[NUM_I2_MCU];
// store a struct to hold temperature and humidity data
struct th_struct th_info;

void send_update(uint8_t sn, uint16_t port);
void start_timerA0(void);
void stop_timerA0(void);
void start_timerA1(void);
void stop_timerA1(void);
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
    // configure the i2c peripheral
    i2c_init();
    // initialize the outlet status
    outlet_status = 1;
    button_state = 0;
    toggle_status = 0;
    display_status = 0;
    update_status = 0;
    refresh_screen_status = 0;

    // configure and start the timerA0
    start_timerA0();
    start_timerA1();

    // flag to indicate timer interrupt
    connection_time_A0 = 0;
    connection_time_A1 = 0;

    // enable interrupts globally
    _enable_interrupts();

    // main while loop
    while(1) {
        // handle updates
        if (update_status) {
            TA0CCTL0 &= ~CCIE;
            TA1CCTL0 &= ~CCIE;
            // get all info from outlets via i2c
            i2c_receive_outlet(i2_addrs[0]);
            get_outlet_info(&outlet_infos[0]);
            while(update_status);
            update_status = 1;
            i2c_receive_outlet(i2_addrs[1]);
            get_outlet_info(&outlet_infos[1]);
            while(update_status);
            update_status = 1;
            // get all info from temperature and humidity sensor
            shtc3_i2c();
            get_th_info(&th_info);
            while(update_status);
            TA0CCTL0 |= CCIE;
            TA1CCTL0 |= CCIE;
            // update the strings
            format_strings(outlet_infos, &th_info);
        }
        // handle timer interrupt for out bound web server
        if (connection_time_A0) {
            TA0CCTL0 &= ~CCIE;
            TA1CCTL0 &= ~CCIE;
            net_process_socket_sender(0, 200);
            TA0CCTL0 |= CCIE;
            TA1CCTL0 |= CCIE;
            connection_time_A0 = 0;
        }
        // handle timer interrupt for in bound web server
        if (connection_time_A1) {
            TA0CCTL0 &= ~CCIE;
            TA1CCTL0 &= ~CCIE;
            net_process_socket_receiver(1, 201);
            TA0CCTL0 |= CCIE;
            TA1CCTL0 |= CCIE;
            connection_time_A1 = 0;
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
    TA0CCR0 =  20480;                     // Interrupt once every 5 second
}

// stop TA0
void stop_timerA0() {
    TA0CCR0 =  0;                     // Interrupt once every 5 seconds
}

// configure and start TA0
void start_timerA1() {
    // configure timer A
    TA1CCTL0 = CCIE;                      // CCR0 interrupt enabled
    TA1CTL = TASSEL_1 + MC_1 + ID_3;      // SMCLK/8, upmode
    TA1CCR0 =  250;                     // Interrupt once every half second
}

// stop TA0
void stop_timerA1() {
    TA1CCR0 =  0;                     // Interrupt once every 5 seconds
}


// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
{
    _disable_interrupts();
    update_status = 1;
    _enable_interrupts();
}

// Timer A1 interrupt service routine
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer_A1 (void)
{
    _disable_interrupts();
    connection_time_A1 = 1;
    connection_time_A0 = 1;
    _enable_interrupts();
}
