/**
 * JBUS.c
 * JBUS Protocol implementation
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
#include <stdio.h>
#include <atmel\at89x52.h>
#include <defs51.h>
#include <jbus.h>


#define _SERIAL 1
// ****************************************************************
// Hardware MACROS
#define DE485()      P1_2=1
#define RE485()      P1_2=0

/*--------------------------**
**                          **
**  Local global variables  **
**--------------------------*/
// Serial Port ************************************************
byte        idata   gIO[30];        // COM buffer
byte        idata   gIOx;           // COM index
byte        idata   gDataLen;       // To read
byte        idata   gCrc;           // RX Cooked crc
byte        idata   gComEvent;      // COM FSM
byte        idata   gComID;         // Device ID

#ifdef _SERIAL
/****************************** serial0 **************************
PURPOSE:  Serial port 0 Interrupt rutine
==================================================================
PARAMETERS:	  none
------------------------------------------------------------------
RETURNS:	 none
******************************************************************/
void serial0(void) interrupt 4 using 1
{
 ES=0;  // no more ints
 if(TI) { // TX DONE NEXT
      TI=0;
	if( gComEvent==JB_TX )
	         SBUF = gIO[gIOx++];
	if( gIOx >= gDataLen )
	{
	  gComEvent=JB_IDLE;
	  ERX();
	}	 
 } else {  // RX
	RI=0;
	gIO[gIOx] = SBUF;
	  		switch(gComEvent)
			{ 
			  case JB_IDLE:
			        if( gIO[gIOx]==JB_STX )
					{
	                   gIOx=0;
			           gCrc=JB_STX;
			           gComEvent = JB_INIT;
					}
			  break;
              case JB_INIT: // ID
			        if( (gIO[gIOx]==gComID) || (gIO[gIOx]==0xFF) ) // is for me?
					{
					  gCrc^=gComID;
					  gComEvent = JB_LEN;
					} else gComEvent = JB_IDLE; // ignore
			  break;
			  case JB_LEN:
			        if( gIO[gIOx] > JB_MAXDL )
					{
                       gComEvent = JB_IDLE; // ignore
                    } else {
					   gComEvent = JB_DATA;
					   gDataLen = gIO[gIOx];
					   gCrc^=gDataLen;
					}
			  break;
			  case JB_DATA:                
			        gCrc^=gIO[gIOx];
			        gIOx++;
					gDataLen--;
					if( gDataLen==0 )
					  gComEvent = JB_ETX;
			  break;
			  case JB_ETX:
			        if( gIO[gIOx]==JB_ETX )
					{
					    gComEvent = JB_CRC;
						gCrc^=JB_ETX;
					} else
					    gComEvent = JB_IDLE;
			  break;
			  case JB_CRC:
			        if( gIO[gIOx] == gCrc ) // match?
					{ 
					    gComEvent = JB_PROCESS;
                        DRX();  // disable reception
					} else
					    gComEvent = JB_IDLE;
			  break;
			}	        
 }
 ES=1; // rerun
} // serial0
#endif


void JB_init(byte id)
{
  DE485(); // FULL DUPLEX - CPAD
  gComEvent = JB_IDLE;
  gIOx=0;
  gComID = id;
}

void JB_send(void)
{
 gIOx=0;
 gIO[0] = JB_STX;
 gIO[1] = gComID;
 gIO[2] = gDataLen;
 gIO[gDataLen+3] = JB_ETX;
 gIO[gDataLen+4] = JB_crc(gIO, gDataLen+4);
 gDataLen+=5;
 gComEvent = JB_TX;
 DRX();
 TI=1;
}



byte JB_crc(byte *d, byte size)
{
 byte crc;
  crc=0x00;
  do {
      crc^=*d;
    d++;
  }while( --size );
 return crc;   
}// JB_crc

// ******************************************************[ENDL]************* 
