#include <msp430.h>
#include "w5500.h"
#include "msp_config.h"
#include "msp_server.h"
#include "i2c.h"
#include "math_engine.h"
#include "strings.h"
#include "LCD_new.h"
#include "in_person_UI.h"
#include "DHT11_LIB.h"

// server network characteristics
const uint8_t sourceIP[4] = {192, 168, 1, 254}; // local IP of msp430
const uint8_t gatewayIP[4] = {192, 168, 1, 1}; // gateway IP
const uint8_t subnetMask[4] = {255, 255, 255, 0}; // subnet mask

// flag for timer interrupt
uint8_t connection_time_A0;
uint8_t connection_time_A1;

// outlet i2c variables
extern uint8_t screen_state;
extern uint8_t button_state;
extern uint8_t display_status;
extern uint8_t toggle_status;
extern uint8_t update_status;
extern uint8_t refresh_screen_status;
uint8_t toggle_ready = 0;

// switching variables
uint8_t i2_address;
uint8_t outlet_num;
uint8_t outlet_num_abs;
extern uint8_t outlet_statuses[6];

// i2c addresses of outlet board mcus
#define NUM_I2_MCU  2
// store the addresses of the outlet board mcus
uint8_t i2_addrs[NUM_I2_MCU] = {0x33, 0x44};
// store an array of the structs that keep track of outlet info for each outlet board mcu
struct outlet_struct outlet_infos[NUM_I2_MCU];
// store a struct to hold temperature and humidity data
struct th_struct th_info;

// variable for DHT11
unsigned char volatile TOUT;

void send_update(uint8_t sn, uint16_t port);
void start_timerA0(void);
void stop_timerA0(void);
void start_timerA1(void);
void stop_timerA1(void);
void stop_watchdog(void);
void init_buttons(void);


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
    // initialize buttons
    init_buttons();
    // initialize LCD
    lcdInit();
    // initialize the outlet status
    button_state = 0;
    toggle_status = 0;
    display_status = 0;
    update_status = 0;
    refresh_screen_status = 0;

    // Display startup screen
    screen_state = INIT;
    display_screen(screen_state);
    __delay_cycles(50000000);
    screen_state = HOME_1;
    display_screen(screen_state);


    // flag to indicate timer interrupt
    connection_time_A0 = 0;
    connection_time_A1 = 0;
    // initialize the outlet status
    button_state = 0;
    toggle_status = 0;
    display_status = 0;
    update_status = 0;
    refresh_screen_status = 0;
    outlet_num_abs = 1;

    // configure and start the timerA0
    start_timerA0();
    start_timerA1();
    init_dht11();
    // enable interrupts globally
    _enable_interrupts();

    // main while loop
    while(1) {
        // handle button toggles
        if (toggle_status) {
            TA0CCTL0 &= ~CCIE;
            P1IE &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
            i2c_send_toggle(i2_address, outlet_num);
            while(toggle_status);
            // update the LCD screens with the new information
            refresh_screen_status = 1;
            toggle_ready = 1;
            P1IE |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
            TA0CCTL0 |= CCIE;
        }

        // handle updates
        if (update_status) {
            TA0CCTL0 &= ~CCIE;
            // get all info from outlets via i2c
            i2c_receive_outlet(i2_addrs[0]);
            while(update_status);
            get_outlet_info(&outlet_infos[0]);
            update_status = 1;
            i2c_receive_outlet(i2_addrs[1]);
            while(update_status);
            get_outlet_info(&outlet_infos[1]);
            update_status = 1;
            // get temp/hum information via dht11
            receive_th();
            // wait for the update to finish
            while(update_status);
            get_th_info(&th_info);
            TA0CCTL0 |= CCIE;
            // update the strings
            format_strings(outlet_infos, &th_info);
            // update the LCD screens with the new information
            refresh_screen_status = 1;
        }
        // refresh the screen
        if (refresh_screen_status) {
            if ((screen_state == TOGGLE_CONF_1_y) || (screen_state == TOGGLE_CONF_1_n) || (screen_state == TOGGLE_CONF_2_y) || (screen_state == TOGGLE_CONF_2_n)
                    || (screen_state == TOGGLE_CONF_3_y) || (screen_state == TOGGLE_CONF_3_n) || (screen_state == TOGGLE_CONF_4_y) || (screen_state == TOGGLE_CONF_4_n)
                    || (screen_state == TOGGLE_CONF_5_y) || (screen_state == TOGGLE_CONF_5_n) || (screen_state == TOGGLE_CONF_6_y) || (screen_state == TOGGLE_CONF_6_n)) {
                if (toggle_ready) {
                    // update the LCD screens with the new information
                    update_screen(screen_state, outlet_num_abs);
                    display_status = 1;
                    refresh_screen_status = 0;
                    toggle_ready = 0;
                }
            }
            else {
                // update the LCD screens with the new information
                update_screen(screen_state, outlet_num_abs);
                // display the new screens
                if ((screen_state == TEMP_HUM) || (screen_state == OUTLET_1) || (screen_state == OUTLET_2) || (screen_state == OUTLET_3) || (screen_state == OUTLET_4)
                        || (screen_state == OUTLET_5) || (screen_state == OUTLET_6)) {
                    display_status = 1;
                }
                refresh_screen_status = 0;
            }
        }
        // update display
        if (display_status) {
            display_screen(screen_state);
            display_status = 0;
        }
        // handle timer interrupt for out bound web server
        if (connection_time_A0) {
            TA0CCTL0 &= ~CCIE;
            TA1CCTL0 &= ~CCIE;
            P1IE &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
            net_process_socket_sender(0, 200);
            TA0CCTL0 |= CCIE;
            TA1CCTL0 |= CCIE;
            P1IE |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
            connection_time_A0 = 0;
        }
        // handle timer interrupt for in bound web server
        if (connection_time_A1) {
            TA0CCTL0 &= ~CCIE;
            TA1CCTL0 &= ~CCIE;
            P1IE &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
            net_process_socket_receiver(1, 201);
            TA0CCTL0 |= CCIE;
            TA1CCTL0 |= CCIE;
            P1IE |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
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

// Initialize buttons
void init_buttons() {
    // Set SW pin -> Input
    P1DIR &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
    // Enable Resistor for SW pin
    P1REN |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
    // Select Pull Up for SW pin
    P1OUT |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
    // Select Interrupt on Falling Edge
    P1IES |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
    // Enable Interrupt on SW pin
    P1IE |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
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

#pragma vector = TIMER2_A0_VECTOR
__interrupt void Timer_A2(void){
    TOUT=1;
    CLR (TA2CCTL0, CCIFG);
}

// Button Interrupt
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    // disable necessary interrupts
    P1IE &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
    TA0CCTL0 &= ~CCIE;
    __delay_cycles(8000);
    uint8_t update_state = 0;

    // if we have a falling edge (button is pressed), we execute everything
    if (button_state == 0) {
        // back button
        if (P1IFG & BIT0) {
            // Clear the interrupt
            P1IFG &= ~BIT0;
            // Select Interrupt on Rising Edge
            P1IES &= ~BIT0;
            // update the screen and logic state
            switch(screen_state) {
                case TEMP_HUM:
                    screen_state = HOME_1;
                    break;
                case OUTLET_1:
                    screen_state = HOME_2;
                    break;
                case OUTLET_2:
                    screen_state = HOME_3;
                    break;
                case OUTLET_3:
                    screen_state = HOME_4;
                    break;
                case OUTLET_4:
                    screen_state = HOME_5;
                    break;
                case OUTLET_5:
                    screen_state = HOME_6;
                    break;
                case OUTLET_6:
                    screen_state = HOME_7;
                    break;
                case TOGGLE_OUTLET_1:
                    screen_state = HOME_8;
                    break;
                case TOGGLE_OUTLET_2:
                    screen_state = HOME_8;
                    break;
                case TOGGLE_OUTLET_3:
                    screen_state = HOME_8;
                    break;
                case TOGGLE_OUTLET_4:
                    screen_state = HOME_8;
                    break;
                case TOGGLE_OUTLET_5:
                    screen_state = HOME_8;
                    break;
                case TOGGLE_OUTLET_6:
                    screen_state = HOME_8;
                    break;
                case TOGGLE_CONF_1_y:
                    screen_state = TOGGLE_OUTLET_1;
                    break;
                case TOGGLE_CONF_1_n:
                    screen_state = TOGGLE_OUTLET_1;
                    break;
                case TOGGLE_CONF_2_y:
                    screen_state = TOGGLE_OUTLET_2;
                    break;
                case TOGGLE_CONF_2_n:
                    screen_state = TOGGLE_OUTLET_2;
                    break;
                case TOGGLE_CONF_3_y:
                    screen_state = TOGGLE_OUTLET_3;
                    break;
                case TOGGLE_CONF_3_n:
                    screen_state = TOGGLE_OUTLET_3;
                    break;
                case TOGGLE_CONF_4_y:
                    screen_state = TOGGLE_OUTLET_4;
                    break;
                case TOGGLE_CONF_4_n:
                    screen_state = TOGGLE_OUTLET_4;
                    break;
                case TOGGLE_CONF_5_y:
                    screen_state = TOGGLE_OUTLET_5;
                    break;
                case TOGGLE_CONF_5_n:
                    screen_state = TOGGLE_OUTLET_5;
                    break;
                case TOGGLE_CONF_6_y:
                    screen_state = TOGGLE_OUTLET_6;
                    break;
                case TOGGLE_CONF_6_n:
                    screen_state = TOGGLE_OUTLET_6;
                    break;
                default:
                    break;
            }
        }

        // up button
        if (P1IFG & BIT1) {
            // Clear the interrupt
            P1IFG &= ~BIT1;
            // Select Interrupt on Rising Edge
            P1IES &= ~BIT1;

            switch(screen_state) {
                case HOME_1:
                    screen_state = HOME_8;
                    break;
                case HOME_2:
                    screen_state = HOME_1;
                    break;
                case HOME_3:
                    screen_state = HOME_2;
                    break;
                case HOME_4:
                    screen_state = HOME_3;
                    break;
                case HOME_5:
                    screen_state = HOME_4;
                    break;
                case HOME_6:
                    screen_state = HOME_5;
                    break;
                case HOME_7:
                    screen_state = HOME_6;
                    break;
                case HOME_8:
                    screen_state = HOME_7;
                    break;
                case TOGGLE_OUTLET_1:
                    screen_state = TOGGLE_OUTLET_6;
                    break;
                case TOGGLE_OUTLET_2:
                    screen_state = TOGGLE_OUTLET_1;
                    break;
                case TOGGLE_OUTLET_3:
                    screen_state = TOGGLE_OUTLET_2;
                    break;
                case TOGGLE_OUTLET_4:
                    screen_state = TOGGLE_OUTLET_3;
                    break;
                case TOGGLE_OUTLET_5:
                    screen_state = TOGGLE_OUTLET_4;
                    break;
                case TOGGLE_OUTLET_6:
                    screen_state = TOGGLE_OUTLET_5;
                    break;
                case TOGGLE_CONF_1_y:
                    screen_state = TOGGLE_CONF_1_n;
                    break;
                case TOGGLE_CONF_1_n:
                    screen_state = TOGGLE_CONF_1_y;
                    break;
                case TOGGLE_CONF_2_y:
                    screen_state = TOGGLE_CONF_2_n;
                    break;
                case TOGGLE_CONF_2_n:
                    screen_state = TOGGLE_CONF_2_y;
                    break;
                case TOGGLE_CONF_3_y:
                    screen_state = TOGGLE_CONF_3_n;
                    break;
                case TOGGLE_CONF_3_n:
                    screen_state = TOGGLE_CONF_3_y;
                    break;
                case TOGGLE_CONF_4_y:
                    screen_state = TOGGLE_CONF_4_n;
                    break;
                case TOGGLE_CONF_4_n:
                    screen_state = TOGGLE_CONF_4_y;
                    break;
                case TOGGLE_CONF_5_y:
                    screen_state = TOGGLE_CONF_5_n;
                    break;
                case TOGGLE_CONF_5_n:
                    screen_state = TOGGLE_CONF_5_y;
                    break;
                case TOGGLE_CONF_6_y:
                    screen_state = TOGGLE_CONF_6_n;
                    break;
                case TOGGLE_CONF_6_n:
                    screen_state = TOGGLE_CONF_6_y;
                    break;
                default:
                    break;
            }
        }

        // not used
        if (P1IFG & BIT2) {
            // Clear the interrupt
            P1IFG &= ~BIT2;
            // Select Interrupt on Rising Edge
            P1IES &= ~BIT2;
        }

        // down button
        if (P1IFG & BIT3) {
            // Clear the interrupt
            P1IFG &= ~BIT3;
            // Select Interrupt on Rising Edge
            P1IES &= ~BIT3;
            switch(screen_state) {
                case HOME_1:
                    screen_state = HOME_2;
                    break;
                case HOME_2:
                    screen_state = HOME_3;
                    break;
                case HOME_3:
                    screen_state = HOME_4;
                    break;
                case HOME_4:
                    screen_state = HOME_5;
                    break;
                case HOME_5:
                    screen_state = HOME_6;
                    break;
                case HOME_6:
                    screen_state = HOME_7;
                    break;
                case HOME_7:
                    screen_state = HOME_8;
                    break;
                case HOME_8:
                    screen_state = HOME_1;
                    break;
                case TOGGLE_OUTLET_1:
                    screen_state = TOGGLE_OUTLET_2;
                    break;
                case TOGGLE_OUTLET_2:
                    screen_state = TOGGLE_OUTLET_3;
                    break;
                case TOGGLE_OUTLET_3:
                    screen_state = TOGGLE_OUTLET_4;
                    break;
                case TOGGLE_OUTLET_4:
                    screen_state = TOGGLE_OUTLET_5;
                    break;
                case TOGGLE_OUTLET_5:
                    screen_state = TOGGLE_OUTLET_6;
                    break;
                case TOGGLE_OUTLET_6:
                    screen_state = TOGGLE_OUTLET_1;
                    break;
                case TOGGLE_CONF_1_y:
                    screen_state = TOGGLE_CONF_1_n;
                    break;
                case TOGGLE_CONF_1_n:
                    screen_state = TOGGLE_CONF_1_y;
                    break;
                case TOGGLE_CONF_2_y:
                    screen_state = TOGGLE_CONF_2_n;
                    break;
                case TOGGLE_CONF_2_n:
                    screen_state = TOGGLE_CONF_2_y;
                    break;
                case TOGGLE_CONF_3_y:
                    screen_state = TOGGLE_CONF_3_n;
                    break;
                case TOGGLE_CONF_3_n:
                    screen_state = TOGGLE_CONF_3_y;
                    break;
                case TOGGLE_CONF_4_y:
                    screen_state = TOGGLE_CONF_4_n;
                    break;
                case TOGGLE_CONF_4_n:
                    screen_state = TOGGLE_CONF_4_y;
                    break;
                case TOGGLE_CONF_5_y:
                    screen_state = TOGGLE_CONF_5_n;
                    break;
                case TOGGLE_CONF_5_n:
                    screen_state = TOGGLE_CONF_5_y;
                    break;
                case TOGGLE_CONF_6_y:
                    screen_state = TOGGLE_CONF_6_n;
                    break;
                case TOGGLE_CONF_6_n:
                    screen_state = TOGGLE_CONF_6_y;
                    break;
                default:
                    break;
            }
        }

        // forward button
        if (P1IFG & BIT4) {
            // Clear the interrupt
            P1IFG &= ~BIT4;
            // Select Interrupt on Rising Edge
            P1IES &= ~BIT4;

            // update the screen and logic state
            switch(screen_state) {
                case HOME_1:
                    screen_state = TEMP_HUM;
                    update_state = 1;
                    break;
                case HOME_2:
                    outlet_num_abs = 1;
                    update_state = 1;
                    screen_state = OUTLET_1;
                    break;
                case HOME_3:
                    outlet_num_abs = 2;
                    update_state = 1;
                    screen_state = OUTLET_2;
                    break;
                case HOME_4:
                    outlet_num_abs = 3;
                    update_state = 1;
                    screen_state = OUTLET_3;
                    break;
                case HOME_5:
                    outlet_num_abs = 4;
                    update_state = 1;
                    screen_state = OUTLET_4;
                    break;
                case HOME_6:
                    outlet_num_abs = 5;
                    update_state = 1;
                    screen_state = OUTLET_5;
                    break;
                case HOME_7:
                    outlet_num_abs = 6;
                    update_state = 1;
                    screen_state = OUTLET_6;
                    break;
                case HOME_8:
                    screen_state = TOGGLE_OUTLET_1;
                    break;
                case TOGGLE_OUTLET_1:
                    screen_state = TOGGLE_CONF_1_n;
                    break;
                case TOGGLE_OUTLET_2:
                    screen_state = TOGGLE_CONF_2_n;
                    break;
                case TOGGLE_OUTLET_3:
                    screen_state = TOGGLE_CONF_3_n;
                    break;
                case TOGGLE_OUTLET_4:
                    screen_state = TOGGLE_CONF_4_n;
                    break;
                case TOGGLE_OUTLET_5:
                    screen_state = TOGGLE_CONF_5_n;
                    break;
                case TOGGLE_OUTLET_6:
                    screen_state = TOGGLE_CONF_6_n;
                    break;
                case TOGGLE_CONF_1_y:
                    i2_address = i2_addrs[0];
                    outlet_num = 2;
                    outlet_num_abs = 1;
                    toggle_status = 1;
                    screen_state = TOGGLE_CONF_1_y;
                    break;
                case TOGGLE_CONF_1_n:
                    screen_state = TOGGLE_OUTLET_1;
                    break;
                case TOGGLE_CONF_2_y:
                    i2_address = i2_addrs[0];
                    outlet_num = 1;
                    outlet_num_abs = 2;
                    toggle_status = 1;
                    screen_state = TOGGLE_CONF_2_y;
                    break;
                case TOGGLE_CONF_2_n:
                    screen_state = TOGGLE_OUTLET_2;
                    break;
                case TOGGLE_CONF_3_y:
                    i2_address = i2_addrs[0];
                    outlet_num = 0;
                    outlet_num_abs = 3;
                    toggle_status = 1;
                    screen_state = TOGGLE_CONF_3_y;
                    break;
                case TOGGLE_CONF_3_n:
                    screen_state = TOGGLE_OUTLET_3;
                    break;
                case TOGGLE_CONF_4_y:
                    i2_address = i2_addrs[1];
                    outlet_num = 2;
                    outlet_num_abs = 4;
                    toggle_status = 1;
                    screen_state = TOGGLE_CONF_4_y;
                    break;
                case TOGGLE_CONF_4_n:
                    screen_state = TOGGLE_OUTLET_4;
                    break;
                case TOGGLE_CONF_5_y:
                    i2_address = i2_addrs[1];
                    outlet_num = 1;
                    outlet_num_abs = 5;
                    toggle_status = 1;
                    screen_state = TOGGLE_CONF_5_y;
                    break;
                case TOGGLE_CONF_5_n:
                    screen_state = TOGGLE_OUTLET_5;
                    break;
                case TOGGLE_CONF_6_y:
                    i2_address = i2_addrs[1];
                    outlet_num = 0;
                    outlet_num_abs = 6;
                    toggle_status = 1;
                    screen_state = TOGGLE_CONF_6_y;
                    break;
                case TOGGLE_CONF_6_n:
                    screen_state = TOGGLE_OUTLET_6;
                    break;
                default:
                    break;
            }
        }

        if (!toggle_status || update_state) {
            // display the screen
            refresh_screen_status = 1;
            display_status = 1;
        }
        // change the button state to 1
        button_state = 1;

    }

    // otherwise we have a release of a button
    else {
        if (P1IFG & BIT0) {
            // Clear the interrupt
            P1IFG &= ~BIT0;
            // Select Interrupt on Falling Edge
            P1IES |= BIT0;
        }
        if (P1IFG & BIT1) {
            // Clear the interrupt
            P1IFG &= ~BIT1;
            // Select Interrupt on Falling Edge
            P1IES |= BIT1;
        }
        if (P1IFG & BIT2) {
            // Clear the interrupt
            P1IFG &= ~BIT2;
            // Select Interrupt on Falling Edge
            P1IES |= BIT2;
        }
        if (P1IFG & BIT3) {
            // Clear the interrupt
            P1IFG &= ~BIT3;
            // Select Interrupt on Falling Edge
            P1IES |= BIT3;
        }
        if (P1IFG & BIT4) {
            // Clear the interrupt
            P1IFG &= ~BIT4;
            // Select Interrupt on Falling Edge
            P1IES |= BIT4;
        }
        // change the button state to 0
        button_state = 0;
    }
    P1IE |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
    TA0CCTL0 |= CCIE;
}

