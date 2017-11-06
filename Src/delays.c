/**
 * DELAYS.c
 * Delays functions 
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
#include <reg51.h>  
#include <defs51.h>
#include <delays.h>

/****************************** bwait **************************
PURPOSE: wait n fractions of timer ticks blocking.
NOTES:
  11.059 Mhz clock  ( cycls/clck ) = instr/s. 
  96 cycl. = 0.00868071 ms                    
 	        	dsw51     calc	 param-call   
  n=1   	             8.6us	    |         
  n=10        71us	            |           
  n=50       332us	           +- 6us       
  n=100      664us	            |           
  n=255     1667us	            |           
REVISIONS:   
=============================================================================
PARAMETERS:	 n - fraction
-----------------------------------------------------------------------------
RETURNS:	  none
-----------------------------------------------------------------------------
GLOBALS:      none
******************************************************************************/
void bwait( byte n )
{
 byte x;
  x=n;
   while( x-- ){ WDTHIT(); }
}

/****************************** wwait **************************
PURPOSE: wait n fractions of timer ticks blocking.
NOTES:
  11.059 Mhz clock  ( cycls/clck ) = instr/s. 
 120 cycl. = 0.01085089067728 ms             
 	        	dsw51     calc	 param-call   
  n=1   	     20us     10.8us  	 28us     
  n=10        108us	 108.5us	115us       
  n=50        499us	 542.5us	505us       
  n=100      	987us	1085  us	994us   
  n=255      2502us	2766.9us   2508us       
  n=0xFFFF		     711.1ms                
REVISIONS:   
=============================================================================
PARAMETERS:	 n - fraction
-----------------------------------------------------------------------------
RETURNS:	 none
-----------------------------------------------------------------------------
GLOBALS:     none
******************************************************************************/
void wwait( word n )
{
   while( n-- ){ WDTHIT(); };
}

/****************************** delay **************************
PURPOSE: wait msecs miliseconds
NOTES:
REVISIONS:   
=============================================================================
PARAMETERS:	 msecs - number of miliseconds delay.
-----------------------------------------------------------------------------
RETURNS:	 none
-----------------------------------------------------------------------------
GLOBALS:     none
******************************************************************************/
void delay( byte msecs )
{
   while( msecs-- )
   {
	 bwait(0x98); /* 1 ms */
   }
}

/****************************** sleep **************************
PURPOSE: wait secs seconds delay
NOTES:
REVISIONS:   
=============================================================================
PARAMETERS:	  secs - seconds to delay
-----------------------------------------------------------------------------
RETURNS:	 none
-----------------------------------------------------------------------------
GLOBALS:     none
******************************************************************************/
void sleep( byte secs )
{
   while( secs-- )
   {
	  wwait(0xB400); /* 500 ms */
	  wwait(0xEA60); /* 651 ms */
   }
}

/* ********************************************************************* */
