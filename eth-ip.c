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
//IP (INTERNET PROTOCOL) C CODE FILE


#include "main.h"					//Global data type definitions (see https://github.com/ibexuk/C_Generic_Header_File )
#define IP_C
#include "eth-ip.h"
#undef IP_C

#include "eth-main.h"
#include "eth-nic.h"

#ifdef STACK_USE_DHCP
#include "eth-dhcp.h"
#endif





//***********************************
//***********************************
//********** GET IP HEADER **********
//***********************************
//***********************************
//Returns 1 if IP header OK, 0 if invalid packet (packet will have been dumped)
//destination_ip = the ip address this packet was sent to (checking for part of our subnet is done by this function)
//remote_device_info = the IP address field is loaded (mac address field is not altered)
//ip_protocol = the ip protocol byte
//length = the length of the data area of this packet

BYTE ip_get_header(IP_ADDR *destination_ip, DEVICE_INFO *remote_device_info, BYTE *ip_protocol, WORD *length)
{
	IP_HEADER ip_header;
	BYTE options_length;
	BYTE header_options_bytes[MAX_IP_OPTIONS_LENGTH];
	WORD received_checksum;
	WORD calculated_checksum;
	BYTE calculated_checksum_next_byte_is_low;


	//----- GET THE IP HEADER -----
	//if (nic_read_array((BYTE*)&ip_header, IP_HEADER_LENGTH) == 0)
	//	goto ip_get_header_dump_packet;							//Error - packet was too small - dump

	if (!nic_read_next_byte(&ip_header.version_header_length))
		return(1);												//Error - packet was too small - dump it
	if (!nic_read_next_byte(&ip_header.type_of_service))
		return(1);												//Error - packet was too small - dump it
	if (!(nic_read_array((BYTE*)&ip_header.length, 2)))
		goto ip_get_header_dump_packet;							//Error - packet was too small - dump
	if (!(nic_read_array((BYTE*)&ip_header.ident, 2)))
		goto ip_get_header_dump_packet;							//Error - packet was too small - dump
	if (!(nic_read_array((BYTE*)&ip_header.flags, 2)))
		goto ip_get_header_dump_packet;							//Error - packet was too small - dump
	if (!nic_read_next_byte(&ip_header.time_to_live))
		return(1);												//Error - packet was too small - dump it
	if (!nic_read_next_byte(&ip_header.protocol))
		return(1);												//Error - packet was too small - dump it
	if (!(nic_read_array((BYTE*)&ip_header.header_checksum, 2)))
		goto ip_get_header_dump_packet;							//Error - packet was too small - dump
	if (!(nic_read_array((BYTE*)&ip_header.source_ip_address, 4)))
		goto ip_get_header_dump_packet;							//Error - packet was too small - dump
	if (!(nic_read_array((BYTE*)&ip_header.destination_ip_address, 4)))
		goto ip_get_header_dump_packet;							//Error - packet was too small - dump


	//IP Version should be V4
	if ((ip_header.version_header_length & 0xf0) != (IP_VERSION << 4))
		goto ip_get_header_dump_packet;

	//Get the options length (if there are any)
	options_length = ((ip_header.version_header_length & 0x0f) << 2) - IP_HEADER_LENGTH;		// x4 as the length field is in DWORDS

	//Read the options bytes if there are any, checking for maximum we can accept
	if (options_length > MAX_IP_OPTIONS_LENGTH)
		goto ip_get_header_dump_packet;

	if (options_length > 0)
	{
		//Get the option bytes
		if (nic_read_array((BYTE*)&header_options_bytes, options_length) == 0)
			goto ip_get_header_dump_packet;
	}


	//----- CALCULATE THE HEADER CHECKSUM AND CHECK IT MATCHES -----
	calculated_checksum = 0;
	calculated_checksum_next_byte_is_low = 0;

	//Store the received checksum and then set to zero in the header ready for checksum calculation
	received_checksum = ip_header.header_checksum;
	ip_header.header_checksum = 0;

	//Get checksum of IP header
	//ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (BYTE*)&ip_header, IP_HEADER_LENGTH);
	ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (BYTE*)&ip_header.version_header_length, 1);
	ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (BYTE*)&ip_header.type_of_service, 1);
	ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (BYTE*)&ip_header.length, 2);
	ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (BYTE*)&ip_header.ident, 2);
	ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (BYTE*)&ip_header.flags, 2);
	ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (BYTE*)&ip_header.time_to_live, 1);
	ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (BYTE*)&ip_header.protocol, 1);
	ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (BYTE*)&ip_header.header_checksum, 2);
	ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (BYTE*)&ip_header.source_ip_address, 4);
	ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (BYTE*)&ip_header.destination_ip_address, 4);


	//Add checksum of any options bytes
    if (options_length > 0)
    {
    	ip_add_bytes_to_ip_checksum (&calculated_checksum, &calculated_checksum_next_byte_is_low, (BYTE*)&header_options_bytes, options_length);
    }	

	//Ensure checksums match
	received_checksum = ~received_checksum;
	received_checksum = swap_word_bytes(received_checksum);

	if (received_checksum != calculated_checksum)
		goto ip_get_header_dump_packet;

	//SWAP LENGTH, IDENT & CHECKSUM WORDS
	ip_header.length = swap_word_bytes(ip_header.length);
	//ip_header.ident = swap_word_bytes(ip_header.ident);						//No need
	//ip_header.header_checksum = swap_word_bytes(ip_header.header_checksum);	//No need
	

	//----- CHECK THE DESTINATION IP ADDRESS MATCHES OURS OR IS BROADCAST ON OUR SUBNET -----
	if (
			#ifdef STACK_USE_DHCP
			(((sm_dhcp == DHCP_WAIT_FOR_OFFER_RESPONSE) || (sm_dhcp == DHCP_WAIT_FOR_REQUEST_RESPONSE)) && (our_ip_address.Val == 0)) ||	//Doing DHCP discover and we don't currently have an IP address?
			#endif
			(ip_header.destination_ip_address.Val == our_ip_address.Val) ||								//Our IP address?
			(
				((ip_header.destination_ip_address.Val & our_subnet_mask.Val) == (our_ip_address.Val & our_subnet_mask.Val)) &&
				((ip_header.destination_ip_address.Val & ~our_subnet_mask.Val) == ~our_subnet_mask.Val)		//Broadcast on our subnet?
			) ||
			(ip_header.destination_ip_address.Val == 0xffffffff)										//Broadcast?
		)
	{
		//DESTINATION IP IS OK
	}
	else
	{
		//DESTINATION IP IS NOT VALID FOR US
		goto ip_get_header_dump_packet;
	}


	//----- LOAD THE CALLERS REGISTERS WITH THE PACKET INFO -----
	//Store the destination IP address
	destination_ip->Val = ip_header.destination_ip_address.Val;
	
	//Store the remote device IP address
	remote_device_info->ip_address.Val = ip_header.source_ip_address.Val;

	//Store the protocol
	*ip_protocol = ip_header.protocol;
	
	//Store the length of the data area of the packet
	*length = (ip_header.length - options_length - IP_HEADER_LENGTH);

	return (1);



ip_get_header_dump_packet:
	//----------------------------------
	//----- BAD HEADER DUMP PACKET -----
	//----------------------------------
	nic_rx_dump_packet();
	return(0);
}





//*************************************
//*************************************
//********** WRITE IP HEADER **********
//*************************************
//*************************************
//The Ethernet header is also written, before the IP header
void ip_write_header(DEVICE_INFO *remote_device_info, BYTE ip_protocol)
{
	IP_HEADER ip_header;


	//----- SETUP THE IP HEADER -----
	ip_header.version_header_length = (IP_VERSION << 4) | (IP_HEADER_LENGTH >> 2);		//Header length is in DWORDs
	ip_header.type_of_service = IP_TYPE_OF_SERVICE_STD;
	ip_header.length = 0;									//This will be written before packet is sent (header length + data length)
	ip_header.ident = ++ip_packet_identifier;				//Incrementing packet ID number)
	ip_header.flags = 0;
	ip_header.time_to_live = IP_DEFAULT_TIME_TO_LIVE;
	ip_header.protocol = ip_protocol;
	ip_header.header_checksum = 0;							//The real value will be written before packet is sent as until the length is known we can't do the checksum
	ip_header.source_ip_address.Val = our_ip_address.Val;
	ip_header.destination_ip_address.Val = remote_device_info->ip_address.Val;

	//SWAP LENGTH, IDENT & CHECKSUM WORDS
	//ip_header.length = swap_word_bytes(ip_header.length);		//Not needed
	ip_header.ident = swap_word_bytes(ip_header.ident);
	//ip_header.header_checksum = swap_word_bytes(ip_header.header_checksum);	//Done later


	//----- START CALCULATION OF IP HEADER CHECKSUM -----
	ip_tx_ip_header_checksum = 0;
	ip_tx_ip_header_checksum_next_byte_low = 0;
	//ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (BYTE*)&ip_header, IP_HEADER_LENGTH);
    ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (BYTE*)&ip_header.version_header_length, 1);
    ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (BYTE*)&ip_header.type_of_service, 1);
    ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (BYTE*)&ip_header.length, 2);
    ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (BYTE*)&ip_header.ident, 2);
    ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (BYTE*)&ip_header.flags, 2);
    ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (BYTE*)&ip_header.time_to_live, 1);
    ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (BYTE*)&ip_header.protocol, 1);
    ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (BYTE*)&ip_header.header_checksum, 2);
    ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (BYTE*)&ip_header.source_ip_address, 4);
    ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (BYTE*)&ip_header.destination_ip_address, 4);


	//----- WRITE THE ETHERNET HEADER -----
	write_eth_header_to_nic(&remote_device_info->mac_address, ETHERNET_TYPE_IP);


	//----- WRITE THE IP HEADER -----
	//nic_write_array((BYTE*)&ip_header, IP_HEADER_LENGTH);
    nic_write_next_byte(ip_header.version_header_length);
    nic_write_next_byte(ip_header.type_of_service);
    nic_write_array((BYTE*)&ip_header.length, 2);
    nic_write_array((BYTE*)&ip_header.ident, 2);
    nic_write_array((BYTE*)&ip_header.flags, 2);
    nic_write_next_byte(ip_header.time_to_live);
    nic_write_next_byte(ip_header.protocol);
    nic_write_array((BYTE*)&ip_header.header_checksum, 2);
    nic_write_array((BYTE*)&ip_header.source_ip_address, 4);
    nic_write_array((BYTE*)&ip_header.destination_ip_address, 4);

	
	//----- NOW JUST WRITE THE IP DATA -----

}






//**********************************
//**********************************
//********** IP TX PACKET **********
//**********************************
//**********************************
void ip_tx_packet (void)
{
	WORD ip_length;

	//GET THE IP PACKET LENGTH FOR THE IP HEADER
	ip_length = (nic_tx_len - ETHERNET_HEADER_LENGTH);
	ip_length = swap_word_bytes(ip_length);
	
	//ADD IT TO THE IP HEADER CHECKSUM
	ip_add_bytes_to_ip_checksum (&ip_tx_ip_header_checksum, &ip_tx_ip_header_checksum_next_byte_low, (BYTE*)&ip_length, 2);

	//WRITE THE IP LENGTH FIELD
	nic_write_tx_word_at_location ((ETHERNET_HEADER_LENGTH + 2), ip_length);
	
	//WRITE THE IP CHECKSUM FIELD
	ip_tx_ip_header_checksum = swap_word_bytes(~ip_tx_ip_header_checksum);
	nic_write_tx_word_at_location ((ETHERNET_HEADER_LENGTH + 10), ip_tx_ip_header_checksum);

	//TX THE PACKET
	nix_tx_packet();
}






//**********************************************
//**********************************************
//********** ADD BYTES TO IP CHECKSUM **********
//**********************************************
//**********************************************
//To do before starting a new checksum:-
//checksum = 0;
//checksum_next_byte_is_low = 0;
void ip_add_bytes_to_ip_checksum (WORD *checksum, BYTE *checksum_next_byte_is_low, BYTE *next_byte, BYTE no_of_bytes_to_add)
{
	DWORD dw_temp;
	BYTE count;


	dw_temp = (DWORD)*checksum;
	
	for (count = 0; count < no_of_bytes_to_add; count++)
	{
		if (*checksum_next_byte_is_low)
		{
			dw_temp += (DWORD)next_byte[count];
			*checksum_next_byte_is_low = 0;
		}
		else
		{
			dw_temp += ((DWORD)next_byte[count] << 8);
			*checksum_next_byte_is_low = 1;
		}

		//Do one's complement (overflow bit is added to checksum)
		if (dw_temp > 0x0000ffff)
		{
			dw_temp &= 0x0000ffff;
			dw_temp++;
		}
	}
	*checksum = (WORD)dw_temp;
}




