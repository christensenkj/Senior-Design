/*
 * msp_config.h
 *
 *  Created on: Oct 17, 2020
 *      Author: Karston Christensen
 */

#ifndef MSP_CONFIG_H_
#define MSP_CONFIG_H_

// include libs
#include <stdbool.h>
#include <msp430.h>

// Define SPI pinouts

//// SIMO on P3.3
//#define SIMO_PIN    BIT3
//#define SIMO_SEL    P3SEL
//#define SIMO_DIR    P3DIR
//#define SIMO_OUT    P3OUT
// SIMO on P4.1
#define SIMO_PIN    BIT1
#define SIMO_SEL    P4SEL
#define SIMO_DIR    P4DIR
#define SIMO_OUT    P4OUT
//// SOMI on P3.4
//#define SOMI_PIN    BIT4
//#define SOMI_SEL    P3SEL
//#define SOMI_DIR    P3DIR
//#define SOMI_OUT    P3OUT
// SOMI on P4.2
#define SOMI_PIN    BIT2
#define SOMI_SEL    P4SEL
#define SOMI_DIR    P4DIR
#define SOMI_OUT    P4OUT
// SCLK on P2.7
//#define SCLK_PIN    BIT7
//#define SCLK_SEL    P2SEL
//#define SCLK_DIR    P2DIR
//#define SCLK_OUT    P2OUT
// SCLK on P4.3
#define SCLK_PIN    BIT3
#define SCLK_SEL    P4SEL
#define SCLK_DIR    P4DIR
#define SCLK_OUT    P4OUT
//// Wiznet CS on P2.0
//#define CS_WIZ_PIN  BIT0
//#define CS_WIZ_SEL  P2SEL
//#define CS_WIZ_DIR  P2DIR
//#define CS_WIZ_OUT  P2OUT
// Wiznet CS on P1.6
#define CS_WIZ_PIN  BIT6
#define CS_WIZ_SEL  P1SEL
#define CS_WIZ_DIR  P1DIR
#define CS_WIZ_OUT  P1OUT
//// Wiznet hardware reset on P1.5
//#define RST_WIZ_PIN  BIT5
//#define RST_WIZ_SEL  P1SEL
//#define RST_WIZ_DIR  P1DIR
//#define RST_WIZ_OUT  P1OUT
// Wiznet hardware reset on P5.5
#define RST_WIZ_PIN  BIT5
#define RST_WIZ_SEL  P5SEL
#define RST_WIZ_DIR  P5DIR
#define RST_WIZ_OUT  P5OUT
//// Wiznet interrupt on P2.2
//#define INT_WIZ_PIN  BIT2
//#define INT_WIZ_SEL  P2SEL
//#define INT_WIZ_DIR  P2DIR
//#define INT_WIZ_OUT  P2OUT
//#define INT_WIZ_REN  P2REN
//#define INT_WIZ_IES  P2IES
//#define INT_WIZ_IE   P2IE
// Wiznet interrupt on P1.5
#define INT_WIZ_PIN  BIT5
#define INT_WIZ_SEL  P1SEL
#define INT_WIZ_DIR  P1DIR
#define INT_WIZ_OUT  P1OUT
#define INT_WIZ_REN  P1REN
#define INT_WIZ_IES  P1IES
#define INT_WIZ_IE   P1IE

// function prototypes
int msp_config_spi_pins();
int msp_config_usci();
int msp_config_clock();
int set_v_core_level(unsigned int level);

#endif /* MSP_CONFIG_H_ */
