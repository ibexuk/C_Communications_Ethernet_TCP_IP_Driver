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
//NETBIOS (NETWORK BASIC INPUT/OUTPUT SYSTEM) C CODE HEADER FILE




//##################################
//##################################
//########## USING DRIVER ##########
//##################################
//##################################

//################################
//##### SET OUR NETWORK NAME #####
//################################
//Netbios requests are automatically dealt with by the stack.  No user application intervention is required.
//All you need to do is set the name of your device which may be changed at any time.
/*
	//----- SET OUR NETBIOS NAME -----
	netbios_our_network_name[0] = 'e';		//16 byte null terminated array (put a 0x00 after the last character)
	netbios_our_network_name[1] = 'm';		//Not case sensitive
	netbios_our_network_name[2] = 'b';
	netbios_our_network_name[3] = 'e';
	netbios_our_network_name[4] = 'd';
	netbios_our_network_name[5] = 'd';
	netbios_our_network_name[6] = 'e';
	netbios_our_network_name[7] = 'd';
	netbios_our_network_name[8] = '-';
	netbios_our_network_name[9] = 'd';
	netbios_our_network_name[10] = 'e';
	netbios_our_network_name[11] = 'v';
	netbios_our_network_name[12] = 'i';
	netbios_our_network_name[13] = 'c';
	netbios_our_network_name[14] = 'e';
	netbios_our_network_name[15] = 0x00;
*/


//For further information please see the project technical manual






//********************************
//********************************
//********** DO DEFINES **********
//********************************
//********************************
#ifndef ETH_NETBIOS_C_INIT		//Do only once the first time this file is used
#define	ETH_NETBIOS_C_INIT


#include "eth-main.h"



typedef enum _NETBIOS_STATE
{
	SM_NETBIOS_OPEN_SOCKET,
	SM_NETBIOS_WAIT_FOR_RX,
	SM_NETBIOS_TX_RESPONSE
} NETBIOS_STATE;


#define	NETBIOS_NAMESERVICE_CLIENT_PORT		137
#define	NETBIOS_NAMESERVICE_SERVER_PORT		137






#endif







//*******************************
//*******************************
//********** FUNCTIONS **********
//*******************************
//*******************************
#ifdef ETH_NETBIOS_C
//-----------------------------------
//----- INTERNAL ONLY FUNCTIONS -----
//-----------------------------------
BYTE netbios_check_rx_packet (void);
void netbios_send_response (void);


//-----------------------------------------
//----- INTERNAL & EXTERNAL FUNCTIONS -----
//-----------------------------------------
//(Also defined below as extern)
void process_netbios_nameservice (void);


#else
//------------------------------
//----- EXTERNAL FUNCTIONS -----
//------------------------------
extern void init_netbios_nameservice (void);
extern void process_netbios_nameservice (void);


#endif




//****************************
//****************************
//********** MEMORY **********
//****************************
//****************************
#ifdef ETH_NETBIOS_C
//--------------------------------------------
//----- INTERNAL ONLY MEMORY DEFINITIONS -----
//--------------------------------------------
BYTE sm_netbios = SM_NETBIOS_OPEN_SOCKET;
WORD_VAL netbios_transaction_id;
BYTE netbios_name_length;
BYTE netbios_requested_name[34];


//--------------------------------------------------
//----- INTERNAL & EXTERNAL MEMORY DEFINITIONS -----
//--------------------------------------------------
//(Also defined below as extern)
BYTE netbios_our_network_name[16];


#else
//---------------------------------------
//----- EXTERNAL MEMORY DEFINITIONS -----
//---------------------------------------
extern BYTE netbios_our_network_name[16];


#endif





