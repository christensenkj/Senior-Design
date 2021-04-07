/*
 * shtc3.h
 *
 *  Created on: Apr 1, 2021
 *      Author: Zach Wilson
 */

#ifndef SHTC3_H_
#define SHTC3_H_

#include <msp430.h>
#include <stdint.h>

#define I2C_REN         P3REN
#define I2C_OUT         P3OUT
#define I2C_SEL         P3SEL
#define I2C_SCL_PIN     BIT0
#define I2C_SDA_PIN     BIT1

void shtc3_i2c_config(void);
void shtc3_i2c(void);
void Wakeup(void);
void Measure(void);
void Read(void);
void Sleep(void);
void Interpret(void);
float returnRH(void);
float returnTemp(void);

struct RET_STRUCT {
    float RH;
    float Temp;
};

#endif /* SHTC3_H_ */
