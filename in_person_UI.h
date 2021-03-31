/*
 * in_person_UI.h
 *
 *  Created on: Jan 28, 2021
 *      Author: karston
 */

#ifndef IN_PERSON_UI_H_
#define IN_PERSON_UI_H_

#include <stdint.h>

enum state{INIT, HOME_1, HOME_2, HOME_3, HOME_4, HOME_5, HOME_6, HOME_7, HOME_8, TEMP_HUM, OUTLET_1,
    TOGGLE_OUTLET_1, TOGGLE_OUTLET_2, TOGGLE_OUTLET_3, TOGGLE_OUTLET_4, TOGGLE_OUTLET_5, TOGGLE_OUTLET_6, TOGGLE_CONF_1_y, TOGGLE_CONF_1_n, ERROR};

// Function prototypes
void display_screen(uint8_t state);
void update_screen(uint8_t state, float v_rms, float i_rms, float p_apparent, float temp, float hum, uint8_t outlet_status);
void update_th_info(float temp, float hum);
void update_outlet_info(float v_rms, float i_rms, float p_apparent);
void update_outlet_status(uint8_t outlet_status);

#endif /* IN_PERSON_UI_H_ */
