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
