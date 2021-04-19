/*
 * in_person_UI.c
 *
 *  Created on: Jan 28, 2021
 *      Author: karston
 */

#include <LCD_new.h>
#include "in_person_UI.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// first outlet mcu
extern char res_v_1[6];
extern char res_i_1_1[6];
extern char res_p_1_1[6];
extern char res_i_2_1[6];
extern char res_p_2_1[6];
extern char res_i_3_1[6];
extern char res_p_3_1[6];
// second outlet mcu
extern char res_v_2[6];
extern char res_i_1_2[6];
extern char res_p_1_2[6];
extern char res_i_2_2[6];
extern char res_p_2_2[6];
extern char res_i_3_2[6];
extern char res_p_3_2[6];
// temperature and humidity
extern char res_t[5];
extern char res_h[5];

// keep track of the status of outlets
extern uint8_t outlet_statuses[6];

// INIT Screen
char screen_init_1[] = "WELCOME TO ";
char screen_init_2[] = "RXL PDU";
char screen_init_3[] = "UNIVERSITY OF";
char screen_init_4[] = "COLORADO BOULDER";

// Home Screen 1
char screen_home_top[] = "PDU HOME";
char screen_home_th[] = "Display Temp/Hum";
char screen_home_o1[] = "Outlet 1";
char screen_home_o2[] = "Outlet 2";
char screen_home_o3[] = "Outlet 3";
char screen_home_o4[] = "Outlet 4";
char screen_home_o5[] = "Outlet 5";
char screen_home_o6[] = "Outlet 6";
char screen_home_to[] = "Toggle Outlets";

// Home screen strings with arrows
char screen_home_th_a[] = "Display Temp/Hum <--";
char screen_home_o1_a[] = "Outlet 1 <--";
char screen_home_o2_a[] = "Outlet 2 <--";
char screen_home_o3_a[] = "Outlet 3 <--";
char screen_home_o4_a[] = "Outlet 4 <--";
char screen_home_o5_a[] = "Outlet 5 <--";
char screen_home_o6_a[] = "Outlet 6 <--";
char screen_home_to_a[] = "Toggle Outlets <--";

// Temp Hum Screen
char screen_temp_hum_1[] = "Temperature and";
char screen_temp_hum_2[] = "Humidity Display";
char screen_temp_hum_3[] = "Temperature: 00.00 C";
char screen_temp_hum_4[] = "Humidity: 00.00% RH";

// Outlet display strings
char screen_outlet_2[] = "RMS Vol:   00.00    ";
char screen_outlet_3[] = "RMS Cur:   00.00    ";
char screen_outlet_4[] = "Aprnt Pwr:   00.00  ";

// Outlet Title Screen
char screen_outlet_1_1[] = "Outlet 1 Status: OFF";
char screen_outlet_2_1[] = "Outlet 2 Status: OFF";
char screen_outlet_3_1[] = "Outlet 3 Status: OFF";
char screen_outlet_4_1[] = "Outlet 4 Status: OFF";
char screen_outlet_5_1[] = "Outlet 5 Status: OFF";
char screen_outlet_6_1[] = "Outlet 6 Status: OFF";

// Toggle Outlet Screen
char screen_toggle_outlets_top[] = "Toggle Outlets";
char screen_toggle_outlets_o1[] = "Outlet 1";
char screen_toggle_outlets_o2[] = "Outlet 2";
char screen_toggle_outlets_o3[] = "Outlet 3";
char screen_toggle_outlets_o4[] = "Outlet 4";
char screen_toggle_outlets_o5[] = "Outlet 5";
char screen_toggle_outlets_o6[] = "Outlet 6";
char screen_toggle_outlets_o1_a[] = "Outlet 1 <--";
char screen_toggle_outlets_o2_a[] = "Outlet 2 <--";
char screen_toggle_outlets_o3_a[] = "Outlet 3 <--";
char screen_toggle_outlets_o4_a[] = "Outlet 4 <--";
char screen_toggle_outlets_o5_a[] = "Outlet 5 <--";
char screen_toggle_outlets_o6_a[] = "Outlet 6 <--";

// Confirmation Toggle Page
char screen_toggle_conf_1_top[] = "Toggle Outlet 1?";
char screen_toggle_conf_2_top[] = "Toggle Outlet 2?";
char screen_toggle_conf_3_top[] = "Toggle Outlet 3?";
char screen_toggle_conf_4_top[] = "Toggle Outlet 4?";
char screen_toggle_conf_5_top[] = "Toggle Outlet 5?";
char screen_toggle_conf_6_top[] = "Toggle Outlet 6?";
char screen_toggle_conf_1_status[] = "Current Status: OFF";
char screen_toggle_conf_2_status[] = "Current Status: OFF";
char screen_toggle_conf_3_status[] = "Current Status: OFF";
char screen_toggle_conf_4_status[] = "Current Status: OFF";
char screen_toggle_conf_5_status[] = "Current Status: OFF";
char screen_toggle_conf_6_status[] = "Current Status: OFF";

// Toggle Confirmation Tags
char screen_toggle_conf_y[] = "YES";
char screen_toggle_conf_n[] = "NO";
char screen_toggle_conf_y_a[] = "YES <--";
char screen_toggle_conf_n_a[] = "NO <--";



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
            lcdSetText(screen_home_top, 0, 0);
            lcdSetText(screen_home_th_a, 1, 0);
            lcdSetText(screen_home_o1, 2, 0);
            lcdSetText(screen_home_o2, 3, 0);
            break;
        case HOME_2:
            lcdSetText(screen_home_top, 0, 0);
            lcdSetText(screen_home_th, 1, 0);
            lcdSetText(screen_home_o1_a, 2, 0);
            lcdSetText(screen_home_o2, 3, 0);
            break;
        case HOME_3:
            lcdSetText(screen_home_top, 0, 0);
            lcdSetText(screen_home_th, 1, 0);
            lcdSetText(screen_home_o1, 2, 0);
            lcdSetText(screen_home_o2_a, 3, 0);
            break;
        case HOME_4:
            lcdSetText(screen_home_o3_a, 0, 0);
            lcdSetText(screen_home_o4, 1, 0);
            lcdSetText(screen_home_o5, 2, 0);
            lcdSetText(screen_home_o6, 3, 0);
            break;
        case HOME_5:
            lcdSetText(screen_home_o3, 0, 0);
            lcdSetText(screen_home_o4_a, 1, 0);
            lcdSetText(screen_home_o5, 2, 0);
            lcdSetText(screen_home_o6, 3, 0);
            break;
        case HOME_6:
            lcdSetText(screen_home_o3, 0, 0);
            lcdSetText(screen_home_o4, 1, 0);
            lcdSetText(screen_home_o5_a, 2, 0);
            lcdSetText(screen_home_o6, 3, 0);
            break;
        case HOME_7:
            lcdSetText(screen_home_o3, 0, 0);
            lcdSetText(screen_home_o4, 1, 0);
            lcdSetText(screen_home_o5, 2, 0);
            lcdSetText(screen_home_o6_a, 3, 0);
            break;
        case HOME_8:
            lcdSetText(screen_home_to_a, 0, 0);
            break;
        case TEMP_HUM:
            lcdSetText(screen_temp_hum_1, 0, 0);
            lcdSetText(screen_temp_hum_2, 1, 0);
            lcdSetText(screen_temp_hum_3, 2, 0);
            lcdSetText(screen_temp_hum_4, 3, 0);
            break;
        case OUTLET_1:
            lcdSetText(screen_outlet_1_1, 0, 0);
            lcdSetText(screen_outlet_2, 1, 0);
            lcdSetText(screen_outlet_3, 2, 0);
            lcdSetText(screen_outlet_4, 3, 0);
            break;
        case OUTLET_2:
            lcdSetText(screen_outlet_2_1, 0, 0);
            lcdSetText(screen_outlet_2, 1, 0);
            lcdSetText(screen_outlet_3, 2, 0);
            lcdSetText(screen_outlet_4, 3, 0);
            break;
        case OUTLET_3:
            lcdSetText(screen_outlet_3_1, 0, 0);
            lcdSetText(screen_outlet_2, 1, 0);
            lcdSetText(screen_outlet_3, 2, 0);
            lcdSetText(screen_outlet_4, 3, 0);
            break;
        case OUTLET_4:
            lcdSetText(screen_outlet_4_1, 0, 0);
            lcdSetText(screen_outlet_2, 1, 0);
            lcdSetText(screen_outlet_3, 2, 0);
            lcdSetText(screen_outlet_4, 3, 0);
            break;
        case OUTLET_5:
            lcdSetText(screen_outlet_5_1, 0, 0);
            lcdSetText(screen_outlet_2, 1, 0);
            lcdSetText(screen_outlet_3, 2, 0);
            lcdSetText(screen_outlet_4, 3, 0);
            break;
        case OUTLET_6:
            lcdSetText(screen_outlet_6_1, 0, 0);
            lcdSetText(screen_outlet_2, 1, 0);
            lcdSetText(screen_outlet_3, 2, 0);
            lcdSetText(screen_outlet_4, 3, 0);
            break;
        case TOGGLE_OUTLET_1:
            lcdSetText(screen_toggle_outlets_top, 0, 0);
            lcdSetText(screen_toggle_outlets_o1_a, 1, 0);
            lcdSetText(screen_toggle_outlets_o2, 2, 0);
            lcdSetText(screen_toggle_outlets_o3, 3, 0);
            break;
        case TOGGLE_OUTLET_2:
            lcdSetText(screen_toggle_outlets_top, 0, 0);
            lcdSetText(screen_toggle_outlets_o1, 1, 0);
            lcdSetText(screen_toggle_outlets_o2_a, 2, 0);
            lcdSetText(screen_toggle_outlets_o3, 3, 0);
            break;
        case TOGGLE_OUTLET_3:
            lcdSetText(screen_toggle_outlets_top, 0, 0);
            lcdSetText(screen_toggle_outlets_o1, 1, 0);
            lcdSetText(screen_toggle_outlets_o2, 2, 0);
            lcdSetText(screen_toggle_outlets_o3_a, 3, 0);
            break;
        case TOGGLE_OUTLET_4:
            lcdSetText(screen_toggle_outlets_o4_a, 0, 0);
            lcdSetText(screen_toggle_outlets_o5, 1, 0);
            lcdSetText(screen_toggle_outlets_o6, 2, 0);
            break;
        case TOGGLE_OUTLET_5:
            lcdSetText(screen_toggle_outlets_o4, 0, 0);
            lcdSetText(screen_toggle_outlets_o5_a, 1, 0);
            lcdSetText(screen_toggle_outlets_o6, 2, 0);
            break;
        case TOGGLE_OUTLET_6:
            lcdSetText(screen_toggle_outlets_o4, 0, 0);
            lcdSetText(screen_toggle_outlets_o5, 1, 0);
            lcdSetText(screen_toggle_outlets_o6_a, 2, 0);
            break;
        case TOGGLE_CONF_1_y:
            lcdSetText(screen_toggle_conf_1_top, 0, 0);
            lcdSetText(screen_toggle_conf_1_status, 1, 0);
            lcdSetText(screen_toggle_conf_y_a, 2, 0);
            lcdSetText(screen_toggle_conf_n, 3, 0);
            break;
        case TOGGLE_CONF_1_n:
            lcdSetText(screen_toggle_conf_1_top, 0, 0);
            lcdSetText(screen_toggle_conf_1_status, 1, 0);
            lcdSetText(screen_toggle_conf_y, 2, 0);
            lcdSetText(screen_toggle_conf_n_a, 3, 0);
            break;
        case TOGGLE_CONF_2_y:
            lcdSetText(screen_toggle_conf_2_top, 0, 0);
            lcdSetText(screen_toggle_conf_2_status, 1, 0);
            lcdSetText(screen_toggle_conf_y_a, 2, 0);
            lcdSetText(screen_toggle_conf_n, 3, 0);
            break;
        case TOGGLE_CONF_2_n:
            lcdSetText(screen_toggle_conf_2_top, 0, 0);
            lcdSetText(screen_toggle_conf_2_status, 1, 0);
            lcdSetText(screen_toggle_conf_y, 2, 0);
            lcdSetText(screen_toggle_conf_n_a, 3, 0);
            break;
        case TOGGLE_CONF_3_y:
            lcdSetText(screen_toggle_conf_3_top, 0, 0);
            lcdSetText(screen_toggle_conf_3_status, 1, 0);
            lcdSetText(screen_toggle_conf_y_a, 2, 0);
            lcdSetText(screen_toggle_conf_n, 3, 0);
            break;
        case TOGGLE_CONF_3_n:
            lcdSetText(screen_toggle_conf_3_top, 0, 0);
            lcdSetText(screen_toggle_conf_3_status, 1, 0);
            lcdSetText(screen_toggle_conf_y, 2, 0);
            lcdSetText(screen_toggle_conf_n_a, 3, 0);
            break;
        case TOGGLE_CONF_4_y:
            lcdSetText(screen_toggle_conf_4_top, 0, 0);
            lcdSetText(screen_toggle_conf_4_status, 1, 0);
            lcdSetText(screen_toggle_conf_y_a, 2, 0);
            lcdSetText(screen_toggle_conf_n, 3, 0);
            break;
        case TOGGLE_CONF_4_n:
            lcdSetText(screen_toggle_conf_4_top, 0, 0);
            lcdSetText(screen_toggle_conf_4_status, 1, 0);
            lcdSetText(screen_toggle_conf_y, 2, 0);
            lcdSetText(screen_toggle_conf_n_a, 3, 0);
            break;
        case TOGGLE_CONF_5_y:
            lcdSetText(screen_toggle_conf_5_top, 0, 0);
            lcdSetText(screen_toggle_conf_5_status, 1, 0);
            lcdSetText(screen_toggle_conf_y_a, 2, 0);
            lcdSetText(screen_toggle_conf_n, 3, 0);
            break;
        case TOGGLE_CONF_5_n:
            lcdSetText(screen_toggle_conf_5_top, 0, 0);
            lcdSetText(screen_toggle_conf_5_status, 1, 0);
            lcdSetText(screen_toggle_conf_y, 2, 0);
            lcdSetText(screen_toggle_conf_n_a, 3, 0);
            break;
        case TOGGLE_CONF_6_y:
            lcdSetText(screen_toggle_conf_6_top, 0, 0);
            lcdSetText(screen_toggle_conf_6_status, 1, 0);
            lcdSetText(screen_toggle_conf_y_a, 2, 0);
            lcdSetText(screen_toggle_conf_n, 3, 0);
            break;
        case TOGGLE_CONF_6_n:
            lcdSetText(screen_toggle_conf_6_top, 0, 0);
            lcdSetText(screen_toggle_conf_6_status, 1, 0);
            lcdSetText(screen_toggle_conf_y, 2, 0);
            lcdSetText(screen_toggle_conf_n_a, 3, 0);
            break;
    }
}


void update_screen(uint8_t screen_state, uint8_t outlet_num_abs) {
    switch(screen_state) {
    case TEMP_HUM:
        update_th_info();
        break;
    case OUTLET_1:
        update_outlet_info(outlet_num_abs);
        break;
    case OUTLET_2:
        update_outlet_info(outlet_num_abs);
        break;
    case OUTLET_3:
        update_outlet_info(outlet_num_abs);
        break;
    case OUTLET_4:
        update_outlet_info(outlet_num_abs);
        break;
    case OUTLET_5:
        update_outlet_info(outlet_num_abs);
        break;
    case OUTLET_6:
        update_outlet_info(outlet_num_abs);
        break;
    case TOGGLE_CONF_1_y:
        update_outlet_status(outlet_num_abs);
        break;
    case TOGGLE_CONF_2_y:
        update_outlet_status(outlet_num_abs);
        break;
    case TOGGLE_CONF_3_y:
        update_outlet_status(outlet_num_abs);
        break;
    case TOGGLE_CONF_4_y:
        update_outlet_status(outlet_num_abs);
        break;
    case TOGGLE_CONF_5_y:
        update_outlet_status(outlet_num_abs);
        break;
    case TOGGLE_CONF_6_y:
        update_outlet_status(outlet_num_abs);
        break;
    default:
        break;
    }
}

void update_th_info(void) {
    unsigned int i;
    for (i = 13; i < 15; i++) {
        screen_temp_hum_3[i] = res_t[i-13];
    }
    for (i = 10; i < 12; i++) {
        screen_temp_hum_4[i] = res_h[i-10];
    }
}

void update_outlet_info(uint8_t outlet_num_abs) {
    unsigned int i;
    unsigned int j;
    unsigned int k;
    switch (outlet_num_abs) {
    case 1:
        for (i = 9; i < 15; i++) {
            screen_outlet_2[i] = res_v_1[i-9];
        }
        for (j = 9; j < 15; j++) {
            screen_outlet_3[j] = res_i_1_1[j-9];
        }
        for (k = 11; k < 17; k++) {
            screen_outlet_4[k] = res_p_1_1[k-11];
        }
        break;
    case 2:
        for (i = 9; i < 15; i++) {
            screen_outlet_2[i] = res_v_1[i-9];
        }
        for (j = 9; j < 15; j++) {
            screen_outlet_3[j] = res_i_2_1[j-9];
        }
        for (k = 11; k < 17; k++) {
            screen_outlet_4[k] = res_p_2_1[k-11];
        }
        break;
    case 3:
        for (i = 9; i < 15; i++) {
            screen_outlet_2[i] = res_v_1[i-9];
        }
        for (j = 9; j < 15; j++) {
            screen_outlet_3[j] = res_i_3_1[j-9];
        }
        for (k = 11; k < 17; k++) {
            screen_outlet_4[k] = res_p_3_1[k-11];
        }
        break;
    case 4:
        for (i = 9; i < 15; i++) {
            screen_outlet_2[i] = res_v_2[i-9];
        }
        for (j = 9; j < 15; j++) {
            screen_outlet_3[j] = res_i_1_2[j-9];
        }
        for (k = 11; k < 17; k++) {
            screen_outlet_4[k] = res_p_1_2[k-11];
        }
        break;
    case 5:
        for (i = 9; i < 15; i++) {
            screen_outlet_2[i] = res_v_2[i-9];
        }
        for (j = 9; j < 15; j++) {
            screen_outlet_3[j] = res_i_2_2[j-9];
        }
        for (k = 11; k < 17; k++) {
            screen_outlet_4[k] = res_p_2_2[k-11];
        }
        break;
    case 6:
        for (i = 9; i < 15; i++) {
            screen_outlet_2[i] = res_v_2[i-9];
        }
        for (j = 9; j < 15; j++) {
            screen_outlet_3[j] = res_i_3_2[j-9];
        }
        for (k = 11; k < 17; k++) {
            screen_outlet_4[k] = res_p_3_2[k-11];
        }
        break;
    }
//    screen_outlet_2[17] = 'V';
//    screen_outlet_2[18] = 'A';
//    screen_outlet_2[19] = 'C';
//    screen_outlet_3[19] = 'A';
//    screen_outlet_4[19] = 'W';


}

void update_outlet_status(uint8_t outlet_num_abs) {
    char status_on[3] = "ON ";
    char status_off[3] = "OFF";
    switch (outlet_num_abs) {
//        case 1:
//            if (outlet_statuses[outlet_num_abs-1]) {
//                unsigned int i;
//                for (i = 17; i < 20; i++) {
//                    screen_outlet_1_1[i] = status_on[i-17];
//                }
//            }
//            else {
//                unsigned int i;
//                for (i = 17; i < 20; i++) {
//                    screen_outlet_1_1[i] = status_off[i-17];
//                }
//            }
//            break;
//        case 2:
//            if (outlet_statuses[outlet_num_abs-1]) {
//                unsigned int i;
//                for (i = 17; i < 20; i++) {
//                    screen_outlet_2_1[i] = status_on[i-17];
//                }
//            }
//            else {
//                unsigned int i;
//                for (i = 17; i < 20; i++) {
//                    screen_outlet_2_1[i] = status_off[i-17];
//                }
//            }
//            break;
//        case 3:
//            if (outlet_statuses[outlet_num_abs-1]) {
//                unsigned int i;
//                for (i = 17; i < 20; i++) {
//                    screen_outlet_3_1[i] = status_on[i-17];
//                }
//            }
//            else {
//                unsigned int i;
//                for (i = 17; i < 20; i++) {
//                    screen_outlet_3_1[i] = status_off[i-17];
//                }
//            }
//            break;
//        case 4:
//            if (outlet_statuses[outlet_num_abs-1]) {
//                unsigned int i;
//                for (i = 17; i < 20; i++) {
//                    screen_outlet_4_1[i] = status_on[i-17];
//                }
//            }
//            else {
//                unsigned int i;
//                for (i = 17; i < 20; i++) {
//                    screen_outlet_4_1[i] = status_off[i-17];
//                }
//            }
//            break;
//        case 5:
//            if (outlet_statuses[outlet_num_abs-1]) {
//                unsigned int i;
//                for (i = 17; i < 20; i++) {
//                    screen_outlet_5_1[i] = status_on[i-17];
//                }
//            }
//            else {
//                unsigned int i;
//                for (i = 17; i < 20; i++) {
//                    screen_outlet_5_1[i] = status_off[i-17];
//                }
//            }
//            break;
//        case 6:
//            if (outlet_statuses[outlet_num_abs-1]) {
//                unsigned int i;
//                for (i = 17; i < 20; i++) {
//                    screen_outlet_6_1[i] = status_on[i-17];
//                }
//            }
//            else {
//                unsigned int i;
//                for (i = 17; i < 20; i++) {
//                    screen_outlet_6_1[i] = status_off[i-17];
//                }
//            }
//            break;
//    }
//
//    if (outlet_statuses[outlet_num_abs-1]) {
//        unsigned int i;
//        for (i = 17; i < 20; i++) {
//            screen_toggle_conf_status[i-1] = status_on[i-17];
//        }
//    }
//    else {
//        unsigned int i;
//        for (i = 17; i < 20; i++) {
//            screen_toggle_conf_status[i-1] = status_off[i-17];
//        }
//    }

    case 1:
        if (outlet_statuses[outlet_num_abs-1]) {
            unsigned int i;
            for (i = 17; i < 20; i++) {
                screen_outlet_1_1[i] = status_on[i-17];
                screen_toggle_conf_1_status[i-1] = status_on[i-17];
            }
        }
        else {
            unsigned int i;
            for (i = 17; i < 20; i++) {
                screen_outlet_1_1[i] = status_off[i-17];
                screen_toggle_conf_1_status[i-1] = status_off[i-17];
            }
        }
        break;
    case 2:
        if (outlet_statuses[outlet_num_abs-1]) {
            unsigned int i;
            for (i = 17; i < 20; i++) {
                screen_outlet_2_1[i] = status_on[i-17];
                screen_toggle_conf_2_status[i-1] = status_on[i-17];
            }
        }
        else {
            unsigned int i;
            for (i = 17; i < 20; i++) {
                screen_outlet_2_1[i] = status_off[i-17];
                screen_toggle_conf_2_status[i-1] = status_off[i-17];
            }
        }
        break;
    case 3:
        if (outlet_statuses[outlet_num_abs-1]) {
            unsigned int i;
            for (i = 17; i < 20; i++) {
                screen_outlet_3_1[i] = status_on[i-17];
                screen_toggle_conf_3_status[i-1] = status_on[i-17];
            }
        }
        else {
            unsigned int i;
            for (i = 17; i < 20; i++) {
                screen_outlet_3_1[i] = status_off[i-17];
                screen_toggle_conf_3_status[i-1] = status_off[i-17];
            }
        }
        break;
    case 4:
        if (outlet_statuses[outlet_num_abs-1]) {
            unsigned int i;
            for (i = 17; i < 20; i++) {
                screen_outlet_4_1[i] = status_on[i-17];
                screen_toggle_conf_4_status[i-1] = status_on[i-17];
            }
        }
        else {
            unsigned int i;
            for (i = 17; i < 20; i++) {
                screen_outlet_4_1[i] = status_off[i-17];
                screen_toggle_conf_4_status[i-1] = status_off[i-17];
            }
        }
        break;
    case 5:
        if (outlet_statuses[outlet_num_abs-1]) {
            unsigned int i;
            for (i = 17; i < 20; i++) {
                screen_outlet_5_1[i] = status_on[i-17];
                screen_toggle_conf_5_status[i-1] = status_on[i-17];
            }
        }
        else {
            unsigned int i;
            for (i = 17; i < 20; i++) {
                screen_outlet_5_1[i] = status_off[i-17];
                screen_toggle_conf_5_status[i-1] = status_off[i-17];
            }
        }
        break;
    case 6:
        if (outlet_statuses[outlet_num_abs-1]) {
            unsigned int i;
            for (i = 17; i < 20; i++) {
                screen_outlet_6_1[i] = status_on[i-17];
                screen_toggle_conf_6_status[i-1] = status_on[i-17];
            }
        }
        else {
            unsigned int i;
            for (i = 17; i < 20; i++) {
                screen_outlet_6_1[i] = status_off[i-17];
                screen_toggle_conf_6_status[i-1] = status_off[i-17];
            }
        }
        break;
    }
}
