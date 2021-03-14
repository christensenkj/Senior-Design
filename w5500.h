/*
 * w5500.h
 *
 *  Created on: Oct 18, 2020
 *      Author: WIZNET staff
 */

#ifndef W5500_H_
#define W5500_H_

#include <stdint.h>

// define the base address of the W5500
#define _W5500_IO_BASE_                     0x00000000

// define macros for read/write command in SPI packets
#define _W5500_SPI_READ_                    (0x00 << 2)
#define _W5500_SPI_WRITE_                   (0x01 << 2)

// define addresses of the different register blocks
#define WIZCHIP_CREG_BLOCK                  0x00
#define WIZCHIP_SREG_BLOCK(N)               (1+4*N)
#define WIZCHIP_TXBUF_BLOCK(N)              (2+4*N)
#define WIZCHIP_RXBUF_BLOCK(N)              (3+4*N)

// compute offset address
#define WIZCHIP_OFFSET_INC(ADDR, N)         (ADDR + (N<<8))

//----------------------------- W5500 Common Registers IOMAP -----------------------------
#define MR                 (_W5500_IO_BASE_ + (0x0000 << 8) + (WIZCHIP_CREG_BLOCK << 3))
#define GAR                (_W5500_IO_BASE_ + (0x0001 << 8) + (WIZCHIP_CREG_BLOCK << 3))
#define SUBR               (_W5500_IO_BASE_ + (0x0005 << 8) + (WIZCHIP_CREG_BLOCK << 3))
#define SHAR               (_W5500_IO_BASE_ + (0x0009 << 8) + (WIZCHIP_CREG_BLOCK << 3))
#define SIPR               (_W5500_IO_BASE_ + (0x000F << 8) + (WIZCHIP_CREG_BLOCK << 3))
#define INTLEVEL           (_W5500_IO_BASE_ + (0x0013 << 8) + (WIZCHIP_CREG_BLOCK << 3))
#define IR                 (_W5500_IO_BASE_ + (0x0015 << 8) + (WIZCHIP_CREG_BLOCK << 3))
#define _IMR_              (_W5500_IO_BASE_ + (0x0016 << 8) + (WIZCHIP_CREG_BLOCK << 3))
#define SIR                (_W5500_IO_BASE_ + (0x0017 << 8) + (WIZCHIP_CREG_BLOCK << 3))
#define SIMR               (_W5500_IO_BASE_ + (0x0018 << 8) + (WIZCHIP_CREG_BLOCK << 3))
#define _RTR_              (_W5500_IO_BASE_ + (0x0019 << 8) + (WIZCHIP_CREG_BLOCK << 3))
#define _RCR_              (_W5500_IO_BASE_ + (0x001B << 8) + (WIZCHIP_CREG_BLOCK << 3))


//----------------------------- W5500 Socket Registers IOMAP -----------------------------
#define Sn_MR(N)           (_W5500_IO_BASE_ + (0x0000 << 8) + (WIZCHIP_SREG_BLOCK(N) << 3))
#define Sn_CR(N)           (_W5500_IO_BASE_ + (0x0001 << 8) + (WIZCHIP_SREG_BLOCK(N) << 3))
#define Sn_IR(N)           (_W5500_IO_BASE_ + (0x0002 << 8) + (WIZCHIP_SREG_BLOCK(N) << 3))
#define Sn_SR(N)           (_W5500_IO_BASE_ + (0x0003 << 8) + (WIZCHIP_SREG_BLOCK(N) << 3))
#define Sn_PORT(N)         (_W5500_IO_BASE_ + (0x0004 << 8) + (WIZCHIP_SREG_BLOCK(N) << 3))
#define Sn_RXBUF_SIZE(N)   (_W5500_IO_BASE_ + (0x001E << 8) + (WIZCHIP_SREG_BLOCK(N) << 3))
#define Sn_TXBUF_SIZE(N)   (_W5500_IO_BASE_ + (0x001F << 8) + (WIZCHIP_SREG_BLOCK(N) << 3))
#define Sn_TX_FSR(N)       (_W5500_IO_BASE_ + (0x0020 << 8) + (WIZCHIP_SREG_BLOCK(N) << 3))
#define Sn_TX_RD(N)        (_W5500_IO_BASE_ + (0x0022 << 8) + (WIZCHIP_SREG_BLOCK(N) << 3))
#define Sn_TX_WR(N)        (_W5500_IO_BASE_ + (0x0024 << 8) + (WIZCHIP_SREG_BLOCK(N) << 3))
#define Sn_RX_RSR(N)       (_W5500_IO_BASE_ + (0x0026 << 8) + (WIZCHIP_SREG_BLOCK(N) << 3))
#define Sn_RX_RD(N)        (_W5500_IO_BASE_ + (0x0028 << 8) + (WIZCHIP_SREG_BLOCK(N) << 3))
#define Sn_RX_WR(N)        (_W5500_IO_BASE_ + (0x002A << 8) + (WIZCHIP_SREG_BLOCK(N) << 3))
#define Sn_IMR(N)          (_W5500_IO_BASE_ + (0x002C << 8) + (WIZCHIP_SREG_BLOCK(N) << 3))


//----------------------------- W5500 Register values  -----------------------------
#define MR_RST                       0x80
#define Sn_MR_TCP                    0x01
#define Sn_MR_CLOSE                  0x00
#define SOCK_STREAM                  Sn_MR_TCP
#define Sn_CR_OPEN                   0x01
#define Sn_CR_LISTEN                 0x02
#define Sn_CR_CONNECT                0x04
#define Sn_CR_DISCON                 0x08
#define Sn_CR_CLOSE                  0x10
#define Sn_CR_SEND                   0x20
#define Sn_CR_SEND_MAC               0x21
#define Sn_CR_SEND_KEEP              0x22
#define Sn_CR_RECV                   0x40
#define Sn_IR_SENDOK                 0x10
#define Sn_IR_TIMEOUT                0x08
#define Sn_IR_RECV                   0x04
#define Sn_IR_DISCON                 0x02
#define Sn_IR_CON                    0x01

//----------------------------- W5500 Socket Statuses  -----------------------------
#define SOCK_CLOSED                  0x00
#define SOCK_INIT                    0x13
#define SOCK_LISTEN                  0x14
#define SOCK_SYNSENT                 0x15
#define SOCK_SYNRECV                 0x16
#define SOCK_ESTABLISHED             0x17
#define SOCK_FIN_WAIT                0x18
#define SOCK_CLOSING                 0x1A
#define SOCK_TIME_WAIT               0x1B
#define SOCK_CLOSE_WAIT              0x1C
#define SOCK_LAST_ACK                0x1D


////////////////////////
// Basic I/O Function //
////////////////////////
uint8_t  WIZCHIP_READ (uint32_t AddrSel);
void     WIZCHIP_WRITE(uint32_t AddrSel, uint8_t wb );
void     WIZCHIP_READ_BUF(uint32_t AddrSel, uint8_t* pBuf, uint16_t len);
void     WIZCHIP_WRITE_BUF(uint32_t AddrSel, uint8_t* pBuf, uint16_t len);


/////////////////////////////////
// Common Register I/O function //
/////////////////////////////////
#define setMR(mr) \
        WIZCHIP_WRITE(MR,mr)

#define getMR() \
        WIZCHIP_READ(MR)

#define setGAR(gar) \
        WIZCHIP_WRITE_BUF(GAR,gar,4)

#define getGAR(gar) \
        WIZCHIP_READ_BUF(GAR,gar,4)

#define setSUBR(subr) \
        WIZCHIP_WRITE_BUF(SUBR, subr,4)

#define getSUBR(subr) \
        WIZCHIP_READ_BUF(SUBR, subr, 4)

#define setSHAR(shar) \
        WIZCHIP_WRITE_BUF(SHAR, shar, 6)

#define getSHAR(shar) \
        WIZCHIP_READ_BUF(SHAR, shar, 6)

#define setSIPR(sipr) \
        WIZCHIP_WRITE_BUF(SIPR, sipr, 4)

#define getSIPR(sipr) \
        WIZCHIP_READ_BUF(SIPR, sipr, 4)

#define setINTLEVEL(intlevel)  {\
        WIZCHIP_WRITE(INTLEVEL,   (uint8_t)(intlevel >> 8)); \
        WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(INTLEVEL,1), (uint8_t) intlevel); \
        }

#define getINTLEVEL() \
        (((uint16_t)WIZCHIP_READ(INTLEVEL) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(INTLEVEL,1)))

#define setIR(ir) \
        WIZCHIP_WRITE(IR, (ir & 0xF0))

#define getIR() \
        (WIZCHIP_READ(IR) & 0xF0)

#define setIMR(imr) \
        WIZCHIP_WRITE(_IMR_, imr)

#define getIMR() \
        WIZCHIP_READ(_IMR_)

#define setSIR(sir) \
        WIZCHIP_WRITE(SIR, sir)

#define getSIR() \
        WIZCHIP_READ(SIR)

#define setSIMR(simr) \
        WIZCHIP_WRITE(SIMR, simr)

#define getSIMR() \
        WIZCHIP_READ(SIMR)

#define setRTR(rtr)   {\
        WIZCHIP_WRITE(_RTR_,   (uint8_t)(rtr >> 8)); \
        WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_RTR_,1), (uint8_t) rtr); \
        }

#define getRTR() \
        (((uint16_t)WIZCHIP_READ(_RTR_) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_RTR_,1)))

#define setRCR(rcr) \
        WIZCHIP_WRITE(_RCR_, rcr)

#define getRCR() \
        WIZCHIP_READ(_RCR_)


///////////////////////////////////
// Socket N register I/O function //
///////////////////////////////////
#define setSn_MR(sn, mr) \
        WIZCHIP_WRITE(Sn_MR(sn),mr)

#define getSn_MR(sn) \
        WIZCHIP_READ(Sn_MR(sn))

#define setSn_CR(sn, cr) \
        WIZCHIP_WRITE(Sn_CR(sn), cr)

#define getSn_CR(sn) \
        WIZCHIP_READ(Sn_CR(sn))

#define setSn_IR(sn, ir) \
        WIZCHIP_WRITE(Sn_IR(sn), (ir & 0x1F))

#define getSn_IR(sn) \
        (WIZCHIP_READ(Sn_IR(sn)) & 0x1F)

#define setSn_IMR(sn, imr) \
        WIZCHIP_WRITE(Sn_IMR(sn), (imr & 0x1F))

#define getSn_IMR(sn) \
        (WIZCHIP_READ(Sn_IMR(sn)) & 0x1F)

#define getSn_SR(sn) \
        WIZCHIP_READ(Sn_SR(sn))

#define setSn_PORT(sn, port)  { \
        WIZCHIP_WRITE(Sn_PORT(sn),   (uint8_t)(port >> 8)); \
        WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(Sn_PORT(sn),1), (uint8_t) port); \
    }

#define getSn_PORT(sn) \
        (((uint16_t)WIZCHIP_READ(Sn_PORT(sn)) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_PORT(sn),1)))

#define setSn_RXBUF_SIZE(sn, rxbufsize) \
        WIZCHIP_WRITE(Sn_RXBUF_SIZE(sn),rxbufsize)

#define getSn_RXBUF_SIZE(sn) \
        WIZCHIP_READ(Sn_RXBUF_SIZE(sn))

#define setSn_TXBUF_SIZE(sn, txbufsize) \
        WIZCHIP_WRITE(Sn_TXBUF_SIZE(sn), txbufsize)

#define getSn_TXBUF_SIZE(sn) \
        WIZCHIP_READ(Sn_TXBUF_SIZE(sn))

uint16_t getSn_TX_FSR(uint8_t sn);

#define getSn_TX_RD(sn) \
        (((uint16_t)WIZCHIP_READ(Sn_TX_RD(sn)) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_TX_RD(sn),1)))

#define setSn_TX_WR(sn, txwr) { \
        WIZCHIP_WRITE(Sn_TX_WR(sn),   (uint8_t)(txwr>>8)); \
        WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(Sn_TX_WR(sn),1), (uint8_t) txwr); \
        }

#define getSn_TX_WR(sn) \
        (((uint16_t)WIZCHIP_READ(Sn_TX_WR(sn)) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_TX_WR(sn),1)))

uint16_t getSn_RX_RSR(uint8_t sn);

#define setSn_RX_RD(sn, rxrd) { \
        WIZCHIP_WRITE(Sn_RX_RD(sn),   (uint8_t)(rxrd>>8)); \
        WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(Sn_RX_RD(sn),1), (uint8_t) rxrd); \
    }

#define getSn_RX_RD(sn) \
        (((uint16_t)WIZCHIP_READ(Sn_RX_RD(sn)) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_RX_RD(sn),1)))


#define getSn_RX_WR(sn) \
        (((uint16_t)WIZCHIP_READ(Sn_RX_WR(sn)) << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_RX_WR(sn),1)))


/////////////////////////////////////
// Sn_TXBUF & Sn_RXBUF IO function //
/////////////////////////////////////
#define getSn_RxMAX(sn) \
        (((uint16_t)getSn_RXBUF_SIZE(sn)) << 10)

#define getSn_TxMAX(sn) \
        (((uint16_t)getSn_TXBUF_SIZE(sn)) << 10)

void wiz_send_data(uint8_t sn, uint8_t *wizdata, uint16_t len);

void wiz_recv_data(uint8_t sn, uint8_t *wizdata, uint16_t len);


//////////////////////////////////////////
// W5500 Startup and Shutdown Functions //
//////////////////////////////////////////
void waitUntilSocketClosed(uint8_t sn);
void socket(uint8_t sn, uint8_t protocol, uint16_t port, uint8_t flag);
void openSocketOnPort(uint8_t sn, uint8_t port);
void listen(uint8_t sn);
void startListening(uint8_t sn);
void disconnect(uint8_t sn);
void close(uint8_t sn);
void clearBuffer(uint8_t* array, uint8_t length);


#endif
