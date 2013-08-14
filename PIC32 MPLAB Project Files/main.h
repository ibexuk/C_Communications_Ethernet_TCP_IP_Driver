/*
IBEX UK LTD http://www.ibexuk.com
Electronic Product Design Specialists
RELEASED SOFTWARE

The MIT License (MIT)

Copyright (c) 2013, IBEX UK Ltd, http://ibexuk.com

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
//Project Name:		TCP/IP DRIVER
//PIC32 SAMPLE PROJECT GENERIC GLOBAL HEADER FILE





//******************************************
//******************************************
//********** PROJECT GLOBAL FLAGS **********
//******************************************
//******************************************





//*************************************
//*************************************
//********** GENERAL DEFINES **********
//*************************************
//*************************************

//INCLUDE THE DEVICE HEADER FILE
#include "plib.h"				//Includes p32 file and all the peripheral libraries

#ifndef Nop
#define Nop() asm("nop")		//Null instruction
#endif


//----------------------
//----- OSCILLATOR -----						//<<<<< CHECK FOR A NEW APPLICATION <<<<<
//----------------------
//8MHz Osc
//= 80MHz with 2x divide, 20x multiply, 1 x divide
//= 80MIPS (5 stage pipeline)
//= 12.5nS per instruction / pipeline stage (IO max toggling speed 80MHz)

#define	INSTRUCTION_CLOCK_FREQUENCY	80000000


//----------------------
//----- INTERRUPTS -----
//----------------------
#define	ENABLE_INT		INTEnableInterrupts();		//Enable all unmasked interupts
#define	DISABLE_INT		INTDisableInterrupts();		//Disable all unmasked interupts








//****************************************
//****************************************
//***** GLOBAL DATA TYPE DEFINITIONS *****
//****************************************
//****************************************
#ifndef GLOBAL_DATA_TYPE_INIT				//(Include this section only once for each source file)
#define	GLOBAL_DATA_TYPE_INIT

#define	CONSTANT	const 					//Define used for this as some compilers require an additional qualifier such as 'rom' to signify that a constant should be stored in program memory

/* Commented out to stop clsh with C32 compiler
#undef BOOL
#undef TRUE
#undef FALSE
#undef BYTE
#undef SIGNED_BYTE
#undef WORD
#undef SIGNED_WORD
#undef DWORD
#undef SIGNED_DWORD
#undef QWORD
#undef SIGNED_QWORD
*/

//BOOLEAN - 1 bit:
//typedef enum _BOOL { FALSE = 0, TRUE } BOOL;
//BYTE - 8 bit unsigned: Commented out to stop clsh with C32 compiler
//typedef unsigned char BYTE;
//SIGNED_BYTE - 8 bit signed:
typedef signed char SIGNED_BYTE;
//WORD - 16 bit unsigned:
//typedef unsigned short WORD;
//SIGNED_WORD - 16 bit signed:
typedef signed short SIGNED_WORD;
//DWORD - 32 bit unsigned:
typedef unsigned long DWORD;
//SIGNED_DWORD - 32 bit signed:
typedef signed long SIGNED_DWORD;
//QWORD - 64 bit unsigned:
typedef unsigned long long QWORD;
//SIGNED_QWORD - 64 bit signed:
typedef signed long long SIGNED_QWORD;

/* Commented out to stop clsh with C32 compiler
//BYTE BIT ACCESS:
typedef union _BYTE_VAL
{
    struct
    {
        unsigned char b0:1;
        unsigned char b1:1;
        unsigned char b2:1;
        unsigned char b3:1;
        unsigned char b4:1;
        unsigned char b5:1;
        unsigned char b6:1;
        unsigned char b7:1;
    } bits;
    BYTE Val;
} BYTE_VAL;


//WORD ACCESS
typedef union _WORD_VAL
{
    WORD Val;
    struct
    {
        BYTE LSB;
        BYTE MSB;
    } byte;
    BYTE v[2];
} WORD_VAL;
#define LSB(a)          ((a).v[0])
#define MSB(a)          ((a).v[1])

//DWORD ACCESS:
typedef union _DWORD_VAL
{
    DWORD Val;
    struct
    {
        BYTE LOLSB;
        BYTE LOMSB;
        BYTE HILSB;
        BYTE HIMSB;
    } byte;
    struct
    {
        WORD LSW;
        WORD MSW;
    } word;
    BYTE v[4];
} DWORD_VAL;
#define LOWER_LSB(a)    ((a).v[0])
#define LOWER_MSB(a)    ((a).v[1])
#define UPPER_LSB(a)    ((a).v[2])
#define UPPER_MSB(a)    ((a).v[3])

//EXAMPLE OF HOW TO USE THE DATA TYPES:-
//	WORD_VAL variable_name;				//Define the variable
//	variable_name = 0xffffffff;			//Writing 32 bit value
//	variable_name.LSW = 0xffff;			//Writing 16 bit value to the lower word 
//	variable_name.LOLSB = 0xff;			//Writing 8 bit value to the low word least significant byte
//	variable_name.v[0] = 0xff;			//Writing 8 bit value to byte 0 (least significant byte)
*/



#endif		//GLOBAL_DATA_TYPE_INIT








