#include <msp430.h> 
#include <math.h>

/**
 * main.c
 */
#include "stdint.h"
#include "LCD_new.h"
#include "in_person_UI.h"

// Definitions
#define BUFLEN  512

// Global Variables
uint8_t RXData;
uint8_t TXData;
volatile int32_t data[BUFLEN/4];
volatile float float_data[BUFLEN/8];
volatile uint16_t RXDataCtr;
volatile uint16_t TXDataCtr;
uint8_t screen_state;
uint8_t outlet_status;
uint8_t button_state;
uint8_t display_status;
uint8_t toggle_status;
uint8_t update_status;
uint8_t refresh_screen_status;
uint8_t nack_ctr;

void i2c_init(void);
void init_buttons(void);
void stopWatchDog(void);
void configure_TA0(void);
void configure_TA1(void);
void i2c_send_toggle(void);
void i2c_receive_outlet(void);
void get_th_info_i2c(void);
void do_math(void);

// Structs
struct outlet_struct {
   float v_rms;
   float i_rms;
   float p_apparent;
} outlet_info;

struct th_struct {
   float temp;
   float hum;
} th_info;


int main(void)
{
    stopWatchDog();

    // Initialization
    i2c_init();
    init_buttons();
    lcdInit();

    // Display startup screen
    screen_state = INIT;
    lcdClear();
    display_screen(screen_state);
    __delay_cycles(3000000);

    // initialize the outlet status
    outlet_status = 1;
    button_state = 0;
    toggle_status = 0;
    display_status = 0;
    update_status = 0;
    refresh_screen_status = 0;

    // Initialize TA0 for updates
    configure_TA0();

    // Enter LPM0 w/ interrupts
    __bis_SR_register(GIE);

    int i;
    for (i = 0; i < 20; i++) {
        update_status = 1;
        if (update_status) {
            TA0CCTL0 &= ~CCIE;
            P1IE &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
            P4DIR |= BIT7;
            P4OUT ^= BIT7;
            // get information about the outlet via i2c
            i2c_receive_outlet();
            // get temp/hum information via i2c
            get_th_info_i2c();
            // wait for i2c to complete
            while(update_status);
            P1IE |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
            TA0CCTL0 |= CCIE;
            // do math on the outlet data
            do_math();
            // update the LCD screens with the new information
            refresh_screen_status = 1;
        }
    }

    if (nack_ctr < 10) {
        lcdClear();
        screen_state = HOME_1;
        display_screen(screen_state);
    }

    // Enter main while loop
    while(1) {
        if (nack_ctr > 5) {
            // Display startup screen
            screen_state = ERROR;
            display_screen(screen_state);
            __delay_cycles(700000);
        }
        if (toggle_status) {
            TA0CCTL0 &= ~CCIE;
            P1IE &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
            i2c_send_toggle();
            while(toggle_status);
            P1IE |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
            TA0CCTL0 |= CCIE;
        }
        if (update_status) {
            TA0CCTL0 &= ~CCIE;
            P1IE &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
            P4DIR |= BIT7;
            P4OUT ^= BIT7;
            // get information about the outlet via i2c
            i2c_receive_outlet();
            // get temp/hum information via i2c
            get_th_info_i2c();
            // wait for i2c to complete
            while(update_status);
            P1IE |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
            TA0CCTL0 |= CCIE;
            // do math on the outlet data
            do_math();
            // update the LCD screens with the new information
            refresh_screen_status = 1;
        }
        if (refresh_screen_status) {
            // update the LCD screens with the new information
            update_screen(screen_state, outlet_info.v_rms, outlet_info.i_rms, outlet_info.p_apparent, th_info.temp, th_info.hum, outlet_status);
            // display the new screens
            if ((screen_state == TEMP_HUM) || (screen_state == OUTLET_1) || (screen_state == TOGGLE_CONF_1_y) || (screen_state == TOGGLE_CONF_1_n)) {
                display_status = 1;
            }
            refresh_screen_status = 0;
        }
        if (display_status) {
            display_screen(screen_state);
            display_status = 0;
        }
    }
}


// Peripheral Functions
void stopWatchDog(void)
{
    WDTCTL = WDTPW + WDTHOLD;         // Stop Watch Dog timer
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

// Initialize i2c
void i2c_init(void) {
    P3REN |= (BIT0 + BIT1);
    P3OUT |= (BIT0 + BIT1);
    P3SEL |= (BIT0 + BIT1);                        // Assign I2C pins to USCI_B1
    UCB0CTL1 |= UCSWRST;                      // Enable SW reset
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
    UCB0BR0 = 18;                             // fSCL = SMCLK/12 = ~100kHz
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
    UCB0IE |= UCRXIE | UCNACKIE |UCTXIE;                       // Enable TX interrupt
    RXData = 0x00;
    TXData = 0x00;
}

// do math on the outlet info
void do_math() {
    float v_rms = 0.0;
    float i_rms = 0.0;
    float p_apparent = 0.0;

    unsigned int i;
    // first calculate v_rms
    for (i = 0; i < BUFLEN/8; i++) {
        if (data[i] == 0x00800000) {
            data[i] = 0x00000000;
        }
        if (data[i+64] == 0x00800000) {
            data[i+64] = 0x00000000;
        }
        float normalized_v = (float)(~data[i] + 1) / (float)(~0x7FFFFF + 1);
        float normalized_i = (float)(~data[i+64] + 1) / (float)(~0x7FFFFF + 1);
        float_data [i] = normalized_i;
        v_rms += normalized_v*normalized_v;
        i_rms += normalized_i*normalized_i;
        p_apparent += normalized_v*normalized_i;
    }
    outlet_info.v_rms = sqrt(v_rms / (BUFLEN/8));
    outlet_info.i_rms = sqrt(17.85 * i_rms / (BUFLEN/8));
    outlet_info.p_apparent = p_apparent / (BUFLEN/8);
}

void get_th_info_i2c() {
    th_info.temp = 00.0;
    th_info.hum = 00.00;
}

void i2c_send_toggle() {
    // Set TXData
    TXData = 'T';
    // initialize data counter to 0
    TXDataCtr = 0;
    // Ensure stop condition was sent
    while (UCB0CTL1 & UCTXSTP);
    // Slave address
    UCB0I2CSA = 0x33;
    // Transmit mode
    UCB0CTL1 |= UCTR;
    // 7-bit addressing mode
    UCB0CTL0 &= ~UCSLA10;
    // send start bit
    UCB0CTL1 |= UCTXSTT;
//    while (UCB0CTL1 & UCTXSTT);
}

void i2c_receive_outlet() {
    // set RX data vessel to 0
    RXData = 0x00;
    // initialize RX data counter to 0
    RXDataCtr = 0;
    // Ensure stop condition was sent
    while (UCB0CTL1 & UCTXSTP);
    // Slave address
    UCB0I2CSA = 0x33;
    // Receive mode
    UCB0CTL1 &= ~UCTR;
    // 7 bit addressing mode
    UCB0CTL0 &= ~UCSLA10;
    // Send start bit
    UCB0CTL1 |= UCTXSTT;
//    while (UCB0CTL1 & UCTXSTT);
}

// Configure the clock
void configure_TA0(void) {
    // configure timer A
    TA0CCTL0 = CCIE;                      // CCR0 interrupt enabled
    TA0CTL = TASSEL_1 + MC_1 + ID_3;      // SMCLK/8, upmode
    TA0CCR0 =  20480/2;
}

// Configure the clock
void configure_TA1(void) {
    // configure timer A
    TA1CCTL0 = CCIE;                      // CCR0 interrupt enabled
    TA1CTL = TASSEL_1 + MC_1 + ID_3;      // SMCLK/8, upmode
    TA1CCR0 =  20480;
}

// Timer A0 routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
{
    // disable button interrupts while in timer interrupt
    P1IE &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
    update_status = 1;
    // re-enable button interrupts
    P1IE |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
}


// I2C routine
#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
{
  switch(__even_in_range(UCB0IV,12))
  {
  // No interrupts
  case  0: break;
  // ALIFG
  case  2: break;
  // NACKIFG
  case  4:
      // Send stop condition
      UCB0CTL1 |= UCTXSTP;
      // Clear RX flag and TX flag
      UCB0IFG &= ~(UCRXIFG + UCTXIFG);
      nack_ctr++;
      // reset i2c status
      toggle_status = 0;
      update_status = 0;
      break;
  // Start interrupts STTIFG
  case  6: break;
  // Stop interrupt STPIFG
  case  8: break;
  // RX interrupt: RXIFG
  case 10:
      nack_ctr = 0;
      // If not the last or second to last expected data packet
      if (RXDataCtr < (BUFLEN-2))
      {
          // Read the RX buffer
          RXData = UCB0RXBUF;
          // Shift the local global array and put the RX buffer into it
          *(((uint8_t*)data) + RXDataCtr) = RXData;
          // Increment the number of bytes received counter
          RXDataCtr++;
      }
      // If second to last byte, send a stop STP along with reading the buffer
      else if (RXDataCtr == (BUFLEN-2))
      {
          RXData = UCB0RXBUF;
          UCB0CTL1 |= UCTXSTP;
          *(((uint8_t*)data) + RXDataCtr) = RXData;
          RXDataCtr++;
      }
      // If last byte, just receive the buffer
      else if (RXDataCtr == (BUFLEN-1))
      {
          RXData = UCB0RXBUF;
          *(((uint8_t*)data) + RXDataCtr) = RXData;
          RXDataCtr++;
          UCB0IFG &= ~UCRXIFG;
          // reset i2c status
          update_status = 0;
      }
      break;
  // Transmit interrupt TXIFG
  case 12:
      nack_ctr = 0;
      if (TXDataCtr < 1) {
          // Load data into TX buffer
          UCB0TXBUF = TXData;
          // increment the bytes sent counter
          TXDataCtr++;
      }
      // If the byte was sent, send a stop condition
      else {
          UCB0CTL1 |= UCTXSTP;
          UCB0IFG &= ~UCTXIFG;
          // toggle outlet status
          outlet_status = !outlet_status;
          // update the LCD screens with the new information
          refresh_screen_status = 1;
          // reset i2c status
          toggle_status = 0;
      }
      break;
  default: break;
  }

}


// Button Interrupt
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    // disable necessary interrupts
    P1IE &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
    TA0CCTL0 &= ~CCIE;
    __delay_cycles(5000);

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
                case ERROR:
                    nack_ctr = 0;
                    screen_state = HOME_1;
                    break;
                case TEMP_HUM:
                    screen_state = HOME_1;
                    break;
                case OUTLET_1:
                    screen_state = HOME_2;
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
                case ERROR:
                    nack_ctr = 0;
                    screen_state = HOME_1;
                    break;
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
                case ERROR:
                    nack_ctr = 0;
                    screen_state = HOME_1;
                    break;
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
                case ERROR:
                    nack_ctr = 0;
                    screen_state = HOME_1;
                    break;
                case HOME_1:
                    screen_state = TEMP_HUM;
                    break;
                case HOME_2:
                    screen_state = OUTLET_1;
                    break;
                case HOME_8:
                    screen_state = TOGGLE_OUTLET_1;
                    break;
                case TOGGLE_OUTLET_1:
                    screen_state = TOGGLE_CONF_1_y;
                    break;
                case TOGGLE_CONF_1_y:
                    toggle_status = 1;
                    screen_state = TOGGLE_CONF_1_y;
                    break;
                case TOGGLE_CONF_1_n:
                    screen_state = TOGGLE_OUTLET_1;
                    break;
                default:
                    break;
            }
        }

        if (!toggle_status) {
            // display the screen
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
