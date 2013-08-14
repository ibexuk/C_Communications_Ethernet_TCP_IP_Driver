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
//UDP (USER DATAGRAM PROTOCOL) C CODE HEADER FILE



//##################################
//##################################
//########## USING DRIVER ##########
//##################################
//##################################

//###########################################
//##### NUMBER OF UDP SOCKETS AVAILABLE #####
//###########################################
//Set using the UDP_NO_OF_AVAILABLE_SOCKETS define below

//#####################################################################################
//##### TO SETUP A UDP SOCKET TO BE USED TO LISTEN FOR COMMUNICATIONS FROM ANYONE #####
//#####################################################################################
/*
	static BYTE our_udp_socket = UDP_INVALID_SOCKET;
	static BYTE our_udp_server_state = SM_OPEN_SOCKET;
	BYTE data;
	BYTE array_buffer[4];


	if (!nic_linked_and_ip_address_valid)
	{
		//----- WE ARE NOT CONNECTED OR DO NOT YET HAVE AN IP ADDRESS -----
		our_udp_server_state = SM_OPEN_SOCKET;
		
		//Ensure our socket is closed if we have just lost the Ethernet connection
		udp_close_socket(&our_udp_socket);
		
		return;										//Exit as we can't do anything without a connection
	}


	switch (our_udp_server_state)
	{
	case SM_OPEN_SOCKET:	
		//----- OPEN SOCKET -----
		our_udp_socket = udp_open_socket(0x00, 6451, 1);		//Leave device_info as null to setup to receive from anyone, remote_port can be anything for rx
		if (our_udp_socket != UDP_INVALID_SOCKET)
		{
			our_udp_server_state = SM_PROCESS_SOCKET;
			break;
		}
		//Could not open a socket - none currently available - keep trying

		break;


	case SM_PROCESS_SOCKET:
		//----- PROCESS SOCKET -----
		if (udp_check_socket_for_rx(our_udp_socket))
		{
			//SOCKET HAS RECEIVED A PACKET - PROCESS IT

			//READ THE PACKET AS REQURIED
			if (!udp_read_next_rx_byte(&data))
			{
				//Error - no more bytes in rx packet
			}
			//OR USE
			if (!udp_read_rx_array (array_buffer, sizeof(array_buffer)))
			{
				//Error - no more bytes in rx packet
			}

			//DUMP THE PACKET
			udp_dump_rx_packet();
		
			//SEND RESPONSE
			our_udp_server_state = SM_TX_RESPONSE;
		}
		break;
			
	case SM_TX_RESPONSE:
		//----- TX RESPONSE -----
		//SETUP TX

		//To respond to the sender leave our sockets remote device info as this already contains the remote device settings
		//Or to broadcast on our subnet do this:
		udp_socket[our_udp_socket].remote_device_info.ip_address.Val = our_ip_address.Val | ~our_subnet_mask.Val;
		udp_socket[our_udp_socket].remote_device_info.mac_address.v[0] = 0xff;
		udp_socket[our_udp_socket].remote_device_info.mac_address.v[1] = 0xff;
		udp_socket[our_udp_socket].remote_device_info.mac_address.v[2] = 0xff;
		udp_socket[our_udp_socket].remote_device_info.mac_address.v[3] = 0xff;
		udp_socket[our_udp_socket].remote_device_info.mac_address.v[4] = 0xff;
		udp_socket[our_udp_socket].remote_device_info.mac_address.v[5] = 0xff;
		udp_socket[our_udp_socket].remote_port = 6450;							//Set the port numbers as desired
		udp_socket[our_udp_socket].local_port = 6451;
		
		if (!udp_setup_tx(our_udp_socket))
		{
			//Can't tx right now - try again next time

			//Return the socket back to broadcast ready to receive from anyone again
			//udp_socket[our_udp_socket].remote_device_info.ip_address.Val = 0xffffffff;		//Only enable this if you are broadcasting responses and don't want to miss incoming packets to this socket from other devices
			
			break;
		}

		//WRITE THE UDP DATA
		udp_write_next_byte('H');
		udp_write_next_byte('e');
		udp_write_next_byte('l');
		udp_write_next_byte('l');
		udp_write_next_byte('o');
		udp_write_next_byte(' ');
		udp_write_next_byte('W');
		udp_write_next_byte('o');
		udp_write_next_byte('r');
		udp_write_next_byte('l');
		udp_write_next_byte('d');
		udp_write_next_byte(0x00);
		//You can also use udp_write_array()

		//SEND THE PACKET
		udp_tx_packet();

		//RETURN THE SOCKET BACK TO BROADCAST READY TO RECEIVE FROM ANYONE AGAIN
		udp_socket[our_udp_socket].remote_device_info.ip_address.Val = 0xffffffff;
			
		our_udp_server_state = SM_PROCESS_SOCKET;
		break;

	} //switch (our_udp_server_state)
*/


//#######################################################################################
//##### TO SETUP A UDP SOCKET TO CARRY OUT SOME COMMUNICATIONS WITH A REMOTE DEVICE #####
//#######################################################################################
/*
	static BYTE our_udp_socket = UDP_INVALID_SOCKET;
	static BYTE our_udp_client_state = SM_OPEN_SOCKET;
	static DEVICE_INFO remote_device_info;
	BYTE data;
	BYTE array_buffer[4];


	if (!nic_linked_and_ip_address_valid)
	{
		//----- WE ARE NOT CONNECTED OR DO NOT YET HAVE AN IP ADDRESS -----
		our_udp_client_state = SM_IDLE;
		
		//Ensure our socket is closed if we have just lost the Ethernet connection
		udp_close_socket(&our_udp_socket);
			
		return;										//Exit as we can't do anything without a connection
	}

	switch (our_udp_client_state)
	{
	case SM_IDLE:
		//----- DO NOTHING -----
		break;


	case SM_OPEN_SOCKET:	
		//----- OPEN SOCKET -----

		//Set to broadcast on our subnet (alternatively set the IP and MAC address to a remote devices address - use ARP first if the MAC address is unknown)
		remote_device_info.ip_address.Val = our_ip_address.Val | ~our_subnet_mask.Val;
		remote_device_info.mac_address.v[0] = 0xff;
		remote_device_info.mac_address.v[1] = 0xff;
		remote_device_info.mac_address.v[2] = 0xff;
		remote_device_info.mac_address.v[3] = 0xff;
		remote_device_info.mac_address.v[4] = 0xff;
		remote_device_info.mac_address.v[5] = 0xff;

		our_udp_socket = udp_open_socket(&remote_device_info, (WORD)6453, (WORD)6452);		//Set the port numbers as desired
		if (our_udp_socket != UDP_INVALID_SOCKET)
		{
			our_udp_client_state = SM_TX_PACKET;
			break;
		}
		//Could not open a socket - none currently available - keep trying
		break;


	case SM_TX_PACKET:
		//----- TX PACKET TO REMOTE DEVICE -----
		//SETUP TX
		if (!udp_setup_tx(our_udp_socket))
		{
			//Can't tx right now - try again next time
			break;
		}
		
		//WRITE THE TCP DATA
		udp_write_next_byte('H');
		udp_write_next_byte('e');
		udp_write_next_byte('l');
		udp_write_next_byte('l');
		udp_write_next_byte('o');
		udp_write_next_byte(' ');
		udp_write_next_byte('W');
		udp_write_next_byte('o');
		udp_write_next_byte('r');
		udp_write_next_byte('l');
		udp_write_next_byte('d');
		udp_write_next_byte(0x00);
		//You can also use udp_write_array()

		//SEND THE PACKET
		udp_tx_packet();
		
		udp_client_socket_timeout_timer = 10;
		our_udp_client_state = SM_WAIT_FOR_RESPONSE;
		break;


	case SM_WAIT_FOR_RESPONSE:
		//----- WAIT FOR RESPONSE -----
		if (udp_check_socket_for_rx(our_udp_socket))
		{
			//SOCKET HAS RECEIVED A PACKET - PROCESS IT

			//READ THE PACKET AS REQURIED
			if (!udp_read_next_rx_byte(&data))
			{
				//Error - no more bytes in rx packet
			}
			//OR USE
			if (!udp_read_rx_array (array_buffer, sizeof(array_buffer)))
			{
				//Error - no more bytes in rx packet
			}

			//DUMP THE PACKET
			udp_dump_rx_packet();
			
			//EXIT
			our_udp_client_state = SM_CLOSE_SOCKET;
		}
		
		if (udp_client_socket_timeout_timer == 0)
		{
			//TIMED OUT - NO RESPONSE FROM REMOTE DEVICE
			our_udp_client_state = SM_CLOSE_SOCKET;
		}
		break;

	case SM_CLOSE_SOCKET:
		//----- CLOSE THE SOCKET -----
		udp_close_socket(&our_udp_socket);
		
		our_udp_client_state = SM_IDLE;
		break;
	
	} //switch (our_udp_client_state)
*/


//#####################################################################
//##### TO GET PACKET SENDER DATA WHEN A PACKET HAS BEEN RECEIVED #####
//#####################################################################
/*
	sender_ip_addr.Val = udp_socket[our_udp_socket].remote_device_info.ip_address.Val;
	sender_mac_addr.v[0] = udp_socket[our_udp_socket].remote_device_info.mac_address.v[0];
	sender_mac_addr.v[1] = udp_socket[our_udp_socket].remote_device_info.mac_address.v[1];
	sender_mac_addr.v[2] = udp_socket[our_udp_socket].remote_device_info.mac_address.v[2];
	sender_mac_addr.v[3] = udp_socket[our_udp_socket].remote_device_info.mac_address.v[3];
	sender_mac_addr.v[4] = udp_socket[our_udp_socket].remote_device_info.mac_address.v[4];
	sender_mac_addr.v[5] = udp_socket[our_udp_socket].remote_device_info.mac_address.v[5];
	//sender_ip_address_packet_was_sent_to = udp_socket[our_udp_socket].destination_ip_address.Val;	//(If you need to know if received packet was broadcast or sent to our unique IP address)
*/


//For further information please see the project technical manual





//*****************************
//*****************************
//********** DEFINES **********
//*****************************
//*****************************
#ifndef UDP_C_INIT		//(Do only once)
#define	UDP_C_INIT


#include "eth-main.h"


//----- UDP SETUP -----
#define UDP_NO_OF_AVAILABLE_SOCKETS		4		//<<<<< SETUP FOR A NEW APPLICATION
												//The number of UDP sockets available to the application (each socket uses sizeof(UDP_SOCKET_INFO) bytes of ram)
												//Note that other stack components that use UDP will use sockets from this pool.

#if (UDP_NO_OF_AVAILABLE_SOCKETS <= 0 || UDP_NO_OF_AVAILABLE_SOCKETS > 255 )
#error UDP_NO_OF_AVAILABLE_SOCKETS value is out of range!
#endif


#define	UDP_CHECKSUMS_ENABLED					//Comment out if you do not want to use checksums for UDP tx and rx


//UDP PORTS:-
#define UDP_PORT_NULL			0xffff
#define	UDP_INVALID_SOCKET		0xff		//Value returned when UDP socket not found


//----- DATA TYPE DEFINITIONS -----
typedef struct _UDP_SOCKET_INFO
{
	DEVICE_INFO		remote_device_info;
	WORD			local_port;
	WORD			remote_port;
	WORD			rx_data_bytes_remaining;
	IP_ADDR			destination_ip_address;		//Contains the IP address the last received packet was sent to (not used for tx)
} UDP_SOCKET_INFO;


typedef struct _UDP_HEADER
{
	WORD		source_port;
	WORD		destination_port;
	WORD		length;
	WORD		checksum;
} UDP_HEADER;
#define	UDP_HEADER_LENGTH			8			//Defined to avoid sizeof problemms with compilers that add padd bytes

#endif



//*******************************
//*******************************
//********** FUNCTIONS **********
//*******************************
//*******************************
#ifdef UDP_C				//(Defined only by associated C file)
//-----------------------------------
//----- INTERNAL ONLY FUNCTIONS -----
//-----------------------------------
static BYTE udp_rx_check_for_matches_socket (UDP_HEADER *rx_udp_header, DEVICE_INFO *rx_device_info);


//-----------------------------------------
//----- INTERNAL & EXTERNAL FUNCTIONS -----
//-----------------------------------------
//(Also defined below as extern)
void udp_initialise (void);
BYTE udp_process_rx (DEVICE_INFO *sender_device_info, IP_ADDR *destination_ip_address, WORD ip_data_area_bytes);
BYTE udp_open_socket (DEVICE_INFO *device_info, WORD local_port, WORD remote_port);
void udp_close_socket (BYTE *socket);
BYTE udp_check_socket_for_rx (BYTE socket);
BYTE udp_read_next_rx_byte (BYTE *data);
BYTE udp_read_rx_array (BYTE *array_buffer, WORD array_length);
void udp_dump_rx_packet (void);
BYTE udp_setup_tx (BYTE socket);
void udp_write_next_byte (BYTE data);
void udp_write_array (BYTE *array_buffer, WORD array_length);
void udp_tx_packet (void);


#else
//------------------------------
//----- EXTERNAL FUNCTIONS -----
//------------------------------
extern void udp_initialise (void);
extern BYTE udp_process_rx (DEVICE_INFO *sender_device_info, IP_ADDR *destination_ip_address, WORD ip_data_area_bytes);
extern BYTE udp_open_socket (DEVICE_INFO *device_info, WORD local_port, WORD remote_port);
extern void udp_close_socket (BYTE *socket);
extern BYTE udp_check_socket_for_rx (BYTE socket);
extern BYTE udp_read_next_rx_byte (BYTE *data);
extern BYTE udp_read_rx_array (BYTE *array_buffer, WORD array_length);
extern void udp_dump_rx_packet (void);
extern BYTE udp_setup_tx (BYTE socket);
extern void udp_write_next_byte (BYTE data);
extern void udp_write_array (BYTE *array_buffer, WORD array_length);
extern void udp_tx_packet (void);


#endif




//****************************
//****************************
//********** MEMORY **********
//****************************
//****************************
#ifdef UDP_C				//(Defined only by associated C file)
//--------------------------------------------
//----- INTERNAL ONLY MEMORY DEFINITIONS -----
//--------------------------------------------
#ifdef UDP_CHECKSUMS_ENABLED
WORD udp_tx_checksum;
BYTE udp_tx_checksum_next_byte_low;
#endif

//--------------------------------------------------
//----- INTERNAL & EXTERNAL MEMORY DEFINITIONS -----
//--------------------------------------------------
//(Also defined below as extern)
BYTE udp_rx_packet_is_waiting_to_be_processed;
UDP_SOCKET_INFO udp_socket[UDP_NO_OF_AVAILABLE_SOCKETS];
BYTE udp_rx_active_socket;


#else
//---------------------------------------
//----- EXTERNAL MEMORY DEFINITIONS -----
//---------------------------------------
extern BYTE udp_rx_packet_is_waiting_to_be_processed;
extern UDP_SOCKET_INFO udp_socket[UDP_NO_OF_AVAILABLE_SOCKETS];
extern BYTE udp_rx_active_socket;


#endif










