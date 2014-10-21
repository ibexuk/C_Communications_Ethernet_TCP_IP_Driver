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
//DHCP (DYNAMIC HOST CONFIGURATION PROTOCOL) C CODE FILE


#include "main.h"					//Global data type definitions (see https://github.com/ibexuk/C_Generic_Header_File )
#define DHCP_C
#include "eth-dhcp.h"
#undef DHCP_C

#include "eth-main.h"
#include "eth-nic.h"
#include "eth-udp.h"		//UDP is requried for DHCP



//--------------------------------------------------------------------------
//----- GENERATE COMPILE ERROR IF DHCP HAS NOT BEEN DEFINED TO BE USED -----
//--------------------------------------------------------------------------
#ifndef STACK_USE_DHCP
#error DHCP file is included in project but not defined to be used - remove file from project to reduce code size.
#endif





//*************************************
//*************************************
//********** DHCP INITIALISE **********
//*************************************
//*************************************
//Called from the tcp_ip_initialise function
void dhcp_initialise (void)
{
	sm_dhcp = DHCP_INIT;
	nic_linked_and_ip_address_valid = 0;

	//If configured to use DHCP then initialise the IP settings
	if (eth_dhcp_using_manual_settings == 0)
	{
		our_ip_address.Val = 0;
		our_subnet_mask.Val = 0;
		our_gateway_ip_address.Val = 0;
	}
}




//**********************************
//**********************************
//********** PROCESS DHCP **********
//**********************************
//**********************************
void process_dhcp (void)
{
	DEVICE_INFO remote_device_info;



	//----------------------------------------------
	//----- CHECK FOR USING MANUAL IP SETTINGS -----
	//----------------------------------------------
	if (eth_dhcp_using_manual_settings)
	{
		//----- WE ARE CURRENTLY CONFIGURED TO USE MANUAL SETTINGS, NOT DHCP -----
		sm_dhcp = DHCP_BOUND;
		eth_dhcp_1sec_renewal_timer = 1000;				//If our application changes our state to use DHCP cause a request to happen quickly
		eth_dhcp_1sec_lease_timer = 2;
	}


	//------------------------------------
	//----- CHECK FOR NIC NOT LINKED -----
	//------------------------------------
	if (nic_is_linked == 0)
	{
		//----- WE HAVE NO ETHERNET LINK - PUT DHCP INTO INITIALISE STATE WAITING FOR NIC TO BE LINKED -----
		sm_dhcp = DHCP_INIT;
	}


	//------------------------
	//----- PROCESS DHCP -----
	//------------------------
	switch (sm_dhcp)
	{
	case DHCP_INIT:
		//----------------------
		//----- INITIALISE -----
		//----------------------
		
		nic_linked_and_ip_address_valid = 0;				//Flag that we do not have valid IP settings
		
		//ENSURE OUR SOCKET IS NOT OPEN
		if (dhcp_socket != UDP_INVALID_SOCKET)
			udp_close_socket(&dhcp_socket);
		
		if (eth_dhcp_using_manual_settings == 0)
		{
			//----- WE ARE CURRENTLY CONFIGURED TO USE DHCP (NOT MANUAL SETTINGS) -----
			our_ip_address.Val = 0;
			our_subnet_mask.Val = 0;
			our_gateway_ip_address.Val = 0;
			eth_dhcp_1sec_lease_timer = 100;			//Set a value so that the discover stage doesn't immediatly return to here

			//GENERATE A RANDOM DELAY OF UP TO 10 SECONDS BEFORE CONTACTING THE DHCP SERVER
			//(Good practice to avoid lots of devices hitting a server at once when connectivety returns)
			#ifdef DHCP_DO_POWERUP_RANDOM_DELAY
				eth_dhcp_1ms_timer = (((WORD)our_mac_address.v[0]) << 8) + (WORD)our_mac_address.v[1];		//Use our mac address as the random variable
				while (eth_dhcp_1ms_timer > 10000)			//Limit to 10 seconds
					eth_dhcp_1ms_timer -= 10000;
				if (eth_dhcp_1ms_timer < 1000)				//Force to at least 1 second
					eth_dhcp_1ms_timer += 1000;
			#else
					eth_dhcp_1ms_timer = 100;
			#endif

			//Do discovery next
			sm_dhcp = DHCP_DISCOVER;
		}
		break;



	case DHCP_DISCOVER:
		//-------------------------
		//----- DHCP DISCOVER -----
		//-------------------------
		//Send a DHCP discover request

		//Wait to do action (if coming from another state that set a delay to occur first)
		if (eth_dhcp_1ms_timer)
			break;

		if (eth_dhcp_1sec_lease_timer == 0)
		{
			//----- OUR LEASE HAS EXPIRED - IP SETTINGS NO LONGER VALID -----
			sm_dhcp = DHCP_INIT;
			break;
		}


		//TRY AND OPEN A UDP SOCKET
		remote_device_info.ip_address.Val = 0xffffffff;			//Set to broadcast
		remote_device_info.mac_address.v[0] = 0xff;
		remote_device_info.mac_address.v[1] = 0xff;
		remote_device_info.mac_address.v[2] = 0xff;
		remote_device_info.mac_address.v[3] = 0xff;
		remote_device_info.mac_address.v[4] = 0xff;
		remote_device_info.mac_address.v[5] = 0xff;
		dhcp_socket = udp_open_socket(&remote_device_info, DHCPCLIENT_PORT, DHCPSERVER_PORT);

		if (dhcp_socket != UDP_INVALID_SOCKET)
		{
			//WE HAVE OBTAINED A UDP SOCKET - SETUP FOR TX
			if (udp_setup_tx(dhcp_socket) == 0)
			{
				//TX IS NOT CURRENTLY AVAILABLE
				//Try again next time
				udp_close_socket(&dhcp_socket);
				break;
			}
			//----- SEND DISCOVER PACKET -----
			dhcp_tx_packet(DHCP_MESSAGE_DISCOVER);

			eth_dhcp_1ms_timer = DHCP_DISCOVER_TIMEOUT;
			sm_dhcp = DHCP_WAIT_FOR_OFFER_RESPONSE;
			break;
		}
		//CANNOT OPEN A UDP SOCKET
		//There is no free UDP socket currently - Try again next time
		break;



	case DHCP_WAIT_FOR_OFFER_RESPONSE:
		//-----------------------------------
		//----- WAIT FOR OFFER RESPONSE -----
		//-----------------------------------
		if (udp_check_socket_for_rx(dhcp_socket))
		{
			//----- RESPONSE RECEVIED -----
			
			//Process the response packet
			if (dhcp_rx_packet())
			{
				//PACKET PROCESSED AND OUR STATE MACHINE HAS BEEN UPDATED - DO NOTHING
				udp_close_socket(&dhcp_socket);
				break;
			}
			else
			{
				//PACKET WAS NOT VALID FOR SOME REASON
				//Try again after a short delay
				udp_close_socket(&dhcp_socket);
				eth_dhcp_1ms_timer = 1000;
				sm_dhcp = DHCP_DISCOVER;
				break;				
			}

		}
		else if (eth_dhcp_1ms_timer == 0)
		{
			//------ TIMED OUT - NO RESPONSE FROM DHCP SERVER -----
			//Setup to try again after a delay
			eth_dhcp_1ms_timer = (((WORD)our_mac_address.v[0]) << 8) + (WORD)our_mac_address.v[1];		//Set timer to a random delay based on our mac address
			while (eth_dhcp_1ms_timer > 8000)
				eth_dhcp_1ms_timer -= 8000;
			if (eth_dhcp_1ms_timer < 4000)
				eth_dhcp_1ms_timer += 4000;

			udp_close_socket(&dhcp_socket);
			sm_dhcp = DHCP_DISCOVER;
		}
		break;



	case DHCP_REQUEST:		//(These 2 states do the same thing)
	case DHCP_RENEWING:
		//-----------------------------------------
		//----- REQUEST OR RENEW DHCP ADDRESS -----
		//-----------------------------------------
		//Wait to do action
		if (eth_dhcp_1ms_timer)
			break;

		//TRY AND OPEN A UDP SOCKET
		//Address must be broadcast for a request (after discover) as a DHCP server will not reply if addressed by its IP address until it has ACK'ed the first request
		//When renewing the individual IP address can be used but there isn't any reason to so we always send broadcast
		remote_device_info.ip_address.Val = 0xffffffff;			//Set to broadcast
		remote_device_info.mac_address.v[0] = 0xff;
		remote_device_info.mac_address.v[1] = 0xff;
		remote_device_info.mac_address.v[2] = 0xff;
		remote_device_info.mac_address.v[3] = 0xff;
		remote_device_info.mac_address.v[4] = 0xff;
		remote_device_info.mac_address.v[5] = 0xff;

		dhcp_socket = udp_open_socket(&remote_device_info, DHCPCLIENT_PORT, DHCPSERVER_PORT);

		if (dhcp_socket != UDP_INVALID_SOCKET)
		{
			//WE HAVE OBTAINED A UDP SOCKET - SETUP FOR TX
			if (udp_setup_tx(dhcp_socket) == 0)
			{
				//TX IS NOT CURRENTLY AVAILABLE
				//Try again next time
				udp_close_socket(&dhcp_socket);
				break;
			}
			//----- SEND REQUEST PACKET -----
			dhcp_tx_packet(DHCP_MESSAGE_REQUEST);

			eth_dhcp_1ms_timer = DHCP_REQUEST_TIMEOUT;
			sm_dhcp = DHCP_WAIT_FOR_REQUEST_RESPONSE;
			break;
		}
		//CANNOT OPEN A UDP SOCKET
		//There is no free UDP socket currently - Try again next time
		break;



	case DHCP_WAIT_FOR_REQUEST_RESPONSE:
		//-------------------------------------
		//----- WAIT FOR REQUEST RESPONSE -----
		//-------------------------------------

		if (udp_check_socket_for_rx(dhcp_socket))
		{
			//----- RESPONSE RECEVIED -----
			
			//Process the response packet
			if (dhcp_rx_packet())
			{
				//PACKET PROCESSED AND OUR STATE MACHINE HAS BEEN UPDATED - DO NOTHING
				udp_close_socket(&dhcp_socket);
				break;
			}
			else
			{
				//PACKET WAS NOT VALID FOR SOME REASON
				//Try again after a short delay
				udp_close_socket(&dhcp_socket);
				eth_dhcp_1ms_timer = 1000;
				sm_dhcp = DHCP_DISCOVER;
				break;				
			}

		}
		else if (eth_dhcp_1ms_timer == 0)
		{
			//------ TIMED OUT - NO RESPONSE FROM DHCP SERVER -----
			//Setup to try again after a delay
			eth_dhcp_1ms_timer = (((WORD)our_mac_address.v[0]) << 8) + (WORD)our_mac_address.v[1];		//Set timer for a retry if this doesn't work
			while (eth_dhcp_1ms_timer > 8000)
				eth_dhcp_1ms_timer -= 8000;
			if (eth_dhcp_1ms_timer < 4000)
				eth_dhcp_1ms_timer += 4000;

			udp_close_socket(&dhcp_socket);
			sm_dhcp = DHCP_DISCOVER;
		}
		break;




	case DHCP_BOUND:
		//----------------------
		//----- DHCP BOUND -----
		//----------------------
		//We have our IP settings - do we need to renew with the DHCP server?

		nic_linked_and_ip_address_valid = 1;				//Flag that we have valid IP settings

		if (eth_dhcp_1sec_renewal_timer == 0)
		{
			//----- WE NEED TO RENEW OUR ADDRES -----
			sm_dhcp = DHCP_RENEWING;			
			break;
		}

		if (eth_dhcp_1sec_lease_timer == 0)
		{
			//----- OUR LEASE HAS EXPIRED - IP SETTINGS NO LONGER VALID -----
			sm_dhcp = DHCP_INIT;
			break;
		}

		break;

	//-------------------------
	//----- END OF SWITCH -----
	//-------------------------
	}	//switch (sm_dhcp)



}





//**************************************
//**************************************
//********** SEND DHCP PACKET **********
//**************************************
//**************************************
void dhcp_tx_packet (BYTE message_type)
{
	WORD w_temp;


	//----- A UDP PACKET HAS BEEN SETUP AND WE ARE READY TO WRITE THE UDP DATA AREA -----

	//Send Boot Request
	udp_write_next_byte(1);

	//Send Hardware Address Type = Ethernet
	udp_write_next_byte(1);

	//Address Length
	udp_write_next_byte(6);

	//Hops
	udp_write_next_byte(0);

	//Transaction ID (Random number - use our mac address)
	udp_write_next_byte(our_mac_address.v[2]);
	udp_write_next_byte(our_mac_address.v[3]);
	udp_write_next_byte(our_mac_address.v[4]);
	udp_write_next_byte(our_mac_address.v[5]);

	//Elapsed Time
	udp_write_next_byte(0);
	udp_write_next_byte(0);

	//Flags
	udp_write_next_byte(0);
	udp_write_next_byte(0);

	//Client IP Address
	udp_write_next_byte(our_ip_address.v[0]);
	udp_write_next_byte(our_ip_address.v[1]);
	udp_write_next_byte(our_ip_address.v[2]);
	udp_write_next_byte(our_ip_address.v[3]);

	//Your (Client) IP Address
	udp_write_next_byte(0);
	udp_write_next_byte(0);
	udp_write_next_byte(0);
	udp_write_next_byte(0);

	//Server IP Address
	udp_write_next_byte(0);
	udp_write_next_byte(0);
	udp_write_next_byte(0);
	udp_write_next_byte(0);

	//Relay agent IP
	udp_write_next_byte(0);
	udp_write_next_byte(0);
	udp_write_next_byte(0);
	udp_write_next_byte(0);

	//Our MAC address
	udp_write_next_byte(our_mac_address.v[0]);
	udp_write_next_byte(our_mac_address.v[1]);
	udp_write_next_byte(our_mac_address.v[2]);
	udp_write_next_byte(our_mac_address.v[3]);
	udp_write_next_byte(our_mac_address.v[4]);
	udp_write_next_byte(our_mac_address.v[5]);

	//Unused bytes
	for (w_temp = 0; w_temp < 202; w_temp++)
		udp_write_next_byte(0);

	//DHCP Magic Cookie
	udp_write_next_byte(0x63);
	udp_write_next_byte(0x82);
	udp_write_next_byte(0x53);
	udp_write_next_byte(0x63);

	//----- BELOW HERE ARE THE DHCP OPTIONS -----
	//(These may be changed if required)

	//Send Option - DHCP Message Type
	udp_write_next_byte(53);					//Option
	udp_write_next_byte(1);						//Length
	udp_write_next_byte(message_type);			//Data

	//Send Option - Client ID
	udp_write_next_byte(61);					//Option
	udp_write_next_byte(7);						//Length
	udp_write_next_byte(1);						//Data - Hardware type (Ethernet)
	udp_write_next_byte(our_mac_address.v[0]);	//Data - our client ID (MAC address)
	udp_write_next_byte(our_mac_address.v[1]);
	udp_write_next_byte(our_mac_address.v[2]);
	udp_write_next_byte(our_mac_address.v[3]);
	udp_write_next_byte(our_mac_address.v[4]);
	udp_write_next_byte(our_mac_address.v[5]);


	//If doing DHCP request then send option - dhcp server IP, and send option - our requested IP
	if (sm_dhcp == DHCP_REQUEST)
	{
		udp_write_next_byte(54);						//Option
		udp_write_next_byte(4);							//Length
		udp_write_next_byte(dhcp_server_ip_addr.v[0]);	//Data
		udp_write_next_byte(dhcp_server_ip_addr.v[1]);
		udp_write_next_byte(dhcp_server_ip_addr.v[2]);
		udp_write_next_byte(dhcp_server_ip_addr.v[3]);

		udp_write_next_byte(50);						//Option
		udp_write_next_byte(4);							//Length
		udp_write_next_byte(dhcp_offer_ip_addr.v[0]);	//Data
		udp_write_next_byte(dhcp_offer_ip_addr.v[1]);
		udp_write_next_byte(dhcp_offer_ip_addr.v[2]);
		udp_write_next_byte(dhcp_offer_ip_addr.v[3]);
    }

	//Send our name if we have one assigned to the pointer variable
	if (eth_dhcp_our_name_pointer)
	{
		udp_write_next_byte(12);					//Option
		udp_write_next_byte(16);					//Length
		udp_write_next_byte(eth_dhcp_our_name_pointer[0]);					//Data - our ascii name
		udp_write_next_byte(eth_dhcp_our_name_pointer[1]);
		udp_write_next_byte(eth_dhcp_our_name_pointer[2]);
		udp_write_next_byte(eth_dhcp_our_name_pointer[3]);
		udp_write_next_byte(eth_dhcp_our_name_pointer[4]);
		udp_write_next_byte(eth_dhcp_our_name_pointer[5]);
		udp_write_next_byte(eth_dhcp_our_name_pointer[6]);
		udp_write_next_byte(eth_dhcp_our_name_pointer[7]);
		udp_write_next_byte(eth_dhcp_our_name_pointer[8]);
		udp_write_next_byte(eth_dhcp_our_name_pointer[9]);
		udp_write_next_byte(eth_dhcp_our_name_pointer[10]);
		udp_write_next_byte(eth_dhcp_our_name_pointer[11]);
		udp_write_next_byte(eth_dhcp_our_name_pointer[12]);
		udp_write_next_byte(eth_dhcp_our_name_pointer[13]);
		udp_write_next_byte(eth_dhcp_our_name_pointer[14]);
		udp_write_next_byte(0x00);
	}


	//End of options marker
	udp_write_next_byte(0xff);


	//----- SEND THE PACKET -----
	udp_tx_packet();
}



//*****************************************
//*****************************************
//********** RECEIVE DHCP PACKET **********
//*****************************************
//*****************************************
//Returns 1 if a DHCP packet was sucessfully processed, 0 if not
//If return value is 1 then the DHCP state machine has been updated
BYTE dhcp_rx_packet (void)
{
	BYTE data;
	BYTE w_temp;
	IP_ADDR received_ip;
	BYTE option_code;
	BYTE option_len;
	DWORD option_data;
	BYTE option_message_type = 0xff;
	DWORD option_lease_time;
	IP_ADDR option_subnet_mask;
	IP_ADDR option_gateway;
	//IP_ADDR option_server_ip_addr;

	received_ip.Val = 0;
	option_subnet_mask.Val = 0;
	option_gateway.Val = 0;
	dhcp_server_ip_addr.val = 0;

	//GET OPCODE
	udp_read_next_rx_byte(&data);
	if (data != 2)
		goto dhcp_rx_packet_dump;

	//GET ADDRESS TYPE
	udp_read_next_rx_byte(&data);

	//GET ADDRESS LENGTH
	udp_read_next_rx_byte(&data);
	if (data != 6)
		goto dhcp_rx_packet_dump;

	//GET HOPS [1]
	//GET TRANSACTION ID [4]
	//GET ELAPSED TIME [2]
	//GET FLAGS [2]
	//GET CLIENT IP [4]
	for(w_temp = 0; w_temp < 13; w_temp++)
		udp_read_next_rx_byte(&data);

	//GET YOUR (CLIENT) IP ADDRESS (The IP address being assigned to us)
	udp_read_next_rx_byte(&received_ip.v[0]);
	udp_read_next_rx_byte(&received_ip.v[1]);
	udp_read_next_rx_byte(&received_ip.v[2]);
	udp_read_next_rx_byte(&received_ip.v[3]);

	//GET SERVER IP [4]
	//GET RELAY AGENT IP [4]
	for(w_temp = 0; w_temp < 8; w_temp++)
		udp_read_next_rx_byte(&data);

	//GET MAC ADDRESS
	udp_read_next_rx_byte(&data);
	if (data != our_mac_address.v[0])
		goto dhcp_rx_packet_dump;
		
	udp_read_next_rx_byte(&data);
	if (data != our_mac_address.v[1])
		goto dhcp_rx_packet_dump;
		
	udp_read_next_rx_byte(&data);
	if (data != our_mac_address.v[2])
		goto dhcp_rx_packet_dump;
		
	udp_read_next_rx_byte(&data);
	if (data != our_mac_address.v[3])
		goto dhcp_rx_packet_dump;
		
	udp_read_next_rx_byte(&data);
	if (data != our_mac_address.v[4])
		goto dhcp_rx_packet_dump;
		
	udp_read_next_rx_byte(&data);
	if (data != our_mac_address.v[5])
		goto dhcp_rx_packet_dump;

	//UNUSED BYTES
	for(w_temp = 0; w_temp < (10 + 64 + 128); w_temp++)
		udp_read_next_rx_byte(&data);

	//GET MAGIC COOKIE
	udp_read_next_rx_byte(&data);
	if (data != 0x63)
		goto dhcp_rx_packet_dump;
		
	udp_read_next_rx_byte(&data);
	if (data != 0x82)
		goto dhcp_rx_packet_dump;
		
	udp_read_next_rx_byte(&data);
	if (data != 0x53)
		goto dhcp_rx_packet_dump;
		
	udp_read_next_rx_byte(&data);
	if (data != 0x63)
		goto dhcp_rx_packet_dump;


	//----- BELOW HERE ARE THE DHCP OPTIONS -----

	//GET EACH OPTION
	while (
			udp_read_next_rx_byte(&option_code) && 
			(option_code != 0xff) &&
			udp_read_next_rx_byte(&option_len)
			)
	{
		//----- GET THE OPTION DATA -----
		option_data = 0;
		for (w_temp = 0; w_temp < option_len; w_temp++)
		{
			option_data <<= 8;
			udp_read_next_rx_byte(&data);
			option_data += (DWORD)data;
		}

		//----- PROCESS THE OPTION -----
		

		if ((option_code == 53) && (option_len == 1))
		{
			//----- MESSAGE TYPE -----
			option_message_type = (BYTE)option_data;
		}	
		else if ((option_code == 51) && (option_len == 4))
		{
			//----- LEASE TIME -----
			option_lease_time = option_data;
		}	
		else if ((option_code == 1) && (option_len == 4) && (option_message_type == DHCP_MESSAGE_ACK))
		{
			//----- SUBNET MASK -----
			option_subnet_mask.v[0] = (BYTE)(option_data >> 24);
			option_subnet_mask.v[1] = (BYTE)((option_data & 0x00ff0000) >> 16);
			option_subnet_mask.v[2] = (BYTE)((option_data & 0x0000ff00) >> 8);
			option_subnet_mask.v[3] = (BYTE)(option_data & 0x000000ff);
		}
		else if ((option_code == 3) && (option_len == 4) && (option_message_type == DHCP_MESSAGE_ACK))
		{
			//----- GATEWAY -----
			option_gateway.v[0] = (BYTE)(option_data >> 24);
			option_gateway.v[1] = (BYTE)((option_data & 0x00ff0000) >> 16);
			option_gateway.v[2] = (BYTE)((option_data & 0x0000ff00) >> 8);
			option_gateway.v[3] = (BYTE)(option_data & 0x000000ff);
		}
		else if ((option_code == 54) && (option_len == 4))		//DHCP_MESSAGE_OFFER or DHCP_MESSAGE_ACK
		{
			//----- DHCP SERVER IP ADDRESS ----- (Enable this if you want it for some reason - we get the server IP address from the packet header)
			dhcp_server_ip_addr.v[0] = (BYTE)(option_data >> 24);
			dhcp_server_ip_addr.v[1] = (BYTE)((option_data & 0x00ff0000) >> 16);
			dhcp_server_ip_addr.v[2] = (BYTE)((option_data & 0x0000ff00) >> 8);
			dhcp_server_ip_addr.v[3] = (BYTE)(option_data & 0x000000ff);
		}


		//<<<< ADD ANY OTHER OPTIONS REQURIED HERE


	}
	

	//----- STORE THE DHCP SERVER IP ADDRESS -----
	if (dhcp_server_ip_addr.val == 0)
		dhcp_server_ip_addr.Val = udp_socket[dhcp_socket].remote_device_info.ip_address.Val;


	//----- PROCESS PACKET -----
	if (option_message_type == DHCP_MESSAGE_NAK)
	{
		//----- NAK RECEIVED -----
		//This should not happen - restart DHCP in case the DHCP server want's to give us a new address

		eth_dhcp_1ms_timer = 0;
		sm_dhcp = DHCP_INIT;
	}
	else if ((sm_dhcp == DHCP_WAIT_FOR_OFFER_RESPONSE) && (option_message_type == DHCP_MESSAGE_OFFER))
	{
		//----- WE ARE WAITING FOR AN OFFER AND IT HAS BEEN RECEIVED -----
		//Store the DHCP server mac address
		dhcp_server_mac_addr.v[0] = udp_socket[dhcp_socket].remote_device_info.mac_address.v[0];
		dhcp_server_mac_addr.v[1] = udp_socket[dhcp_socket].remote_device_info.mac_address.v[1];
		dhcp_server_mac_addr.v[2] = udp_socket[dhcp_socket].remote_device_info.mac_address.v[2];
		dhcp_server_mac_addr.v[3] = udp_socket[dhcp_socket].remote_device_info.mac_address.v[3];
		dhcp_server_mac_addr.v[4] = udp_socket[dhcp_socket].remote_device_info.mac_address.v[4];
		dhcp_server_mac_addr.v[5] = udp_socket[dhcp_socket].remote_device_info.mac_address.v[5];

		//Store the offered IP address
		dhcp_offer_ip_addr.Val = received_ip.Val;

		//Set DHCP state machine to new state
		eth_dhcp_1ms_timer = 0;
		sm_dhcp = DHCP_REQUEST;
	}
	else if (
			(sm_dhcp == DHCP_WAIT_FOR_REQUEST_RESPONSE) &&
			(option_message_type == DHCP_MESSAGE_ACK)
			)
	{
		//----- WE HAVE SENT A REQUEST OR RENEWAL REQUEST AND WE HAVE RECEIVED THE RESPONSE -----

		//Get the lease time	
		if (option_lease_time > DHCP_LEASE_MAX_SECS)
			eth_dhcp_1sec_lease_timer = DHCP_LEASE_MAX_SECS;
		else if (option_lease_time < DHCP_LEASE_MIN_SECS)
			eth_dhcp_1sec_lease_timer = DHCP_LEASE_MIN_SECS;
		else
			eth_dhcp_1sec_lease_timer = option_lease_time;

		eth_dhcp_1sec_renewal_timer = (eth_dhcp_1sec_lease_timer >> 1);			//We will ask to renew the lease half way through our lease time

		//Store our new IP address
		our_ip_address.Val = received_ip.Val;

		//Store our new subnet mask
		if (option_subnet_mask.Val)
			our_subnet_mask.Val = option_subnet_mask.Val;
		
		//Store our new gateway address
		if (option_gateway.Val)
			our_gateway_ip_address.Val = option_gateway.Val;

		//Set DHCP state machine to new state
		sm_dhcp = DHCP_BOUND;

	}
	else
	{
		goto dhcp_rx_packet_dump;
	}

	//-----------------------------------------------------
	//----- GOOD DHCP RESPONSE RECEIVED AND PROCESSED -----
	//-----------------------------------------------------
	udp_dump_rx_packet();
	return(1);

dhcp_rx_packet_dump:
	//-------------------------------------------
	//----- RESPONSE RECEIVED BUT NOT VALID -----
	//-------------------------------------------
	udp_dump_rx_packet();
	return(0);
}





