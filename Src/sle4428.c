/**
 * sle4428.c
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
 /*********************************** sle4428.c ******************************
 todo: .complete write with protect bit routines and verifications.
            - Write and erase with protect bit
			- Write protect bit with data comparison
       .Multiple IFD handle.
******************************************************************************/
#include <stdio.h>
#include <atmel\at89x52.h>
#include <defs51.h>
#include <delays.h>
#include <sle4428.h>


#if (__C51__ < 610 ) 
 #error This program requires KEIL C 6.10 or higher
#endif



#define BUSY  P1_1
#define PE    P1_3

#define CLK   P1_2
#define DIO   P1_1
#define RST   P1_0


void SendCommands(byte *cmd);
void ProcessCommands(word nprocesos);

word CHECK_SIZE(byte h2)
{
 byte n;
 word t;
  h2=h2&0x78;
  h2=h2>>3;
  if( !h2 ) return 0;
  t=128;
  for(n=1; n<h2; n++)  t=t*2;
return t;
}


byte GetReaderState(void)
{
byte cardon=0x00;
/* 	 Get Card State */
    if(!PE) cardon=0x20;
return cardon;
}


void ProcessCommands(word nprocesos)
{
word Index;
 DELAY;
 CLK=0; 
 DIO=1; 
 DELAY;
 for(Index=0; Index<nprocesos; Index++)
 {
	CLK=1; 
	DELAY;
	CLK=0; 
	DELAY;
	if(!DIO) break;
 }
}

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
}
void ReadATR(byte *atr)
{
word Contador1;
word Contador2;
byte atr_byte=0;

	startcond();

	for(Contador1 = 0; Contador1<4; Contador1++)
	{
	  atr_byte=0;
	  for(Contador2=0; Contador2<8; Contador2++ )
	  {
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
}



void SendCommands(byte *cmd)
{
byte Contador2;
byte Contador;
byte out_byte;

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
}

void ReadMemory(word addr, byte *buff, word length)
{
 word Contador1;
 word Contador2;
 byte in_byte=0;
 byte cmd[3];
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
}

void ReadMemoryPr(word addr, byte *buff, word length, byte *pr)
{
 word Contador1;
 word Contador2;
 byte in_byte=0;
 byte cmd[3];
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
}


byte Verify(byte *psc)
{
byte Index;
byte cmd[3];	 
byte err;


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
}


void WriteByte(word addr, byte src)
{
byte cmd[3];

    cmd[0]=((addr&0x0300)>>2)|0x33; // Write and erase without protect bit
	cmd[1]=(byte)(addr&0x00FF);
	cmd[2]=src;
	startcond();
	SendCommands(cmd);
	ProcessCommands(203);
}

void WriteMemory( word addr, byte *buff, word len)
{
  word Index;
   for( Index=0; Index<len; Index++ )   
     WriteByte(addr+Index, *(buff+Index)); 	   
}


void WritePassword(byte *newpsc)
{
	  WriteByte(1022, *newpsc);
	  WriteByte(1023, *(newpsc+1));
}


/********************************************************************[ENDL]**/

