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
//ARP (ADDRESS RESOLUTION PROTOCOL) C CODE HEADER FILE





//##################################
//##################################
//########## USING DRIVER ##########
//##################################
//##################################

//#################################
//##### INCOMING ARP REQUESTS #####
//#################################
//Are dealt with automatically by the stack - no application intervention is required

//####################################
//##### TO RESOLVE AN IP ADDRESS #####
//####################################
//Call the 'arp_resolve_ip_address' function with the IP address that needs resolving (that you want the mac address to be returning from)
//Then periodically call the 'arp_is_resolve_complete' function to determine when the ARP response has been received.
//The calling function must use a timeout in case of no response.
//If trying to resolve an IP address that is not on our subnet then the resolve function will automatically resolve the address for the gateway
//(our_gateway_ip_address) which will then do its job and forward on communications to the remote device.  Therefore for IP addresses outside of our
//subnet bear in mind that just because ARP was successful the remote IP address is not necessarily actually available.
//
//RESOLVING AN IP ADDRESS EXAMPLE:-
/*
	IP_ADDR destination_ip_address;
	MAC_ADDR destination_mac_address;


	case SM_GET_DESTINATION_MAC_ADDRESS:
		//----- DO THE ARP REQUEST -----
		destination_ip_address.v[0] = 213;
		destination_ip_address.v[1] = 171;
		destination_ip_address.v[2] = 193;
		destination_ip_address.v[3] = 5;

		if (arp_resolve_ip_address(&destination_ip_address))
		{
			arp_response_timeout_timer = 10;
			our_state = SM_GET_DESTINATION_MAC_ADDRESS_WAIT
		}
		//Could not transmit right now - try again next time
		break;

	case SM_GET_DESTINATION_MAC_ADDRESS_WAIT:
		//----- WAIT FOR ARP RESPONSE -----
		if (arp_is_resolve_complete (&destination_ip_address, &destination_mac_address))
		{
			//ARP IS COMPLETE
		}

		if (arp_response_timeout_timer == 0)
		{
			//ARP FAILED - REMOTE DEVICE NOT FOUND
		}
		break;
*/


//For further information please see the project technical manual






//*****************************
//*****************************
//********** DEFINES **********
//*****************************
//*****************************
#ifndef ARP_C_INIT		//(Do only once)
#define	ARP_C_INIT


#include "eth-main.h"



#define	ETHERNET_PROTOCOL_ARP		0x0800
#define ARP_OPCODE_REQUEST 			0x0001
#define ARP_OPCODE_RESPONSE 		0x0002


//----- ARP STATE MACHINE STATES -----
typedef enum _ARP_STATE
{
	SM_ARP_IDLE,
	SM_ARP_SEND_REPLY
} ARP_STATE;

//----- DATA TYPE DEFINITIONS -----
//ARP PACKET
typedef struct _ARP_PACKET
{
	WORD		hardware_type;
	WORD		protocol;
	BYTE		mac_addr_len;
	BYTE		protocol_len;
	WORD		op_code;
	MAC_ADDR	sender_mac_addr;
	IP_ADDR		sender_ip_addr;
	MAC_ADDR	target_mac_addr;
	IP_ADDR		target_ip_addr;
} ARP_PACKET;


#endif



//*******************************
//*******************************
//********** FUNCTIONS **********
//*******************************
//*******************************
#ifdef ARP_C				//(Defined only by associated C file)
//-----------------------------------
//----- INTERNAL ONLY FUNCTIONS -----
//-----------------------------------
void arp_tx_packet (DEVICE_INFO *remote_device_info, WORD op_code);


//-----------------------------------------
//----- INTERNAL & EXTERNAL FUNCTIONS -----
//-----------------------------------------
//(Also defined below as extern)
void arp_initialise (void);
BYTE arp_resolve_ip_address(IP_ADDR *ip_address_to_resolve);
BYTE arp_is_resolve_complete (IP_ADDR *ip_address_being_resolved, MAC_ADDR *resolved_mac_address);
BYTE arp_process_rx (void);


#else
//------------------------------
//----- EXTERNAL FUNCTIONS -----
//------------------------------
extern void arp_initialise (void);
extern BYTE arp_resolve_ip_address(IP_ADDR *ip_address_to_resolve);
extern BYTE arp_is_resolve_complete (IP_ADDR *ip_address_being_resolved, MAC_ADDR *resolved_mac_address);
extern BYTE arp_process_rx (void);


#endif




//****************************
//****************************
//********** MEMORY **********
//****************************
//****************************
#ifdef ARP_C				//(Defined only by associated C file)
//--------------------------------------------
//----- INTERNAL ONLY MEMORY DEFINITIONS -----
//--------------------------------------------
BYTE sm_arp;


//--------------------------------------------------
//----- INTERNAL & EXTERNAL MEMORY DEFINITIONS -----
//--------------------------------------------------
//(Also defined below as extern)
DEVICE_INFO arp_last_received_response;


#else
//---------------------------------------
//----- EXTERNAL MEMORY DEFINITIONS -----
//---------------------------------------
extern DEVICE_INFO arp_last_received_response;


#endif










