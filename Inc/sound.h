/**
 * SOUND.H
 * Interface definitions 
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
#ifndef _SOUND_H_
#define _SOUND_H_ 1


//     NOTE
#define redo    128
#define blan	64  		
#define negr	32  		
#define corc	16  		
#define scor	8  		
#define fusa	4  		
#define sfus	2  		

#define redo2	192		// REDO2 ES REDONDA CON PUNTILLO Y DURA  		
#define blan2	96		// 1,5 VECES MAS QUE REDO		
#define negr2	48	  		
#define corc2	24		
#define scor2	12 		
#define fusa2	6	 		
#define sfus2	3	

#define do0		0		// DO DE LA ESCALA MAS BAJA
#define dos0	1		// DO SOSTENIDO DE LA ESCALA MAS BAJA
#define re0	    2
#define res0	3
#define mi0		4
#define fa0		5
#define fas0	6	
#define sol0	7
#define sols0	8
#define la0		9
#define las0	10
#define si0		11
	  		
#define do1		12
#define dos1	13
#define re1		14
#define res1	15
#define mi1		16
#define fa1		17
#define fas1	18
#define sol1	19
#define sols1	20
#define la1		21
#define las1    22
#define si1		23
	  		
#define do2		24
#define dos2	25
#define re2		26
#define res2	27
#define mi2		28
#define fa2		29
#define fas2	30	
#define sol2	31
#define sols2	32
#define la2		33
#define las2	34
#define si2		35
		
#define do3		36
#define dos3	37
#define re3		38
#define res3	39
#define mi3		40
#define fa3		41
#define fas3	42
#define sol3	43
#define sols3   44
#define la3		45
#define las3	46
#define si3		47
		
#define do4		48
#define dos4	49
#define re4		50
#define res4	51
#define mi4		52
#define fa4		53
#define fas4	54
#define sol4	55
#define sols4	56
#define la4		57
#define las4	58
#define si4		59
		
#define do5  	60
#define dos5	61
#define re5		62
#define res5	63
#define mi5		64
#define fa5		65
#define fas5	66
#define sol5	67
#define sols5	68
#define la5		69
#define las5	70
#define si5		71
			
#define do6		72
#define dos6	73
#define re6		74
#define res6	75
#define mi6		76
#define fa6		77
#define fas6	78
#define sol6	79
#define sols	80
#define la6		81
#define las6	82
#define si6		83
		
#define do7		84	
#define dos7	85
#define re7		86
#define res7	87
#define mi7		88
#define fa7		89
#define fas7	90
#define sol		91
#define sols7	92
#define la7		93
#define las7	94
#define si7		95

#define silen	254
#define final	255

extern byte idata _snd_h1;
extern byte idata _snd_h2;
extern const byte code * idata _snd;

/*------------------**
**  Timer variables **
**------------------*/
#ifdef _USETIMER
	extern bit  _snd_run;
	extern word interruptcnt;
	extern word hdw_secs;
	#define TIMER0L    4608  /* for 11.059 Mhz aprox. 1s */
	#define HWTime()   hdw_secs
#endif

extern void SND_init(void);
extern void SND_play(const byte code *snd);
extern byte SND_task(void);
extern void SND_set(const byte code *snd);

#define SND_Wait() while(SND_task());

#define _ELIZA_ 1
extern const byte code ELIZA[];
extern const byte code INTRO[];
extern const byte code BEEP[];
extern const byte code OK[];
extern const byte code ERROR[];

#endif
/* ***************************************************************[ENDL]**** */



