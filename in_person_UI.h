/*
 * in_person_UI.h
 *
 *  Created on: Jan 28, 2021
 *      Author: karston
 */

#ifndef IN_PERSON_UI_H_
#define IN_PERSON_UI_H_

#include <stdint.h>

enum state{INIT, HOME_1, HOME_2, HOME_3, HOME_4, HOME_5, HOME_6, HOME_7, HOME_8, HOME_9, HOME_10, HOME_11, HOME_12, HOME_13, HOME_14, TEMP_HUM,
    OUTLET_1, OUTLET_2, OUTLET_3, OUTLET_4, OUTLET_5, OUTLET_6, OUTLET_7, OUTLET_8, OUTLET_9, OUTLET_10, OUTLET_11, OUTLET_12,
    TOGGLE_OUTLET_1, TOGGLE_OUTLET_2, TOGGLE_OUTLET_3, TOGGLE_OUTLET_4, TOGGLE_OUTLET_5, TOGGLE_OUTLET_6,
    TOGGLE_OUTLET_7, TOGGLE_OUTLET_8, TOGGLE_OUTLET_9, TOGGLE_OUTLET_10, TOGGLE_OUTLET_11, TOGGLE_OUTLET_12,
    TOGGLE_CONF_1_y, TOGGLE_CONF_1_n, TOGGLE_CONF_2_y, TOGGLE_CONF_2_n, TOGGLE_CONF_3_y, TOGGLE_CONF_3_n, TOGGLE_CONF_4_y, TOGGLE_CONF_4_n, TOGGLE_CONF_5_y, TOGGLE_CONF_5_n, TOGGLE_CONF_6_y, TOGGLE_CONF_6_n,
    TOGGLE_CONF_7_y, TOGGLE_CONF_7_n, TOGGLE_CONF_8_y, TOGGLE_CONF_8_n, TOGGLE_CONF_9_y, TOGGLE_CONF_9_n, TOGGLE_CONF_10_y, TOGGLE_CONF_10_n, TOGGLE_CONF_11_y, TOGGLE_CONF_11_n, TOGGLE_CONF_12_y, TOGGLE_CONF_12_n};

// Function prototypes
void display_screen(uint8_t state);
void update_screen(uint8_t state, uint8_t outlet_num_abs);
void update_th_info(void);
void update_outlet_info(uint8_t outlet_num_abs);
void update_outlet_status(uint8_t outlet_num_abs);

#endif /* IN_PERSON_UI_H_ */
