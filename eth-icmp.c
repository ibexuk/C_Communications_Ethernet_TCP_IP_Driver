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
//ICMP (INTERNET CONTROL MESSAGE PROTOCOL) C CODE FILE


#include "main.h"					//Global data type definitions (see https://github.com/ibexuk/C_Generic_Header_File )
#define ICMP_C
#include "eth-icmp.h"
#undef ICMP_C

#include "eth-main.h"
#include "eth-nic.h"
#include "eth-ip.h"


//--------------------------------------------------------------------------
//----- GENERATE COMPILE ERROR IF ICMP HAS NOT BEEN DEFINED TO BE USED -----
//--------------------------------------------------------------------------
#ifndef STACK_USE_ICMP
#error ICMP file is included in project but not defined to be used - remove file from project to reduce code size.
#endif






//***************************************************************
//***************************************************************
//********** PROCESS RECEIVED ICMP ECHO REQUEST PACKET **********
//***************************************************************
//***************************************************************
//Returns 1 if a valid ICMP ECHO request packet is received, 0 if not
BYTE icmp_process_received_echo_request (WORD *icmp_id, WORD *icmp_sequence, BYTE *icmp_data_buffer, WORD data_remaining_bytes)
{
	ICMP_HEADER icmp_header;
	WORD received_checksum;
	WORD calculated_checksum;
	BYTE calculate_checksum_next_byte_is_low;


	//----- GET THE ICMP HEADER -----
	//if (nic_read_array((BYTE*)&icmp_header, ICMP_HEADER_LENGTH) == 0)
	//	return (0);								//Error - packet was too small - dump

	if (!nic_read_next_byte(&icmp_header.type))
		return(1);									//Error - packet was too small - dump it

	if (!nic_read_next_byte(&icmp_header.code))
		return(1);									//Error - packet was too small - dump it

	if (!nic_read_array((BYTE*)&icmp_header.checksum, 2))
		return(1);									//Error - packet was too small - dump it

	if (!nic_read_array((BYTE*)&icmp_header.identifier, 2))
		return(1);									//Error - packet was too small - dump it

	if (!nic_read_array((BYTE*)&icmp_header.sequence_number, 2))
		return(1);									//Error - packet was too small - dump it


	//----- GET THE ICMP DATA -----
	if (nic_read_array((BYTE*)icmp_data_buffer, (data_remaining_bytes - ICMP_HEADER_LENGTH)) == 0)
		return (0);								//Error - packet was too small - dump

	//----- CALCULATE THE CHECKSUM AND CHECK IT MATCHES -----
	//Store the received checksum and then set to zero in the header ready for checksum calculation
    received_checksum = icmp_header.checksum;
    icmp_header.checksum = 0;

	//Calculate the checksum for the header
	calculated_checksum = 0;
	calculate_checksum_next_byte_is_low = 0;
	//ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculate_checksum_next_byte_is_low, (BYTE*)&icmp_header, ICMP_HEADER_LENGTH);
    ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculate_checksum_next_byte_is_low, (BYTE*)&icmp_header.type, 1);
    ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculate_checksum_next_byte_is_low, (BYTE*)&icmp_header.code, 1);
    ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculate_checksum_next_byte_is_low, (BYTE*)&icmp_header.checksum, 2);
    ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculate_checksum_next_byte_is_low, (BYTE*)&icmp_header.identifier, 2);
    ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculate_checksum_next_byte_is_low, (BYTE*)&icmp_header.sequence_number, 2);

	//Calculate the checksum for the data
	ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculate_checksum_next_byte_is_low, (BYTE*)icmp_data_buffer, (data_remaining_bytes - ICMP_HEADER_LENGTH));
	
	//Ensure checksums match
	received_checksum = ~received_checksum;
	received_checksum = swap_word_bytes(received_checksum);

	if (received_checksum != calculated_checksum)
		return(0);

	//SWAP IDENTIFIER, SEQ NUMBER & CHECKSUM WORDS
	icmp_header.identifier = swap_word_bytes(icmp_header.identifier);
	icmp_header.sequence_number = swap_word_bytes(icmp_header.sequence_number);
	//icmp_header.checksum = swap_word_bytes(icmp_header.checksum);		//Don't need to

	//----- CHECK CODE IS ECHO REQUEST -----
	if (icmp_header.type != ICMP_ECHO_REQUEST)
		return(0);


	//STORE THE RETURN VALUES TO BE USED IN THE REPLY
	*icmp_id = icmp_header.identifier;
	*icmp_sequence = icmp_header.sequence_number;

	return(1);
}


//**************************************
//**************************************
//********** SEND ICMP PACKET **********
//**************************************
//**************************************
//nic_setup_tx() must have been called first
void icmp_send_packet(DEVICE_INFO *remote_device_info,BYTE icmp_packet_type, BYTE *data_buffer, WORD data_length, WORD *icmp_id, WORD *icmp_sequence)
{
	ICMP_HEADER icmp_header;
	WORD calculated_checksum;
	BYTE calculated_checksum_next_byte_is_low;


	//----- SETUP THE HEADER -----
	icmp_header.type = icmp_packet_type;
	icmp_header.code = 0;
	icmp_header.checksum = 0;
	icmp_header.identifier = *icmp_id;
	icmp_header.sequence_number = *icmp_sequence;

	//SWAP IDENTIFIER, SEQ NUMBER & CHECKSUM WORDS
	icmp_header.identifier = swap_word_bytes(icmp_header.identifier);
	icmp_header.sequence_number = swap_word_bytes(icmp_header.sequence_number);
	//icmp_header.checksum = swap_word_bytes(icmp_header.checksum);		//Done later


	//----- CALCULATE THE HEADER CHECKSUM -----
	calculated_checksum = 0;
	calculated_checksum_next_byte_is_low = 0;
	//ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (BYTE*)&icmp_header, ICMP_HEADER_LENGTH);
    ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (BYTE*)&icmp_header.type, 1);
    ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (BYTE*)&icmp_header.code, 1);
    ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (BYTE*)&icmp_header.checksum, 2);
    ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (BYTE*)&icmp_header.identifier, 2);
    ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (BYTE*)&icmp_header.sequence_number, 2);

	//Add the data buffer to the checksum
	ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (BYTE*)data_buffer, data_length);

	//Add the calculated checksum to the header
	icmp_header.checksum = swap_word_bytes(~calculated_checksum);


	//----- WRITE THE IP HEADER -----
	ip_write_header(remote_device_info, IP_PROTOCOL_ICMP);


	//----- WRITE THE ICMP HEADER -----
	//nic_write_array((BYTE*)&icmp_header, ICMP_HEADER_LENGTH);
    nic_write_next_byte(icmp_header.type);
    nic_write_next_byte(icmp_header.code);
    nic_write_array((BYTE*)&icmp_header.checksum, 2);
    nic_write_array((BYTE*)&icmp_header.identifier, 2);
    nic_write_array((BYTE*)&icmp_header.sequence_number, 2);


	//----- WRITE THE ICMP DATA -----
	nic_write_array(data_buffer, data_length);


	//----- SEND THE PACKET -----
	ip_tx_packet();
}






