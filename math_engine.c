#include <math.h>
#include <stdint.h>
#include "math_engine.h"

extern int32_t data[BUFELEM];
extern float Humid;
extern float Temp;

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
        float normalized_v = (float)(~data[i] + 1) / (float)(~0x7FFFFF + 1);
        float normalized_i_1 = (float)(~data[i+32] + 1) / (float)(~0x7FFFFF + 1);
        float normalized_i_2 = (float)(~data[i+64] + 1) / (float)(~0x7FFFFF + 1);
        float normalized_i_3 = (float)(~data[i+96] + 1) / (float)(~0x7FFFFF + 1);
        v_rms += normalized_v*normalized_v;
        i_rms_1 += normalized_i_1*normalized_i_1;
        p_apparent_1 += normalized_v*normalized_i_1;
        i_rms_2 += normalized_i_2*normalized_i_2;
        p_apparent_2 += normalized_v*normalized_i_2;
        i_rms_3 += normalized_i_3*normalized_i_3;
        p_apparent_3 += normalized_v*normalized_i_3;
    }
    outlet_info->v_rms = sqrt(v_rms / BUFELEM);
    outlet_info->i_rms_1 = sqrt(17.85 * i_rms_1 / BUFELEM);
    outlet_info->p_apparent_1 = p_apparent_1 / BUFELEM;
    outlet_info->i_rms_2 = sqrt(17.85 * i_rms_2 / BUFELEM);
    outlet_info->p_apparent_2 = p_apparent_2 / BUFELEM;
    outlet_info->i_rms_3 = sqrt(17.85 * i_rms_3 / BUFELEM);
    outlet_info->p_apparent_3 = p_apparent_3 / BUFELEM;
}


void get_th_info(struct th_struct *th_info) {
    th_info->temp = Temp;
    th_info->hum = Humid;
}
