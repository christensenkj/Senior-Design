/*
 * math_engine.h
 *
 *  Created on: Mar 22, 2021
 *      Author: Karston Christensen
 */

#ifndef MATH_ENGINE_H_
#define MATH_ENGINE_H_

#include <stdint.h>

#define BUFLEN      1024
#define BUFELEM     128

// Structs
struct outlet_struct {
   float v_rms;
   float i_rms_1;
   float p_apparent_1;
   float i_rms_2;
   float p_apparent_2;
   float i_rms_3;
   float p_apparent_3;
};

struct th_struct {
   uint8_t temp;
   uint8_t hum;
};


void get_outlet_info(struct outlet_struct *outlet_info);
void get_th_info(struct th_struct *th_info);

#endif /* MATH_ENGINE_H_ */
