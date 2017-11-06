/**
 * 3wire.c
 * 3-wire SC Protocol  
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
/*****************************************************************
 todo: .complete write with protect bit routines and verifications.
            - Write and erase with protect bit
			- Write protect bit with data comparison
       .Multiple IFD handle.
*******************************************************************/
#include <stdio.h>
#include <atmel\at89x52.h>
#include <defs51.h>
#include <delays.h>
#include <3wire.h>


#if (__C51__ < 610 ) 
 #error This program requires KEIL C 6.10 or higher
#endif


#ifndef CPAD
 // DESA51?
	#define BUSY  P1_1  // IO
	#define PE    P1_3  // Card Detect
	#define CLK   P1_2  // Synchronous CLOCK
	#define DIO   P1_1  // IO
	#define RST   P1_0  // Reset
#else  // CPAD board
	#define BUSY  P1_3  // IO
	#define PE    P2_5  // Card Detect
	#define CLK   P3_7  // Synchronous CLOCK
	#define DIO   P1_3  // IO
	#define RST   P2_4  // Reset
#endif



static void SendCommands(byte *cmd);
static void ProcessCommands(word nprocesos);

/****************************** CHECK_SIZE **************************
PURPOSE:  Return card size depending on H2 ATR
NOTES:  ISO7816.
=============================================================================
PARAMETERS:	 h2 - ATR H2
-----------------------------------------------------------------------------
RETURNS:   word - size in bytes
******************************************************************************/
word CHECK_SIZE(byte h2)
{
 byte idata n;
 word idata t;
  h2=h2&0x78;
  h2=h2>>3;
  if( !h2 ) return 0;
  t=128;
  for(n=1; n<h2; n++)  t=t*2;
return t;
} // CHECK_SIZE


/****************************** GetReaderState **************************
PURPOSE:  Sense card presence
NOTES:  Hardware specific
=============================================================================
PARAMETERS:	 none
-----------------------------------------------------------------------------
RETURNS:   card presence test flag
******************************************************************************/
byte GetReaderState(void)
{
    if(!PE) return 0x20;
return 0x00;
}// GetReaderState


/****************************** ProcessCommands **************************
PURPOSE: Generate clock pulses to match erase and write times.
NOTES:  ISO7816
=============================================================================
PARAMETERS:	 nprocesos - Number of clock cycles
-----------------------------------------------------------------------------
RETURNS:  none
******************************************************************************/
static void ProcessCommands(word nprocesos)
{
word Index;
 DELAY;
 CLK=0; 
 DIO=1; 
 DELAY;
 for(Index=0; Index<nprocesos; Index++)
 {
    WDTHIT();
	CLK=1; 
	DELAY;
	CLK=0; 
	DELAY;
	if(!DIO) break;
 }
} //ProcessCommands

/****************************** startcond **************************
PURPOSE: Generate Start condition (Reset)
NOTES:  ISO7816
=============================================================================
PARAMETERS:	 none
-----------------------------------------------------------------------------
RETURNS:  none
******************************************************************************/
static void startcond(void)
{
	CLK=0; 
	RST=0; 
	DELAY;
	RST=1; 
	DELAY;
	CLK=1; 
	DELAY;
	CLK=0; 
	DELAY;
	RST=0; 
	DIO=1; 
    DELAY;
} // startcond


/****************************** ReadATR **************************
PURPOSE: Answer to Reset
NOTES:  ISO7816
=============================================================================
PARAMETERS:	 atr - 3 byte buffer to store ATR
-----------------------------------------------------------------------------
RETURNS:  none
******************************************************************************/
void ReadATR(byte *atr)
{
byte Contador1;
byte Contador2;
byte idata atr_byte=0;

	startcond();

	for(Contador1 = 0; Contador1<4; Contador1++)
	{
	  atr_byte=0;
	  for(Contador2=0; Contador2<8; Contador2++ )
	  {
        WDTHIT();
        atr_byte=atr_byte>>1;
		CLK=1; 
		DELAY;
         if( BUSY )
             atr_byte |= 0x80;
         else
             atr_byte &= 0x7F;                
		CLK=0; 
        DELAY;
	   }
	   *atr++=atr_byte;
	}
}// ReadATR



/****************************** SendCommands **************************
PURPOSE: Entry Command to the card
NOTES:  3-wire
=============================================================================
PARAMETERS:	 cmd - 3 byte command
-----------------------------------------------------------------------------
RETURNS:  none
******************************************************************************/
static void SendCommands(byte *cmd)
{
byte Contador2;
byte Contador;
byte idata out_byte;

	DIO=1; 
	DELAY;      
	RST=1; 
	DELAY;      
	CLK=0; 

	for(Contador = 0; Contador<3; Contador++)
	{
		out_byte=*(cmd+Contador);
	   for( Contador2=0; Contador2<8; Contador2++ )
	   {
        WDTHIT();
		DIO=(out_byte&0x01); 
		DELAY;     
		CLK=1;
		DELAY;   
		CLK=0;  
		out_byte=out_byte>>1;
	   }
	}	 
	 DELAY;       
	 RST=0; 
}// SendCommands

/****************************** ReadMemory **************************
PURPOSE: Continuos Main memory read without protect bits
NOTES:  3-wire
=============================================================================
PARAMETERS:	 addr - start address
             buff - buffer to store read 
			 length - number of bytes to read
-----------------------------------------------------------------------------
RETURNS:  none
******************************************************************************/
void ReadMemory(word addr, byte *buff, word length)
{
 word Contador1;
 word Contador2;
 byte in_byte=0;
 byte idata cmd[3];
	cmd[0]=((addr&0x0300)>>2)|0x0E; // Read 8 bits data without protect bit
	cmd[1]=(byte)(addr&0x00FF);
	cmd[2]=0x00;
	startcond();
	SendCommands(cmd);
	DELAY;
	CLK=1;
	for(Contador1 = 0; Contador1< length; Contador1++) //Tiene que ser desde 1
	{
	 in_byte=0;
	 DIO=1;
	  for(Contador2=0; Contador2<8; Contador2++ )
	  {
       WDTHIT();
       in_byte=in_byte>>1;
	   CLK=0;   
       DELAY;  
	   CLK=1; 
	   DELAY;
         if( BUSY )
             in_byte |= 0x80;
         else
             in_byte &= 0x7F;                
	   }
	   *buff++=in_byte;
	}
	DELAY;
	CLK=0;
} // ReadMemory

/****************************** ReadMemoryPr **************************
PURPOSE: Continuos Main memory read with protect bits
NOTES:  3-wire
=============================================================================
PARAMETERS:	 addr - start address
             buff - buffer to store read 
			 length - number of bytes to read
			 pr - buffer to store protect bit's data
-----------------------------------------------------------------------------
RETURNS:  none
******************************************************************************/
void ReadMemoryPr(word addr, byte *buff, word length, byte *pr)
{
 word Contador1;
 word Contador2;
 byte in_byte=0;
 byte idata cmd[3];

	cmd[0]=((addr&0x0300)>>2)|0x0C; // Read 8 bits data with protect bit
	cmd[1]=(byte)(addr&0x00FF);
	cmd[2]=0x00;
	startcond();
	SendCommands(cmd);
	DELAY;
	CLK=1;
	for(Contador1 = 0; Contador1< length; Contador1++) //Tiene que ser desde 1
	{
	 in_byte=0;
	 DIO=1;
	  for(Contador2=0; Contador2<8; Contador2++ )
	  {
       WDTHIT();
       in_byte=in_byte>>1;
	   CLK=0;   
       DELAY;  
	   CLK=1; 
	   DELAY;
         if( BUSY )
             in_byte |= 0x80;
         else
             in_byte &= 0x7F;                
	   }
	   *buff++=in_byte;
       /* read protect bit */
	   in_byte=0;
	   CLK=0;   
       DELAY;  
	   CLK=1; 
	   DELAY;
         if( BUSY )
             in_byte |= 0x80;
         else
             in_byte &= 0x7F;                
       *pr++=in_byte;
	}
	DELAY;
	CLK=0; 
}// ReadMemoryPr


/****************************** Verify **************************
PURPOSE: User validation procedure
NOTES:  3-wire
=============================================================================
PARAMETERS:	 psc - 2 byte PSC
-----------------------------------------------------------------------------
RETURNS:  SC_OK - succesfull
          SC_BLOCKED - card attempts expires
		  value - number of attempts left (1 XXX XXXX )
******************************************************************************/
byte Verify(byte *psc)
{
byte Index;
byte err;
byte idata cmd[3];	 


	// read error counter 
	ReadMemory(1021, cmd, 1);
	err = cmd[0];
	if(err==0)
	    return SC_BLOCKED;
	
	cmd[0]=0xF2;  /* Write Error counter */
	cmd[1]=0xFD;  
	err=err<<1;
	err&=0x7E;
    cmd[2]=err;
	startcond();
	SendCommands(cmd);
	ProcessCommands(103);

	cmd[0]=0xCD;  /* Compare PSC mem */
	for(Index = 0; Index<2; Index++)
	{
		 cmd[1] = Index+0xFE;  
		 cmd[2] = *(psc+Index);
	    startcond();
		SendCommands(cmd);
	    ProcessCommands(103);
	}

	cmd[0]=0xF3; // Erase Error Counter 
	cmd[1]=0xFD;  
	cmd[2]=0x7F;  
	startcond();
	SendCommands(cmd);
	ProcessCommands(103);
	// read error counter 
	ReadMemory(1021, cmd, 1);
	if(cmd[0]==err)
	{
      return (err|SC_ER_MASK);  
	}    
return SC_OK;
}// Verify


/****************************** WriteByte **************************
PURPOSE: Write and erase data without protect bit
NOTES:  3-wire
=============================================================================
PARAMETERS:	 addr - Address
             src - data to store             
-----------------------------------------------------------------------------
RETURNS:   none
******************************************************************************/
void WriteByte(word addr, byte src)
{
byte idata cmd[3];

    cmd[0]=((addr&0x0300)>>2)|0x33; // Write and erase without protect bit
	cmd[1]=(byte)(addr&0x00FF);
	cmd[2]=src;
	startcond();
	SendCommands(cmd);
	ProcessCommands(203);
} // WriteByte

/****************************** WriteMemory **************************
PURPOSE: Write and erase data without protect bit
NOTES:  3-wire
=============================================================================
PARAMETERS:	 addr - Address
             buff - data to store             
			 len - number of bytes to store
-----------------------------------------------------------------------------
RETURNS:   none
******************************************************************************/
void WriteMemory( word addr, byte *buff, word len)
{
  word Index;
   for( Index=0; Index<len; Index++ )   
     WriteByte(addr+Index, *(buff+Index)); 	   
} // WriteMemory


/****************************** WritePassword **************************
PURPOSE: Change PSC procedure
NOTES:  3-wire
=============================================================================
PARAMETERS:	 newpsc - Set 2 byte new PSC
-----------------------------------------------------------------------------
RETURNS:   none
******************************************************************************/
void WritePassword(byte *newpsc)
{
	  WriteByte(1022, *newpsc);
	  WriteByte(1023, *(newpsc+1));
} // WritePassword
/********************************************************************[ENDL]**/

