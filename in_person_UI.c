/*
 * in_person_UI.c
 *
 *  Created on: Jan 28, 2021
 *      Author: karston
 */

#include "in_person_UI.h"
#include "LCD.h"

// INIT Screen
char screen_init_1[] = "ROSES ARE RED";
char screen_init_2[] = "VIOLETS ARE BLUE";
char screen_init_3[] = "KARSTON IS THE BEST";
char screen_init_4[] = "PROGRAMMER AT CU";

// Home Screen 1
char screen_home_1_1[] = "PDU HOME";
char screen_home_1_2[] = "Display Temp/Hum <--";
char screen_home_1_3[] = "Outlet 1";
char screen_home_1_4[] = "Outlet 2";

// Home Screen 1
char screen_home_2_1[] = "Outlet 3";
char screen_home_2_2[] = "Outlet 4";
char screen_home_2_3[] = "Toggle Outlets";

// Temp Hum Screen
char screen_temp_hum_1[] = "Temperature and";
char screen_temp_hum_2[] = "Humidity Display";
char screen_temp_hum_3[] = "Temperature: 23.2 C";
char screen_temp_hum_4[] = "Humidity: 32.45%";

// Outlet 1 Screen
char screen_outlet_1_1[] = "Outlet 1";
char screen_outlet_1_2[] = "RMS Vol: 120.01 VAC";
char screen_outlet_1_3[] = "RMS Cur: 10.51 A";
char screen_outlet_1_4[] = "Status: ON";

// Toggle Outlet Screen
char screen_toggle_outlets_1[] = "Toggle Outlets";
char screen_toggle_outlets_2[] = "Outlet 1 <--";
char screen_toggle_outlets_3[] = "Outlet 2";
char screen_toggle_outlets_4[] = "Outlet 3";

// Confirmation Toggle Page
char screen_toggle_conf_1[] = "Toggle Outlet 1?";
char screen_toggle_conf_2[] = "Current Status: ON";
char screen_toggle_conf_3[] = "YES <--";
char screen_toggle_conf_4[] = "NO";



void display_screen(uint8_t screen_state) {
    lcdClear();
    switch(screen_state) {
        case INIT:
            lcdSetText(screen_init_1, 0, 0);
            lcdSetText(screen_init_2, 1, 0);
            lcdSetText(screen_init_3, 2, 0);
            lcdSetText(screen_init_4, 3, 0);
            break;
        case HOME_1:
            lcdSetText(screen_home_1_1, 0, 0);
            lcdSetText(screen_home_1_2, 1, 0);
            lcdSetText(screen_home_1_3, 2, 0);
            lcdSetText(screen_home_1_4, 3, 0);
            break;
        case HOME_2:
            lcdSetText(screen_home_2_1, 0, 0);
            lcdSetText(screen_home_2_2, 1, 0);
            lcdSetText(screen_home_2_3, 2, 0);
            break;
        case TEMP_HUM:
            lcdSetText(screen_temp_hum_1, 0, 0);
            lcdSetText(screen_temp_hum_2, 1, 0);
            lcdSetText(screen_temp_hum_3, 2, 0);
            lcdSetText(screen_temp_hum_4, 3, 0);
            break;
        case OUTLET_1:
            lcdSetText(screen_outlet_1_1, 0, 0);
            lcdSetText(screen_outlet_1_2, 1, 0);
            lcdSetText(screen_outlet_1_3, 2, 0);
            lcdSetText(screen_outlet_1_4, 3, 0);
            break;
        case TOGGLE_OUTLET:
            lcdSetText(screen_toggle_outlets_1, 0, 0);
            lcdSetText(screen_toggle_outlets_2, 1, 0);
            lcdSetText(screen_toggle_outlets_3, 2, 0);
            lcdSetText(screen_toggle_outlets_4, 3, 0);
            break;
        case TOGGLE_CONF_1:
            lcdSetText(screen_toggle_conf_1, 0, 0);
            lcdSetText(screen_toggle_conf_2, 1, 0);
            lcdSetText(screen_toggle_conf_3, 2, 0);
            lcdSetText(screen_toggle_conf_4, 3, 0);
            break;
    }
}


//void update_screen(screen_state) {
//    switch(screen_state) {
//    case TEMP_HUM:
//        temp_hum_struct = update_temp_hum();
//        update_string(temp_hum_string, temp_hum_struct);
//        display_screen();
//        break;
//    case OUTLET_1:
//        power_struct = update_outlet(1);
//        update_string(outlet_1_string, power_struct);
//        display_screen();
//        break;
//    default:
//        break;
//    }
//    display_screen(uint8_t screen_state)
//}
