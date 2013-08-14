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
//ICMP (INTERNET CONTROL MESSAGE PROTOCOL) C CODE HEADER FILE



//##################################
//##################################
//########## USING DRIVER ##########
//##################################
//##################################

//###########################################
//##### DEALING WITH ICMP ECHO REQUESTS #####
//###########################################
//ICMP ECHO Requests are automatically dealt with by the stack - no user application intervention is required



//For further information please see the project technical manual





//*****************************
//*****************************
//********** DEFINES **********
//*****************************
//*****************************
#ifndef ICMP_C_INIT		//(Do only once)
#define	ICMP_C_INIT


#include "eth-main.h"


#define ICMP_MAX_DATA_LENGTH	32


//ICMP HEADER CODES:-
#define ICMP_ECHO_REQUEST		8
#define	ICMP_ECHO_REPLY			0


//----- DATA TYPE DEFINITIONS -----
typedef struct _ICMP_HEADER
{
	BYTE		type;
	BYTE		code;
	WORD		checksum;
	WORD		identifier;
	WORD		sequence_number;
} ICMP_HEADER;
#define	ICMP_HEADER_LENGTH		8

#endif



//*******************************
//*******************************
//********** FUNCTIONS **********
//*******************************
//*******************************
#ifdef ICMP_C				//(Defined only by associated C file)
//-----------------------------------
//----- INTERNAL ONLY FUNCTIONS -----
//-----------------------------------


//-----------------------------------------
//----- INTERNAL & EXTERNAL FUNCTIONS -----
//-----------------------------------------
//(Also defined below as extern)
BYTE icmp_process_received_echo_request (WORD *icmp_id, WORD *icmp_sequence, BYTE *icmp_data_buffer, WORD data_remaining_bytes);
void icmp_send_packet(DEVICE_INFO *remote_device_info,BYTE icmp_packet_type, BYTE *data_buffer, WORD data_length, WORD *icmp_id, WORD *icmp_sequence);


#else
//------------------------------
//----- EXTERNAL FUNCTIONS -----
//------------------------------
extern BYTE icmp_process_received_echo_request (WORD *icmp_id, WORD *icmp_sequence, BYTE *icmp_data_buffer, WORD data_remaining_bytes);
extern void icmp_send_packet(DEVICE_INFO *remote_device_info,BYTE icmp_packet_type, BYTE *data_buffer, WORD data_length, WORD *icmp_id, WORD *icmp_sequence);


#endif




//****************************
//****************************
//********** MEMORY **********
//****************************
//****************************
#ifdef ICMP_C				//(Defined only by associated C file)
//--------------------------------------------
//----- INTERNAL ONLY MEMORY DEFINITIONS -----
//--------------------------------------------


//--------------------------------------------------
//----- INTERNAL & EXTERNAL MEMORY DEFINITIONS -----
//--------------------------------------------------
//(Also defined below as extern)


#else
//---------------------------------------
//----- EXTERNAL MEMORY DEFINITIONS -----
//---------------------------------------


#endif










