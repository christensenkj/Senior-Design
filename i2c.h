/*
 * i2c.h
 *
 *  Created on: Mar 2, 2021
 *      Author: karston
 */

#ifndef I2C_H_
#define I2C_H_

#define BUFLEN  512

void init_i2c(void);
void send_byte_i2c(void);
void request_data_i2c(void);

#endif /* I2C_H_ */
