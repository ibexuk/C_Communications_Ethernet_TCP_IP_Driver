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
//NETBIOS (NETWORK BASIC INPUT/OUTPUT SYSTEM) C CODE FILE


#include "main.h"					//Global data type definitions (see https://github.com/ibexuk/C_Generic_Header_File )
#define	ETH_NETBIOS_C
#include "eth-netbios.h"
#undef	ETH_NETBIOS_C

#include "eth-main.h"
#include "eth-nic.h"
#include "eth-dhcp.h"
#include "eth-udp.h"		//UDP is requried for NETBIOS


//-----------------------------------------------------------------------------
//----- GENERATE COMPILE ERROR IF NETBIOS HAS NOT BEEN DEFINED TO BE USED -----
//-----------------------------------------------------------------------------
#ifndef STACK_USE_NETBIOS
#error NETBIOS file is included in project but not defined to be used - remove file from project to reduce code size.
#endif




//*************************************************
//*************************************************
//********** PROCESS NETBIOS NAMESERVICE **********
//*************************************************
//*************************************************
//This function is called reguarly by tcp_ip_process_stack
void process_netbios_nameservice (void)
{
	static BYTE our_udp_socket = UDP_INVALID_SOCKET;


	if (!nic_linked_and_ip_address_valid)
	{
		//----- WE ARE NOT CONNECTED OR DO NOT YET HAVE AN IP ADDRESS -----
		sm_netbios = SM_NETBIOS_OPEN_SOCKET;
		
		//Ensure our socket is closed if we have just lost the Ethernet connection
		udp_close_socket(&our_udp_socket);
		
		return;										//Exit as we can't do anything without a connection
	}


	switch (sm_netbios)
	{
	case SM_NETBIOS_OPEN_SOCKET:
		//-----------------------
		//----- OPEN SOCKET -----
		//-----------------------
		if (nic_linked_and_ip_address_valid)
		{
			our_udp_socket = udp_open_socket(0x00, NETBIOS_NAMESERVICE_SERVER_PORT, NETBIOS_NAMESERVICE_CLIENT_PORT);		//Leave device_info as null to setup to receive from
																															//anyone, remote_port can be anything for rx
			if (our_udp_socket != UDP_INVALID_SOCKET)
			{
				sm_netbios = SM_NETBIOS_WAIT_FOR_RX;
			}
			else
			{
				//Could not open a socket - none currently available - keep trying
			}
		}
		break;


	case SM_NETBIOS_WAIT_FOR_RX:
		//---------------------------------------------------
		//----- IDLE - WAIT FOR A PACKET TO BE RECEIVED -----
		//---------------------------------------------------
		if (udp_check_socket_for_rx(our_udp_socket))
		{
			//OUR SOCKET HAS RECEIVED A PACKET - PROCESS IT
			if (netbios_check_rx_packet())
			{
				//NETBIOS NAMESERVICE REQUEST FOR OUR NAME - SEND RESPOSNE
				sm_netbios = SM_NETBIOS_TX_RESPONSE;
			}
			else
			{
				//NOT FOR US - RETURN THE SOCKET BACK TO BROADCAST READY TO RECEIVE FROM ANYONE AGAIN
				udp_socket[our_udp_socket].remote_device_info.ip_address.Val = 0xffffffff;
			}

			//DUMP THE PACKET
			udp_dump_rx_packet();
		}

		if (sm_netbios != SM_NETBIOS_TX_RESPONSE)		//Fall into next state if we need to send a response
			break;
	

	case SM_NETBIOS_TX_RESPONSE:
		//-----------------------
		//----- TX RESPONSE -----
		//-----------------------

		//SETUP TX
		if (udp_setup_tx(our_udp_socket) == 0)
		{
			//Can't tx right now - try again next time
			break;
		}
			
		netbios_send_response();

		//SEND THE PACKET
		udp_tx_packet();

		//RETURN THE SOCKET BACK TO BROADCAST READY TO RECEIVE FROM ANYONE AGAIN
		udp_socket[our_udp_socket].remote_device_info.ip_address.Val = 0xffffffff;
			
		sm_netbios = SM_NETBIOS_WAIT_FOR_RX;
		break;
	}
}




//***************************************************************
//***************************************************************
//********** CHECK RECEIVED NETBIOS NAMESERVICE PACKET **********
//***************************************************************
//***************************************************************
//Returns:
//	1 if packet was a request for our name, 0 if not
BYTE netbios_check_rx_packet (void)
{
	WORD_VAL header_flags;
	WORD_VAL qdcount;
	WORD_VAL ancount;
	WORD_VAL nscount;
	WORD_VAL arcount;
	BYTE data;
	BYTE data1;
	BYTE count;
	BYTE no_of_bytes_to_check;


	//---------------------------
	//----- GET THE HEADER -----
	//---------------------------

	//----- TRANSACTION ID ----- [2]
	//(Message ID number)
	udp_read_next_rx_byte(&netbios_transaction_id.v[1]);
	udp_read_next_rx_byte(&netbios_transaction_id.v[0]);

	//----- FLAGS ----- [2]
	udp_read_next_rx_byte(&header_flags.v[1]);
	udp_read_next_rx_byte(&header_flags.v[0]);

	//QR (bit 15)
	//	0 = Query
	//	1 = response
	//OPCODE (bits 14:11)
	//	0 = standard query (QUERY)
	//	1 = inverse query (IQUERY)
	//	2 = server status request (STATUS)
	//	3-15 = unused)
	//AA (bit 10)
	//	(Authoritative Answer (valid in responses - specifies that the responding name server is an authority for the domain name in question section)
	//	(May have multiple owner names because of aliases.  The AA bit corresponds to the name which matches the query name, or the first owner name in the answer section)
	//TC (bit 9)
	//	(TrunCation - specifies that this message was truncated due to length greater than that permitted on the transmission channel)
	//RD (bit 8)
	//	(Recursion Desired - may be set in a query and is copied into the response.  If RD is set, it directs the name server to pursue the query recursively)
	//RA (bit 7)
	//	(Recursion Available - set or cleared in a response, and denotes whether recursive query support is available in the name server)
	//Z (bit 6:5)
	//	(Reserved for future use.  Must be zero in all queries and responses)
	//B (bit 4)
	//	Broadcast)
	//RCODE (bits 3:0)
	//	Response code
	//	0 = No error condition
	//	1 = Format error - The name server was unable to interpret the query
	//	2 = Server failure
	//	3 = Name Error (Meaningful only for responses from an authoritative name server, this code signifies that the domain name referenced in the query does not exist)
	//	4 = Not Implemented (The name server does not support the requested kind of query)
	//	5 = Refused
	//	6-15 = unused
	if (header_flags.Val & 0x8000)		//Ensure message is a query
		return(0);

	//----- QDCOUNT [2] -----
	//(The number of entries in the question section)
	udp_read_next_rx_byte(&qdcount.v[1]);
	udp_read_next_rx_byte(&qdcount.v[0]);

	//----- ANCOUNT [2] -----
	//(The number of resource records in the answer section)
	udp_read_next_rx_byte(&ancount.v[1]);
	udp_read_next_rx_byte(&ancount.v[0]);

	//----- NSCOUNT [2] -----
	//(The number of name server resource records in the authority records section)
	udp_read_next_rx_byte(&nscount.v[1]);
	udp_read_next_rx_byte(&nscount.v[0]);

	//----- ARCOUNT [2] -----
	//(The number of resource records in the additional records section)
	udp_read_next_rx_byte(&arcount.v[1]);
	udp_read_next_rx_byte(&arcount.v[0]);


	//------------------------------------
	//----- GET THE QUESTION ENTRIES -----
	//------------------------------------

	//----- QNAME [#] -----
	//(A domain name represented as a sequence of labels, where each label consists of a length byte followed by that
	//number of bytes.  The domain name terminates with the zero length byte for the null label of the root.  Note
	//that this field may be an odd number of bytes; no padding is used)

	if (qdcount.Val == 0)		//There should be at least one question
		return(0);

	//----- READ THE NAME -----
	//Get the length
	udp_read_next_rx_byte(&netbios_name_length);

	if (netbios_name_length > 0x20)			//Check for max length
		return(0);

	for (count = 0; count < netbios_name_length; count++)
	{
		udp_read_next_rx_byte(&netbios_requested_name[count]);
	}

	//THE NEXT BYTE SHOULD BE 0x00 (FOR NO MORE NAME SECTIONS)
	udp_read_next_rx_byte(&data);
	if (data != 0x00)
		return(0);


	//----- SEE IF THE NAME MATCHES US -----
	
	//Limit length to 15 characters as we don't bother storing the terminating null
	if (netbios_name_length <= 30)
		no_of_bytes_to_check = netbios_name_length;
	else
		no_of_bytes_to_check = 30;

	for (count = 0; count < no_of_bytes_to_check; count += 2)
	{

		//Get and decode the NetBios first level decoding (this is different to DNS)
		data = ((netbios_requested_name[count] - 'A') << 4);
		data += (netbios_requested_name[count + 1] - 'A');

		data = convert_character_to_lower_case(data);
		data1 = convert_character_to_lower_case(netbios_our_network_name[count >> 1]);

		if (
		(data == ' ') &&
		((data1 < 'a') || (data1 > 'z'))
		)
		{
			//Source name character is space so if our name is not a character then ignore this character
			continue;
		}

		//Check for null termination of requested name - if our name is the null termination also then we have a match
		if (data == 0x00)
		{
			if (data1 == 0x00)
				return(1);
			else
				return(0);
		}

		//Otherwise just check this character matches
		if (data != data1)
			return(0);
		
	}

	return(1);

	//DON'T BOTHER WITH THE REST OF THE PACKET AS WE JUST CARE ABOUT THE QUESTION NAME

}




//*******************************************************
//*******************************************************
//********** SEND NETBIOS NAMESERVICE RESPONSE **********
//*******************************************************
//*******************************************************
void netbios_send_response (void)
{
	BYTE count;


	//---------------------------
	//----- SEND THE HEADER -----
	//---------------------------

	//----- TRANSACTION ID ----- [2]
	//(Message ID numbmer)
	udp_write_next_byte(netbios_transaction_id.v[1]);
	udp_write_next_byte(netbios_transaction_id.v[0]);

	//----- FLAGS ----- [2]
	//QR (bit 15)
	//	0 = Query
	//	1 = response
	//OPCODE (bits 14:11)
	//	0 = standard query (QUERY)
	//	1 = inverse query (IQUERY)
	//	2 = server status request (STATUS)
	//	3-15 = unused)
	//AA (bit 10)
	//	(Authoritative Answer (valid in responses - specifies that the responding name server is an authority for the domain name in question section)
	//	(May have multiple owner names because of aliases.  The AA bit corresponds to the name which matches the query name, or the first owner name in the answer section)
	//TC (bit 9)
	//	(TrunCation - specifies that this message was truncated due to length greater than that permitted on the transmission channel)
	//RD (bit 8)
	//	(Recursion Desired - may be set in a query and is copied into the response.  If RD is set, it directs the name server to pursue the query recursively)
	//RA (bit 7)
	//	(Recursion Available - set or cleared in a response, and denotes whether recursive query support is available in the name server)
	//Z (bit 6:5)
	//	(Reserved for future use.  Must be zero in all queries and responses)
	//B (bit 4)
	//	Broadcast)
	//RCODE (bits 3:0)
	//	Response code
	//	0 = No error condition
	//	1 = Format error - The name server was unable to interpret the query
	//	2 = Server failure
	//	3 = Name Error (Meaningful only for responses from an authoritative name server, this code signifies that the domain name referenced in the query does not exist)
	//	4 = Not Implemented (The name server does not support the requested kind of query)
	//	5 = Refused
	//	6-15 = unused
	udp_write_next_byte(0x84);
	udp_write_next_byte(0x80);

	//----- QDCOUNT [2] -----
	//(The number of entries in the question section)
	udp_write_next_byte(0x00);
	udp_write_next_byte(0x00);

	//----- ANCOUNT [2] -----
	//(The number of resource records in the answer section)
	udp_write_next_byte(0x00);
	udp_write_next_byte(0x01);

	//----- NSCOUNT [2] -----
	//(The number of name server resource records in the authority records section)
	udp_write_next_byte(0x00);
	udp_write_next_byte(0x00);

	//----- ARCOUNT [2] -----
	//(The number of resource records in the additional records section)
	udp_write_next_byte(0x00);
	udp_write_next_byte(0x00);


	//-----------------------------
	//----- SEND THE QUESTIONS -----
	//-----------------------------

	//----- QNAME [#] -----
	//(A domain name represented as a sequence of labels, where each label consists of a length byte followed by that
	//number of bytes.  The domain name terminates with the zero length byte for the null label of the root.  Note
	//that this field may be an odd number of bytes; no padding is used)


	//------------------
	//----- ANSWER -----
	//------------------
	
	//----- NAME [#] -----
	//(A domain name to which this resource record pertains)

	udp_write_next_byte(netbios_name_length);						//Name length
	
	for (count = 0; count < netbios_name_length; count++)		//Our name
	{
		udp_write_next_byte(netbios_requested_name[count]);
	}

	udp_write_next_byte(0x00);				//Null termination

	//----- TYPE [2] -----
	//(Two bytes containing one of the RR type codes.  This field specifies the meaning of the data in the RDATA field)
	udp_write_next_byte(0x00);
	udp_write_next_byte(0x20);

	//----- CLASS [2] -----
	//(Two bytes which specify the class of the data in the RDATA field)
	udp_write_next_byte(0x00);
	udp_write_next_byte(0x01);

	//----- TTL [4] -----
	//(The time interval (in seconds) that the resource record may be cached before it should be discarded.  Zero values are
	//interpreted to mean that the RR can only be used for the transaction in progress, and should not be cached)
	udp_write_next_byte(0x00);
	udp_write_next_byte(0x00);
	udp_write_next_byte(0x01);		//0x12c = 300 secs = 5 mins
	udp_write_next_byte(0x2c);

	//----- RDLENGTH [2] -----
	//(The length in bytes of the RDATA field)
	udp_write_next_byte(0x00);
	udp_write_next_byte(0x06);

	//----- RDATA [#] -----
	//(A variable length string of bytes that describes the resource.  The format of this information varies
	//according to the TYPE and CLASS of the resource record.  For example, the if the TYPE is A and the CLASS is IN,
	//the RDATA field is a 4 byte ARPA Internet address)

	//ADDRESS ENTRY RECORDS
	udp_write_next_byte(0x00);					//NB FLAGS (bit 15: 1=group 0=unique, bits 14:13 (ONT): 00=B Node, bits 4:0 reserved)
	udp_write_next_byte(0x00);

	udp_write_next_byte(our_ip_address.v[0]);	//Address
	udp_write_next_byte(our_ip_address.v[1]);
	udp_write_next_byte(our_ip_address.v[2]);
	udp_write_next_byte(our_ip_address.v[3]);


	//---------------------
	//----- AUTHORITY -----
	//---------------------
	
	//----- NAME [#] -----
	//(A domain name to which this resource record pertains)

	//----- TYPE [2] -----
	//(Two bytes containing one of the RR type codes.  This field specifies the meaning of the data in the RDATA field)

	//----- CLASS [2] -----
	//(Two bytes which specify the class of the data in the RDATA field)

	//----- TTL [4] -----
	//(The time interval (in seconds) that the resource record may be cached before it should be discarded.  Zero values are
	//interpreted to mean that the RR can only be used for the transaction in progress, and should not be cached)

	//----- RDLENGTH [2] -----
	//(The length in bytes of the RDATA field)

	//----- RDATA [#] -----
	//(A variable length string of bytes that describes the resource.  The format of this information varies
	//according to the TYPE and CLASS of the resource record.  For example, the if the TYPE is A and the CLASS is IN,
	//the RDATA field is a 4 byte ARPA Internet address)


	//----------------------
	//----- ADDITIONAL -----
	//----------------------
	
	//----- NAME [#] -----
	//(A domain name to which this resource record pertains)

	//----- TYPE [2] -----
	//(Two bytes containing one of the RR type codes.  This field specifies the meaning of the data in the RDATA field)

	//----- CLASS [2] -----
	//(Two bytes which specify the class of the data in the RDATA field)

	//----- TTL [4] -----
	//(The time interval (in seconds) that the resource record may be cached before it should be discarded.  Zero values are
	//interpreted to mean that the RR can only be used for the transaction in progress, and should not be cached)

	//----- RDLENGTH [2] -----
	//(The length in bytes of the RDATA field)

	//----- RDATA [#] -----
	//(A variable length string of bytes that describes the resource.  The format of this information varies
	//according to the TYPE and CLASS of the resource record.  For example, the if the TYPE is A and the CLASS is IN,
	//the RDATA field is a 4 byte ARPA Internet address)

}











