/**
 * KEYPAD.c
 * Key pad functions
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
#include <keypad.h>

#include <delays.h>
#include <string.h>
#include <LCD.h>

// CPAD board
	#define KC_MASK_I  0x0F // on low nibble
	#define KC_MASK_O  0xF0
	#define KF_MASK_I  0x0F
	#define KF_MASK_O  0xF0

// KEYPAD 4X4
	#define KC_PORT P2
	#define KF_PORT P0

//        FC
#define K_11      0xEE    // 1110 1110
#define K_12      0xED    // 1110 1101
#define K_13      0xEB    // 1110 1011
#define K_14      0xE7    // 1110 0111

#define K_21      0xDE    // 1101 1110
#define K_22      0xDD    // 1101 1101
#define K_23      0xDB    // 1101 1011
#define K_24      0xD7    // 1101 0111

#define K_31      0xBE    // 1011 1110
#define K_32      0xBD    // 1011 1101
#define K_33      0xBB    // 1011 1011
#define K_34      0xB7    // 1011 0111

#define K_41      0x7E    // 0111 1110
#define K_42      0x7D    // 0111 1101
#define K_43      0x7B    // 0111 1011
#define K_44      0x77    // 0111 0111


const byte code keymap[]={
    K_11,K_12,K_13,K_14,
    K_21,K_22,K_23,K_24,
    K_31,K_32,K_33,K_34,
    K_41,K_42,K_43,K_44,
	K_NOTDEF
};

#ifdef _KPINVERT
 const byte code keyicon[]={
    'A','0','E','S',
	'1','2','3','D',
	'4','5','6','P',
	'7','8','9','C'
 };
#else
 const byte code keyicon[]={
    '9','8','7','C',
	'6','5','4','P',
	'3','2','1','D',
	'E','0','A','S'
 };
#endif

byte KP_kbhit(void)
{
  byte f;
  byte xdata *p = (byte xdata *)0xF00F;
   f = *p & KF_MASK_I;
   if( f!=KF_MASK_I )
    return 1;
return 0;   
}

#pragma disable
byte KP_scan(void)
{
  byte f;
  byte c;
  byte x;
  byte xdata *p = (byte xdata *)0xF00F;
   f = *p;
   f &= KF_MASK_I;
   if( f!=KF_MASK_I ) // catch key press
   {
     x = 0xFE;
     while(x!=0xEF)
	 {	    
       p = (byte xdata *)(x<<8|0x000F);
	   c = *p & KF_MASK_I;	   
       if( c==f ){
	     f=f<<4;
		 c=x&KF_MASK_I;
		 f|=c;
		 break;
	   }
	   x=x<<1;
	   x|=0x01;
	 }
	 c = 0;
     while( keymap[c]!=K_NOTDEF )
	 {
         WDTHIT();
	     if( keymap[c]==f ){
		    f = keyicon[c];
            KP_init();
		   return f;
		 }		                      
	   c++;
	 }
   }
  return K_NOTDEF;
}


/* ********************************************************************* */

