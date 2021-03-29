/*
 * strings.c
 *
 *  Created on: Mar 22, 2021
 *      Author: Karston Christensen
 */

#include <stdio.h>
#include "strings.h"



// Define strings for displays and website

// first outlet mcu
char res_v_1[5];
char res_i_1_1[5];
char res_p_1_1[5];
char res_i_2_1[5];
char res_p_2_1[5];
char res_i_3_1[5];
char res_p_3_1[5];
// second outlet mcu
char res_v_2[5];
char res_i_1_2[5];
char res_p_1_2[5];
char res_i_2_2[5];
char res_p_2_2[5];
char res_i_3_2[5];
char res_p_3_2[5];
// temperature and humidity
char res_t[5];
char res_h[5];


void format_strings(struct outlet_struct *outlet_infos, struct th_struct *th_info) {
    // update strings for first outlet mcu
    sprintf(res_v_1, "%.2f", outlet_infos[0].v_rms);
    sprintf(res_i_1_1, "%.2f", outlet_infos[0].i_rms_1);
    sprintf(res_p_1_1, "%.2f", outlet_infos[0].p_apparent_1);
    sprintf(res_i_2_1, "%.2f", outlet_infos[0].i_rms_2);
    sprintf(res_p_2_1, "%.2f", outlet_infos[0].p_apparent_2);
    sprintf(res_i_3_1, "%.2f", outlet_infos[0].i_rms_3);
    sprintf(res_p_3_1, "%.2f", outlet_infos[0].p_apparent_3);
    // update strings for second outlet mcu
    sprintf(res_v_2, "%.2f", outlet_infos[1].v_rms);
    sprintf(res_i_1_2, "%.2f", outlet_infos[1].i_rms_1);
    sprintf(res_p_1_2, "%.2f", outlet_infos[1].p_apparent_1);
    sprintf(res_i_2_2, "%.2f", outlet_infos[1].i_rms_2);
    sprintf(res_p_2_2, "%.2f", outlet_infos[1].p_apparent_2);
    sprintf(res_i_3_2, "%.2f", outlet_infos[1].i_rms_3);
    sprintf(res_p_3_2, "%.2f", outlet_infos[1].p_apparent_3);
    // update strings for temperature and humidity
    sprintf(res_t, "%.2f", th_info->temp);
    sprintf(res_h, "%.2f", th_info->hum);
}



