#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "math_engine.h"

extern int16_t data[BUFELEM];
extern uint8_t temp;
extern uint8_t hum;

void get_outlet_info(struct outlet_struct *outlet_info) {
    float v_rms = 0.0;
    float i_rms_1 = 0.0;
    float p_apparent_1 = 0.0;
    float i_rms_2 = 0.0;
    float p_apparent_2 = 0.0;
    float i_rms_3 = 0.0;
    float p_apparent_3 = 0.0;

    unsigned int i;
    // first calculate v_rms
    for (i = 0; i < BUFELEM/4; i++) {
        float normalized_v = 226*(1.5/32768.0)*(float)data[i];
        float normalized_i_1 = 17.86*(1.5/32768.0)*(float)data[i+32];
        float normalized_i_2 = 17.86*(1.5/32768.0)*(float)data[i+64];
        float normalized_i_3 = 17.86*(1.5/32768.0)*(float)data[i+96];
        v_rms += normalized_v*normalized_v;
        i_rms_1 += normalized_i_1*normalized_i_1;
//        p_apparent_1 += normalized_v*normalized_i_1;
        i_rms_2 += normalized_i_2*normalized_i_2;
//        p_apparent_2 += normalized_v*normalized_i_2;
        i_rms_3 += normalized_i_3*normalized_i_3;
//        p_apparent_3 += normalized_v*normalized_i_3;
    }
    outlet_info->v_rms = sqrt(v_rms / (BUFELEM/4));
    outlet_info->i_rms_1 = sqrt(i_rms_1 / (BUFELEM/4));
    outlet_info->p_apparent_1 = outlet_info->v_rms*outlet_info->i_rms_1;
    outlet_info->i_rms_2 = sqrt(i_rms_2 / (BUFELEM/4));
    outlet_info->p_apparent_2 = outlet_info->v_rms*outlet_info->i_rms_2;
    outlet_info->i_rms_3 = sqrt(i_rms_3 / (BUFELEM/4));
    outlet_info->p_apparent_3 = outlet_info->v_rms*outlet_info->i_rms_3;
//    outlet_info->v_rms = fabs(226*(1.5/32768.0)*(float)data[0]);
//    outlet_info->i_rms_1 = fabs(17.86*(1.5/32768.0)*(float)data[32]);
//    outlet_info->p_apparent_1 = fabs(outlet_info->v_rms*outlet_info->i_rms_1);
//    outlet_info->i_rms_2 = fabs(17.86*(1.5/32768.0)*(float)data[64]);
//    outlet_info->p_apparent_2 = fabs(outlet_info->v_rms*outlet_info->i_rms_2);
//    outlet_info->i_rms_3 = fabs(17.86*(1.5/32768.0)*(float)data[96]);
//    outlet_info->p_apparent_3 = fabs(outlet_info->v_rms*outlet_info->i_rms_3);
}


void get_th_info(struct th_struct *th_info) {
    th_info->temp = temp;
    th_info->hum = hum;
}
