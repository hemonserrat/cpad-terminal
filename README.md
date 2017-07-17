# CPAD - Lightweight Terminal for small payments
(C) 2003 Hernan Monserrat, GNU General Public License version 3

The name CPAD comes from key-pad, "C" means card-pad.  
A generic terminal with 2 Smart Card sockets,  magstripe reader,  2x16 char LCD,
internal Serial Data Flash for storage, RTC, integrated Keypad and GPIO's for external control.

An small low cost device for:
- Loyalty apps based on smart cards
- Micro-payments with smart cards: machine rental, close-finance cash in advance, etc
- NGV pumps payments
- Physical Access Control 
- and more

![CPAD](/doc/CPAD-up.png?raw=true)

![CPAD](/doc/CPAD-back.png?raw=true)


The sample code included here targets a demo app for a NGV Pump dispenser w/ smart cards.
All the drivers for the device are included fully.

## Electrical diagram:

[CPAD Schematics](/schematics/001-0430.pdf)


## Pre-requisites
* Compiler:  KEIL uVision 2 for 8051 family
* Target uC:  SST89C58


## Drivers

* 1-Wire:
* 3-Wire: For handling Logical Cards
* AT45xxx:  Serial Flash 
* Gost: Block cipher - encrypt/decrypt
* LCD: 2x16 LCD driver
* RTC:  DS1302 Real time clock
* Keypad: Matrix keypad driver
* SLE4428:  Logical card driver
* SOUND: Piezo driver
* SPI: SPI bus driver
* JBUS: Serial communication driver

Happy hacking!  

