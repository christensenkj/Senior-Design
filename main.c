#include <msp430.h>
#include "w5500.h"
#include "msp_config.h"
#include "msp_server.h"
#include "LCD.h"
#include "in_person_UI.h"
#include "i2c.h"

// define buttons
#define SW_1    BIT1
#define SW_2    BIT1

// counter for proof of concept for placeholder for temperature, humidity, voltage, current, and power values
uint8_t poc_counter = 48;
uint8_t screen_state;


// server network characteristics
const uint8_t sourceIP[4] = {192, 168, 1, 254}; // local IP of msp430
const uint8_t gatewayIP[4] = {192, 168, 1, 1}; // gateway IP
const uint8_t subnetMask[4] = {255, 255, 255, 0}; // subnet mask

// define an error code function using P4.7 LED
void blink_error_code(void);
void send_update(uint8_t sn, uint16_t port);
void receive_from_server(uint8_t sn, uint16_t port);

int main(void) {
    // stop the watchdog timer
     WDTCTL = WDTPW | WDTHOLD;

    // configure timer A
    TA0CCTL0 = CCIE;                      // CCR0 interrupt enabled
    TA0CTL = TASSEL_1 + MC_1 + ID_3;      // SMCLK/8, upmode
    TA0CCR0 =  20480/2;                     // Interrupt once every 5 seconds

//    TA1CCTL0 = CCIE;                      // CCR0 interrupt enabled
//    TA1CTL = TASSEL_1 + MC_1 + ID_3;      // SMCLK/8, upmode
//    TA1CCR0 =  10;                     // Interrupt once every 5 seconds

    //configure the pins

    // initialize buttons with interrupts
    P1DIR &= ~SW_1;                       // Set SW pin -> Input
    P1REN |= SW_1;                        // Enable Resistor for SW pin
    P1OUT |= SW_1;                        // Select Pull Up for SW pin
    P1IES &= ~SW_1;                       // Select Interrupt on Rising Edge
    P1IE |= SW_1;                         // Enable Interrupt on SW pin

    P2DIR &= ~SW_2;                       // Set SW pin -> Input
    P2REN |= SW_2;                        // Enable Resistor for SW pin
    P2OUT |= SW_2;                        // Select Pull Up for SW pin
    P2IES &= ~SW_2;                       // Select Interrupt on Rising Edge
    P2IE |= SW_2;                         // Enable Interrupt on SW pin

    // configure the msp430 clock
    if (msp_config_clock() != 0) {
        // error code
        blink_error_code();
    }

    //configure the pins
    if (msp_config_spi_pins() != 0) {
        // error code
        blink_error_code();
    }

    // configure usci controller for SPI
    if (msp_config_usci() != 0) {
        // error code
        blink_error_code();
    }

    // turn on RED LED to indicate reaching this far
    P1DIR |= BIT0;
    P1OUT |= BIT0;

    reset_w5500();
    w5500_config(sourceIP, gatewayIP, subnetMask);

    lcdInit();// Initialize LCD

    screen_state = INIT;
    lcdClear();
    display_screen(screen_state);
    __delay_cycles(50000000);
    lcdClear();
    screen_state = HOME_1;
    display_screen(screen_state);

    init_i2c();

    _enable_interrupts();

    // reset the W5500 after the SPI communication is set up on the msp430
    while(1){
//        send_update(0, 200);
//        receive_from_server(1, 201);
    }
}



void blink_error_code() {
    P4DIR |= BIT7;
    P4OUT |= BIT7;
    // toggle P4.7 LED
    while(1) {
        P4OUT ^= BIT7;
        __delay_cycles(500000);
    }
}

void send_update(uint8_t sn, uint16_t port) {
    // start server with socket 0, port 200
    start_server(sn, port);
    // wait for connection on socket 0
    if (wait_for_connection(sn) == 0) {
        uint16_t len = 100;
        send_data_onitsown(sn, len);
    }
    // close socket 0
    stop_server(sn);
}

void receive_from_server(uint8_t sn, uint16_t port) {
    // start server with socket 0, port 200
    start_server(sn, port);
    // wait for connection on socket 0
    if (wait_for_connection(sn) == 0) {
        // wait for data reception
        wait_for_data(sn);
        uint16_t len = 100;
        receive_cmd(sn, len);
    }
    // close socket 0
    stop_server(sn);
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    _disable_interrupts();
    _delay_cycles(20000);
    P1IFG &= ~SW_1;                       // Clear SW interrupt flag
    switch(screen_state) {
        case HOME_1:
            screen_state = HOME_2;
            break;
        case HOME_2:
            screen_state = TEMP_HUM;
            break;
        case TEMP_HUM:
            screen_state = OUTLET_1;
            break;
        case OUTLET_1:
            screen_state = TOGGLE_OUTLET;
            break;
        case TOGGLE_OUTLET:
            screen_state = TOGGLE_CONF_1;
            break;
        case TOGGLE_CONF_1:
            screen_state = HOME_1;
            break;
        default:
            break;
    }

    display_screen(screen_state);
    _enable_interrupts();
}

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    _disable_interrupts();
    _delay_cycles(20000);
    P2IFG &= ~SW_2;                       // Clear SW interrupt flag
    switch(screen_state) {
        case HOME_1:
            screen_state = TOGGLE_CONF_1;
            break;
        case HOME_2:
            screen_state = HOME_1;
            break;
        case TEMP_HUM:
            screen_state = HOME_2;
            break;
        case OUTLET_1:
            screen_state = TEMP_HUM;
            break;
        case TOGGLE_OUTLET:
            screen_state = OUTLET_1;
            break;
        case TOGGLE_CONF_1:
            screen_state = HOME_1;
            break;
        default:
            break;
    }

    display_screen(screen_state);
    _enable_interrupts();
}


// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
{
//    request_data_i2c();
    send_update(0, 200);
//    receive_from_server(0, 201);
}

// Timer A1 interrupt service routine
//#pragma vector=TIMER0_A1_VECTOR
//__interrupt void Timer_A1 (void)
//{
//    receive_from_server(0, 201);
//}
