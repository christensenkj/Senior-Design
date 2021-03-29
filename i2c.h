/*
 * i2c.h
 *
 *  Created on: Mar 13, 2021
 *      Author: karston
 */

#ifndef I2C_H_
#define I2C_H_

#define DEV_KIT

#ifdef DEV_KIT
#define I2C_REN         P4REN
#define I2C_OUT         P4OUT
#define I2C_SEL         P4SEL
#define I2C_SCL_PIN     BIT1
#define I2C_SDA_PIN     BIT2
#endif

#ifndef DEV_KIT
#define I2C_REN         P3REN
#define I2C_OUT         P3OUT
#define I2C_SEL         P3SEL
#define I2C_SCL_PIN     BIT0
#define I2C_SDA_PIN     BIT1
#endif

// Definitions of buffer length
#define BUFLEN      1024
#define BUFELEM    128

void i2c_init(void);
void i2c_send_toggle(uint8_t i2_addr, uint8_t outlet_num);
void i2c_receive_outlet(uint8_t i2_addr);
void i2c_receive_th(void);




#endif /* I2C_H_ */
