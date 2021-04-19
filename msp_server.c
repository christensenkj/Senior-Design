/*
 * msp_server.c
 *
 *  Created on: Oct 18, 2020
 *      Author: Karston Christensen
 */

#include <msp430.h>
#include <stdint.h>
#include <stdio.h>
#include "msp_server.h"
#include "msp_config.h"
#include "w5500.h"
#include "i2c.h"

// status from Sn_SR
extern uint8_t status;
// get the toggle status from the i2c module
extern uint8_t toggle_status;
extern uint8_t outlet_statuses[12];
// strings from i2c updates
extern char res_v_1[5];
extern char res_i_1_1[5];
extern char res_p_1_1[5];
extern char res_i_2_1[5];
extern char res_p_2_1[5];
extern char res_i_3_1[5];
extern char res_p_3_1[5];
extern char res_v_2[5];
extern char res_i_1_2[5];
extern char res_p_1_2[5];
extern char res_i_2_2[5];
extern char res_p_2_2[5];
extern char res_i_3_2[5];
extern char res_p_3_2[5];
extern char res_t[5];
extern char res_h[5];
extern char outlet_status_string[23];

// W5500 MAC address (physical address listed on a sticker on the wiznet board itself)
const uint8_t MAC_ADDR[6] = {0x00, 0x08, 0xDC, 0x5F, 0xED, 0x0A};

// local RX and TX buffer to store data on the MSP430
#define TX_MAX_BUF_SIZE         0xFFF
#define RX_MAX_BUF_SIZE         0xF
uint8_t tx_buffer[TX_MAX_BUF_SIZE];
uint8_t rx_buffer[RX_MAX_BUF_SIZE];

// keep track of index in rx_buffer
uint8_t rx_buffer_pointer = 0;
uint8_t tx_buffer_pointer = 0;
uint16_t bytes_received = 0;

/*
 * Reset the W5500 upon startup by toggling the hardware reset pin
 */
void reset_w5500(void) {
    // set reset pin low
    RST_WIZ_OUT &= ~RST_WIZ_PIN;
    // wait 20 us
    __delay_cycles(480);
    // set reset pin high
    RST_WIZ_OUT |= RST_WIZ_PIN;
    // wait 200 ms
    __delay_cycles(4800000);
}

/*
 * Configure the W5500 at startup
 */
void w5500_config(const uint8_t *sourceIP, const uint8_t *gatewayIP, const uint8_t *subnetMask) {
    // set the MAC address in common register block
    setSHAR((uint8_t * ) MAC_ADDR);
    // set the subnet mask in the common register block (this is the DNS address typically)
    setSUBR((uint8_t * ) subnetMask);
    // set the gateway IP address in the common register block (this is the IP address of the router)
    setGAR((uint8_t * ) gatewayIP);
    // set the local IP address in the common register block
    setSIPR((uint8_t * ) sourceIP);
    // set retry time to 6000*100us = 600ms in the common register block
    setRTR(6000);
    // set the retry count to 3 in the common register block
    setRCR(3);
}

/*
 * Start the server
 */
void start_server(uint8_t sn, uint8_t port) {
    // make sure the desired socket s is closed
    waitUntilSocketClosed(sn);
    // open a socket on desired port
    openSocketOnPort(sn, port);
    // start listening for clients on the socket
    startListening(sn);
}

/*
 * Wait for connection on socket s
 */
void wait_for_connection(uint8_t sn) {
    // hang here until the status changes to SOCK_ESTABLISHED
    while((status = getSn_SR(sn)) != SOCK_ESTABLISHED);
}

/*
 * Return success code if there is a connection on socket s
 */
int is_connected(uint8_t sn) {
    if ((status = getSn_SR(sn)) == SOCK_ESTABLISHED) {
        return 0;
    }
    else {
        return -1;
    }
}

/*
 * Wait for data on socket s
 */
void wait_for_data(uint8_t sn) {
    // hang here until bytes are received from a client
    while((bytes_received = getSn_RX_RSR(sn)) == 0);
}


/*
 * Stop the server
 */
void stop_server(uint8_t sn) {
    status = getSn_SR(sn);
    // disconnect from client
    disconnect(sn);
    // close the socket
    close(sn);
}


/*
 * Send the data in the tx_buffer to the W5500
 */
void send(uint8_t sn) {
    uint16_t length = tx_buffer_pointer & 0x00FF;
    wiz_send_data(sn, tx_buffer, length);
    tx_buffer_pointer = 0;
    clearBuffer(tx_buffer, TX_MAX_BUF_SIZE);
}

/*
 * Receive data from client by reading out of the RX buffer
 */
uint32_t recv(uint8_t sn, uint16_t len) {
    clearBuffer(rx_buffer, RX_MAX_BUF_SIZE);
    uint8_t  tmp = 0;
    uint16_t recvsize = 0;
    recvsize = getSn_RxMAX(sn);
    if(recvsize < len) len = recvsize;

    while(1) {
        recvsize = getSn_RX_RSR(sn);
        tmp = getSn_SR(sn);
        if (tmp != SOCK_ESTABLISHED) {
            if (tmp == SOCK_CLOSE_WAIT) {
                if (recvsize != 0) break;
                else if (getSn_TX_FSR(sn) == getSn_TxMAX(sn)) {
                    close(sn);
                }
            }
            else {
                close(sn);
            }
        }
        if (recvsize != 0) break;
    }
    wiz_recv_data(sn, rx_buffer, recvsize);
    setSn_CR(sn,Sn_CR_RECV);
    while(getSn_CR(sn));
    return (int32_t)len;
}


/*
 * Add character to the write buffer, tx_buffer
 */
void write_char_to_tx_buffer(char character) {
    if (tx_buffer_pointer < TX_MAX_BUF_SIZE) {
        tx_buffer[tx_buffer_pointer++] = character;
    }
}

/*
 * Add a string to the write buffer, tx_buffer
 */
void write_string_to_tx_buffer(char *string) {
    while (*string) {
        write_char_to_tx_buffer(*string++);
    }
}


/*
 * Convert an int or a char to hex
 */
uint8_t to_hex(uint8_t c) {
    return "0123456789ABCDEF"[c & 0x0F];
}

/*
 * Send and receive a byte over SPI
 */

#ifdef DEV_KIT
uint8_t send_receive_byte_SPI(uint8_t byte) {
    uint8_t receive_byte = 0;
    // send byte to the SPI transmission buffer
    UCA0TXBUF = byte;
    // wait for transmission or receive to complete
    while (UCA0STAT & UCBUSY);
    // retrieve the returned byte
    receive_byte = UCA0RXBUF;
    return receive_byte;
}
#endif

#ifndef DEV_KIT
uint8_t send_receive_byte_SPI(uint8_t byte) {
    uint8_t receive_byte = 0;
    // send byte to the SPI transmission buffer
    UCB1TXBUF = byte;
    // wait for transmission or receive to complete
    while (UCB1STAT & UCBUSY);
    // retrieve the returned byte
    receive_byte = UCB1RXBUF;
    return receive_byte;
}
#endif


/*
 * Process a received command from a client
 */
void receive_cmd(uint8_t sn, uint16_t len) {
    uint8_t i2_address;
    uint8_t outlet_num;
    uint8_t abs_outlet_num;
    uint32_t recvd_len = recv(sn, len);
    printf("\n%d\n", rx_buffer[0]);

    if (rx_buffer[0] == 'C') {
        char str[28] = "OK\n12\n";
        write_string_to_tx_buffer(str);
        send(sn);
    }

    else if (rx_buffer[0] == '1' || rx_buffer[0] == '2' || rx_buffer[0] == '3' || rx_buffer[0] == '4' || rx_buffer[0] == '5' || rx_buffer[0] == '6' ||
            rx_buffer[0] == '7' || rx_buffer[0] == '8' || rx_buffer[0] == '9' || rx_buffer[0] == '10' || rx_buffer[0] == '11' || rx_buffer[0] == '12') {
        if (rx_buffer[0] == '1') {
            i2_address = 0x33;
            outlet_num = 2;
            abs_outlet_num = 1;
        }
        else if (rx_buffer[0] == '2') {
            i2_address = 0x33;
            outlet_num = 1;
            abs_outlet_num = 2;
        }
        else if (rx_buffer[0] == '3') {
            i2_address = 0x33;
            outlet_num = 0;
            abs_outlet_num = 3;
        }
        else if (rx_buffer[0] == '4') {
            i2_address = 0x44;
            outlet_num = 2;
            abs_outlet_num = 4;
        }
        else if (rx_buffer[0] == '5') {
            i2_address = 0x44;
            outlet_num = 1;
            abs_outlet_num = 5;
        }
        else if (rx_buffer[0] == '6') {
            i2_address = 0x44;
            outlet_num = 0;
            abs_outlet_num = 6;
        }
        else if (rx_buffer[0] == '7') {
            i2_address = 0x55;
            outlet_num = 2;
            abs_outlet_num = 7;
        }
        else if (rx_buffer[0] == '8') {
            i2_address = 0x55;
            outlet_num = 1;
            abs_outlet_num = 8;
        }
        else if (rx_buffer[0] == '9') {
            i2_address = 0x55;
            outlet_num = 0;
            abs_outlet_num = 9;
        }
        else if (rx_buffer[0] == '10') {
            i2_address = 0x66;
            outlet_num = 2;
            abs_outlet_num = 10;
        }
        else if (rx_buffer[0] == '11') {
            i2_address = 0x66;
            outlet_num = 1;
            abs_outlet_num = 11;
        }
        else if (rx_buffer[0] == '12') {
            i2_address = 0x66;
            outlet_num = 0;
            abs_outlet_num = 12;
        }
        // send a toggle command to desired outlet via i2c
        toggle_status = 1;
        i2c_send_toggle(i2_address, outlet_num);
        while(toggle_status);
        char str[128] = "OK\n\n";
        write_string_to_tx_buffer(str);
        outlet_statuses[abs_outlet_num-1] = !outlet_statuses[abs_outlet_num-1];
        send(sn);
    }

    else {
        char str[28] = "BAD\nUnknown Command\n";
        write_string_to_tx_buffer(str);
        send(sn);
    }
}

/*
 * Send data on own accord to a client that is connected
 */
void send_data_onitsown(uint8_t sn, uint16_t len) {
    // send OK flag
    write_string_to_tx_buffer("OK\n");
    // send temperature and humidity data
    write_string_to_tx_buffer(res_t);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_h);
    write_string_to_tx_buffer(",");
    // send outlet board mcu 1 info
    write_string_to_tx_buffer(res_v_1);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_i_1_1);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_p_1_1);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_i_2_1);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_p_2_1);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_i_3_1);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_p_3_1);
    write_string_to_tx_buffer(",");
    // send outlet board mcu 2 info
    write_string_to_tx_buffer(res_v_2);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_i_1_2);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_p_1_2);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_i_2_2);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_p_2_2);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_i_3_2);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_p_3_2);
    write_string_to_tx_buffer("\n");
    // send outlet board mcu 3 info
    write_string_to_tx_buffer(res_v_3);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_i_1_3);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_p_1_3);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_i_2_3);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_p_2_3);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_i_3_3);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_p_3_3);
    write_string_to_tx_buffer(",");
    // send outlet board mcu 4 info
    write_string_to_tx_buffer(res_v_4);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_i_1_4);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_p_1_4);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_i_2_4);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_p_2_4);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_i_3_4);
    write_string_to_tx_buffer(",");
    write_string_to_tx_buffer(res_p_3_4);
    // send outlet statuses
    write_string_to_tx_buffer(outlet_status_string);
    write_string_to_tx_buffer("\n");
}

/*
 * Handler for all socket transactions
 */
void net_process_socket_receiver(uint8_t sn, uint16_t port)
{
    unsigned char state = SOCK_CLOSED;
    unsigned short len = 0;

    state = getSn_SR(sn);
    switch (state) {
        case SOCK_INIT:
            listen(sn);
            break;
        case SOCK_ESTABLISHED:
            len = getSn_RX_RSR(sn);
            if (len > 0) {
                /* process the recv data */
                receive_cmd(sn, len);
            }
            break;
        case SOCK_CLOSE_WAIT:
            stop_server(sn);
            break;
        case SOCK_CLOSED:
            socket(sn, Sn_MR_TCP, port, 0x00);
            break;
        default:
            break;
    }
}

/*
 * Handler for all socket transactions
 */
void net_process_socket_sender(uint8_t sn, uint16_t port)
{
    unsigned char state = SOCK_CLOSED;
    unsigned short len = 400;

    state = getSn_SR(sn);
    switch (state) {
        case SOCK_INIT:
            listen(sn);
            break;
        case SOCK_ESTABLISHED:
            send_data_onitsown(sn, len);
            break;
        case SOCK_CLOSE_WAIT:
            stop_server(sn);
            break;
        case SOCK_CLOSED:
            socket(sn, Sn_MR_TCP, port, 0x00);
            break;
        default:
            break;
    }
}
