/*
 * msp_server.h
 *
 *  Created on: Oct 18, 2020
 *      Author: Karston Christensen
 */

#ifndef MSP_SERVER_H_
#define MSP_SERVER_H_

// function prototypes (in order of which they're presented in msp_server.c (and used, roughly))

// startup and shutdown of socket
void reset_w5500(void);
void w5500_config(const uint8_t *sourceIP, const uint8_t *gatewayIP, const uint8_t *subnetMask);
void start_server(uint8_t s, uint8_t port);
void wait_for_connection(uint8_t s);
int is_connected(uint8_t s);
void wait_for_data(uint8_t s);
void stop_server(uint8_t s);

// reading and writing to RX and TX buffers on W5500
//int read_into_rx_buffer(uint8_t *buf);
void send(uint8_t sn);
uint32_t recv(uint8_t sn, uint16_t len);

// manipulating local buffers to store data to/from W5500
void write_char_to_tx_buffer(uint8_t character);
void write_string_to_tx_buffer(const uint8_t *string);
void write_int_to_tx_buffer_as_hex(uint16_t i);
void write_char_to_tx_buffer_as_hex(uint8_t c);
uint8_t to_hex(uint8_t c);

// Conduit function to perform SPI operations on a byte-by-byte basis between msp430 and W5500
uint8_t send_receive_byte_SPI(uint8_t byte);

// Application Code for testing
void receive_cmd(uint8_t sn, uint16_t len);
void send_data_onitsown(uint8_t sn, uint16_t len);
void send_update(uint8_t sn, uint16_t port);
void receive_from_server(uint8_t sn, uint16_t port);
void net_process_socket_sender(uint8_t sn, uint16_t port);
void net_process_socket_receiver(uint8_t sn, uint16_t port);

#endif /* MSP_SERVER_H_ */
