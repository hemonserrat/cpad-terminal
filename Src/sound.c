/**
 * SOUND.c
 * Sound functions for speaker control
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
#include <delays.h>
#include <sound.h>

#define SOUND_PORT P1_6


// TABNOTAS CONTIENE LOS VALORES CON LOS QUE HAY QUE RECARGAR EL TIMER
// PARA GENERAR LA FRECUENCIA ADECUADA DE LAS NOTAS MUSICALES
const word code TABNOTAS[]={
 		    -14090, // DO0	32.7 HZ
			-13302, // DOS0
			-12555, // RE0
			-11849, // RES0
			-11184, // MI0
			-10555, // FA0
			-9963, // FAS0
			-9404, // SOL0
			-8878, // SOLS0
			-8378, // LA0
			-7909, // LAS0
			-7464, // SI0

			-7045, // DO1
			-6651, // DOS1
			-6277, // RE1
			-5924, // RES1
			-5582, // MI1
			-5278, // FA1
			-4982, // FAS1
			-4702, // SOL1
			-4189, // LA1
			-3954, // LAS1
		    -3732, // SI1

			-3523, // DO2
			-3325, // DOS2
			-3139, // RE2
			-2962, // RES2
			-2796, // MI2
			-2639, // FA2
			-2491, // FAS2
			-2351, // SOL2
			-2219, // SOLS2
			-2095, // LA2
			-1977, // LAS2
		    -1866, // SI2

			-1761, // DO3
			-1663, // DOS3
			-1569, // RE3
			-1481, // RES3
			-1398, // MI3
			-1319, // FA3
			-1245, // FAS3
			-1176, // SOL3
			-1110, // SOLS3
			-1047, // LA3
			-989, // LAS3
			-933, // SI3

			-881, // DO4
			-831, // DOS4
			-785, // RE4
			-741, // RES4
			-699, // MI4
			-660, // FA4
			-622, // FAS4
			-588, // SOL4
			-555, // SOLS4
			-524, // LA4
			-494, // LAS4
			-467, // SI4

			-440, // DO5
			-416, // DOS5
			-392, // RE5
			-370, // RES5
			-349, // MI5
			-330, // FA5
			-311, // FAS5
			-294, // SOL5
			-277, // SOLS5
			-262, // LA5
			-247, // LAS5
			-233, // SI5

			-220, // DO6		2093 HZ
			-208, // DOS6		2217 HZ
			-196, // RE6		2349 HZ
			-185, // RES6		2489 HZ
			-175, // MI6		2637 HZ
			-165, // FA6		2794 HZ
			-156, // FAS6		2960 HZ
			-147, // SOL6		3136 HZ
			-139, // SOLS6	3322 HZ
			-131, // LA6		3520 HZ
			-124, // LAS6		3729 HZ
			-117, // SI6		3951 HZ
};

const byte code BEEP[]={
    si3	,corc,final
};

const byte code OK[]={
	mi5	,corc	,res5	,corc, final
};

const byte code ERROR[]={
  do5,corc,la4	,negr	,final
};

#ifdef _ELIZA_
//   Tabla de notas de "Para Elisa", de Beethoven.
//   Compas de 3 x 4.
const byte code INTRO[]={
	mi5	,corc	,res5	,corc,
	mi5	,corc	,res5	,corc	,mi5	,corc,
    si4	,corc	,re5	,corc	,do5	,corc,
	la4	,negr	,final
};

const byte code ELIZA[]={
	mi5	,corc	,res5	,corc,
	mi5	,corc	,res5	,corc	,mi5	,corc,
    si4	,corc	,re5	,corc	,do5	,corc,
	la4	,negr	,silen	,corc	,do4	,corc,
	mi4	,corc	,la4	,corc,
	si4	,negr	,silen	,corc	,mi4	,corc,
	sols4	 ,corc	  ,si4	 ,corc,
	do5	,negr	,silen	,corc	,mi4	,corc,
	mi5	,corc	,res5	,corc,
	mi5	,corc	,res5	,corc	,mi5	,corc,
	si4	,corc	,re5	,corc	,do5	,corc,
	la4	,negr	,silen	,corc	,do4	,corc,
	mi4	,corc	,la4	,corc,
	si4	,negr	,silen	,corc	,mi4	,corc,
	do5	,corc	,si4	,corc,
	la4	,negr	,silen	,corc	,si4	,corc,
	do5	,corc	,re5	,corc,
	mi5,negr2	,sol4	,corc	,fa5	,corc,
	mi5	,corc,
	re5	,negr2	,fa4	,corc	,mi5	,corc,
	re5	,corc,
	do5	,negr2	,mi4	,corc	,re5	,corc,
	do5	,corc,
	si4	,negr	,silen	,corc	,mi4	,corc,
    mi5	,corc	,res5	,corc,
	mi5	,corc	,res5	,corc	,mi5	,corc,
	si4	,corc	,re5	,corc	,do5	,corc,
	la4	,negr	,silen	,corc	,do4	,corc,
	mi4	,corc	,la4	,corc,
	si4	,negr	,silen	,corc	,mi4	,corc,
	sols4	,corc	,si4	,corc,
	do5	,negr	,silen	,corc	,mi4	,corc,
	mi5	,corc	,res5	,corc,
	mi5	,corc	,res5	,corc	,mi5	,corc,
    si4	,corc	,re5	,corc	,do5	,corc,
	la4	,negr	,silen	,corc	,do4	,corc,
	mi4	,corc	,la4	,corc,
	si4	,negr	,silen	,corc	,mi4	,corc,
	do5	,corc	,si4	,corc,
	la4	,negr	,silen	,corc	,do5	,corc,
	do5	,corc	,do5	,corc,
	do5	,blan	,fa5	,negr2,mi5	,scor,
	mi5	,negr	,re5	,negr	,las5	,corc2,
	la5	,scor,
	la5	,corc	,sol5	,corc	,fa5	,corc,
	mi5	,corc	,re5    ,corc    ,do5      ,corc,
	las4	,negr	,la4	,negr	,la4	,scor,
	sol4	 ,scor	 ,la4       ,scor	 ,si4	  ,scor,
	do5	,blan	,re5	,corc	,res5	,corc,
	mi5	,negr2	,mi5	,corc	,fa5	,corc,
	la4	,corc,
	do5	,blan	,re5	,corc2,si4	,scor,
	do5	,negr	,silen	,corc	,mi4	,corc,
 	mi5	,corc	,res5	,corc,
	mi5	,corc	,res5	,corc	,mi5	,corc,
	si4	,corc,	re5	,corc	,do5	,corc,
	la4	,negr	,silen	,corc	,do4	,corc,
	mi4	,corc	,la4	,corc,
	si4	,negr	,silen	,corc	,mi4	,corc,
	do5	,corc	,si4	,corc,
	la4	,blan,
	final
};
#endif

byte idata _snd_h1;
byte idata _snd_h2;
const byte code * idata _snd;

#ifdef _USETIMER
    bit _snd_run;
	word interruptcnt;
	word hdw_secs;
#endif

void _timer0(void) interrupt 1 using 1
{
   
#ifdef _USETIMER
  if( _snd_run )
  {
#endif
	TR0=0;    // DETIENE AL TIMER1
	TH0=_snd_h1;   // RECARGA EL TIMER CON EL
	TL0=_snd_h2;   // SEMIPERIODO DE LAS NOTAS
	TR0=1;    // HACE CORRER AL TIMER
	SOUND_PORT=!SOUND_PORT;
#ifdef _USETIMER
  }  else {
    if( ++interruptcnt == TIMER0L )  
    {                              
      hdw_secs++;                    
      if(hdw_secs>60) hdw_secs=0;         
      interruptcnt=0;                
    }                                
  }
#endif
} /* _timer0 */



void SND_init(void)
{
 byte c;
  c = TMOD;
  c&=0xF0;
  c |= 0x01;
  TMOD = c;
  TR0=0;
  TF0=0;
#ifdef _USETIMER
  _snd_run=0;
#endif
//   ET0=1;
  SOUND_PORT=1;
  ET0=0;
  _snd = &BEEP[2];
}

#ifdef _USETIMER
void TMR_init(void)
{
  c = TMOD;
  c&=0xF0;
  c |= 0x02;
  TMOD = c;
  TR0=1;
  TF0=0;
  hdw_secs=0;
  ET0=1;
}
#endif


void SND_set(const byte code *snd)
{
   ET0=1; // accept interrupts
   _snd = snd;
}

byte SND_task(void)
{
  byte n;
  if( *_snd!=final )
  {
       if( *_snd==silen )
	   {
	     TR0=0;
	   } else {
	     n = *_snd;
         _snd_h1 = HIBYTE(TABNOTAS[n]);
		 _snd_h2 = LOBYTE(TABNOTAS[n]);
		 TH0 = _snd_h1;
         TL0 = _snd_h2;
         TR0=1;
	   }
     _snd++;
	 n = *_snd;
	 while(--n) delay(10);
     TR0=0;
	 SOUND_PORT=1;
	 _snd++; // NEXT 
  } else { // END
//    SOUND_PORT=1;
    ET0=0;
    TR0=0;
	return 0;
  }
return 1;
}

void SND_play(const byte code *snd)
{
  SND_set(snd);
  SND_Wait();
}

#if 0
void SND_play(const byte code *snd)
{
  byte n;
  word c;
#ifdef _USETIMER
   _snd_run=1;
#endif
   ET0=1; // accept interrupts
   c=0;
   while(  snd[c]!=final ){
       if( snd[c]==silen )
	   {
	     TR0=0;
	   } else {
	     n = snd[c];
         _snd_h1 = HIBYTE(TABNOTAS[n]);
		 _snd_h2 = LOBYTE(TABNOTAS[n]);
		 TH0 = _snd_h1;
         TL0 = _snd_h2;
         TR0=1;
	   }
	 c++;
	 n = snd[c];
	 while(--n) delay(10);
     TR0=0;
	 SOUND_PORT=1;
	 c++;
   }
   SOUND_PORT=1;
   ET0=0;
#ifdef _USETIMER
   _snd_run=0;
#endif
}
#endif

/* ********************************************************************* */
