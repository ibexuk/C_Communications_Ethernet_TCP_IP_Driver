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
//DHCP (DYNAMIC HOST CONFIGURATION PROTOCOL) C CODE HEADER FILE




//##################################
//##################################
//########## USING DRIVER ##########
//##################################
//##################################

//###################################################
//##### ADD TO YOUR APPLICATION HEARTBEAT TIMER #####
//###################################################
/*
	//-----------------------------
	//----- HERE EVERY 1 mSec -----
	//-----------------------------
	
	//----- NIC DHCP TIMER -----
	if (eth_dhcp_1ms_timer)
		eth_dhcp_1ms_timer--;


	//-------------------------------
	//----- HERE EVERY 1 Second -----
	//-------------------------------
	
	//----- NIC DHCP TIMERS -----
	if (eth_dhcp_1sec_renewal_timer)
		eth_dhcp_1sec_renewal_timer--;
	if (eth_dhcp_1sec_lease_timer)
		eth_dhcp_1sec_lease_timer--;
*/


//################################################
//##### ADD TO APPLICATION STARTUP FUNCTION  #####
//################################################
//(These variables may also be changed later at any time to switch between manual and DHCP IP settings - this driver will automatically handle the changeover)
/*
	//----- TO USE A MANUALLY CONFIGURED IP SETTINGS -----
	eth_dhcp_using_manual_settings = 1;
	our_ip_address.v[0] = 192;			//MSB
	our_ip_address.v[1] = 168;
	our_ip_address.v[2] = 0;
	our_ip_address.v[3] = 51;			//LSB
	our_subnet_mask.v[0] = 255;			//MSB
	our_subnet_mask.v[1] = 255;
	our_subnet_mask.v[2] = 255;
	our_subnet_mask.v[3] = 0;			//LSB
	our_gateway_ip_address.v[0] = 192;
	our_gateway_ip_address.v[1] = 168;
	our_gateway_ip_address.v[2] = 0;
	our_gateway_ip_address.v[3] = 1;

	//----- TO USE DHCP CONFIGURED IP SETTINGS -----
	eth_dhcp_using_manual_settings = 0;
*/


//################################################################
//##### TO SEND OUR NAME AS PART OF TRANSMITTED DHCP PACKETS #####
//################################################################
//(Some DHCP servers will store received names of devices, others will not and will use NetBIOS instead)
/*
	//SET NAME TO BE RETURED TO DHCP SERVER IN DHCP PACKETS
	eth_dhcp_our_name_pointer = &netbios_our_network_name[0];		//Use the netbios name (if netbios is being used) or replace with any ram array containing
																	//your null terminated device name - fixed length of 15 bytes excluding terminating null.
*/


//##########################################
//##### OBTAINING IP ADDRES USING DHCP #####
//##########################################
//DHCP is dealt with automatically by the stack if DHCP is included - no application intervention is required
//When enabled this driver will automatically obtain an IP address from a DHCP server and renew it as required

//#############################################################
//##### REGISTERS THAT MAY BE USEFUL FOR YOUR APPLICATION #####
//#############################################################
//our_ip_address_is_valid		//= 1 if we have valid IP settings (either manually or from a DHCP server), 0 otherwise



//For further information please see the project technical manual







//********************************
//********************************
//********** DO DEFINES **********
//********************************
//********************************
#ifndef DHCP_C_INIT		//Do only once the first time this file is used
#define	DHCP_C_INIT


#include "eth-main.h"


//----- USER OPTIONS -----
//#define	DHCP_DO_POWERUP_RANDOM_DELAY	//Comment out to cause a DHCP request to be sent immediatly on powerup without a random delay (random delays should
											//be used if a large number of devices could powerup at the same time)


//----- TIMEOUTS -----
#define	DHCP_DISCOVER_TIMEOUT		10000	//x1mS
#define	DHCP_REQUEST_TIMEOUT		10000	//x1mS
#define DHCP_LEASE_MIN_SECS 		60		//Minimum lease renewal time for us in seconds
#define DHCP_LEASE_MAX_SECS 		86400	//Maximum lease renewal time for us in seconds


//----- DHCP MESSAGE TYPES -----
#define DHCP_MESSAGE_DISCOVER		1
#define DHCP_MESSAGE_OFFER			2
#define DHCP_MESSAGE_REQUEST		3
#define DHCP_MESSAGE_DECLINE		4
#define DHCP_MESSAGE_ACK			5
#define DHCP_MESSAGE_NAK			6
#define DHCP_MESSAGE_RELEASE		7
#define DHCP_MESSAGE_INFORM			8

//----- ETHERNET VALUES -----
#define DHCPSERVER_PORT		67
#define DHCPCLIENT_PORT		68

//----- DHCP STATE MACHINE STATES -----
typedef enum _DHCP_STATE
{
	DHCP_INIT,
	DHCP_DISCOVER,
	DHCP_WAIT_FOR_OFFER_RESPONSE,
	DHCP_REQUEST,
	DHCP_RENEWING,
	DHCP_WAIT_FOR_REQUEST_RESPONSE,
	DHCP_BOUND
} DHCP_STATE;

#endif


//*******************************
//*******************************
//********** FUNCTIONS **********
//*******************************
//*******************************
#ifdef DHCP_C
//-----------------------------------
//----- INTERNAL ONLY FUNCTIONS -----
//-----------------------------------
void dhcp_tx_packet (BYTE message_type);
BYTE dhcp_rx_packet (void);


//-----------------------------------------
//----- INTERNAL & EXTERNAL FUNCTIONS -----
//-----------------------------------------
//(Also defined below as extern)
void dhcp_initialise (void);
void process_dhcp(void);


#else
//------------------------------
//----- EXTERNAL FUNCTIONS -----
//------------------------------
extern void dhcp_initialise (void);
extern void process_dhcp(void);


#endif




//****************************
//****************************
//********** MEMORY **********
//****************************
//****************************
#ifdef DHCP_C
//--------------------------------------------
//----- INTERNAL ONLY MEMORY DEFINITIONS -----
//--------------------------------------------
BYTE dhcp_socket;
IP_ADDR dhcp_server_ip_addr;
MAC_ADDR dhcp_server_mac_addr;
IP_ADDR dhcp_offer_ip_addr;


//--------------------------------------------------
//----- INTERNAL & EXTERNAL MEMORY DEFINITIONS -----
//--------------------------------------------------
//(Also defined below as extern)
BYTE sm_dhcp;
BYTE eth_dhcp_using_manual_settings;
DWORD eth_dhcp_1sec_renewal_timer;
DWORD eth_dhcp_1sec_lease_timer;
WORD eth_dhcp_1ms_timer;
BYTE *eth_dhcp_our_name_pointer = 0;


#else
//---------------------------------------
//----- EXTERNAL MEMORY DEFINITIONS -----
//---------------------------------------
extern BYTE sm_dhcp;
extern BYTE eth_dhcp_using_manual_settings;
extern DWORD eth_dhcp_1sec_renewal_timer;
extern DWORD eth_dhcp_1sec_lease_timer;
extern WORD eth_dhcp_1ms_timer;
extern BYTE *eth_dhcp_our_name_pointer;


#endif







