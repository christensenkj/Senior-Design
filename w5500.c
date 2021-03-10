/*
 * w5500.c
 *
 *  Created on: Oct 18, 2020
 *      Author: Karston Christensen
 */


#include <stdint.h>
#include <stdbool.h>
#include "w5500.h"
#include "msp_config.h"

uint8_t status;

// bring in the byte-by-byte SPI function from msp_server.c
extern uint8_t send_receive_byte_SPI(uint8_t byte);

// define data length operation modes for SPI packets
#define _W5500_SPI_VDM_OP_          0x00
#define _W5500_SPI_FDM_OP_LEN1_     0x01
#define _W5500_SPI_FDM_OP_LEN2_     0x02
#define _W5500_SPI_FDM_OP_LEN4_     0x03

// provide a port for the case where a port is not specified by the user
uint16_t localPort = 0;

/**
 * wait until socket closed status
 * Check while the Sn_SR register (status register) isn't closed
 */
void waitUntilSocketClosed(uint8_t sn) {
    status = getSn_SR(sn);
    while ((status = getSn_SR(sn)) != SOCK_CLOSED) {
//        if (status == SOCK_ESTABLISHED) {
//            status = getSn_SR(sn);
//            return;
//        }
//        else if (status == SOCK_LISTEN) {
//            status = getSn_SR(sn);
//            return;
//        }
//        disconnect(sn);
    }
}
/**
 * initialize the channel in particular mode, set the port, and wait for W5500 to finish.
 * Set the Sn_MR register (socket mode register) on socket s to the desired protocol
 * Set the Sn_PORT register to listen on the specified port
 * Send the OPEN command to the Sn_CR register (socket command register)
 */
void socket(uint8_t sn, uint8_t protocol, uint16_t port, uint8_t flag) {
    setSn_MR(sn, protocol | flag);
    if (port != 0) {
        setSn_PORT(sn, port);
    } else {
        localPort++; // if not provided, set the source port number to an arbitrary number
        setSn_PORT(sn, localPort);
    }
    setSn_CR(sn, Sn_CR_OPEN);
    // wait for the command register to be set back to 0x00, i.e. done processing the command
    while (getSn_CR(sn));
}
/**
 * open socket and wait for initiated status
 * Wait until the status register (Sn_SR) is SOCK_INIT (socket initialized)
 */
void openSocketOnPort(uint8_t sn, uint8_t port) {
    socket(sn, Sn_MR_TCP, port, 0x00);
    status = getSn_SR(sn);
    while ((status = getSn_SR(sn)) != SOCK_INIT) {
        if (status == SOCK_ESTABLISHED) {
            status = getSn_SR(sn);
            return;
        }
    }
}
/**
 * establish connection for the channel in passive (server) mode.
 * Send LISTEN command to Sn_CR (command register)
 * Wait for Sn_CR to return to 0x00
 */
void listen(uint8_t sn) {
    setSn_CR(sn, Sn_CR_LISTEN);
    while (getSn_CR(sn));
}
/**
 * listen and wait for listening status
 * Wait until Sn_SR (status register) becomes SOCK_LISTEN (listening active)
 */
void startListening(uint8_t sn) {
    listen(sn);
    status = getSn_SR(sn);
    while ((status = getSn_SR(sn)) != SOCK_LISTEN) {
        if (status == SOCK_ESTABLISHED) {
            status = getSn_SR(sn);
            return;
        }
    }
}
/**
 * disconnect
 * Send DISCON command to Sn_CR (command register)
 * Wait for Sn_CR to return to 0x00
 */
void disconnect(uint8_t sn) {
    setSn_CR(sn, Sn_CR_DISCON);
    while (getSn_CR(sn));
}
/**
 * close the socket
 * Send CLOSE command to Sn_CR register
 * Wait for Sn_CR to return to 0x00
 * Write a value of 1 to all bits of Sn_IR (interrupt register), clearing all the interrupts
 */
void close(uint8_t sn) {
    setSn_CR(sn, Sn_CR_CLOSE);
    while (getSn_CR(sn));
    setSn_IR(sn, 0xFF);
}


/*
 * Clear a local buffer, such as the local rx buffer or tx buffer
 */
void clearBuffer(uint8_t* array, uint8_t length) {
    while ( length != 0) {
        *array++ = 0;
        length--;
    }
}


//////////////////////////////////////////////////
// Socket N register RX/TX Buffer R/W functions //
//////////////////////////////////////////////////

void WIZCHIP_select() {
    P2OUT &= ~CS_WIZ_PIN;
}

void WIZCHIP_deselect() {
    P2OUT |= CS_WIZ_PIN;
}

uint8_t WIZCHIP_READ(uint32_t AddrSel)
{
   uint8_t ret;
   uint8_t spi_data[3];

   WIZCHIP_select();

   AddrSel |= (_W5500_SPI_READ_ | _W5500_SPI_VDM_OP_);

   send_receive_byte_SPI((AddrSel & 0x00FF0000) >> 16);
   send_receive_byte_SPI((AddrSel & 0x0000FF00) >>  8);
   send_receive_byte_SPI((AddrSel & 0x000000FF) >>  0);
   ret = send_receive_byte_SPI(0);
   WIZCHIP_deselect();
   return ret;
}


void WIZCHIP_WRITE(uint32_t AddrSel, uint8_t wb )
{
   uint8_t spi_data[4];

   WIZCHIP_select();

   AddrSel |= (_W5500_SPI_WRITE_ | _W5500_SPI_VDM_OP_);

   send_receive_byte_SPI((AddrSel & 0x00FF0000) >> 16);
   send_receive_byte_SPI((AddrSel & 0x0000FF00) >>  8);
   send_receive_byte_SPI((AddrSel & 0x000000FF) >>  0);
   send_receive_byte_SPI(wb);
   WIZCHIP_deselect();
}


void WIZCHIP_READ_BUF (uint32_t AddrSel, uint8_t* pBuf, uint16_t len)
{
   uint8_t spi_data[3];
   uint16_t i;

   WIZCHIP_select();

   AddrSel |= (_W5500_SPI_READ_ | _W5500_SPI_VDM_OP_);

   send_receive_byte_SPI((AddrSel & 0x00FF0000) >> 16);
   send_receive_byte_SPI((AddrSel & 0x0000FF00) >>  8);
   send_receive_byte_SPI((AddrSel & 0x000000FF) >>  0);
   for(i = 0; i < len; i++)
      pBuf[i] = send_receive_byte_SPI(0);

   WIZCHIP_deselect();
}


void WIZCHIP_WRITE_BUF(uint32_t AddrSel, uint8_t* pBuf, uint16_t len)
{
   uint8_t spi_data[3];
   uint16_t i;

   WIZCHIP_select();

   AddrSel |= (_W5500_SPI_WRITE_ | _W5500_SPI_VDM_OP_);

   send_receive_byte_SPI((AddrSel & 0x00FF0000) >> 16);
   send_receive_byte_SPI((AddrSel & 0x0000FF00) >>  8);
   send_receive_byte_SPI((AddrSel & 0x000000FF) >>  0);
   for(i = 0; i < len; i++)
       send_receive_byte_SPI(pBuf[i]);

   WIZCHIP_deselect();
}


uint16_t getSn_TX_FSR(uint8_t sn)
{
   uint16_t val=0,val1=0;

   do
   {
      val1 = WIZCHIP_READ(Sn_TX_FSR(sn));
      val1 = (val1 << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_TX_FSR(sn),1));
      if (val1 != 0)
      {
        val = WIZCHIP_READ(Sn_TX_FSR(sn));
        val = (val << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_TX_FSR(sn),1));
      }
   }while (val != val1);
   return val;
}


uint16_t getSn_RX_RSR(uint8_t sn)
{
   uint16_t val=0,val1=0;

   do
   {
      val1 = WIZCHIP_READ(Sn_RX_RSR(sn));
      val1 = (val1 << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_RX_RSR(sn),1));
      if (val1 != 0)
      {
        val = WIZCHIP_READ(Sn_RX_RSR(sn));
        val = (val << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_RX_RSR(sn),1));
      }
   }while (val != val1);
   return val;
}


void wiz_send_data(uint8_t sn, uint8_t *wizdata, uint16_t len)
{
   uint16_t ptr = 0;
   uint32_t addrsel = 0;

   if(len == 0)  return;
   ptr = getSn_TX_WR(sn);
   //M20140501 : implict type casting -> explict type casting
   addrsel = ((uint32_t)ptr << 8) + (WIZCHIP_TXBUF_BLOCK(sn) << 3);
   // write the buffer to the W5500 TX buffer
   WIZCHIP_WRITE_BUF(addrsel, wizdata, len);
   // set a new TX buffer write pointer after writing the data
   ptr += len;
   setSn_TX_WR(sn,ptr);
   // Send the SEND command to Sn_CR (command register) to transmit all the data in the TX buffer
   setSn_CR(sn, Sn_CR_SEND);
   // wait until Sn_CR returns to 0x00
   while (getSn_CR(sn));
   uint16_t timeout = 0;
   // wait until the data is successfully send by the SEND_OK interrupt in Sn_IR (interrupt register)
   while ((getSn_IR(sn) & Sn_IR_SENDOK) != Sn_IR_SENDOK) {
       timeout++;
       // if instead there is a socket closed interrupt, close the socket and return an error code
       if ((getSn_IR(sn) == SOCK_CLOSED) || (timeout > 100000)) {
           close(sn);
       }
   }
   // clear the SEND_OK interrupt by writing 1 to its position in Sn_SR
   setSn_IR(sn, Sn_IR_SENDOK);
}



void wiz_recv_data(uint8_t sn, uint8_t *wizdata, uint16_t len)
{
   uint16_t ptr = 0;
   uint32_t addrsel = 0;

   if(len == 0) return;
   ptr = getSn_RX_RD(sn);
   //M20140501 : implict type casting -> explict type casting
   //addrsel = ((ptr << 8) + (WIZCHIP_RXBUF_BLOCK(sn) << 3);
   addrsel = ((uint32_t)ptr << 8) + (WIZCHIP_RXBUF_BLOCK(sn) << 3);
   //
   WIZCHIP_READ_BUF(addrsel, wizdata, len);
   ptr += len;

   setSn_RX_RD(sn,ptr);

   setSn_CR(sn, Sn_CR_RECV);
   while (getSn_CR(sn));
}
