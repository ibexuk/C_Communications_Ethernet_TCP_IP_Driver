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
//IP (INTERNET PROTOCOL) C CODE HEADER FILE



//##################################
//##################################
//########## USING DRIVER ##########
//##################################
//##################################


//###################################
//##### TO RECEIVE AN IP PACKET #####
//###################################
//Reception of IP packets is automatically dealt with by the stack, with the different packet types then handled as required


//####################################
//##### TO TRANSMIT AN IP PACKET #####
//####################################
//Typically you will not need to do this directly, but will instead be calling other stack functions that will in turn automatically call
//these IP functions.  However if you want to genreate your own IP packet:
/*
DEVICE_INFO *remote_device_info

	//setup transmit
	if (!nic_ok_to_do_tx())		//Exit if nic is not currently able to send a new packet
		return(0);

	if (!nic_setup_tx())		//Setup the nic ready to tx a new packet
		return(0);				//Nic is not ready currently to tranmit a new packet

	//WRITE THE IP HEADER
	ip_write_header(remote_device_info, ip_protocol);
	
	//WRITE THE DATA
	//Write each byte of the packet using:
	nic_write_next_byte(0x00);
	//or
	nic_write_array (my_array, sizeof(my_array))

	//TRANSMIT THE PACKET
	ip_tx_packet();

//Note:
//When transmitting a packet the total packet length does not need to be known - the length is automatically calculated by the functions
//and written when the ip_tx_packet function is called.
//The IP checksum only includes the IP header, not the data area.  Checksumming of the data area is dealt with by the higher level
//protocols that use the data area (e.g. ICMP, UDP, TCP)
*/

//For further information please see the project technical manual






//*****************************
//*****************************
//********** DEFINES **********
//*****************************
//*****************************
#ifndef IP_C_INIT		//(Do only once)
#define	IP_C_INIT


#include "eth-main.h"


#define IP_PROTOCOL_ICMP			1
#define IP_PROTOCOL_TCP				6
#define IP_PROTOCOL_UDP				17

#define IP_VERSION					0x04

#define MAX_IP_OPTIONS_LENGTH		20

#define IP_TYPE_OF_SERVICE_STD		0

#define	IP_DEFAULT_TIME_TO_LIVE		100


//----- DATA TYPE DEFINITIONS -----
typedef struct _IP_HEADER
{
	BYTE		version_header_length;
	BYTE		type_of_service;
	WORD		length;
	WORD		ident;
	WORD		flags;
	BYTE		time_to_live;
	BYTE		protocol;			//(ICMP=1, TCP=6, EGP=8, UDP=17)
	WORD		header_checksum;
	IP_ADDR		source_ip_address;
	IP_ADDR		destination_ip_address;
} IP_HEADER;
#define	IP_HEADER_LENGTH			20			//Defined to avoid sizeof problemms with compilers that add padd bytes

#endif



//*******************************
//*******************************
//********** FUNCTIONS **********
//*******************************
//*******************************
#ifdef IP_C				//(Defined only by associated C file)
//-----------------------------------
//----- INTERNAL ONLY FUNCTIONS -----
//-----------------------------------


//-----------------------------------------
//----- INTERNAL & EXTERNAL FUNCTIONS -----
//-----------------------------------------
//(Also defined below as extern)
BYTE ip_get_header(IP_ADDR *destination_ip, DEVICE_INFO *remote_device_info, BYTE *ip_protocol, WORD *length);
void ip_write_header(DEVICE_INFO *remote_device_info, BYTE ip_protocol);
void ip_add_bytes_to_ip_checksum (WORD *checksum, BYTE *checksum_next_byte_is_low, BYTE *next_byte, BYTE no_of_bytes_to_add);
void ip_tx_packet (void);


#else
//------------------------------
//----- EXTERNAL FUNCTIONS -----
//------------------------------
extern BYTE ip_get_header(IP_ADDR *destination_ip, DEVICE_INFO *remote_device_info, BYTE *ip_protocol, WORD *length);
extern void ip_write_header(DEVICE_INFO *remote_device_info, BYTE ip_protocol);
extern void ip_add_bytes_to_ip_checksum (WORD *checksum, BYTE *checksum_next_byte_is_low, BYTE *next_byte, BYTE no_of_bytes_to_add);
extern void ip_tx_packet (void);


#endif




//****************************
//****************************
//********** MEMORY **********
//****************************
//****************************
#ifdef IP_C				//(Defined only by associated C file)
//--------------------------------------------
//----- INTERNAL ONLY MEMORY DEFINITIONS -----
//--------------------------------------------
WORD ip_packet_identifier = 0;
WORD ip_tx_ip_header_checksum;
BYTE ip_tx_ip_header_checksum_next_byte_low;


//--------------------------------------------------
//----- INTERNAL & EXTERNAL MEMORY DEFINITIONS -----
//--------------------------------------------------
//(Also defined below as extern)


#else
//---------------------------------------
//----- EXTERNAL MEMORY DEFINITIONS -----
//---------------------------------------


#endif










