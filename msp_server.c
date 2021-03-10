/*
 * msp_server.c
 *
 *  Created on: Oct 18, 2020
 *      Author: Karston Christensen
 */

#include <msp430.h>
#include <stdint.h>
#include "msp_server.h"
#include "msp_config.h"
#include "w5500.h"

// status from Sn_SR
extern uint8_t status;

// W5500 MAC address (physical address listed on a sticker on the wiznet board itself)
const uint8_t MAC_ADDR[6] = {0x00, 0x08, 0xDC, 0x5F, 0xED, 0x0A};

// local RX and TX buffer to store data on the MSP430
#define TX_MAX_BUF_SIZE         0x7F
#define RX_MAX_BUF_SIZE         0x7F
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

//    uint16_t count = 0;
//    while(is_connected(sn) == -1) {
//        count++;
//        if (count > 10000) {
//            return -1;
//        }
//    }
//    return 0;
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
//    status = getSn_SR(sn);
//    uint16_t timeout = 0;
//    while((bytes_received = getSn_RX_RSR(sn)) == 0) {
//        timeout++;
//        if (timeout > 50000) {
//            close(sn);
//        }
//    }
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
void write_char_to_tx_buffer(uint8_t character) {
    if (tx_buffer_pointer < TX_MAX_BUF_SIZE) {
        tx_buffer[tx_buffer_pointer++] = character;
    }
}

/*
 * Add a string to the write buffer, tx_buffer
 */
void write_string_to_tx_buffer(const uint8_t *string) {
    while (*string) {
        write_char_to_tx_buffer(*string++);
    }
}

/*
 * Add an integer to the write buffer, tx_buffer, as hex
 */
void write_int_to_tx_buffer_as_hex(uint16_t i) {
    write_string_to_tx_buffer((const uint8_t *) "0x");
    write_char_to_tx_buffer(to_hex(i >> 12));
    write_char_to_tx_buffer(to_hex(i >> 8));
    write_char_to_tx_buffer(to_hex(i >> 4));
    write_char_to_tx_buffer(to_hex(i >> 0));
}

/*
 * Add a char to the write buffer, tx_buffer, as hex
 */
void write_char_to_tx_buffer_as_hex(uint8_t c) {
    write_string_to_tx_buffer((const uint8_t * ) "0x");
    write_char_to_tx_buffer(to_hex(c >> 4));
    write_char_to_tx_buffer(to_hex(c >> 0));
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

/*
 * Process a received command from a client
 */
void receive_cmd(uint8_t sn, uint16_t len) {

    uint32_t recvd_len = recv(sn, len);
    if (rx_buffer[0] == 'T') {
        P4DIR |= BIT7;
        P4OUT ^= BIT7;
        uint8_t str[128] = "OK\n\n";
        write_string_to_tx_buffer(str);
        send(sn);
    }
    else if (rx_buffer[0] == 'C') {
        uint8_t str[28] = "OK\n1\n";
        write_string_to_tx_buffer(str);
        send(sn);
    }
    else {
        uint8_t str[28] = "BAD\nUnknown Command\n";
        write_string_to_tx_buffer(str);
        send(sn);
    }
}

/*
 * Send data on own accord to a client that is connected
 */
uint8_t j = 0;
void send_data_onitsown(uint8_t sn, uint16_t len) {
    write_string_to_tx_buffer("OK\n");
    write_string_to_tx_buffer("Temperature ");
    write_int_to_tx_buffer_as_hex(j);
    write_string_to_tx_buffer(", ");
    write_string_to_tx_buffer("Humidity ");
    write_int_to_tx_buffer_as_hex(j);
    write_string_to_tx_buffer(", ");
    write_string_to_tx_buffer("Voltage ");
    write_int_to_tx_buffer_as_hex(j);
    write_string_to_tx_buffer(", ");
    write_string_to_tx_buffer("Current ");
    write_int_to_tx_buffer_as_hex(j);
    write_string_to_tx_buffer("\n");
    send(sn);
    j++;
}


// send an update via TCP to client
void send_update(uint8_t sn, uint16_t port) {

    if (status == SOCK_ESTABLISHED) {
        stop_server(sn);
        return;
    }
    else {
        if (status != SOCK_LISTEN) {
            _disable_interrupts();
            // start server with socket 0, port 200
            start_server(sn, port);
            _enable_interrupts();
        }
        // wait for connection on socket 0
        wait_for_connection(sn);
        uint16_t len = 100;
        send_data_onitsown(sn, len);
        // close socket 0
//        stop_server(sn);
    }

}

void receive_from_server(uint8_t sn, uint16_t port) {
    // start server with socket 0, port 200
    start_server(sn, port);
    // wait for connection on socket 0
    wait_for_connection(sn);
//        // wait for data reception
//        wait_for_data(sn);
//        uint16_t len = 100;
//        receive_cmd(sn, len);
//    }

    // close socket and disconnect
    stop_server(sn);
}
