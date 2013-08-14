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
//TCP (TRANSMISSION CONTROL PROTOCOL) C CODE FILE


#include "main.h"					//Global data type definitions (see https://github.com/ibexuk/C_Generic_Header_File )
#define	TCP_C
#include "eth-tcp.h"
#undef	TCP_C

#include "eth-main.h"
#include "eth-nic.h"
#include "eth-ip.h"
#include "eth-arp.h"



//-------------------------------------------------------------------------
//----- GENERATE COMPILE ERROR IF TCP HAS NOT BEEN DEFINED TO BE USED -----
//-------------------------------------------------------------------------
#ifndef STACK_USE_TCP
#error TCP file is included in project but not defined to be used - remove file from project to reduce code size.
#endif





//************************************
//************************************
//********** INITIALISE TCP **********
//************************************
//************************************
//Called from the tcp_ip_initialise function
void tcp_initialise (void)
{
	BYTE socket;

	//----- INITIALISE EACH TCP SOCKET -----
	for	(socket = 0; socket < TCP_NO_OF_AVAILABLE_SOCKETS; socket++)
	{
		tcp_socket[socket].sm_socket_state = SM_TCP_CLOSED;
		tcp_socket[socket].rx_data_bytes_remaining = 0;
		tcp_socket[socket].time_out_value = TCP_INITIAL_TIMEOUT_VALUE_10MS;
		tcp_socket[socket].flags.ready_for_tx = 1;
		tcp_socket[socket].flags.tx_last_tx_awaiting_ack	= 0;
		tcp_socket[socket].flags.tx_last_tx_had_data = 0;
		tcp_socket[socket].flags.tx_resend_last_tx = 0;
		tcp_socket[socket].flags.tx_send_waiting_command = 0;
	}

	next_local_tcp_port_to_use = TCP_LOCAL_PORT_NUMBER_START;
	//next_connection_start_sequence_number = 0;							//Don't initialise as it does not need to be and when debugging it can be useful if this varaiable
																			//is in an uninitialised ram area and won't always be returned to the same value when re-running.

	tcp_rx_packet_is_waiting_to_be_processed = 0;							//There is no packet waiting to be processed
	tcp_rx_active_socket = 0;												//No socket is receiving
}





//*****************************************
//*****************************************
//********** PROCESS TCP SOCKETS **********
//*****************************************
//*****************************************
//Do background processing of all sockets checking for timeout etc
//This function is called reguarly by tcp_ip_process_stack
void process_tcp (void)
{
	BYTE socket;
	BYTE tcp_response_flags;
	DWORD ethernet_10ms_clock_time_since_start;


	tcp_response_flags = 0x00;				//Set default value

	//---------------------------------
	//---------------------------------
	//----- CHECK EACH TCP SOCKET -----
	//---------------------------------
	//---------------------------------
	for (socket = 0; socket < TCP_NO_OF_AVAILABLE_SOCKETS; socket++)
	{
		if (tcp_socket[socket].rx_data_bytes_remaining)
		{
			//----------------------------------------------------------------------
			//----- RX PACKET IS WAITING FOR APPLICATION TO PROCESS IT SO SKIP -----
			//----------------------------------------------------------------------
			continue;
		}

		if (tcp_socket[socket].sm_socket_state == SM_TCP_CLOSED)
		{
			//------------------------------------
			//----- SOCKET IS CLOSED SO SKIP -----
			//------------------------------------
			continue;
		}

		if (tcp_socket[socket].sm_socket_state == SM_TCP_LISTEN)
		{
			//----------------------------------------------------------------------------
			//----- SOCKET IS LISTENING FOR A NEW CONNECTION (NOT CONNECTED) SO SKIP -----
			//----------------------------------------------------------------------------
			continue;
		}

		//----------------------------
		//----- SOCKET IS ACTIVE -----
		//----------------------------


		//-----------------------------------------
		//----- CHECK FOR SEND QUEUED COMMAND -----
		//-----------------------------------------
		if (tcp_socket[socket].flags.tx_send_waiting_command)
		{
			//----- WE ARE WAITING FOR NIC TO BE AVAILABLE TO SEND A QUEUED COMMAND - TRY AGAIN NOW -----
			tcp_send_command_packet_from_socket (socket, tcp_socket[socket].waiting_command_flags);
			continue;
		}

		//----------------------------------------------
		//----- CHECK FOR CONNECT TO CLIENT DO ARP -----
		//----------------------------------------------
		if (tcp_socket[socket].sm_socket_state == SM_TCP_CONNECT_SEND_ARP_REQUEST)
		{
			//Try and send the ARP request
			if (arp_resolve_ip_address(&tcp_socket[socket].remote_device_info.ip_address))
			{
				tcp_socket[socket].sm_socket_state = SM_TCP_CONNECT_WAIT_ARP_RESPONSE;
			}
			//Could not transmit right now - try again next time
			continue;
		}
		if (tcp_socket[socket].sm_socket_state == SM_TCP_CONNECT_WAIT_ARP_RESPONSE)
		{
			if (arp_is_resolve_complete(&tcp_socket[socket].remote_device_info.ip_address, &tcp_socket[socket].remote_device_info.mac_address))
			{
				//ARP IS COMPLETE - SEND THE SYN REQUEST
				tcp_connect_socket_send_syn_request(socket);				//This will set the socket state
			}
		}


		//-----------------------------
		//----- CHECK FOR TIMEOUT -----
		//-----------------------------

		//GET THE TIME SINCE THE TIMEOUT TIMER WAS RESET
		ethernet_10ms_clock_time_since_start = (ethernet_10ms_clock_timer - tcp_socket[socket].start_time);		//(ethernet_10ms_clock_timer is loaded with the current timer value in the main stack processing function)

		//CHECK FOR TIMEOUT
		if (ethernet_10ms_clock_time_since_start <= tcp_socket[socket].time_out_value)
		{
			//------------------------------------------------------------
			//----- SOCKET HAS NOT TIMED OUT - MOVE ONTO NEXT SOCKET -----
			//------------------------------------------------------------
			continue;
		}

		//-------------------------------------------------------------
		//----- TIMEOUT HAS OCCURED - RE-TRANSMISSION IS REQUIRED -----
		//-------------------------------------------------------------

		//All states can require re-transmission so exit if the nic is not currently able to tx
		if (!nic_ok_to_do_tx())
			return;

		//RESET THE TIMEOUT TIMER START TIME TO NOW
		tcp_socket[socket].start_time = ethernet_10ms_clock_timer;

		//DOUBLE THE TIMEOUT TIME (this provides a good timeout compromise between local network and internet communications)
		if ((tcp_socket[socket].time_out_value << 1) < TCP_MAX_TIMEOUT_VALUE_10MS)
			tcp_socket[socket].time_out_value <<= 1;
		else
			tcp_socket[socket].time_out_value = TCP_MAX_TIMEOUT_VALUE_10MS;

		//INCREMENT THE NUMBER OF RETRYS COUNT
		tcp_socket[socket].retry_count++;


		//---------------------------------------------------------------
		//---------------------------------------------------------------
		//---- DO RE-TRASNMIT DEPENDING ON THE CURRENT SOCKET STATE -----
		//---------------------------------------------------------------
		//---------------------------------------------------------------
		switch(tcp_socket[socket].sm_socket_state)
		{
		case SM_TCP_SYN_REQUEST_SENT:
			//------------------------------------------------
			//----- SENDING SYN REQUEST TO REMOTE DEVICE -----
			//------------------------------------------------
			//We just keep on sending.  The application process trying to make a connection should deal with connection fail timeout
			tcp_response_flags = TCP_SYN;
			break;


		case SM_TCP_SYN_REQUEST_RECEIVED:
			//-----------------------------------------------------------------------------------------
			//----- WE REPLIED TO SYN REQUEST FROM REMOTE DEVICE BUT NO ACK RECEVIED - SEND AGAIN -----
			//-----------------------------------------------------------------------------------------
			//Abort, if maximum attempts counts are reached.
			if (tcp_socket[socket].retry_count <= TCP_MAX_NUM_OF_RETRIES)
			{
				//SEND AGAIN
				tcp_response_flags = TCP_SYN | TCP_ACK;
			}
			else
			{
				//DONE MAX NUMBER OF RETRIES - CONNECTION FAILED
				tcp_close_socket(socket);
			}
			break;


		case SM_TCP_CONNECTION_ESTABLISHED:
			//---------------------------------------------------------------------------------------
			//----- CONNECTION ESTABLISHED - WAITING FOR ACK TO LAST TX OR LAST SENT ACK FOR RX -----
			//---------------------------------------------------------------------------------------
			//(Connections should not be open and idle - once all data has been sent between devices the connection should be closed
			//If not then after sufficient timeout this function will close the connection
			if (tcp_socket[socket].retry_count <= TCP_MAX_NUM_OF_RETRIES)
			{
				//RETRY
				if (tcp_socket[socket].flags.tx_last_tx_awaiting_ack == 1)
				{
					//-------------------------------------
					//----- RESEND PREVIOUS TX PACKET -----
					//-------------------------------------
					if (tcp_socket[socket].flags.tx_last_tx_had_data == 0)
					{
						//LAST PACKET HAD NO DATA SO JUST RESEND THE FLAGS
						tcp_response_flags = tcp_socket[socket].tx_last_tx_flags;
					}
					else
					{
						//LAST PACKET HAD DATA - TELL APPLICATION FUNCTION TO RE-SEND IT
						tcp_socket[socket].flags.tx_resend_last_tx = 1;
						tcp_socket[socket].flags.tx_last_tx_awaiting_ack = 0;			//(These flags need to be cleared to allow the resend packet to happen)
						tcp_socket[socket].flags.ready_for_tx = 1;
					}
				}
				else
				{
					//---------------------------------------------------------
					//----- IDLE - DO NOTHING OR SEND RESEND PREVIOUS ACK -----
					//---------------------------------------------------------
					#ifdef TCP_USE_SOCKET_INACTIVITY_TIMOUT
						tcp_response_flags = TCP_ACK;
					#else
						tcp_socket[socket].retry_count = 0;
						tcp_response_flags = 0;
					#endif
				}
			}
			else
			{
				//DONE MAX NUMBER OF RETRIES
				if (tcp_socket[socket].flags.tx_last_tx_awaiting_ack == 1)
				{
					//-----------------------------------------------------
					//----- DUMP PREVIOUS TX PACKET - NO ACK RECEIVED -----
					//-----------------------------------------------------
					tcp_socket[socket].flags.tx_resend_last_tx = 0;
					tcp_socket[socket].flags.tx_last_tx_awaiting_ack = 0;
				}
				//---------------------------------------------
				//----- REQUEST CLOSURE OF THE CONNECTION -----
				//---------------------------------------------
				tcp_response_flags = (TCP_FIN | TCP_ACK);
				tcp_socket[socket].sm_socket_state = SM_TCP_FIN_WAIT_1;
			}
			break;


		case SM_TCP_FIN_WAIT_1:
		case SM_TCP_LAST_ACK_SENT:
			//--------------------------------------
			//----- TRYING TO CLOSE CONNECTION -----
			//--------------------------------------
			if (tcp_socket[socket].retry_count <= TCP_MAX_NUM_OF_RETRIES)
			{
				//----- REQUEST CLOSE CONNECTION AGAIN -----
				tcp_response_flags = TCP_FIN;
			}
			else
			{
				//----- NO RESPONSE - JUST CLOSE THE CONNECTION -----
				tcp_close_socket(socket);
			}
			break;


		case SM_TCP_CLOSING:
			//------------------------------------------------------------------------
			//----- TRYING TO COMPLETE CONNECTION CLOSE REQUEST BY REMOTE DEVICE -----
			//------------------------------------------------------------------------
			if (tcp_socket[socket].retry_count <= TCP_MAX_NUM_OF_RETRIES)
			{
				//----- REQUEST CLOSE CONNECTION AGAIN -----
				tcp_response_flags = TCP_ACK;
			}
			else
			{
				//----- NO RESPONSE - JUST CLOSE THE CONNECTION -----
				tcp_close_socket(socket);
			}
			break;

		} //switch(tcp_socket[socket].sm_socket_state)


		//-----------------------------
		//----- SEND ANY RESPONSE -----
		//-----------------------------
		if (tcp_response_flags)
		{
			//SEND THE PACKET
			tcp_send_command_packet_from_socket(socket, tcp_response_flags);

			//IF PACKET IS NOT JUST AN ACK THEN ADJUST THE SEQUENCE NUMBER NEXT TIME
			if (tcp_response_flags != TCP_ACK)
			{
				tcp_socket[socket].next_segment_sequence_increment_value = 1;
			}
		}

	} //for (socket = 0; socket < TCP_NO_OF_AVAILABLE_SOCKETS; socket++)

}




//*************************************************
//*************************************************
//********** PROCESS RECEIVED TCP PACKET **********
//*************************************************
//*************************************************
//Called from the recevied packet ethernet stack state machine
//Returns 1 if tcp packet processing is done, 0 if not yet done (waiting on the application process that owns the socket to read the packet and discard it)
BYTE tcp_process_rx (DEVICE_INFO *sender_device_info, IP_ADDR *destination_ip_address, WORD ip_data_area_bytes)
{

	TCP_HEADER tcp_header;
	BYTE tcp_socket_number;
	PSEUDO_HEADER pseudo_header;
	WORD tcp_rx_checksum;
	WORD tcp_rx_checksum_recevied;
	BYTE tcp_rx_checksum_next_byte_low;
	BYTE b_data;
	WORD tcp_data_length;
	WORD count;

	if (tcp_rx_packet_is_waiting_to_be_processed)
	{
		//--------------------------------------------------------------------------------------------------------------------------------
		//----- TCP PACKET HAS ALREADY BEEN RECEVEIVED AND WE ARE WAITING ON THE APPLICATION PROCESS USING THIS SOCKET TO PROCESS IT -----
		//--------------------------------------------------------------------------------------------------------------------------------
		if (tcp_socket[tcp_rx_active_socket].rx_data_bytes_remaining == 0)		//(The application process that owns this socket flags the packet is processed and dumped by writing 0 here)
		{
			//------------------------------------------------
			//----- PACKET HAS BEEN PROCESSED AND DUMPED -----
			//------------------------------------------------
			if (tcp_tx_resp_after_proc_rx_resp_flags)
			{
				//---------------------------------------------------
				//----- SEND STORED AUTO GENERATED TCP RESPONSE -----
				//---------------------------------------------------
				tcp_send_command_packet_from_socket(tcp_tx_resp_after_proc_rx_socket, tcp_tx_resp_after_proc_rx_resp_flags);
				tcp_tx_resp_after_proc_rx_resp_flags = 0;
			}

			//THE TCP PACKET HAS BEEN PROCESSED
			tcp_rx_active_socket = TCP_INVALID_SOCKET;
			tcp_rx_packet_is_waiting_to_be_processed = 0;
			return (1);
		}
		else
		{
			//---------------------------------------------
			//----- PACKET HAS NOT BEEN PROCESSED YET -----
			//---------------------------------------------

			//IF THE SOCKET THATS GOT DATA HAS LOST ITS CONNECTION THEN DUMP THE RX
			if (tcp_socket[tcp_rx_active_socket].sm_socket_state != SM_TCP_CONNECTION_ESTABLISHED)
				tcp_dump_rx_packet();
			
			return(0);
		}
	}


	//-----------------------------------------------
	//----- NEW RECEIVED PACKET TO BE PROCESSED -----
	//-----------------------------------------------

	//----- GET THE TCP HEADER -----
	//if (nic_read_array((BYTE*)&tcp_header, TCP_HEADER_LENGTH) == 0)
	//	goto tcp_process_rx_dump_packet;						//Error - packet was too small - dump

	if (nic_read_array((BYTE*)&tcp_header.source_port, 2) == 0)
		goto tcp_process_rx_dump_packet;						//Error - packet was too small - dump
	if (nic_read_array((BYTE*)&tcp_header.destination_port, 2) == 0)
		goto tcp_process_rx_dump_packet;						//Error - packet was too small - dump
	if (nic_read_array((BYTE*)&tcp_header.sequence_number, 4) == 0)
		goto tcp_process_rx_dump_packet;						//Error - packet was too small - dump
	if (nic_read_array((BYTE*)&tcp_header.acknowledgment_number, 4) == 0)
		goto tcp_process_rx_dump_packet;						//Error - packet was too small - dump
	if (!nic_read_next_byte(&tcp_header.header_length.byte))
		return(1);												//Error - packet was too small - dump
	if (!nic_read_next_byte(&tcp_header.flags.byte))
		return(1);												//Error - packet was too small - dump
	if (nic_read_array((BYTE*)&tcp_header.window, 2) == 0)
		goto tcp_process_rx_dump_packet;						//Error - packet was too small - dump
	if (nic_read_array((BYTE*)&tcp_header.checksum, 2) == 0)
		goto tcp_process_rx_dump_packet;						//Error - packet was too small - dump
	if (nic_read_array((BYTE*)&tcp_header.urgent_pointer, 2) == 0)
		goto tcp_process_rx_dump_packet;						//Error - packet was too small - dump



	//----- CREATE TCP PSEUDOHEADER FOR THE CHECKSUM -----
	pseudo_header.source_address.Val = sender_device_info->ip_address.Val;
	pseudo_header.destination_address.Val = our_ip_address.Val;
	pseudo_header.zero = 0;
	pseudo_header.protocol = IP_PROTOCOL_TCP;
	pseudo_header.tcp_length = ip_data_area_bytes;

	pseudo_header.tcp_length = swap_word_bytes(pseudo_header.tcp_length);

	//----- START CALCULATION OF TCP CHECKSUM -----
	tcp_rx_checksum = 0;
	tcp_rx_checksum_next_byte_low = 0;
	//ip_add_bytes_to_ip_checksum (&tcp_rx_checksum, &tcp_rx_checksum_next_byte_low, (BYTE*)&pseudo_header, PSEUDO_HEADER_LENGTH);
    ip_add_bytes_to_ip_checksum (&tcp_rx_checksum, &tcp_rx_checksum_next_byte_low, (BYTE*)&pseudo_header.source_address, 4);
    ip_add_bytes_to_ip_checksum (&tcp_rx_checksum, &tcp_rx_checksum_next_byte_low, (BYTE*)&pseudo_header.destination_address, 4);
    ip_add_bytes_to_ip_checksum (&tcp_rx_checksum, &tcp_rx_checksum_next_byte_low, (BYTE*)&pseudo_header.zero, 1);
    ip_add_bytes_to_ip_checksum (&tcp_rx_checksum, &tcp_rx_checksum_next_byte_low, (BYTE*)&pseudo_header.protocol, 1);
    ip_add_bytes_to_ip_checksum (&tcp_rx_checksum, &tcp_rx_checksum_next_byte_low, (BYTE*)&pseudo_header.tcp_length, 2);


	//----- ADD THE TCP HEADER TO THE CHECKSUM -----
	tcp_rx_checksum_recevied = tcp_header.checksum;
	tcp_header.checksum = 0;
	//ip_add_bytes_to_ip_checksum (&tcp_rx_checksum, &tcp_rx_checksum_next_byte_low, (BYTE*)&tcp_header, TCP_HEADER_LENGTH);
    ip_add_bytes_to_ip_checksum (&tcp_rx_checksum, &tcp_rx_checksum_next_byte_low, (BYTE*)&tcp_header.source_port, 2);
    ip_add_bytes_to_ip_checksum (&tcp_rx_checksum, &tcp_rx_checksum_next_byte_low, (BYTE*)&tcp_header.destination_port, 2);
    ip_add_bytes_to_ip_checksum (&tcp_rx_checksum, &tcp_rx_checksum_next_byte_low, (BYTE*)&tcp_header.sequence_number, 4);
    ip_add_bytes_to_ip_checksum (&tcp_rx_checksum, &tcp_rx_checksum_next_byte_low, (BYTE*)&tcp_header.acknowledgment_number, 4);
    ip_add_bytes_to_ip_checksum (&tcp_rx_checksum, &tcp_rx_checksum_next_byte_low, (BYTE*)&tcp_header.header_length.byte, 1);
    ip_add_bytes_to_ip_checksum (&tcp_rx_checksum, &tcp_rx_checksum_next_byte_low, (BYTE*)&tcp_header.flags.byte, 1);
    ip_add_bytes_to_ip_checksum (&tcp_rx_checksum, &tcp_rx_checksum_next_byte_low, (BYTE*)&tcp_header.window, 2);
    ip_add_bytes_to_ip_checksum (&tcp_rx_checksum, &tcp_rx_checksum_next_byte_low, (BYTE*)&tcp_header.checksum, 2);
    ip_add_bytes_to_ip_checksum (&tcp_rx_checksum, &tcp_rx_checksum_next_byte_low, (BYTE*)&tcp_header.urgent_pointer, 2);


	//----- ADD THE OPTIONS AND TCP DATA AREA TO THE CHECKSUM -----
	for (count = 0; count < (ip_data_area_bytes - TCP_HEADER_LENGTH); count++)
	{
		nic_read_next_byte(&b_data);
		ip_add_bytes_to_ip_checksum (&tcp_rx_checksum, &tcp_rx_checksum_next_byte_low, (BYTE*)&b_data, 1);
	}

	tcp_rx_checksum = swap_word_bytes(~tcp_rx_checksum);

	//----- CHECK THAT THE CHECKSUMS MATCH -----
	if (tcp_rx_checksum_recevied != tcp_rx_checksum)
		goto tcp_process_rx_dump_packet;

	//----- MOVE NIC POINTER AND DATA BYTE COUNT BACK TO START OF TCP DATA AREA -----
	//Ignore any options if present after the TCP header
	nic_move_pointer (ETHERNET_HEADER_LENGTH + IP_HEADER_LENGTH + (tcp_header.header_length.bits.val << 2));	//Use the header length value so that any option bytes are included

	//SWAP THE HEADER WORDS (AFTER CHECKSUMMING)
    swap_tcp_header(&tcp_header);

	//GET THE TCP DATA LENGTH
	tcp_data_length = ip_data_area_bytes - (tcp_header.header_length.bits.val << 2);

	//----- LOOK FOR A MATCHING SOCKET -----
	tcp_socket_number = tcp_rx_check_for_matches_socket(&tcp_header, sender_device_info);

	if (tcp_socket_number == TCP_INVALID_SOCKET)
	{
		//----- THERE IS NO MATCHING SOCKET FOR THIS TCP PACKET - DUMP IT AND SEND RESET COMMAND TO REMOTE DEVICE -----
		goto tcp_process_rx_dump_packet_and_send_reset;
	}
	else
	{
		//----- MATCHING SOCKET FOUND - PROCESS THE PACKET & FLAG THAT SOCKET HAS DATA READY TO BE READ IF NECESSARY -----
		//(Done in seperate function to keep things organised
		if (process_tcp_segment(tcp_socket_number, sender_device_info, &tcp_header, tcp_data_length))
		{
			//NO RX DATA WAITING TO BE READ AND DUMPED BY USER APPLICATION SOCKET OWNER
			return(1);
		}
		else
		{
			//RX DATA STILL TO BE READ AND DUMPED BY USER APPLICATION SOCKET OWNER
			//(processing will be done by the application routine that is using this socket)
			tcp_rx_active_socket = tcp_socket_number;
			tcp_rx_packet_is_waiting_to_be_processed = 1;
			return(0);
		}
	}



tcp_process_rx_dump_packet_and_send_reset:
	//------------------------------------------------------
	//----- DUMP THE TCP PACKET AND SEND RESET COMMAND -----
	//------------------------------------------------------

	//DUMP THE PACKET
    nic_rx_dump_packet();

	//SEND THE RESET COMMAND
	//Update the acknowledgement number
	tcp_header.acknowledgment_number += tcp_data_length;
	if (tcp_header.flags.bits.flag_syn || tcp_header.flags.bits.flag_fin)
		tcp_header.acknowledgment_number++;

	tcp_send_command_packet_no_socket(sender_device_info, tcp_header.destination_port, tcp_header.source_port, tcp_header.acknowledgment_number,
                    tcp_header.sequence_number, TCP_RST);
	return(1);


tcp_process_rx_dump_packet:
	//-------------------------------
	//----- DUMP THE TCP PACKET -----
	//-------------------------------
    nic_rx_dump_packet();
	return(1);
}





//*****************************************
//*****************************************
//********** PROCESS TCP SEGMENT **********
//*****************************************
//*****************************************
//Called by tcp_process_rx
//Returns 1 if tcp packet processing is done, 0 if not yet done (waiting on other application process to read the packet and discard it)
//This fucnction is only called by tcp_process_rx and is a seperate function to make the program more readable
static BYTE process_tcp_segment(BYTE socket, DEVICE_INFO *remote_device_info, TCP_HEADER *tcp_header,  WORD tcp_data_length)
{
	BYTE tcp_response_flags;
	BYTE rx_has_been_dumped = 0;


	tcp_response_flags = 0x00;			//Set default value

	//UPDATE THE SEQ VALUE IF IT NEEDS UPDATING AFTER THE LAST SEGMENT HAS BEEN ACCEPTED BY THE REMOTE DEVICE
	if (tcp_socket[socket].next_segment_sequence_increment_value)
	{
		tcp_socket[socket].send_sequence_number += (DWORD)tcp_socket[socket].next_segment_sequence_increment_value;
		tcp_socket[socket].next_segment_sequence_increment_value = 0;
	}


	//Clear retry counts and timeout timer
	tcp_socket[socket].retry_count = 0;
	tcp_socket[socket].start_time = ethernet_10ms_clock_timer;
	tcp_socket[socket].time_out_value = TCP_INITIAL_TIMEOUT_VALUE_10MS;

	if (tcp_socket[socket].sm_socket_state == SM_TCP_LISTEN)
	{
		//-----------------------------------------------------
		//-----------------------------------------------------
		//----- SOCKET IS NOT CONNECTED - IN LISTEN STATE -----
		//-----------------------------------------------------
		//-----------------------------------------------------

		//DISCARD THE RECEIVED PACKET
		nic_rx_dump_packet();
		rx_has_been_dumped = 1;

		//SET THE SEQUENCE ACKNOWLEDGEMENT NUMBERS
		tcp_socket[socket].send_sequence_number = ++next_connection_start_sequence_number;
		tcp_socket[socket].next_segment_sequence_increment_value = 0;

		tcp_socket[socket].send_acknowledgement_number = tcp_header->sequence_number + (DWORD)tcp_data_length + 1;

		if (tcp_header->flags.bits.flag_syn)
		{
			//----- PACKET IS SYN - CONNECTION REQUEST FROM REMOTE DEVICE -----
			tcp_socket[socket].remote_device_info.mac_address.v[0] = remote_device_info->mac_address.v[0];
			tcp_socket[socket].remote_device_info.mac_address.v[1] = remote_device_info->mac_address.v[1];
			tcp_socket[socket].remote_device_info.mac_address.v[2] = remote_device_info->mac_address.v[2];
			tcp_socket[socket].remote_device_info.mac_address.v[3] = remote_device_info->mac_address.v[3];
			tcp_socket[socket].remote_device_info.mac_address.v[4] = remote_device_info->mac_address.v[4];
			tcp_socket[socket].remote_device_info.mac_address.v[5] = remote_device_info->mac_address.v[5];
			tcp_socket[socket].remote_device_info.ip_address.Val = remote_device_info->ip_address.Val;

			tcp_socket[socket].remote_port = tcp_header->source_port;

			//CONNECT TO REMOTE DEVICE
			tcp_response_flags = (TCP_SYN | TCP_ACK);
			tcp_socket[socket].next_segment_sequence_increment_value = 1;
			tcp_socket[socket].sm_socket_state = SM_TCP_SYN_REQUEST_RECEIVED;
		}
		else
		{
			//----- NOT A CONNECTION REQUEST SO INVALID PACKET AS WE ARE IN LISTEN STATE -----
			//SEND RESET COMMAND TO REMOTE DEVICE
			tcp_send_command_packet_no_socket(remote_device_info, tcp_header->destination_port, tcp_header->source_port, tcp_header->acknowledgment_number,
											tcp_header->sequence_number, TCP_RST);
			tcp_response_flags = 0;

			tcp_socket[socket].remote_device_info.ip_address.Val = 0x00000000;
		}
	}
	else
	{
		//-------------------------------------------------------------
		//-------------------------------------------------------------
		//----- SOCKET IS ALREADY CONNECTED (NOT IN LISTEN STATE) -----
		//-------------------------------------------------------------
		//-------------------------------------------------------------
		if (tcp_header->flags.bits.flag_rst)
		{
			//-----------------------------------------------------
			//----- RST FLAG RECEIVED - RESET THIS CONNECTION -----
			//-----------------------------------------------------

			//DISCARD THE RECEIVED PACKET
			nic_rx_dump_packet();
			rx_has_been_dumped = 1;

			tcp_close_socket(socket);

			return(1);
		}
		else if ((tcp_socket[socket].send_sequence_number == tcp_header->acknowledgment_number) || (tcp_header->flags.bits.flag_fin))
		{
			//----------------------------------------------------------------------
			//----------------------------------------------------------------------
			//----- ACKNOWLEDGEMENT NUMBER RECEIVED IS CORRECT (OR FIN IS SET) -----
			//----------------------------------------------------------------------
			//----------------------------------------------------------------------
			//(Letting through a bad ack number when FIN is set avoids a potential lockup state where the remote device hasn't seen
			//one or more of our attempts to close the connection and is therefore out of step with our seq value and acknowleding a previous seq value)
			if (tcp_socket[socket].sm_socket_state == SM_TCP_SYN_REQUEST_RECEIVED)
			{
				//----- SYN REQUEST RECEIVED LAST -----
				if (tcp_header->flags.bits.flag_ack)
				{
					//------------------------------------------------------------------------------------
					//----- CONNECTION HAS BEEN ESTABLISHED FOLLOWING SYN REQUEST FROM REMOTE DEVICE -----
					//------------------------------------------------------------------------------------
					tcp_socket[socket].send_acknowledgement_number = tcp_header->sequence_number + (DWORD)tcp_data_length;
					tcp_socket[socket].retry_count = 0;
					tcp_socket[socket].start_time = ethernet_10ms_clock_timer;
					tcp_socket[socket].sm_socket_state = SM_TCP_CONNECTION_ESTABLISHED;

					if (tcp_data_length > 0)
					{
						//----------------------------------------------
						//----- PACKET HAS DATA WAITING TO BE READ -----
						//----------------------------------------------
						tcp_socket[socket].rx_data_bytes_remaining = tcp_data_length;
					}
					else
					{
						//----- NO DATA INCLUDED IN THE PACKET -----
						nic_rx_dump_packet();
						rx_has_been_dumped = 1;
					}
				}
				else
				{
					//----- THIS IS NOT AN ACK TO OUR RESPONSE TO A SYN REQUEST SO JUST DUMP IT -----
					nic_rx_dump_packet();
					rx_has_been_dumped = 1;
				}
			}
			else if (tcp_socket[socket].sm_socket_state == SM_TCP_SYN_REQUEST_SENT)
			{
				//----- SYN REQUEST SENT LAST -----
				if (tcp_header->flags.bits.flag_syn)
				{
					//----------------------------------------------------------
					//----- REMOTE DEVICE HAS RESPONDED TO OUR SYN REQUEST -----
					//----------------------------------------------------------
					tcp_socket[socket].send_acknowledgement_number = tcp_header->sequence_number + (DWORD)tcp_data_length + 1;
					if (tcp_header->flags.bits.flag_ack)
					{
						//---------------------------------------------------------------------
						//----- CONNECTION HAS BEEN ESTABLISHED FOLLOWING OUR SYN REQUEST -----
						//---------------------------------------------------------------------
						tcp_response_flags = TCP_ACK;
						tcp_socket[socket].sm_socket_state = SM_TCP_CONNECTION_ESTABLISHED;
					}
					else
					{
						//--------------------------------------------------------------------------------------------------------
						//----- RESPONSE TO OUR SYN REQUEST RECEVIED BUT NO ACK SO TREAT AS A SYN REQUEST FROM REMOTE DEVICE -----
						//--------------------------------------------------------------------------------------------------------
						tcp_response_flags = (TCP_SYN | TCP_ACK);
						tcp_socket[socket].sm_socket_state = SM_TCP_SYN_REQUEST_RECEIVED;
						tcp_socket[socket].send_sequence_number++;
						tcp_socket[socket].next_segment_sequence_increment_value = 0;
					}
					tcp_socket[socket].flags.ready_for_tx = 1;

					if (tcp_data_length > 0)
					{
						//----------------------------------------------
						//----- PACKET HAS DATA WAITING TO BE READ -----
						//----------------------------------------------
						tcp_socket[socket].rx_data_bytes_remaining = tcp_data_length;
					}
					else
					{
						nic_rx_dump_packet();
						rx_has_been_dumped = 1;
					}
				}
				else
				{
					//----- NOT A VALID RESPONSE TO OUR SYN REQUEST - JUST DUMP PACKET -----
					nic_rx_dump_packet();
					rx_has_been_dumped = 1;
				}
			}
			else
			{
				//-------------------------------------------------------------------------
				//----- SYN NOT SENT OR RECEIVED LAST - SOCKET IS IN SOME OTHER STATE -----
				//-------------------------------------------------------------------------
				if (tcp_header->sequence_number != tcp_socket[socket].send_acknowledgement_number)
				{
					//-----------------------------------------------------------------------------------
					//----- RECEIVED SEQUENCE NUMBER DOES NOT MATCH OUR SEND ACKNOWLEDGEMENT NUMBER -----
					//-----------------------------------------------------------------------------------
					//This probably means we have missed a packet - dump the packet and wait for the missed packet to be re-sent
					nic_rx_dump_packet();
					rx_has_been_dumped = 1;
					return(1);
				}

				//UPDATE THE ACKNOWLEDGEMENT NUMBER WE WILL NEXT SEND BACK
				tcp_socket[socket].send_acknowledgement_number = tcp_header->sequence_number + (DWORD)tcp_data_length;

				if (tcp_socket[socket].sm_socket_state == SM_TCP_CONNECTION_ESTABLISHED)
				{
					//-----------------------------------------------
					//----- A CONNECTION IS ALREADY ESTABLISHED -----
					//-----------------------------------------------
					if (tcp_header->flags.bits.flag_ack)
					{
						//--------------------------------------------------
						//----- THIS IS AN ACK TO OUR LAST PACKET SENT -----
						//--------------------------------------------------
						if (tcp_socket[socket].flags.tx_last_tx_awaiting_ack == 1)
						{
							tcp_socket[socket].flags.tx_resend_last_tx = 0;
							tcp_socket[socket].flags.tx_last_tx_awaiting_ack = 0;
							tcp_socket[socket].flags.ready_for_tx = 1;
						}
					}
					
					if (tcp_header->flags.bits.flag_fin)
					{
						//----------------------------------------------------------------------------------------------------
						//----- FIN RECEIVED - REMOTE DEVICE HAS NO MORE DATA TO SEND AND WISHES TO CLOSE THE CONNECTION -----
						//----------------------------------------------------------------------------------------------------
						tcp_response_flags = (TCP_FIN | TCP_ACK);
						tcp_socket[socket].next_segment_sequence_increment_value = 1;
						tcp_socket[socket].send_acknowledgement_number++;
						tcp_socket[socket].sm_socket_state = SM_TCP_LAST_ACK_SENT;
					}

					if (tcp_data_length > 0)
					{
						//----------------------------------------------
						//----- PACKET HAS DATA WAITING TO BE READ -----
						//----------------------------------------------
						tcp_socket[socket].rx_data_bytes_remaining = tcp_data_length;

						if (!(tcp_response_flags & TCP_ACK))		//Don't delete fin flag from above if its been set
							tcp_response_flags = TCP_ACK;
					}
					else
					{
						//----- NO DATA SO JUST DUMP THE PACKET -----
						nic_rx_dump_packet();
						rx_has_been_dumped = 1;
					}

				}
				else if (tcp_socket[socket].sm_socket_state == SM_TCP_LAST_ACK_SENT)
				{
					//----- LAST ACK HAS BEEN SENT -----
					nic_rx_dump_packet();
					rx_has_been_dumped = 1;

					if ( tcp_header->flags.bits.flag_ack )
					{
						//-----------------------------------------------------------------------------
						//----- ACKNOWLEDGEMENT FROM REMOTE DEVICE TO LAST ACK - CLOSE THE SOCKET -----
						//-----------------------------------------------------------------------------
						tcp_close_socket(socket);
					}
				}
				else if (tcp_socket[socket].sm_socket_state == SM_TCP_FIN_WAIT_1)
				{
					//----- FIN HAS BEEN SENT -----
					nic_rx_dump_packet();
					rx_has_been_dumped = 1;

					if (tcp_header->flags.bits.flag_fin)
					{
						//----------------------------------------------------
						//----- FIN HAS BEEN RECEIVED FROM REMOTE DEVICE -----
						//----------------------------------------------------
						tcp_response_flags = TCP_ACK;
						tcp_socket[socket].send_acknowledgement_number++;
						if (tcp_header->flags.bits.flag_ack)
						{
							//--------------------------------------------------------------------------------
							//----- ACKNOWLEDGEMENT FROM REMOTE DEVICE TO FIN REQUEST - CLOSE THE SOCKET -----
							//--------------------------------------------------------------------------------
							tcp_close_socket(socket);
						}
						else
						{
							//----- NO ACKNOWLEDGEMENT - CHANGE STATE TO CLOSING -----
							tcp_socket[socket].sm_socket_state = SM_TCP_CLOSING;
						}
					}
				}
				else if (tcp_socket[socket].sm_socket_state == SM_TCP_CLOSING)
				{
					//----- IN CLOSING STATE -----
					nic_rx_dump_packet();
					rx_has_been_dumped = 1;

					if (tcp_header->flags.bits.flag_ack)
					{
						//------------------------------------------------------------
						//----- ACK RECEIVED TO CLOSING STATE - CLOSE THE SOCKET -----
						//------------------------------------------------------------
						tcp_close_socket(socket);
					}
				}
			}
		}
		else
		{
			//----------------------------------------------------------
			//----------------------------------------------------------
			//----- ACKNOWLEDGEMENT NUMBER RECEIVED IS NOT CORRECT -----
			//----------------------------------------------------------
			//----------------------------------------------------------
			nic_rx_dump_packet();
			rx_has_been_dumped = 1;
			
			if (tcp_socket[socket].sm_socket_state == SM_TCP_SYN_REQUEST_SENT)
			{
				//----------------------------------------------------------------------------------------------------------
				//----- WE ARE TRYING TO CONNECT BUT FOR SOME REASON THE REMOTE DEVICE IS ACKNOWLEDGING ANOTHER PACKET -----
				//----------------------------------------------------------------------------------------------------------
				//SEND RESET COMMAND TO REMOTE DEVICE SO THAT HOPEFULLY IT WILL CONNECT ON OUR NEXT ATTEMPT
				tcp_send_command_packet_no_socket(remote_device_info, tcp_header->destination_port, tcp_header->source_port, tcp_header->acknowledgment_number,
												tcp_header->sequence_number, TCP_RST);
				tcp_response_flags = 0;
			}

		}
	}

	//-------------------------------------
	//----- CHECK FOR SEND A RESPONSE -----
	//-------------------------------------
	tcp_tx_resp_after_proc_rx_resp_flags = 0;
	if (tcp_response_flags > 0x00)
	{
		if (rx_has_been_dumped)
		{
			//--------------------------------
			//----- SEND RESPONSE PACKET -----
			//--------------------------------
			tcp_send_command_packet_from_socket(socket, tcp_response_flags);
		}
		else
		{
			//--------------------------------------------------------------------------------------------------
			//----- SEND RESPONSE PACKET AFTER APPLICATION PROCESS HAS READ AND DUMPED THE CURRENT RX DATA -----
			//--------------------------------------------------------------------------------------------------
			//Store the response to be sent once the packet has been read and dumped
			tcp_tx_resp_after_proc_rx_socket = socket;
			tcp_tx_resp_after_proc_rx_resp_flags = tcp_response_flags;						//(Loading this variable will cause the response to be sent)


		}
	}

	if (rx_has_been_dumped)
		return(1);								//Flag that the nic packet has been dumped
	else
		return(0);								//Flag that the nic packet has not been dumped and is awaiting processing by the application

}





//*******************************************
//*******************************************
//*********** FIND MATCHING SOCKET **********
//*******************************************
//*******************************************
static BYTE tcp_rx_check_for_matches_socket (TCP_HEADER *rx_tcp_header, DEVICE_INFO *rx_device_info)
{
	BYTE tcp_socket_number;
	BYTE listening_socket_socket_number = TCP_INVALID_SOCKET;

	//----------------------------------------------------------------------
	//----- CHECK EACH TCP SOCKET LOOKING FOR A MATCH WITH THIS PACKET -----
	//----------------------------------------------------------------------
	for (tcp_socket_number = 0; tcp_socket_number < TCP_NO_OF_AVAILABLE_SOCKETS; tcp_socket_number++ )
	{
		//CHECK SOCKET IS ACTIVE
        if (tcp_socket[tcp_socket_number].sm_socket_state != SM_TCP_CLOSED)
        {
	        //DOES THE SOCKET LOCAL PORT MATCH THE SENDER DESTIANTION PORT?
			if (tcp_socket[tcp_socket_number].local_port == rx_tcp_header->destination_port)
			{
				//-----------------------------------------------------------------
				//----- THIS PACKET HAS BEEN SENT FOR THIS SOCKETS LOCAL PORT -----
				//-----------------------------------------------------------------
				if (tcp_socket[tcp_socket_number].sm_socket_state == SM_TCP_LISTEN)
				{
					//THIS SOCKET IS IN A LISTEN STATE - THIS IS A MATCH BUT TO ALLOW FOR MULTIPLE SOCKETS
					//SET TO THE SAME PORT TRY REMAINING SOCKETS TO SEE IF ONE IS ALREADY CONNECTED TO THIS DEVICE
					listening_socket_socket_number = tcp_socket_number;
				}

				if (
					(tcp_socket[tcp_socket_number].remote_device_info.ip_address.Val == rx_device_info->ip_address.Val) &&
					(tcp_socket[tcp_socket_number].remote_port == rx_tcp_header->source_port)
					)
				{
					//THIS SOCKET IS ALREADY CONNECTED TO THIS REMOTE DEVICE - JUST EXIT WITH THE SOCKET NUMBER
					//(IP address and remote port match)
					return(tcp_socket_number);
				}

			}
		}
	}

	//------------------------------------------------------------------------------------------
	//----- THIS PACKET IS NOT FOR A SOCKET THAT IS ALREADY CONNECTED TO THE REMOTE DEVICE -----
	//------------------------------------------------------------------------------------------

	//----- IF THE PACKET IS FOR A FREE LISTENING SOCKET THEN SETUP THAT SOCKET TO CONNECT WITH THE REMOTE DEVICE -----
	if (listening_socket_socket_number != TCP_INVALID_SOCKET)
	{
		//----- NEW CONNECTION FOR THIS LISTENING SOCKET -----
		tcp_socket[listening_socket_socket_number].remote_device_info.mac_address.v[0] = rx_device_info->mac_address.v[0];
		tcp_socket[listening_socket_socket_number].remote_device_info.mac_address.v[1] = rx_device_info->mac_address.v[1];
		tcp_socket[listening_socket_socket_number].remote_device_info.mac_address.v[2] = rx_device_info->mac_address.v[2];
		tcp_socket[listening_socket_socket_number].remote_device_info.mac_address.v[3] = rx_device_info->mac_address.v[3];
		tcp_socket[listening_socket_socket_number].remote_device_info.mac_address.v[4] = rx_device_info->mac_address.v[4];
		tcp_socket[listening_socket_socket_number].remote_device_info.mac_address.v[5] = rx_device_info->mac_address.v[5];
		tcp_socket[listening_socket_socket_number].remote_device_info.ip_address.Val = rx_device_info->ip_address.Val;

		//Store the remote port that should be replied to
		tcp_socket[listening_socket_socket_number].remote_port = rx_tcp_header->source_port;
	
		tcp_socket[listening_socket_socket_number].rx_data_bytes_remaining = 0;
		tcp_socket[listening_socket_socket_number].flags.ready_for_tx = 1;
		tcp_socket[listening_socket_socket_number].flags.tx_send_waiting_command = 0;

		return(listening_socket_socket_number);
	}
	
	//--------------------------------------------------------------
	//----- PACKET IS NOT FOR ANY AVAILABLE SOCKET - REJECT IT -----
	//--------------------------------------------------------------
	return(TCP_INVALID_SOCKET);

}





//*******************************************************************
//*******************************************************************
//********** OPEN SOCKET TO LISTEN FOR INCOMING CONNECTION **********
//*******************************************************************
//*******************************************************************
//Returns the socket number assigned to the calling process - this should be saved and used to access the socket, or TCP_INVALID_SOCKET is returned if no socket is available.
BYTE tcp_open_socket_to_listen (WORD port)
{
	BYTE socket;

	//----- FIND THE FIRST AVAILABLE SOCKET -----
	for (socket = 0; socket < TCP_NO_OF_AVAILABLE_SOCKETS; socket++)
	{
		if (tcp_socket[socket].sm_socket_state == SM_TCP_CLOSED)
		{
			//------------------------------------
			//----- THIS SOCKET IS AVAILABLE -----
			//------------------------------------
			//Set socket to listen
			tcp_socket[socket].sm_socket_state = SM_TCP_LISTEN;
			tcp_socket[socket].local_port = port;
			tcp_socket[socket].remote_port = 0;
			tcp_socket[socket].remote_device_info.ip_address.Val = 0x00000000;
			tcp_socket[socket].rx_data_bytes_remaining = 0;
			tcp_socket[socket].flags.ready_for_tx = 0;
			tcp_socket[socket].flags.tx_last_tx_awaiting_ack = 0;
			tcp_socket[socket].flags.tx_resend_last_tx = 0;
			tcp_socket[socket].flags.socket_is_server = 1;
			tcp_socket[socket].flags.tx_send_waiting_command = 0;

			return(socket);
		}
	}
	
	//------------------------------------------
	//----- ALL SOCKETS ARE ALREADY IN USE -----
	//------------------------------------------
	return(TCP_INVALID_SOCKET);
}




//********************************************************
//********************************************************
//********** CONNECT TO A REMOTE HOST USING TCP **********
//********************************************************
//********************************************************
//Sends a connection request to a remote host on a specified remote port.
//A new socket is automatically created, and a socket id is returned, or TCP_INVALID_SOCKET is returned if no socket is available.
//If remote_device_info.mac_address = 0x000000000000 then ARP will automatically be performed
BYTE tcp_connect_socket(DEVICE_INFO *remote_device_info, WORD remote_port)
{
	BYTE socket;
	BYTE bCount;
	BYTE bTemp;

	//----- FIND AN AVAILABLE SOCKET -----
	for (socket = 0; socket < TCP_NO_OF_AVAILABLE_SOCKETS; socket++)
	{
		if (tcp_socket[socket].sm_socket_state == SM_TCP_CLOSED )
		{
			//----- THIS SOCKET IS AVAILABLE -----
			break;
		}

		if (socket == (TCP_NO_OF_AVAILABLE_SOCKETS - 1))
		{
			//----- THERE IS NO SOCKET CURRENTLY AVAILABLE -----
			return(TCP_INVALID_SOCKET);
		}
	}

	//----- SOCKET ASSIGNED - SETUP SOCKET AND SEND SYN PACKET -----
	tcp_socket[socket].rx_data_bytes_remaining = 0;
	tcp_socket[socket].flags.ready_for_tx = 0;
	tcp_socket[socket].flags.tx_last_tx_awaiting_ack = 0;
	tcp_socket[socket].flags.tx_resend_last_tx = 0;
	tcp_socket[socket].flags.socket_is_server = 0;
	tcp_socket[socket].flags.tx_send_waiting_command = 0;


	//ASSIGN THE NEXT SEQUENTIAL PORT NUMBER
	bTemp = 1;
	while(bTemp)
	{
		tcp_socket[socket].local_port = next_local_tcp_port_to_use++;
		if (next_local_tcp_port_to_use > TCP_LOCAL_PORT_NUMBER_END)
			next_local_tcp_port_to_use = TCP_LOCAL_PORT_NUMBER_START;

		bTemp = 0;
		//Ensure the port number is not being used by another socket
		for (bCount = 0; bCount < TCP_NO_OF_AVAILABLE_SOCKETS; bCount++)
		{
			if (bCount != socket)
			{
				if ((tcp_socket[bCount].sm_socket_state != SM_TCP_CLOSED) && (tcp_socket[bCount].local_port == tcp_socket[socket].local_port))
					bTemp = 1;									//Get next socket number as this one is already in use
			}
		}
	}

	//ASSIGN THE NEXT SEQUENTIAL START SEQUENCE NUMBER
	tcp_socket[socket].send_sequence_number = ++next_connection_start_sequence_number;
	tcp_socket[socket].send_acknowledgement_number = 0;
	tcp_socket[socket].next_segment_sequence_increment_value = 0;

	//COPY THE REMOTE DEVICE INFO TO THE SOCKET
	tcp_socket[socket].remote_device_info.ip_address.Val = remote_device_info->ip_address.Val;
	tcp_socket[socket].remote_device_info.mac_address.v[0] = remote_device_info->mac_address.v[0];
	tcp_socket[socket].remote_device_info.mac_address.v[1] = remote_device_info->mac_address.v[1];
	tcp_socket[socket].remote_device_info.mac_address.v[2] = remote_device_info->mac_address.v[2];
	tcp_socket[socket].remote_device_info.mac_address.v[3] = remote_device_info->mac_address.v[3];
	tcp_socket[socket].remote_device_info.mac_address.v[4] = remote_device_info->mac_address.v[4];
	tcp_socket[socket].remote_device_info.mac_address.v[5] = remote_device_info->mac_address.v[5];


	//SET THE PORT TO CONNECT TO
	tcp_socket[socket].remote_port = remote_port;

	if (
	(remote_device_info->mac_address.v[0] != 0x00) ||
	(remote_device_info->mac_address.v[1] != 0x00) ||
	(remote_device_info->mac_address.v[2] != 0x00) ||
	(remote_device_info->mac_address.v[3] != 0x00) ||
	(remote_device_info->mac_address.v[4] != 0x00) ||
	(remote_device_info->mac_address.v[5] != 0x00)
	)
	{
		//----- WE KNOW THE MAC ADDRESS SO SEND SYN REQUEST NOW -----
		tcp_connect_socket_send_syn_request(socket);								//This will set the socket state
	}
	else
	{
		//----- WE DON'T KNOW THE MAC ADDRESS SO USE ARP FIRST -----

		//RESET THE TIMEOUT TIMER FOR THE SOCKET
		tcp_socket[socket].start_time = ethernet_10ms_clock_timer;
		tcp_socket[socket].time_out_value = TCP_INITIAL_TIMEOUT_VALUE_10MS;

		//SET SOCKET STATE MACHINE
		tcp_socket[socket].sm_socket_state = SM_TCP_CONNECT_SEND_ARP_REQUEST;		//Flag that we need to send an ARP request
	}

	//THE STATE MACHINE WILL DEAL WITH THE REST
	return(socket);
}


//********************************************************************
//********************************************************************
//********** SEND SYN REQUEST TO CONNECT TO THE REMOTE HOST **********
//********************************************************************
//********************************************************************
//Called by tcp_connect_socket or once ARP has been performed if the mac address was unknown when tcp_connect_socket was first called
void tcp_connect_socket_send_syn_request (BYTE socket)
{

	//SEND SYN PACKET
	tcp_send_command_packet_from_socket(socket, TCP_SYN);

	//UPDATE SOCKET STATE MACHINE
	tcp_socket[socket].sm_socket_state = SM_TCP_SYN_REQUEST_SENT;
	
	//UPDATE SOCKET SEQUENCE NUMBER
	tcp_socket[socket].next_segment_sequence_increment_value = 1;

	//RESET THE TIMEOUT TIMER FOR THE SOCKET
	tcp_socket[socket].start_time = ethernet_10ms_clock_timer;
	tcp_socket[socket].time_out_value = TCP_INITIAL_TIMEOUT_VALUE_10MS;

	return;
}



//**************************************************
//**************************************************
//********** CHECK IF SOCKET IS CONNECTED **********
//**************************************************
//**************************************************
BYTE tcp_is_socket_connected (BYTE socket)
{
	if (tcp_socket[socket].sm_socket_state == SM_TCP_CONNECTION_ESTABLISHED)
		return(1);
	else
		return(0);
}





//***************************************************
//***************************************************
//********** REQUEST DISCONNECT TCP SOCKET **********
//***************************************************
//***************************************************
//Send request to remote device to disconnect
void tcp_request_disconnect_socket (BYTE socket)
{
	BYTE flags;


	//----- IF SOCKET IS NOT CONNECTED THEN JUST CLOSE IT AND EXIT -----
	//It may be closed already or in process of closing.
	if (tcp_socket[socket].sm_socket_state != SM_TCP_CONNECTION_ESTABLISHED )
	{
		tcp_close_socket(socket);
		return;
	}

	//----- IF THE SOCKET HAS A RX PACKET WAITING DUMP IT -----
	if (tcp_socket[socket].rx_data_bytes_remaining)
	{
		nic_rx_dump_packet();
		tcp_socket[socket].rx_data_bytes_remaining = 0;
	}

	//----- SEND FIN PACKET -----
	flags = (TCP_FIN | TCP_ACK);
	//If we have just processed an RX packet and there are flags waiting to be sent in response then send them now to avoid them being sent
	//after this fin request.	
	if (tcp_rx_packet_is_waiting_to_be_processed && tcp_tx_resp_after_proc_rx_resp_flags)
	{
		flags |= tcp_tx_resp_after_proc_rx_resp_flags;
		tcp_tx_resp_after_proc_rx_resp_flags = 0;
	}

	tcp_send_command_packet_from_socket(socket, flags);

	//Update socket state machine
	tcp_socket[socket].sm_socket_state = SM_TCP_FIN_WAIT_1;

	//Update socket sequence number
	tcp_socket[socket].next_segment_sequence_increment_value = 1;

	return;
}



//**********************************
//**********************************
//********** CLOSE SOCKET **********
//**********************************
//**********************************
//This function is called when a socket is closed after calling tcp_request_disconnect_socket.  It may also be used to force the closure of a socket
//without requesting a closure with the remote device (for instance if the remote device is lost).
//If socket is a server close any curerent connection and reset back to waiting for a client connection
//If socket is a client close it.
void tcp_close_socket(BYTE socket)
{

	//EXIT IF SOCKET IS INVALID
	if ((socket == TCP_INVALID_SOCKET) || (socket >= TCP_NO_OF_AVAILABLE_SOCKETS))
		return;

	if (tcp_socket[socket].flags.tx_last_tx_awaiting_ack == 1)
	{
		tcp_socket[socket].flags.ready_for_tx = 1;
		tcp_socket[socket].flags.tx_last_tx_awaiting_ack = 0;
		tcp_socket[socket].flags.tx_resend_last_tx = 0;
	}

	//tcp_socket[socket].remote_device_info.ip_address.Val = 0x00000000;		//Don't do this as when a connection is closed following tcp_request_disconnect_socket this
	//tcp_socket[socket].remote_port = 0x00;									//information is needed to return a final ACK to the remote device.

	//IF THE SOCKET HAS A RX PACKET WAITING DUMP IT
	if (tcp_socket[socket].rx_data_bytes_remaining)
	{
		nic_rx_dump_packet();
		tcp_socket[socket].rx_data_bytes_remaining = 0;
	}

	tcp_socket[socket].time_out_value = TCP_INITIAL_TIMEOUT_VALUE_10MS;

	if (tcp_socket[socket].flags.socket_is_server == 0)
	{
		//----- SOCKET IS CLIENT - CLOSE IT SO ITS RELEASED -----
		tcp_socket[socket].sm_socket_state = SM_TCP_CLOSED;						//Flag that socket is closed and available to anyone
	}
	else
	{
		//----- SOCKET IS SERVER - RESET IT BACK TO LISTEN STATE READY FOR NEXT INCOMING CONNECTION -----
		tcp_socket[socket].sm_socket_state = SM_TCP_LISTEN;
	}

	return;
}



//*************************************************************************
//*************************************************************************
//********** CLOSE SOCKET FROM LISTENING FOR INCOMING CONNECTION **********
//*************************************************************************
//*************************************************************************
//Close a server socket (completely close it - tcp_close_socket only resets server sockets back to waiting for a new connection)
void tcp_close_socket_from_listen (BYTE socket)
{

	//EXIT IF SOCKET IS INVALID
	if ((socket == TCP_INVALID_SOCKET) || (socket >= TCP_NO_OF_AVAILABLE_SOCKETS))
		return;

	tcp_socket[socket].flags.socket_is_server = 0;
	tcp_close_socket(socket);
	return;

}



//******************************************
//******************************************
//********** IS TCP SOCKET CLOSED **********
//******************************************
//******************************************
//Returns 1 if the socket is closed
BYTE tcp_is_socket_closed (BYTE socket)
{
	if (tcp_socket[socket].sm_socket_state == SM_TCP_CLOSED)
		return(1);
	else
		return(0);
}




//****************************************************************
//****************************************************************
//********** TCP CHECK SOCKET IS READY TO TX NEW PACKET **********
//****************************************************************
//****************************************************************
//A socket is ready to transmit when it is connected to a remote node and its transmit buffer is empty.
//If tx has just occured then the socket will not be ready to transmit again until the data packet has been acknowledged by the remote device.
BYTE tcp_is_socket_ready_to_tx_new_packet (BYTE socket)
{

	//IS NIC READY TO TX?
	if (!nic_ok_to_do_tx())
		return(0);

	//DO WE NEED TO SEND A RESPONSE TO THE LAST RECEIVED PACKET FIRST?
	if ((tcp_rx_packet_is_waiting_to_be_processed > 0) & (tcp_tx_resp_after_proc_rx_resp_flags > 0))
		return(0);

	//IS SOCKET READY TO TX?
	if ((tcp_socket[socket].flags.tx_last_tx_awaiting_ack == 1) || (tcp_socket[socket].flags.ready_for_tx == 0))
		return(0);
	else
		return(1);
}




//************************************************************
//************************************************************
//********** TCP SEND COMMAND PACKET FROM NO SOCKET **********
//************************************************************
//************************************************************
//This function sends a normal TCP packet, but with no data and not from an open socket.
//It should only be used to communicate with a remote device that we are not connected to or going to connect
//to (i.e. to send a RST response to a conneciton request that cannot be serviced) as if it is used for a socket
//command the transfer flags and states will not be updated.
//The TCP header with the flags field is the command.
void tcp_send_command_packet_no_socket (DEVICE_INFO *remote_device_info, WORD local_port, WORD remote_port, DWORD tx_sequence_number,
					DWORD tx_acknowledgment_number, BYTE flags)
{

	//----- EXIT IF NIC IS NOT CURRENTLY ABLE TO SEND A NEW PACKET -----
	//(We accept the command will be lost in this instance but this is OK as this command is being sent in a not connected state so
	//we don't actually care if the command doesn't get there, its just a good idea for us to send it if we can.
	if (!nic_ok_to_do_tx())
	{
		return;
	}

	//----- SETUP TCP TX -----
	if (!tcp_setup_tx(remote_device_info, local_port, remote_port, tx_sequence_number, tx_acknowledgment_number, flags))
	{
		//WE CANNOT TX RIGHT NOW
		return;
	}

	//----- THERE IS NO DATA - TX THE PACKET -----
	tcp_tx_packet();
}




//*********************************************************
//*********************************************************
//********** TCP SEND COMMAND PACKET FROM SOCKET **********
//*********************************************************
//*********************************************************
//This function sends a normal TCP packet, but with no data, from a specified socket.
//The TCP header with the flags field is the command.
//If the nic can't tx then it will flag that the command needs to be sent the next time the sockets are processed
void tcp_send_command_packet_from_socket (BYTE socket, BYTE flags)
{

	//----- EXIT IF NIC IS NOT CURRENTLY ABLE TO SEND A NEW PACKET -----
	if (!nic_ok_to_do_tx())
	{
		tcp_socket[socket].flags.tx_send_waiting_command = 1;	//Flag that command needs to be sent
		tcp_socket[socket].waiting_command_flags = flags;
		return;
	}

	//----- SETUP TCP TX -----
	if (!tcp_setup_tx(&tcp_socket[socket].remote_device_info, tcp_socket[socket].local_port, tcp_socket[socket].remote_port,
			tcp_socket[socket].send_sequence_number, tcp_socket[socket].send_acknowledgement_number, flags))
	{
		//WE CANNOT TX RIGHT NOW
		tcp_socket[socket].flags.tx_send_waiting_command = 1;	//Flag that command needs to be sent
		tcp_socket[socket].waiting_command_flags = flags;
		return;
	}

	//----- DO SOCKET TX FLAGS -----
	tcp_socket[socket].flags.tx_last_tx_had_data = 0;
	tcp_socket[socket].flags.tx_resend_last_tx = 0;
	tcp_socket[socket].flags.tx_send_waiting_command = 0;

	//----- STORE THE LAST TX FLAGS IN CASE PACKET NEEDS TO BE RE-SENT -----
	tcp_socket[socket].tx_last_tx_flags = flags;

	//----- THERE IS NO DATA - TX THE PACKET -----
	tcp_tx_packet();
}




//*********************************************
//*********************************************
//********** TCP SETUP SOCKET FOR TX **********
//*********************************************
//*********************************************
//Returns 1 if tx setup and ready to send data to nic, 0 if unable to setup tx at this time (no tx buffer available from the nic)
//Use this function to setup tx from a socket (its the same as tcp_setup_tx but the socket parameters are automatically used)
BYTE tcp_setup_socket_tx (BYTE socket)
{
	BYTE temp;

	//----- ENSURE SOCKET IS READY TO TX -----
	if (!tcp_is_socket_ready_to_tx_new_packet(socket))
		return(0);

	//----- ADJUST THE SEND SEQUENCE NUMBER FROM THE LAST SEGMENT IF THIS IS NOT A RE-SEND -----
	if (tcp_socket[socket].flags.tx_resend_last_tx == 0)
	{
		if (tcp_socket[socket].next_segment_sequence_increment_value)
		{
			tcp_socket[socket].send_sequence_number += (DWORD)tcp_socket[socket].next_segment_sequence_increment_value;
			tcp_socket[socket].next_segment_sequence_increment_value = 0;
		}
	}

	//----- SETUP TX -----
	temp = tcp_setup_tx(&tcp_socket[socket].remote_device_info, tcp_socket[socket].local_port, tcp_socket[socket].remote_port, tcp_socket[socket].send_sequence_number,
						tcp_socket[socket].send_acknowledgement_number, TCP_ACK);

	//----- DO SOCKET TX FLAGS -----
	tcp_socket[socket].flags.tx_last_tx_had_data = 0;			//Default to no data in the packet

	//----- STORE THE LAST TX FLAGS IN CASE PACKET NEEDS TO BE RE-SENT -----
	tcp_socket[socket].tx_last_tx_flags = TCP_ACK;

    return(temp);
}




//**********************************
//**********************************
//********** TCP SETUP TX **********
//**********************************
//**********************************
//Returns 1 if tx setup and ready to send data to nic, 0 if unable to setup tx at this time (no tx buffer available from the nic)
//Use tcp_is_socket_ready_to_tx_new_packet first (if transmitting from a socket)
BYTE tcp_setup_tx (DEVICE_INFO *remote_device_info, WORD local_port, WORD remote_port, DWORD tx_sequence_number,
                       DWORD tx_acknowledgment_number, BYTE tx_flags)
{
	TCP_HEADER tcp_header;
	TCP_OPTIONS tcp_options;
	PSEUDO_HEADER pseudo_header;

	//----- EXIT IF NIC IS NOT CURRENTLY ABLE TO SEND A NEW PACKET -----
	if (!nic_ok_to_do_tx())
		return(0);

	//----- SETUP THE NIC READY TO TX A NEW PACKET -----
	if (!nic_setup_tx())
		return(0);							//No tx buffer currently available from the nic - try again alter

	//----- SETUP THE TCP HEADER -----
	tcp_header.source_port = local_port;
	tcp_header.destination_port = remote_port;
	tcp_header.sequence_number = tx_sequence_number;
	tcp_header.acknowledgment_number = tx_acknowledgment_number;
	tcp_header.flags.bits.reserved = 0;
	tcp_header.header_length.bits.reserved = 0;
	tcp_header.flags.byte = tx_flags;
	tcp_header.window = TCP_MAX_SEGMENT_SIZE;
	tcp_header.checksum = 0;
	tcp_header.urgent_pointer = 0;

	//SWAP THE HEADER WORDS READY FOR CHECKSUMMING AND TX
    swap_tcp_header(&tcp_header);

	//----- IF SENDING SYN PACKET THEN INCLUDE MAXIMUM SEGMENT SIZE IN OPTION FIELD -----
	//----- ALSO SET THE HEADER LENGTH -----
	if (tx_flags & TCP_SYN)
	{
		tcp_options.id = TCP_OPTIONS_MAX_SEG_SIZE;			//OPTION TYPE
		tcp_options.length = 4;
		tcp_options.max_seg_size.v[0] = (BYTE)(TCP_MAX_SEGMENT_SIZE >> 8);
		tcp_options.max_seg_size.v[1] = (BYTE)(TCP_MAX_SEGMENT_SIZE & 0x00ff);

		tcp_header.header_length.bits.val = ((TCP_HEADER_LENGTH + TCP_OPTIONS_LENGTH) >> 2);
	}
	else
	{
		tcp_header.header_length.bits.val = (TCP_HEADER_LENGTH >> 2);
	}

	//----- CREATE TCP PSEUDOHEADER FOR THE CHECKSUM -----
	pseudo_header.source_address.Val = our_ip_address.Val;
	pseudo_header.destination_address.Val = remote_device_info->ip_address.Val;
	pseudo_header.zero = 0;
	pseudo_header.protocol = IP_PROTOCOL_TCP;
	pseudo_header.tcp_length = 0;					//We will add this to the checksum at the end just before the packet is transmitted as we don't know the length right now

	//----- START CALCULATION OF TCP CHECKSUM -----
	tcp_tx_checksum = 0;
	tcp_tx_checksum_next_byte_low = 0;
	//ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, (BYTE*)&pseudo_header, PSEUDO_HEADER_LENGTH);
    ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, (BYTE*)&pseudo_header.source_address, 4);
    ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, (BYTE*)&pseudo_header.destination_address, 4);
    ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, (BYTE*)&pseudo_header.zero, 1);
    ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, (BYTE*)&pseudo_header.protocol, 1);
    ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, (BYTE*)&pseudo_header.tcp_length, 2);


	//----- ADD THE TCP HEADER TO THE CHECKSUM -----
	//ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, (BYTE*)&tcp_header, TCP_HEADER_LENGTH);
    ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, (BYTE*)&tcp_header.source_port, 2);
    ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, (BYTE*)&tcp_header.destination_port, 2);
    ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, (BYTE*)&tcp_header.sequence_number, 4);
    ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, (BYTE*)&tcp_header.acknowledgment_number, 4);
    ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, (BYTE*)&tcp_header.header_length.byte, 1);
    ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, (BYTE*)&tcp_header.flags.byte, 1);
    ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, (BYTE*)&tcp_header.window, 2);
    ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, (BYTE*)&tcp_header.checksum, 2);
    ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, (BYTE*)&tcp_header.urgent_pointer, 2);

	//----- ADD THE OPTIONS TO THE CHECKSUM -----
	if (tx_flags & TCP_SYN)
	{
		//ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, (BYTE*)&tcp_options, TCP_OPTIONS_LENGTH);
        ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, (BYTE*)&tcp_options.id, 1);
        ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, (BYTE*)&tcp_options.length, 1);
        ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, (BYTE*)&tcp_options.max_seg_size.Val, 2);
	}

	//----- WRITE THE ETHERNET & IP HEADER TO THE NIC -----
	ip_write_header(remote_device_info, IP_PROTOCOL_TCP);

	//----- WRITE THE TCP HEADER TO THE NIC -----
	//nic_write_array((BYTE*)&tcp_header, TCP_HEADER_LENGTH);
    nic_write_array((BYTE*)&tcp_header.source_port, 2);
    nic_write_array((BYTE*)&tcp_header.destination_port, 2);
    nic_write_array((BYTE*)&tcp_header.sequence_number, 4);
    nic_write_array((BYTE*)&tcp_header.acknowledgment_number, 4);
    nic_write_next_byte(tcp_header.header_length.byte);
    nic_write_next_byte(tcp_header.flags.byte);
    nic_write_array((BYTE*)&tcp_header.window, 2);
    nic_write_array((BYTE*)&tcp_header.checksum, 2);
    nic_write_array((BYTE*)&tcp_header.urgent_pointer, 2);

	//----- WRITE THE OPTIONS TO THE NIC -----
	if (tx_flags & TCP_SYN)
	{
		//nic_write_array((BYTE*)&tcp_options, TCP_OPTIONS_LENGTH);
        nic_write_next_byte(tcp_options.id);
        nic_write_next_byte(tcp_options.length);
        nic_write_array((BYTE*)&tcp_options.max_seg_size.Val, 2);
	}

	//----- NOW JUST WRITE THE TCP DATA ----
	tcp_tx_data_byte_length = 0;

	return(1);
}




//*****************************************
//*****************************************
//********** TCP WRITE NEXT BYTE **********
//*****************************************
//*****************************************
//Returns 0 if byte could not be written as max packet size reached
BYTE tcp_write_next_byte (BYTE data)
{

	//EXIT IF TOO MANY BYTES WRITTEN
	if (tcp_tx_data_byte_length > MAX_TCP_DATA_LEN)
		return(0);

	//WRITE THE BYTE
	nic_write_next_byte(data);

	//ADD TO THE CHECKSUM
	ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, (BYTE*)&data, 1);

	//UPDATE THE DATA BYTE COUNT
	tcp_tx_data_byte_length++;
	
	return(1);
}




//*************************************
//*************************************
//********** TCP WRITE ARRAY **********
//*************************************
//*************************************
//Returns 0 if byte could not be written as max packet size reached
BYTE tcp_write_array (BYTE *array_buffer, WORD array_length)
{

	//EXIT IF TOO MANY BYTES WRITTEN
	if ((tcp_tx_data_byte_length + array_length) > MAX_TCP_DATA_LEN)
		return(0);

	//WRITE THE ARRAY
	nic_write_array(array_buffer, array_length);

	//ADD TO THE CHECKSUM
	ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, array_buffer, array_length);

	//UPDATE THE DATA BYTE COUNT
	tcp_tx_data_byte_length += array_length;
	
	return(1);
}




//******************************************
//******************************************
//********** TCP SOCKET TX PACKET **********
//******************************************
//******************************************
//Use this function to transmit packet from a socket (its the same as tcp_tx_packet but the socket parameters are automatically updated)
void tcp_socket_tx_packet (BYTE socket)
{

	//----- FLAG IF THIS PACKET CONTAINS DATA -----
	//(Needs to be known if a re-transmission is required)
	if (nic_tx_len - ETHERNET_HEADER_LENGTH - IP_HEADER_LENGTH - TCP_HEADER_LENGTH)
	{
		tcp_socket[socket].flags.tx_last_tx_had_data = 1;
	}

	//----- SEND THE PACKET -----
	tcp_tx_packet();

	//----- UPDATE THE SEQUENCE NUBMER WITH THE NUMBER OF BYTES SENT -----
	tcp_socket[socket].next_segment_sequence_increment_value = tcp_tx_data_byte_length;

	//----- RESET THE TIMEOUT TIMER FOR THE SOCKET -----
	if (!tcp_socket[socket].flags.tx_resend_last_tx)		//Don't do if this is a re-send of a previous packet
	{
		tcp_socket[socket].start_time = ethernet_10ms_clock_timer;
		tcp_socket[socket].time_out_value = TCP_INITIAL_TIMEOUT_VALUE_10MS;
	}

	//----- DO SOCKET TX FLAGS -----
	tcp_socket[socket].flags.ready_for_tx = 0;					//TX can only happen again when this packet gets acknowledged)
	tcp_socket[socket].flags.tx_last_tx_awaiting_ack = 1;
	tcp_socket[socket].flags.tx_resend_last_tx = 0;

}




//***********************************
//***********************************
//********** TCP TX PACKET **********
//***********************************
//***********************************
void tcp_tx_packet (void)
{
	WORD tcp_length;


	//----- GET THE TCP PACKET LENGTH -----
	tcp_length = (nic_tx_len - ETHERNET_HEADER_LENGTH - IP_HEADER_LENGTH);
	tcp_length = swap_word_bytes(tcp_length);

	//----- ADD THE LENGTH TO THE TCP CHECKSUM -----
	//(In place of where it should have been for the pseudo header)
	tcp_tx_checksum_next_byte_low = 0;
	ip_add_bytes_to_ip_checksum (&tcp_tx_checksum, &tcp_tx_checksum_next_byte_low, (BYTE*)&tcp_length, 2);

	//----- WRITE THE TCP CHECKSUM FIELD -----
	tcp_tx_checksum = swap_word_bytes(~tcp_tx_checksum);
	nic_write_tx_word_at_location ((ETHERNET_HEADER_LENGTH + IP_HEADER_LENGTH + 16), tcp_tx_checksum);

	//----- TX THE PACKET -----
	ip_tx_packet();
}




//*******************************************************************
//*******************************************************************
//********** TCP DOES SOCKET REQUIRE RESEND OF LAST PACKET **********
//*******************************************************************
//*******************************************************************
BYTE tcp_does_socket_require_resend_of_last_packet (BYTE socket)
{

	if (tcp_socket[socket].sm_socket_state == SM_TCP_CLOSED)
		return(0);

	if (tcp_socket[socket].flags.tx_resend_last_tx == 1)
		return(1);
	else
		return(0);
}






//*********************************************
//*********************************************
//********** TCP CHECK FOR SOCKET RX **********
//*********************************************
//*********************************************
//Every application process that has an open TCP socket must reguarly call this funciton to see if the 
//socket has recevied a packet and if so process and dump the packet (the ethernet stack is halted until the
//received packet is dumped)
BYTE tcp_check_socket_for_rx (BYTE socket)
{

	if (tcp_rx_packet_is_waiting_to_be_processed)
	{
		if (tcp_rx_active_socket == socket)
			return(1);
	}
	return (0);
}




//******************************************************
//******************************************************
//********** TCP GET NEXT BYTE FROM RX PACKET **********
//******************************************************
//******************************************************
//Returns 0 if no more data available, 1 if byte retreived
BYTE tcp_read_next_rx_byte (BYTE *data)
{
	//Check that there are bytes still to be retreived
	if (tcp_socket[tcp_rx_active_socket].rx_data_bytes_remaining == 0)
		return (0);

	//Decrement the byte count
	tcp_socket[tcp_rx_active_socket].rx_data_bytes_remaining--;

	//Get the next byte from the nic
	if (nic_read_next_byte(data))
		return (1);
	else
		return(0);				//Error - nic routine says no more bytes available - doesn't concur with TCP rx function
}




//***************************************************
//***************************************************
//********** TCP READ ARRAY FROM RX PACKET **********
//***************************************************
//***************************************************
//Returns 0 if no more data available, 1 if byte retreived
BYTE tcp_read_rx_array (BYTE *array_buffer, WORD array_length)
{
	
	for ( ;array_length > 0; array_length--)
	{
		//Check that there are bytes still to be retreived
		if (tcp_socket[tcp_rx_active_socket].rx_data_bytes_remaining == 0)
			return (0);

		//Decrement the byte count
		tcp_socket[tcp_rx_active_socket].rx_data_bytes_remaining--;

		//Get the next byte from the nic
		if (nic_read_next_byte(array_buffer++) == 0)
			return(0);				//Error - nic routine says no more bytes available - doesn't concur with TCP rx function

	}
	return(1);
}




//****************************************
//****************************************
//********** TCP DUMP RX PACKET **********
//****************************************
//****************************************
void tcp_dump_rx_packet (void)
{

	nic_rx_dump_packet();

    tcp_socket[tcp_rx_active_socket].rx_data_bytes_remaining = 0;		//(Doing this flags to the tcp_process_rx function that rx is complete and the stack can move on)
}






//*************************************
//*************************************
//********** SWAP TCP HEADER **********
//*************************************
//*************************************
static void swap_tcp_header (TCP_HEADER* tcp_header)
{
	tcp_header->source_port = swap_word_bytes(tcp_header->source_port);
	tcp_header->destination_port = swap_word_bytes(tcp_header->destination_port);
	tcp_header->sequence_number = swap_dword_bytes(tcp_header->sequence_number);
	tcp_header->acknowledgment_number = swap_dword_bytes(tcp_header->acknowledgment_number);
	tcp_header->window = swap_word_bytes(tcp_header->window);
	tcp_header->checksum = swap_word_bytes(tcp_header->checksum);
	tcp_header->urgent_pointer = swap_word_bytes(tcp_header->urgent_pointer);
}








