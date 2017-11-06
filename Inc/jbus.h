/**
 * JBUS.H
 * JBUS Protocol defs.
 *
 * This file is part of CPAD Lightweght terminal for small payments
 *
 * Copyright (C) 2002,  Hernan Monserrat hemonserrat<at>gmail<dot>com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _JBUS_H
#define _JBUS_H 1

//  PACKET FORMAT:
//  [STX][ID:1][LEN:1][CMD:1][DATA:LEN-1][ETX][CRC:1]

#define JB_STX  0x02
#define JB_ETX  0x03

#define JB_MAXDL  30  // MFL DATA

// COMMANDS
#define JB_ERROR  0x01    // +GET STATUS
#define JB_FDWR   0x02    // WRITE FLASH DISK 
#define JB_FDRD   0x03    // READ FLASH DISK
#define JB_EREC   0x04    // ERASE RECORD-->NO
#define JB_EALL   0x05    // ERASE ALL DATABASE
#define JB_GREC   0x06    // GET RECORD
#define JB_GNUM   0x07    // +GET NUMBER OF RECORDS
#define JB_GFE    0x08    // GET FREE SPACE
#define JB_SREC   0x09    // SET RECORD
#define JB_STM    0x0A    // SET TIME
#define JB_GTM    0x0B    // GET TIME
#define JB_SRL    0x0C    // SET RELAY1|RELAY2|SPACE TIME
#define JB_VER    0x0D    // [VERH][VERL]

// STATES
#define JB_IDLE      0x01
#define JB_TX        0x02
#define JB_PROCESS   0x03
#define JB_INIT      0x04
#define JB_LEN       0x05
#define JB_DATA      0x06
#define JB_CRC       0x07


// Variables
extern byte idata   gIO[30];        // COM buffer
extern byte idata   gIOx;           // COM index
extern byte idata   gCrc;           // RX Cooked crc
extern byte idata   gComEvent;      // COM FSM
extern byte idata   gDataLen;       // To read

#define ERX()        REN=1
#define DRX()        REN=0

extern byte JB_crc(byte *d, byte size);
extern void JB_init(byte id);
extern void JB_send(void);
#define SEND(DATA, OFFSET)  gIO[3+OFFSET]=DATA
#define GET(OFFSET)  &gIO[3+OFFSET]
#define SETLEN(D) gDataLen=D


#endif
// **********************************************************[ENDL]************* 
