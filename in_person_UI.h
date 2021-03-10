/*
 * in_person_UI.h
 *
 *  Created on: Jan 28, 2021
 *      Author: karston
 */

#ifndef IN_PERSON_UI_H_
#define IN_PERSON_UI_H_

#include <stdint.h>

enum state{INIT, HOME_1, HOME_2, TEMP_HUM, OUTLET_1, TOGGLE_OUTLET, TOGGLE_CONF_1};

// Function prototypes
void display_screen(uint8_t state);
void update_screen(uint8_t state);


#endif /* IN_PERSON_UI_H_ */
