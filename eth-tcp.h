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
//TCP (TRANSMISSION CONTROL PROTOCOL) C CODE HEADER FILE




//##################################
//##################################
//########## USING DRIVER ##########
//##################################
//##################################

//###########################################
//##### NUMBER OF TCP SOCKETS AVAILABLE #####
//###########################################
//Set using the TCP_NO_OF_AVAILABLE_SOCKETS define below


//####################################################################################
//##### TO SETUP A TCP SERVER SOCKET TO ACCEPT A CONNECTION FROM A REMOTE CLIENT #####
//####################################################################################
/*
	static BYTE our_tcp_server_socket = TCP_INVALID_SOCKET;
	static BYTE our_tcp_server_state = SM_OPEN_SOCKET;
	BYTE data;
	BYTE array_buffer[4];

	if (!nic_linked_and_ip_address_valid)
	{
		//----- WE ARE NOT CONNECTED OR DO NOT YET HAVE AN IP ADDRESS -----
		our_tcp_server_state = SM_OPEN_SOCKET;
		
		//Ensure our socket is closed if we have just lost the Ethernet connection
		tcp_close_socket_from_listen(our_tcp_server_socket);
		
		return;										//Exit as we can't do anything without a connection
	}


	switch (our_tcp_server_state)
	{
	case SM_OPEN_SOCKET:	
		//----- OPEN SOCKET -----
		if (our_tcp_server_socket != TCP_INVALID_SOCKET)			//We shouldn't have a socket currently, but make sure
			tcp_close_socket(our_tcp_server_socket);

		our_tcp_server_socket = tcp_open_socket_to_listen(4101);		//We will listen on port 4101 (change as required)
		if (our_tcp_server_socket != TCP_INVALID_SOCKET)
		{
			our_tcp_server_state = SM_WAIT_FOR_CONNECTION;
			break;
		}
		//Could not open a socket - none currently available - keep trying
		
		break;
	
	case SM_WAIT_FOR_CONNECTION:
		//----- WAIT FOR A CLIENT TO CONNECT -----
		if(tcp_is_socket_connected(our_tcp_server_socket))
		{
			//A CLIENT HAS CONNECTED TO OUR SOCKET
			our_tcp_server_state = SM_PROCESS_CONNECTION;
			tcp_server_socket_timeout_timer = 10;				//Set our client has been lost timeout (to avoid client disappearing and causing this socket to never be closed)
		}
		break;

	case SM_PROCESS_CONNECTION:
		//----- PROCESS CLIENT CONNECTION -----

		if (tcp_server_socket_timeout_timer == 0)
		{
			//THERE HAS BEEN NO COMMUNICATIONS FROM CLIENT TIMEOUT - RESET SOCKET AS WE ASSUME CLIENT HAS BEEN LOST
			tcp_close_socket(our_tcp_server_socket);		//As this socket is a server the existing connection will be closed but the socket will be reset to wait for a new connection (use tcp_close_socket_from_listen if you want to fully close it)
			our_tcp_server_state = SM_WAIT_FOR_CONNECTION;
		}

		if (tcp_check_socket_for_rx(our_tcp_server_socket))
		{
			//SOCKET HAS RECEIVED A PACKET - PROCESS IT
			tcp_server_socket_timeout_timer = 10;				//Reset our timeout timer

			//READ THE PACKET AS REQURIED
			if (tcp_read_next_rx_byte(&data) == 0)
			{
				//Error - no more bytes in rx packet
			}
			//OR USE
			if (tcp_read_rx_array (array_buffer, sizeof(array_buffer)) == 0)
			{
				//Error - no more bytes in rx packet
			}

			//DUMP THE PACKET
			tcp_dump_rx_packet();
		
			//SEND RESPONSE
			our_tcp_server_state = SM_TX_RESPONSE;
		}

		if (tcp_does_socket_require_resend_of_last_packet(our_tcp_server_socket))
		{
			//RE-SEND LAST PACKET TRANSMITTED
			//(TCP requires resending of packets if they are not acknowledged and to
			//avoid requiring a large RAM buffer the application needs to remember
			//the last packet sent on a socket so it can be resent if requried).
			our_tcp_server_state = SM_TX_RESPONSE;
		}

		if(!tcp_is_socket_connected(our_tcp_server_socket))
		{
			//THE CLIENT HAS DISCONNECTED
			our_tcp_server_state = SM_WAIT_FOR_CONNECTION;
		}

		break;


	case SM_TX_RESPONSE:
		//----- TX RESPONSE -----
		if (!tcp_setup_socket_tx(our_tcp_server_socket))
		{
			//Can't tx right now - try again next time
			break;
		}

		//WRITE THE TCP DATA
		tcp_write_next_byte('H');
		tcp_write_next_byte('e');
		tcp_write_next_byte('l');
		tcp_write_next_byte('l');
		tcp_write_next_byte('o');
		tcp_write_next_byte(' ');
		tcp_write_next_byte('W');
		tcp_write_next_byte('o');
		tcp_write_next_byte('r');
		tcp_write_next_byte('l');
		tcp_write_next_byte('d');
		tcp_write_next_byte(0x00);
		//You can also use tcp_write_array()

		//SEND THE PACKET
		tcp_socket_tx_packet(our_tcp_server_socket);
		
		our_tcp_server_state = SM_PROCESS_CONNECTION;
		break;
	}
*/

//########################################################################
//##### TO GET REMOTE CLIENT INFORMATION WHILE A CLIENT IS CONNECTED #####
//########################################################################
/*
	sender_ip_addr.Val = tcp_socket[our_tcp_server_socket].remote_device_info.ip_address.Val;
	sender_mac_addr.v[0] = tcp_socket[our_tcp_server_socket].remote_device_info.mac_address.v[0];
	sender_mac_addr.v[1] = tcp_socket[our_tcp_server_socket].remote_device_info.mac_address.v[1];
	sender_mac_addr.v[2] = tcp_socket[our_tcp_server_socket].remote_device_info.mac_address.v[2];
	sender_mac_addr.v[3] = tcp_socket[our_tcp_server_socket].remote_device_info.mac_address.v[3];
	sender_mac_addr.v[4] = tcp_socket[our_tcp_server_socket].remote_device_info.mac_address.v[4];
	sender_mac_addr.v[5] = tcp_socket[our_tcp_server_socket].remote_device_info.mac_address.v[5];
*/



//##########################################################################
//##### TO SETUP A TCP CLIENT CONNECTION TO A REMOTE TCP SERVER SOCKET #####
//##########################################################################
/*
	static BYTE our_tcp_client_socket = TCP_INVALID_SOCKET;
	static WORD our_tcp_client_local_port;
	static BYTE our_tcp_client_state = SM_OPEN_SOCKET;
	static DEVICE_INFO remote_device_info;
	BYTE data;
	BYTE array_buffer[4];


	if (!nic_linked_and_ip_address_valid)
	{
		//----- WE ARE NOT CONNECTED OR DO NOT YET HAVE AN IP ADDRESS -----
		our_tcp_client_state = SM_OPEN_SOCKET;
		
		//Ensure our socket is closed if we have just lost the Ethernet connection
		tcp_close_socket(our_tcp_client_socket);
		
		return;										//Exit as we can't do anything without a connection
	}

	if (our_tcp_client_socket != TCP_INVALID_SOCKET)
	{
		//----- CHECK OUR CLIENT SOCKET HASN'T DISCONNECTED -----
		if ((tcp_socket[our_tcp_client_socket].sm_socket_state == SM_TCP_CLOSED) || (tcp_socket[our_tcp_client_socket].local_port != our_tcp_client_local_port))		//If the local port has changed then our socket was closed and taken by some other application process since we we're last called
			our_tcp_client_socket = TCP_INVALID_SOCKET;

		if (our_tcp_client_state == SM_WAIT_FOR_DISCONNECT)
			our_tcp_client_state = SM_OPEN_SOCKET;
		else if (our_tcp_client_state != SM_OPEN_SOCKET)
			our_tcp_client_state = SM_COMMS_FAILED;		
	}


	switch (our_tcp_client_state)
	{
	case SM_OPEN_SOCKET:	
		//----- OPEN SOCKET -----
		remote_device_info.ip_address.v[0] = 192;		//The IP address of the remote device we want to connect to (change as required)
		remote_device_info.ip_address.v[1] = 168;
		remote_device_info.ip_address.v[2] = 0;
		remote_device_info.ip_address.v[3] = 20;
		remote_device_info.mac_address.v[0] = 0;			//Set to zero so TCP driver will automatically use ARP to find MAC address
		remote_device_info.mac_address.v[1] = 0;
		remote_device_info.mac_address.v[2] = 0;
		remote_device_info.mac_address.v[3] = 0;
		remote_device_info.mac_address.v[4] = 0;
		remote_device_info.mac_address.v[5] = 0;

		if (our_tcp_client_socket != TCP_INVALID_SOCKET)			//We shouldn't have a socket currently, but make sure
			tcp_close_socket(our_tcp_client_socket);
			
		our_tcp_client_socket = tcp_connect_socket(&remote_device_info, 4102);		//Connect to remote device port 4102 (the port it is listening on - change as required)
		if (our_tcp_client_socket != TCP_INVALID_SOCKET)
		{
			our_tcp_client_local_port = tcp_socket[our_tcp_client_socket].local_port;

			tcp_client_socket_timeout_timer = 10;			//Set our wait for connection timeout
			our_tcp_client_state = SM_WAIT_FOR_CONNECTION;
			break;
		}
		//Could not open a socket - none currently available - keep trying
		
		break;

	case SM_WAIT_FOR_CONNECTION:
		//----- WAIT FOR SOCKET TO CONNECT -----
		if (tcp_is_socket_connected(our_tcp_client_socket));
			our_tcp_client_state = SM_TX_PACKET;

		if (tcp_client_socket_timeout_timer == 0)
		{
			//CONNECTION REQUEST FAILED
			our_tcp_client_state = SM_COMMS_FAILED;
		}
		
		break;

	case SM_TX_PACKET:
		//----- TX PACKET TO REMOTE DEVICE -----
		if (!tcp_setup_socket_tx(our_tcp_client_socket))
		{
			//Can't tx right now - try again next time
			break;
		}

		//WRITE THE TCP DATA
		tcp_write_next_byte('H');
		tcp_write_next_byte('e');
		tcp_write_next_byte('l');
		tcp_write_next_byte('l');
		tcp_write_next_byte('o');
		tcp_write_next_byte(' ');
		tcp_write_next_byte('W');
		tcp_write_next_byte('o');
		tcp_write_next_byte('r');
		tcp_write_next_byte('l');
		tcp_write_next_byte('d');
		tcp_write_next_byte(0x00);
		//You can also use tcp_write_array()

		//SEND THE PACKET
		tcp_socket_tx_packet(our_tcp_client_socket);

		tcp_client_socket_timeout_timer = 10;			//Set our wait for response timeout
		our_tcp_client_state = SM_WAIT_FOR_RESPONSE;
		break;

	case SM_WAIT_FOR_RESPONSE:
		//----- WAIT FOR RESPONSE -----

		if (tcp_client_socket_timeout_timer == 0)
		{
			//WAIT FOR RESPOSNE TIMEOUT
			tcp_close_socket(our_tcp_client_socket);
			our_tcp_client_state = SM_COMMS_FAILED;
		}

		if (tcp_check_socket_for_rx(our_tcp_client_socket))
		{
			//SOCKET HAS RECEIVED A PACKET - PROCESS IT

			//READ THE PACKET AS REQURIED
			if (tcp_read_next_rx_byte(&data) == 0)
			{
				//Error - no more bytes in rx packet
			}
			//OR USE
			if (tcp_read_rx_array (array_buffer, sizeof(array_buffer)) == 0)
			{
				//Error - no more bytes in rx packet
			}

			//DUMP THE PACKET
			tcp_dump_rx_packet();

			our_tcp_client_state = SM_REQUEST_DISCONNECT;
		}

		if (tcp_does_socket_require_resend_of_last_packet(our_tcp_client_socket))
		{
			//RE-SEND LAST PACKET TRANSMITTED
			//(TCP requires resending of packets if they are not acknowledged and to
			//avoid requiring a large RAM buffer the application needs to remember
			//the last packet sent on a socket so it can be resent if requried).
			our_tcp_client_state = SM_TX_PACKET;
		}

		if(!tcp_is_socket_connected(our_tcp_client_socket))
		{
			//THE CLIENT HAS DISCONNECTED
			our_tcp_client_state = SM_COMMS_FAILED;
		}
		
		break;

	case SM_REQUEST_DISCONNECT:
		//----- REQUEST TO DISCONNECT FROM REMOTE SERVER -----
		tcp_request_disconnect_socket (our_tcp_client_socket);

		tcp_client_socket_timeout_timer = 10;			//Set our wait for disconnect timeout
		our_tcp_client_state = SM_WAIT_FOR_DISCONNECT;
		break;

	case SM_WAIT_FOR_DISCONNECT:
		//----- WAIT FOR SOCKET TO BE DISCONNECTED -----

		if (tcp_is_socket_closed(our_tcp_client_socket))
		{
			our_tcp_client_state = SM_COMMS_COMPLETE;
		}

		if (tcp_client_socket_timeout_timer == 0)
		{
			//WAIT FOR DISCONNECT TIMEOUT
			tcp_close_socket(our_tcp_client_socket);	//Force the socket closed at our end
			our_tcp_client_state = SM_COMMS_FAILED;
		}
		break;

	case SM_COMMS_COMPLETE:
		//----- COMMUNICATIONS COMPLETE -----
	
		break;

	case SM_COMMS_FAILED:
		//----- COMMUNICATIONS FAILED -----
	
		break;

	}
*/


//For further information please see the project technical manual



//*****************************
//*****************************
//********** DEFINES **********
//*****************************
//*****************************
#ifndef TCP_C_INIT		//Do only once the first time this file is used
#define	TCP_C_INIT

#include "eth-main.h"

//----- TCP SETUP -----
#define TCP_NO_OF_AVAILABLE_SOCKETS		6		//<<<<< SETUP FOR A NEW APPLICATION
												//The number of TCP sockets available to the application (each socket uses sizeof(TCP_SOCKET_INFO) bytes of ram)
												//Note that other stack components that use TCP will use sockets from this pool.

#if (TCP_NO_OF_AVAILABLE_SOCKETS <= 0 || TCP_NO_OF_AVAILABLE_SOCKETS > 255 )
#error TCP_NO_OF_AVAILABLE_SOCKETS value is out of range!
#endif

#define TCP_INITIAL_TIMEOUT_VALUE_10MS		(DWORD)300		//Initial re-transmission timeout time, doubled after each timeout to provide a good compromise between local network and internet communications)
#define TCP_MAX_NUM_OF_RETRIES				3				//The number of retries to attempt before closing a TCP connection
#define	TCP_MAX_TIMEOUT_VALUE_10MS			(DWORD)3000		//The maximum timeout value we should use

#define	TCP_USE_SOCKET_INACTIVITY_TIMOUT			//Comment out to allow a socket to be opened and remain open indefinately even if there is no activity

#define MAX_TCP_DATA_LEN    		(1500 - 40)		//Maximum size of the tcp packet data area (-40 allows for 20 byte tcp header and 20 byte ip header)
#define	TCP_MAX_SEGMENT_SIZE		(1500 - 40)		//Maximum size of tcp packet data area this device is willing to accept.  We don't use a variable window
													//size as we're not buffering to memory.  We use a fixed window size of 1 packet to avoid a remote device
													//sending lots of packets which we can't process fast enough and therefore get lost, to then be re-sent by
													//the remote device after a delay.  -40 allows for 20 byte TCP header and 20 byte IP header
//TCP HEADER FLAGS
#define TCP_FIN						0x01
#define TCP_SYN						0x02
#define TCP_RST						0x04
#define TCP_PSH						0x08			//We don't use PSH as its use isn't properly defined anyway
#define TCP_ACK						0x10
#define TCP_URG						0x20			//We don't use URG, but it could easily be used if requried by an application

//TCP OPTIONS
#define TCP_OPTIONS_MAX_SEG_SIZE	0x02

#define TCP_LOCAL_PORT_NUMBER_START	1024			//Port number range available for automatic use as the local TCP port.
#define TCP_LOCAL_PORT_NUMBER_END	5000

#define TCP_INVALID_SOCKET			0xfe
#define TCP_INVALID_TX_BUFFER		0xff



//----- TCP STATE MACHINE STATES FOR EACH SOCKET -----
typedef enum _TCP_STATE
{
	SM_TCP_CLOSED,
	SM_TCP_LISTEN,
	SM_TCP_SYN_REQUEST_RECEIVED,
	SM_TCP_CONNECT_SEND_ARP_REQUEST,
	SM_TCP_CONNECT_WAIT_ARP_RESPONSE,
	SM_TCP_SYN_REQUEST_SENT,
	SM_TCP_CONNECTION_ESTABLISHED,
	SM_TCP_FIN_WAIT_1,
	SM_TCP_CLOSING,
	SM_TCP_LAST_ACK_SENT
} TCP_STATE;



//----- TCP SOCKET STRUCTURE -----
typedef struct _TCP_SOCKET_INFO
{
	BYTE sm_socket_state;
	DEVICE_INFO remote_device_info;
	WORD local_port;
	WORD remote_port;
	BYTE tx_last_tx_flags;
	DWORD send_sequence_number;
	DWORD send_acknowledgement_number;
	WORD next_segment_sequence_increment_value;
	BYTE retry_count;
	WORD rx_data_bytes_remaining;
	BYTE waiting_command_flags;
	DWORD start_time;
	DWORD time_out_value;
	struct
	{
		BYTE ready_for_tx				:1;
		BYTE tx_last_tx_awaiting_ack	:1;
		BYTE tx_last_tx_had_data		:1;
		BYTE tx_resend_last_tx			:1;			//If set the application function that owns the socket needs to re-send the last transmission
		BYTE tx_send_waiting_command	:1;
		BYTE socket_is_server			:1;
	} flags;

} TCP_SOCKET_INFO;



//----- TCP PACKET HEADER -----
typedef struct _TCP_HEADER
{
	WORD source_port;
	WORD destination_port;
	DWORD sequence_number;
	DWORD acknowledgment_number;

	union
	{
		struct
		{
			BYTE reserved		:4;
			BYTE val			:4;
		} bits;
		BYTE byte;
	} header_length;

	union
	{
		struct
		{
			BYTE flag_fin	:1;
			BYTE flag_syn	:1;
			BYTE flag_rst	:1;
			BYTE flag_psh	:1;
			BYTE flag_ack	:1;
			BYTE flag_urg	:1;
			BYTE reserved	:2;
		} bits;
		BYTE byte;
	} flags;

	WORD window;
	WORD checksum;
	WORD urgent_pointer;
} TCP_HEADER;
#define	TCP_HEADER_LENGTH			20			//Defined to avoid sizeof problemms with compilers that add padd bytes


typedef struct _TCP_OPTIONS
{
	BYTE id;
	BYTE length;
	WORD_VAL max_seg_size;
} TCP_OPTIONS;
#define	TCP_OPTIONS_LENGTH			4			//Defined to avoid sizeof problemms with compilers that add padd bytes


//PSEUDO HEADER AS DEFINED BY RFC793
typedef struct _PSEUDO_HEADER
{
	IP_ADDR source_address;
	IP_ADDR destination_address;
	BYTE zero;
	BYTE protocol;
	WORD tcp_length;
} PSEUDO_HEADER;
#define	PSEUDO_HEADER_LENGTH		12			//Defined to avoid sizeof problemms with compilers that add padd bytes

#endif





//*******************************
//*******************************
//********** FUNCTIONS **********
//*******************************
//*******************************
#ifdef TCP_C
//-----------------------------------
//----- INTERNAL ONLY FUNCTIONS -----
//-----------------------------------
static BYTE tcp_rx_check_for_matches_socket (TCP_HEADER *rx_tcp_header, DEVICE_INFO *rx_device_info);
static BYTE process_tcp_segment(BYTE socket, DEVICE_INFO *remote_device_info, TCP_HEADER *tcp_header,  WORD tcp_data_length);
void tcp_send_command_packet_no_socket (DEVICE_INFO *remote_device_info, WORD local_port, WORD remote_port, DWORD tx_sequence_number,	DWORD tx_acknowledgment_number, BYTE flags);
void tcp_send_command_packet_from_socket (BYTE socket, BYTE flags);
static void swap_tcp_header(TCP_HEADER* header);


//-----------------------------------------
//----- INTERNAL & EXTERNAL FUNCTIONS -----
//-----------------------------------------
//(Also defined below as extern)
void tcp_initialise (void);
BYTE tcp_process_rx (DEVICE_INFO *sender_device_info, IP_ADDR *destination_ip_address, WORD ip_data_area_bytes);
void tcp_request_disconnect_socket (BYTE socket);
BYTE tcp_open_socket_to_listen (WORD port);
BYTE tcp_connect_socket(DEVICE_INFO *remote_device_info, WORD port);
void tcp_connect_socket_send_syn_request (BYTE socket);
void tcp_close_socket_from_listen (BYTE socket);
void tcp_close_socket(BYTE socket);
BYTE tcp_is_socket_closed (BYTE socket);
BYTE tcp_is_socket_connected (BYTE socket);
BYTE tcp_is_socket_ready_to_tx_new_packet (BYTE socket);
BYTE tcp_setup_socket_tx (BYTE socket);
BYTE tcp_setup_tx (DEVICE_INFO *remote_device_info, WORD local_port, WORD remote_port, DWORD tx_sequence_number, DWORD tx_acknowledgment_number, BYTE tx_flags);
BYTE tcp_write_next_byte (BYTE data);
BYTE tcp_write_array (BYTE *array_buffer, WORD array_length);
void tcp_socket_tx_packet (BYTE socket);
void tcp_tx_packet (void);
BYTE tcp_does_socket_require_resend_of_last_packet (BYTE socket);
BYTE tcp_check_socket_for_rx (BYTE socket);
BYTE tcp_read_next_rx_byte (BYTE *data);
BYTE tcp_read_rx_array (BYTE *array_buffer, WORD array_length);
void tcp_dump_rx_packet (void);
void process_tcp (void);


#else
//------------------------------
//----- EXTERNAL FUNCTIONS -----
//------------------------------
extern void tcp_initialise (void);
extern BYTE tcp_process_rx (DEVICE_INFO *sender_device_info, IP_ADDR *destination_ip_address, WORD ip_data_area_bytes);
extern void tcp_request_disconnect_socket (BYTE socket);
extern BYTE tcp_open_socket_to_listen (WORD port);
extern BYTE tcp_connect_socket(DEVICE_INFO *remote_device_info, WORD port);
extern void tcp_connect_socket_send_syn_request (BYTE socket);
extern void tcp_close_socket_from_listen (BYTE socket);
extern void tcp_close_socket(BYTE socket);
extern BYTE tcp_is_socket_closed (BYTE socket);
extern BYTE tcp_is_socket_connected (BYTE socket);
extern BYTE tcp_is_socket_ready_to_tx_new_packet (BYTE socket);
extern BYTE tcp_setup_socket_tx (BYTE socket);
extern BYTE tcp_setup_tx (DEVICE_INFO *remote_device_info, WORD local_port, WORD remote_port, DWORD tx_sequence_number, DWORD tx_acknowledgment_number, BYTE tx_flags);
extern BYTE tcp_write_next_byte (BYTE data);
extern BYTE tcp_write_array (BYTE *array_buffer, WORD array_length);
extern void tcp_socket_tx_packet (BYTE socket);
extern void tcp_tx_packet (void);
extern BYTE tcp_does_socket_require_resend_of_last_packet (BYTE socket);
extern BYTE tcp_check_socket_for_rx (BYTE socket);
extern BYTE tcp_read_next_rx_byte (BYTE *data);
extern BYTE tcp_read_rx_array (BYTE *array_buffer, WORD array_length);
extern void tcp_dump_rx_packet (void);
extern void process_tcp (void);


#endif




//****************************
//****************************
//********** MEMORY **********
//****************************
//****************************
#ifdef TCP_C
//--------------------------------------------
//----- INTERNAL ONLY MEMORY DEFINITIONS -----
//--------------------------------------------
WORD next_local_tcp_port_to_use;
DWORD next_connection_start_sequence_number;
WORD tcp_tx_checksum;
BYTE tcp_tx_checksum_next_byte_low;
WORD tcp_tx_data_byte_length;
BYTE tcp_tx_resp_after_proc_rx_socket;
BYTE tcp_tx_resp_after_proc_rx_resp_flags;


//--------------------------------------------------
//----- INTERNAL & EXTERNAL MEMORY DEFINITIONS -----
//--------------------------------------------------
//(Also defined below as extern)
BYTE tcp_rx_packet_is_waiting_to_be_processed;
TCP_SOCKET_INFO tcp_socket[TCP_NO_OF_AVAILABLE_SOCKETS];
BYTE tcp_rx_active_socket;


#else
//---------------------------------------
//----- EXTERNAL MEMORY DEFINITIONS -----
//---------------------------------------
extern BYTE tcp_rx_packet_is_waiting_to_be_processed;
extern TCP_SOCKET_INFO tcp_socket[TCP_NO_OF_AVAILABLE_SOCKETS];
extern BYTE tcp_rx_active_socket;


#endif




