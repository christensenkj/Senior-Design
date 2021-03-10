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

// SOMI on P3.4
#define SOMI_PIN    BIT4
#define SOMI_PORT   BIT3
// SIMO on P3.3
#define SIMO_PIN    BIT3
#define SIMO_PORT   BIT3
// SCLK on P2.7
#define SCLK_PIN    BIT7
#define SCLK_PORT   BIT2
// Wiznet CS on P2.0
#define CS_WIZ_PIN  BIT0
#define CS_WIZ_PORT BIT2
// Memory CS on P2.1 (not totally sure what this one is for)
#define CS_MEM_PIN  BIT1
#define CS_MEM_PORT BIT2
// Wiznet hardware reset on P1.5
#define RST_WIZ_PIN  BIT5
#define RST_WIZ_PORT BIT1
// Wiznet interrupt on P2.2
#define INT_WIZ_PIN  BIT2
#define INT_WIZ_PORT BIT2

// function prototypes
int msp_config_spi_pins();
int msp_config_usci();
int msp_config_clock();
int set_v_core_level(unsigned int level);

#endif /* MSP_CONFIG_H_ */
