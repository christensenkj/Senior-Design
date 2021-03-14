/*
 * i2c.h
 *
 *  Created on: Mar 13, 2021
 *      Author: karston
 */

#ifndef I2C_H_
#define I2C_H_

#define I2C_REN     P4REN
#define I2C_OUT     P4OUT
#define I2C_SEL     P4SEL
#define I2C_SCL_PIN     BIT1
#define I2C_SDA_PIN     BIT2
// Definitions of buffer length
#define BUFLEN  512

void i2c_init(void);
void i2c_send_toggle(void);
void i2c_receive_outlet(void);
void get_th_info_i2c(void);
void do_math(void);
void format_strings(void);




#endif /* I2C_H_ */
