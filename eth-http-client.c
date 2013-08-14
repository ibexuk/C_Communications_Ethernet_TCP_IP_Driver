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
//HTTP CLIENT C CODE FILE


#include "main.h"					//Global data type definitions (see https://github.com/ibexuk/C_Generic_Header_File )
#define HTTP_CLIENT_C
#include "eth-http-client.h"

#undef HTTP_CLIENT_C

#include "eth-main.h"
#include "eth-tcp.h"
#include "eth-dns.h"


//--------------------------------------------------------------------------
//----- GENERATE COMPILE ERROR IF HTTP HAS NOT BEEN DEFINED TO BE USED -----
//--------------------------------------------------------------------------
#ifndef STACK_USE_HTTP_CLIENT
#error HTTP client file is included in project but not defined to be used - remove file from project to reduce code size.
#endif





//***********************************************
//***********************************************
//********** START HTTP CLIENT REQUEST **********
//***********************************************
//***********************************************
//host_url
//	Pointer to host URL string (e.g. "www.google.com"). 
//filename
//	Pointer to filename string (e.g. "index.html", or "files/file01.txt"). 
//Returns 1 if starting request, 0 if cannot at this time (e.g. not connected or currently busy carrying out a previous request)
//The HTTP_CLIENT_REQUEST_RECEIVE_FUNCTION function will deal with receiving the response if sucessful
BYTE start_http_client_request (CONSTANT BYTE *host_url, CONSTANT BYTE *filename)
{
	BYTE count;

	//CHECK WE CAN START THE REQUEST
	if (!nic_linked_and_ip_address_valid)
		return(0);
	
	if (http_client_state != SM_HTTP_CLIENT_IDLE)
		return(0);				//Already doing a request
	

	//Get the strings
	for (count = 0; count < HTTP_CLIENT_MAX_URL_LENGTH; count++)
		http_client_host_url[count] = *host_url++;
	http_client_host_url[(HTTP_CLIENT_MAX_URL_LENGTH - 1)] = 0x00;			//Ensure its null terminated

	for (count = 0; count < HTTP_CLIENT_MAX_FILENAME_LENGTH; count++)
		http_client_filename[count] = *filename++;
	http_client_filename[(HTTP_CLIENT_MAX_FILENAME_LENGTH - 1)] = 0x00;		//Ensure its null terminated


    http_client_state = SM_HTTP_CLIENT_RESOLVE_URL_START;
    http_client_100ms_timeout_timer = 250;									//Set timeout to 25 seconds
	return(1);
}




//*****************************************
//*****************************************
//********** PROCESS HTTP CLIENT **********
//*****************************************
//*****************************************
//This function is automatically called by tcp_ip_process_stack().
void process_http_client (void)
{
	static BYTE our_tcp_client_socket = TCP_INVALID_SOCKET;
	static WORD our_tcp_client_local_port;
	static DEVICE_INFO remote_device_info;
	static BYTE eth_http_client_10ms_clock_timer_last;
	static DWORD http_client_content_length;
	static DWORD http_client_content_received;
	static BYTE header_started;
	static BYTE header_ended;
	static BYTE data_buffer[4];
	static BYTE content_length_match_char;



	//-------------------------
	//----- UPDATE TIMERS -----
	//-------------------------
	if ((BYTE)((BYTE)(ethernet_10ms_clock_timer & 0x000000ff) - eth_http_client_10ms_clock_timer_last) >=	10)
	{
		eth_http_client_10ms_clock_timer_last = (BYTE)(ethernet_10ms_clock_timer & 0x000000ff);
		
		//TIMEOUT TIMER
		if (http_client_100ms_timeout_timer)
			http_client_100ms_timeout_timer--;
	}

	if (
	(http_client_state != SM_HTTP_CLIENT_IDLE) &&
	((http_client_100ms_timeout_timer == 0) || (!nic_linked_and_ip_address_valid))
	)
	{
		http_client_state = SM_HTTP_CLIENT_FAIL;
	}



	if (our_tcp_client_socket != TCP_INVALID_SOCKET)
	{
		//----- CHECK OUR SOCKET HASN'T DISCONNECTED -----
		if ((tcp_socket[our_tcp_client_socket].sm_socket_state == SM_TCP_CLOSED) || (tcp_socket[our_tcp_client_socket].local_port != our_tcp_client_local_port))		//If the local port has changed then our socket was closed and taken by some other application process since we we're last called
			our_tcp_client_socket = TCP_INVALID_SOCKET;

		if (http_client_state == SM_HTTP_CLIENT_WAIT_FOR_DISCONNECT)
			http_client_state = SM_HTTP_CLIENT_IDLE;
		else if (http_client_state != SM_HTTP_CLIENT_IDLE)
			http_client_state = SM_HTTP_CLIENT_FAIL;
	}


	switch (http_client_state)
	{
	case SM_HTTP_CLIENT_IDLE:
		//----------------
		//----- IDLE -----
		//----------------
		tcp_close_socket(our_tcp_client_socket);			//Ensure socket is closed
		break;



	case SM_HTTP_CLIENT_RESOLVE_URL_START:
		//-----------------------------
		//----- RESOLVE URL START -----
		//-----------------------------
		//----- START NEW DNS QUERY -----
		if (do_dns_query(http_client_host_url, QNS_QUERY_TYPE_HOST))			//If false DNS query not currently available (already doing a query) - try again next time
			http_client_state = SM_HTTP_CLIENT_RESOLVE_URL_WAIT;



	case SM_HTTP_CLIENT_RESOLVE_URL_WAIT:
		//----------------------------
		//----- RESOLVE URL WAIT -----
		//----------------------------
	
		//----- IS DNS QUERY IS COMPLETE? -----
		remote_device_info.ip_address = check_dns_response();
		if (remote_device_info.ip_address.Val == 0xffffffff)
		{
			//DNS QUERY FAILED
			http_client_state = SM_HTTP_CLIENT_FAIL;		//(Timed out or invalid response)
		}
		else if (remote_device_info.ip_address.Val)
		{
			//DNS QUERY SUCESSFUL
            http_client_state = SM_HTTP_CLIENT_OPEN_SOCKET;
		}
		else
		{
			//DNS NOT YET COMPLETE
		}
		break;



	case SM_HTTP_CLIENT_OPEN_SOCKET:
		//-----------------------
		//----- OPEN SOCKET -----
		//-----------------------
		//remote_device_info.ip_address.Val =			//Already got
		remote_device_info.mac_address.v[0] = 0;		//Set to zero so TCP will automatically use ARP to find the MAC address
		remote_device_info.mac_address.v[1] = 0;
		remote_device_info.mac_address.v[2] = 0;
		remote_device_info.mac_address.v[3] = 0;
		remote_device_info.mac_address.v[4] = 0;
		remote_device_info.mac_address.v[5] = 0;
	
		//Connect to remote device port 80
		if (our_tcp_client_socket != TCP_INVALID_SOCKET)		//We shouldn't have a socket currently, but make sure
				tcp_close_socket(our_tcp_client_socket);
	
		our_tcp_client_socket = tcp_connect_socket(&remote_device_info, 80);	
		if (our_tcp_client_socket != TCP_INVALID_SOCKET)
		{	
			our_tcp_client_local_port = tcp_socket[our_tcp_client_socket].local_port;
			http_client_state = SM_HTTP_CLIENT_WAIT_FOR_CONNECTION;
			break;
		}
		//Could not open a socket - none currently available - keep trying
		break;
	


	case SM_HTTP_CLIENT_WAIT_FOR_CONNECTION:
		//--------------------------------------
		//----- WAIT FOR SOCKET TO CONNECT -----
		//--------------------------------------
		if (tcp_is_socket_connected(our_tcp_client_socket))
			http_client_state = SM_HTTP_CLIENT_TX_REQUEST;
	
		break;



	case SM_HTTP_CLIENT_TX_REQUEST:
		//----------------------------------------------
		//----- TX REQUEST PACKET TO REMOTE DEVICE -----
		//----------------------------------------------
		if (!tcp_setup_socket_tx(our_tcp_client_socket))
		{
			//Can't tx right now - try again next time
			break;
		}
	
		//WRITE THE TCP DATA
		http_client_send_const_string(http_client_request_text_get_start);
        http_client_send_const_string((CONSTANT BYTE*)http_client_filename);
		http_client_send_const_string(http_client_request_text_get_end);
		http_client_send_const_string(http_client_request_text_host_start);
        http_client_send_const_string((CONSTANT BYTE*)http_client_host_url);
		http_client_send_const_string(http_client_request_text_remainder);
	
		//SEND THE PACKET
		tcp_socket_tx_packet(our_tcp_client_socket);

		http_client_state = SM_HTTP_CLIENT_WAIT_FOR_RESPONSE;

		//Setup for receive (headers may span more than 1 packet)
		http_client_request_ok = 0;
		http_client_content_length = 0xffffffff;			//Default to length not specified by server
		http_client_content_received = 0;
		data_buffer[1] = data_buffer[2] = data_buffer[3] = 0x00;
		header_started = 0;
		header_ended = 0;
		content_length_match_char = 0;
		break;
	


	case SM_HTTP_CLIENT_WAIT_FOR_RESPONSE:
		//-----------------------------
		//----- WAIT FOR RESPONSE -----
		//-----------------------------
		if (tcp_check_socket_for_rx(our_tcp_client_socket))
		{
			//----- RESPONSE RECEIVED - PROCESS IT -----
			while (tcp_read_next_rx_byte(&data_buffer[0]))		//Function will return 0 if there are no more bytes to read
			{
				if ((data_buffer[0] != ' ') && (data_buffer[0] != 0x0d) && (data_buffer[0] != 0x0a))		//Ignore any leading blank lines before header
					header_started = 1;
				
				if ((data_buffer[3] == 0x0d) && (data_buffer[2] == 0x0a) && (data_buffer[1] == 0x0d) && (data_buffer[0] == 0x0a) && (header_started))
				{
					//----- END OF HEADER -----
					//File data follows
                    header_ended = 1;
					break;
				}
				else if ((data_buffer[3] == ' ') && (data_buffer[2] == '2') && (data_buffer[1] == '0') && (data_buffer[0] == '0'))
				{
					//----- 200 OK RESPONSE -----
					http_client_request_ok = 1;
				}

				//LOOK FOR CONTENT LENGTH
                if (content_length_match_char < 0xff)
				{
					if (content_length_match_char >= 15)
					{
						//WE ARE READING THE VALUE
						if (data_buffer[0] != ' ')
						{
							if ((data_buffer[0] < '0') || (data_buffer[0] > '9'))
							{
								//All done
								content_length_match_char = 0xff;
							}
							else
							{
								//Add next character to value
								http_client_content_length *= 10;
								http_client_content_length += (data_buffer[0] - 0x30);
							}
						}
					}
					else
					{
						//NOT MATCHED ENTIRE HEADER TITLE YET
						if (convert_character_to_lower_case(data_buffer[0]) == http_client_content_length_text[content_length_match_char])
							content_length_match_char++;
						else
							content_length_match_char = 0;
					}
				}

				data_buffer[3] = data_buffer[2];
				data_buffer[2] = data_buffer[1];
				data_buffer[1] = data_buffer[0];
			} //while (tcp_read_next_rx_byte(&data[0]))	

			//----- HEADER COMPLETE -----
			if (!http_client_request_ok)
			{
				//----- RESPONSE WAS NOT 200 OK - INFORM USER APPLICATION -----
                http_client_content_length = 0;
				HTTP_CLIENT_REQUEST_RECEIVE_FUNCTION(0, http_client_content_length, &http_client_host_url[0], &http_client_filename[0]);		//Send error message to user application
			}
			else if (!header_ended)
			{
				//----- OK RESPONSE BUT NOT REACHED THE END OF THE HEADERS YET -----
                tcp_dump_rx_packet();
				break;

			}
			else
			{
				//----- GET USER APPLICATION TO READ THE FILE DATA -----
                http_client_content_received += tcp_socket[our_tcp_client_socket].rx_data_bytes_remaining;
				HTTP_CLIENT_REQUEST_RECEIVE_FUNCTION(1, http_client_content_length, &http_client_host_url[0], &http_client_filename[0]);
				http_client_state = SM_HTTP_CLIENT_WAIT_FOR_RESPONSE_1;
			}

			//----- DUMP THE PACKET -----
			tcp_dump_rx_packet();
			
			if (http_client_content_received < http_client_content_length)
				http_client_state = SM_HTTP_CLIENT_WAIT_FOR_RESPONSE_1;
			else
				http_client_state = SM_HTTP_CLIENT_REQUEST_DISCONNECT;
		}
	
		if (tcp_does_socket_require_resend_of_last_packet(our_tcp_client_socket))
		{
			//----- RE-SEND LAST PACKET TRANSMITTED -----
			//(TCP requires resending of packets if they are not acknowledged and to avoid requiring a large RAM buffer the application needs to remember
			//the last packet sent on a socket so it can be resent if requried)
			http_client_state = SM_HTTP_CLIENT_TX_REQUEST;
		}
	
		if(!tcp_is_socket_connected(our_tcp_client_socket))
		{
			//----- THE CLIENT HAS DISCONNECTED -----
			HTTP_CLIENT_REQUEST_RECEIVE_FUNCTION(0xff, http_client_content_length, &http_client_host_url[0], &http_client_filename[0]);		//Send disconnect message to user application
			http_client_state = SM_HTTP_CLIENT_IDLE;
		}
		break;





	case SM_HTTP_CLIENT_WAIT_FOR_RESPONSE_1:
		//------------------------------------------------
		//----- WAIT FOR ADDITIONAL RESPONSE PACKETS -----
		//------------------------------------------------
		if (tcp_check_socket_for_rx(our_tcp_client_socket))
		{
			//----- RESPONSE RECEIVED - PROCESS IT -----

			//----- GET USER APPLICATION TO READ THE FILE DATA -----
            http_client_content_received += tcp_socket[our_tcp_client_socket].rx_data_bytes_remaining;
			HTTP_CLIENT_REQUEST_RECEIVE_FUNCTION(2, http_client_content_length, &http_client_host_url[0], &http_client_filename[0]);		//Send error message to user application

			//DUMP THE PACKET
			tcp_dump_rx_packet();

			if (http_client_content_received >= http_client_content_length)
				http_client_state = SM_HTTP_CLIENT_REQUEST_DISCONNECT;
		}

		if(!tcp_is_socket_connected(our_tcp_client_socket))
		{
			//----- THE CLIENT HAS DISCONNECTED -----
			HTTP_CLIENT_REQUEST_RECEIVE_FUNCTION(0xff, http_client_content_length, &http_client_host_url[0], &http_client_filename[0]);		//Send disconnect message to user application
			http_client_state = SM_HTTP_CLIENT_IDLE;
		}
		
		break;




	case SM_HTTP_CLIENT_REQUEST_DISCONNECT:
		//----------------------------------------------------
		//----- REQUEST TO DISCONNECT FROM REMOTE SERVER -----
		//----------------------------------------------------
		tcp_request_disconnect_socket (our_tcp_client_socket);
	
		http_client_state = SM_HTTP_CLIENT_WAIT_FOR_DISCONNECT;
		break;



	case SM_HTTP_CLIENT_WAIT_FOR_DISCONNECT:
		//----------------------------------------------
		//----- WAIT FOR SOCKET TO BE DISCONNECTED -----
		//----------------------------------------------	
		if (tcp_is_socket_closed(our_tcp_client_socket))
		{
			http_client_state = SM_HTTP_CLIENT_IDLE;
		}
		break;
	


	case SM_HTTP_CLIENT_FAIL:
		//--------------------------
		//----- PROCESS FAILED -----
		//--------------------------
        tcp_close_socket(our_tcp_client_socket);			//Ensure socket is closed

		HTTP_CLIENT_REQUEST_RECEIVE_FUNCTION(0, 0, &http_client_host_url[0], &http_client_filename[0]);		//Send error message to user application

        http_client_state = SM_HTTP_CLIENT_IDLE;
		break;
	}

}




//******************************************
//******************************************
//********** SEND CONSTANT STRING **********
//******************************************
//******************************************
void http_client_send_const_string (CONSTANT BYTE *string_to_send)
{
	BYTE data;
	
	while (1)
	{
		data = *string_to_send++;
		
		if (data == 0x00)
			return;
			
		tcp_write_next_byte(data);
	}
}





