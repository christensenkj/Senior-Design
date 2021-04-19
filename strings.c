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
char res_v_1[6];
char res_i_1_1[6];
char res_p_1_1[6];
char res_i_2_1[6];
char res_p_2_1[6];
char res_i_3_1[6];
char res_p_3_1[6];
// second outlet mcu
char res_v_2[6];
char res_i_1_2[6];
char res_p_1_2[6];
char res_i_2_2[6];
char res_p_2_2[6];
char res_i_3_2[6];
char res_p_3_2[6];
// third outlet mcu
char res_v_3[6];
char res_i_1_3[6];
char res_p_1_3[6];
char res_i_2_3[6];
char res_p_2_3[6];
char res_i_3_3[6];
char res_p_3_3[6];
// fourth outlet mcu
char res_v_4[6];
char res_i_1_4[6];
char res_p_1_4[6];
char res_i_2_4[6];
char res_p_2_4[6];
char res_i_3_4[6];
char res_p_3_4[6];
// temperature and humidity
char res_t[5];
char res_h[5];
// outlet statuses
char outlet_status_string[23];

extern uint8_t outlet_statuses[12];



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
    // update strings for third outlet mcu
    sprintf(res_v_3, "%.2f", outlet_infos[2].v_rms);
    sprintf(res_i_1_3, "%.2f", outlet_infos[2].i_rms_1);
    sprintf(res_p_1_3, "%.2f", outlet_infos[2].p_apparent_1);
    sprintf(res_i_2_3, "%.2f", outlet_infos[2].i_rms_2);
    sprintf(res_p_2_3, "%.2f", outlet_infos[2].p_apparent_2);
    sprintf(res_i_3_3, "%.2f", outlet_infos[2].i_rms_3);
    sprintf(res_p_3_3, "%.2f", outlet_infos[2].p_apparent_3);
    // update strings for second outlet mcu
    sprintf(res_v_4, "%.2f", outlet_infos[3].v_rms);
    sprintf(res_i_1_4, "%.2f", outlet_infos[3].i_rms_1);
    sprintf(res_p_1_4, "%.2f", outlet_infos[3].p_apparent_1);
    sprintf(res_i_2_4, "%.2f", outlet_infos[3].i_rms_2);
    sprintf(res_p_2_4, "%.2f", outlet_infos[3].p_apparent_2);
    sprintf(res_i_3_4, "%.2f", outlet_infos[3].i_rms_3);
    sprintf(res_p_3_4, "%.2f", outlet_infos[3].p_apparent_3);
    // update strings for temperature and humidity
    sprintf(res_t, "%d", th_info->temp);
    sprintf(res_h, "%d", th_info->hum);
    // upadte outlet statuses
    sprintf(outlet_status_string, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", outlet_statuses[0], outlet_statuses[1], outlet_statuses[2], outlet_statuses[3], outlet_statuses[4], outlet_statuses[5],
            outlet_statuses[6], outlet_statuses[7], outlet_statuses[8], outlet_statuses[9], outlet_statuses[10], outlet_statuses[11]);
}



