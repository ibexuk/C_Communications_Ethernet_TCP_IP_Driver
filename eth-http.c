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
//HTTP HOST C CODE FILE


#include "main.h"					//Global data type definitions (see https://github.com/ibexuk/C_Generic_Header_File )
#define HTTP_C
#include "eth-http.h"

#ifdef HTTP_USING_C_FILES
#include "html_c.h"
#endif
#ifdef HTTP_USING_BINARY_FILES
#include "html_bin.h"
#endif 

#undef HTTP_C

#include "eth-main.h"
#include "eth-tcp.h"


#ifdef HTTP_DYNAMIC_DATA_FUNCTION
#include "ap-main.h"			//File that includes the definition for the function used to process http output dynamic data variables (only requried if this function is being used)
#endif

#ifdef HTTP_PROCESS_INPUT_FUNCTION
#include "ap-main.h"			//File that includes the definition for the function used to process http request input values (only requried if this function is being used)
#endif

#ifdef HTTP_AUTHORISE_REQUEST_FUNCTION
#include "ap-main.h"			//File that includes the definition for the function used to process http request input values (only requried if this function is being used)
#endif


//--------------------------------------------------------------------------
//----- GENERATE COMPILE ERROR IF HTTP HAS NOT BEEN DEFINED TO BE USED -----
//--------------------------------------------------------------------------
#ifndef STACK_USE_HTTP
#error HTTP file is included in project but not defined to be used - remove file from project to reduce code size.
#endif







//*************************************
//*************************************
//********** INITIALISE HTTP **********
//*************************************
//*************************************
//Called from the tcp_ip_initialise function
void http_initialise (void)
{
	BYTE socket_number;
	
	//Set all our sockets to default not assigned state
	for (socket_number = 0; socket_number < HTTP_NO_OF_AVAILABLE_SOCKETS; socket_number++)
	{
		http_socket[socket_number].tcp_socket_id = TCP_INVALID_SOCKET;
		http_socket[socket_number].sm_http_state = HTTP_WAITING_FOR_CONNECTION;
	}

}



//**********************************
//**********************************
//********** PROCESS HTTP **********
//**********************************
//**********************************
//This function is called reguarly by tcp_ip_process_stack
void process_http (void)
{
	BYTE socket_number;


	//---------------------------------------
	//---------------------------------------
	//----- CHECK FOR NIC NOT CONNECTED -----
	//---------------------------------------
	//---------------------------------------
	if (!nic_linked_and_ip_address_valid)
	{
		//--------------------------------------------------------
		//----- NIC IS NOT CONNECTED OR DHCP IS NOT COMPLETE -----
		//--------------------------------------------------------
		//ENSURE ALL OF OUR SOCKETS ARE CLOSED
		for (socket_number = 0; socket_number < HTTP_NO_OF_AVAILABLE_SOCKETS; socket_number++)
		{
			//Do for each of our sockets
			if (http_socket[socket_number].tcp_socket_id != TCP_INVALID_SOCKET)
			{
				//Socket is not currently closed - close it
				tcp_close_socket_from_listen(http_socket[socket_number].tcp_socket_id);
				http_socket[socket_number].tcp_socket_id = TCP_INVALID_SOCKET;
				http_socket[socket_number].sm_http_state = HTTP_WAITING_FOR_CONNECTION;
			}
		}
		//----- EXIT -----
		return;
	}


	//--------------------------------
	//--------------------------------
	//----- PROCESS HTTP SOCKETS -----
	//--------------------------------
	//--------------------------------
	for (socket_number = 0; socket_number < HTTP_NO_OF_AVAILABLE_SOCKETS; socket_number++)
	{
		//-------------------------------
		//----- PROCESS EACH SOCKET -----
		//-------------------------------
		if(!tcp_is_socket_connected(http_socket[socket_number].tcp_socket_id))			//If a socket has disconnected during a transfer reset its state
			http_socket[socket_number].sm_http_state = HTTP_WAITING_FOR_CONNECTION;

		switch (http_socket[socket_number].sm_http_state)
		{
		case HTTP_WAITING_FOR_CONNECTION:
			//------------------------------------------------
			//------------------------------------------------
			//----- WAITING FOR CONNECTION FROM A CLIENT -----
			//------------------------------------------------
			//------------------------------------------------
			//CHECK TO SEE IF THIS HTTP SOCKET HAS NOT YET OBTAINED A TCP SOCKET
			if (http_socket[socket_number].tcp_socket_id == TCP_INVALID_SOCKET)
			{
				//Open port 80 ready to receive incoming http requests (multiple sockets on same port are permitted by the TCP driver)
				http_socket[socket_number].tcp_socket_id = tcp_open_socket_to_listen(HTTP_PORT);
			}	

			//CHECK TO SEE IF THIS SOCKET HAS BECOME CONNECTED
			if (http_socket[socket_number].tcp_socket_id != TCP_INVALID_SOCKET)
			{
				if(tcp_is_socket_connected(http_socket[socket_number].tcp_socket_id))
				{
					//----- THIS SOCKET HAS BECOME CONNECTED TO A CLIENT -----
					http_socket[socket_number].sm_http_state = HTTP_CONNECTED_TO_CLIENT;
					http_socket[socket_number].last_activity_time = ethernet_10ms_clock_timer;		//Reset the inactivity timeout timer
				}
			}
			break;
	
	
		case HTTP_CONNECTED_TO_CLIENT:
		case HTTP_PROCESSING_POST:
			//---------------------------------------------
			//---------------------------------------------
			//----- HTTP SERVER IS ONLINE AND WAITING -----
			//---------------------------------------------
			//---------------------------------------------

			//-----------------------------------------
			//----- CHECK FOR NO ACTIVITY TIMEOUT -----
			//-----------------------------------------
			if ((ethernet_10ms_clock_timer - http_socket[socket_number].last_activity_time) > HTTP_NO_ACTIVITY_TIMEOUT_TIME)
			{
				//---------------------------------------------------------------------------
				//----- SOCKET HAS HAD NO ACTIVITY FOR TOO LONG - DISCONNECT THE CLIENT -----
				//---------------------------------------------------------------------------
				if (tcp_is_socket_connected(http_socket[socket_number].tcp_socket_id))
				{
						tcp_close_socket(http_socket[socket_number].tcp_socket_id);
				}
				http_socket[socket_number].sm_http_state = HTTP_WAITING_FOR_CONNECTION;
				break;
			}


			//-------------------------------------
			//----- CHECK FOR PACKET RECEIVED -----
			//-------------------------------------
			if (tcp_check_socket_for_rx(http_socket[socket_number].tcp_socket_id))
			{
				//----------------------------------------------
				//----- SOCKET HAS DATA WAITING TO BE READ -----
				//----------------------------------------------
				//CLIENT HAS SENT US A TCP PACKET
				
				http_socket[socket_number].last_activity_time = ethernet_10ms_clock_timer;		//Reset the inactivity timeout timer

				//PROCESS THE PACKET
				#ifdef HTTP_ACCEPT_POST_REQUESTS
					if (http_socket[socket_number].sm_http_state == HTTP_PROCESSING_POST)
						http_process_post_message_body(socket_number);								//This is the next packet of a http POST request
					else
						http_process_rx(socket_number);												//This is a new request packet
				#else
						http_process_rx(socket_number);												//This is a new request packet
				#endif
			}

			//---------------------------------------------------
			//----- CHECK TO SEE IF CLIENT HAS DISCONNECTED -----
			//---------------------------------------------------
			if(!tcp_is_socket_connected(http_socket[socket_number].tcp_socket_id))
			{
				//----- THIS SOCKET HAS BECOME DISCONNECTED FROM ITS A CLIENT -----
				http_socket[socket_number].sm_http_state = HTTP_WAITING_FOR_CONNECTION;
			}

			break;


		case HTTP_START_HEAD_RESPONSE:
		case HTTP_START_RESPONSE:
		case HTTP_CONTINUE_RESPONSE:
			//----------------------------------------------
			//----------------------------------------------
			//----- SENDING RESPONSE TO CLIENT REQUEST -----
			//----------------------------------------------
			//----------------------------------------------
			if (tcp_does_socket_require_resend_of_last_packet(http_socket[socket_number].tcp_socket_id))
			{
				//-----------------------------------------------
				//----- RE-SEND THE LAST PACKET TRANSMITTED -----
				//-----------------------------------------------
				//(TCP requires resending of packets if they are not acknowledged.  To avoid requiring a large RAM buffer we
				//remember how to re-send the last packet sent on a socket so it can be re-sent if requried)
				if (tcp_is_socket_ready_to_tx_new_packet(http_socket[socket_number].tcp_socket_id))
				{
					http_transmit_next_response_packet(socket_number, 1);
				}
			}
			else
			{
				//----------------------------
				//----- SEND NEXT PACKET -----
				//----------------------------
				if (tcp_is_socket_ready_to_tx_new_packet(http_socket[socket_number].tcp_socket_id))
				{
					http_transmit_next_response_packet(socket_number, 0);
				}
			}
			break;


		case HTTP_RETURN_BAD_REQUEST:
		case HTTP_RETURN_NOT_FOUND:
		case HTTP_RETURN_NOT_IMPLEMENTED:
		case HTTP_RETURN_SERVICE_UNAVAILABLE:
			//-------------------------------
			//-------------------------------
			//----- SEND ERROR RESPONSE -----
			//-------------------------------
			//-------------------------------
			if (tcp_does_socket_require_resend_of_last_packet(http_socket[socket_number].tcp_socket_id))
			{
				//-----------------------------------------------
				//----- RE-SEND THE LAST PACKET TRANSMITTED -----
				//-----------------------------------------------
				//(TCP requires resending of packets if they are not acknowledged.  To avoid requiring a large RAM buffer we
				//remember how to re-send the last packet sent on a socket so it can be re-sent if requried)
				if (tcp_is_socket_ready_to_tx_new_packet(http_socket[socket_number].tcp_socket_id))
				{
					http_transmit_error_response(socket_number, 1);
				}
			}
			else
			{
				//----------------------------
				//----- SEND NEXT PACKET -----
				//----------------------------
				if (tcp_is_socket_ready_to_tx_new_packet(http_socket[socket_number].tcp_socket_id))
				{
					http_transmit_error_response(socket_number, 0);
				}
			}
			break;

	
		case HTTP_CLOSE_CONNECTION:
			//-----------------------------------
			//-----------------------------------
			//----- CLOSE CLIENT CONNECTION -----
			//-----------------------------------
			//-----------------------------------
			if (tcp_is_socket_connected(http_socket[socket_number].tcp_socket_id))
			{
					tcp_request_disconnect_socket(http_socket[socket_number].tcp_socket_id);
			}
			http_socket[socket_number].sm_http_state = HTTP_WAITING_FOR_CONNECTION;
			break;


			//-----------------------------------
			//----- END OF SWITCH STATEMENT -----
			//-----------------------------------
		} //switch (http_socket[socket_number].sm_http_state)

		//---------------------------------------------
		//----- END OF TASKS FOR THIS HTTP SOCKET -----
		//---------------------------------------------
	} //for (socket_number = 0; socket_number < HTTP_NO_OF_AVAILABLE_SOCKETS; socket_number++)

}



//**************************************************************
//**************************************************************
//********** PROCESS RECEIVED PACKET FROM HTTP CLIENT **********
//**************************************************************
//**************************************************************
//Recevied requests are passed to this function and it will accept GET, HEAD and POST request packets.
void http_process_rx (BYTE socket_number)
{
	BYTE data_buffer[4];
	BYTE request_type;
	BYTE waiting_for_forward_slash;
	BYTE filename[HTTP_MAX_FILENAME_LENGTH];
	BYTE file_extension[3];
	BYTE filename_next_character;
	BYTE file_extension_next_character;
	BYTE reading_filename;
	BYTE inputs_present;
	BYTE b_count;
	BYTE *p_string;
	BYTE data;


	//-----------------------------------------------
	//----- THIS IS A NEW REQUEST FROM A CLIENT -----
	//-----------------------------------------------
	http_socket[socket_number].file_bytes_sent_last_time = 0;		//Ensure any previous flag values are not present in this register

	//------------------------------------------------------------------
	//------------------------------------------------------------------
	//----- GET THE REQUEST TYPE FROM THE START OF THE PACKET DATA -----
	//------------------------------------------------------------------
	//------------------------------------------------------------------
	data_buffer[1] = data_buffer[2] = data_buffer[3] = 0x00;
	request_type = HTTP_REQUEST_TYPE_NULL;
	
	while (http_read_next_byte_to_buffer(&data_buffer[0]))		//Function will return 0 if there are no more bytes to read
	{
		if ((data_buffer[2] == 'G') && (data_buffer[1] == 'E') && (data_buffer[0] == 'T'))	//(will always be uppercase)
		{
			//----- GET REQUEST -----
			request_type = HTTP_REQUEST_TYPE_GET;
			break;
		}
		else if ((data_buffer[3] == 'H') && (data_buffer[2] == 'E') && (data_buffer[1] == 'A') && (data_buffer[0] == 'D'))		//(will always be uppercase)
		{
			//----- HEAD REQUEST -----
			request_type = HTTP_REQUEST_TYPE_HEAD;
			break;
		}
		#ifdef HTTP_ACCEPT_POST_REQUESTS
			else if ((data_buffer[3] == 'P') && (data_buffer[2] == 'O') && (data_buffer[1] == 'S') && (data_buffer[0] == 'T'))		//(will always be uppercase)
			{
				//----- POST REQUEST -----
				request_type = HTTP_REQUEST_TYPE_POST;
				break;
			}
		#endif
		//We don't insist on the first line containing the request as the http specification allows there to be blank lines first, so just keep looking until we find the request
	
	} //while (tcp_read_next_rx_byte(&data[0]))	

	if (request_type == HTTP_REQUEST_TYPE_NULL)		//Ensure the request type was found
	{
		//---------------------------
		//----- INVALID REQUEST -----
		//---------------------------
		http_socket[socket_number].sm_http_state = HTTP_RETURN_NOT_IMPLEMENTED;
		tcp_dump_rx_packet();
		return;
	}

	//------------------------------------
	//----- WE HAVE THE REQUEST TYPE -----
	//------------------------------------


	//----------------------------
	//----------------------------
	//----- GET THE FILENAME -----
	//----------------------------
	//----------------------------
	//The filename follows immediately after the request

	//IGNORE SPACES BEFORE THE FILENAME STARTS
	data_buffer[0] = ' ';
	while (data_buffer[0] == ' ')
	{
		if (tcp_read_next_rx_byte(&data_buffer[0]) == 0)
		{
			//ERROR - NOT ENOUGH BYTES IN PACKET
			http_socket[socket_number].sm_http_state = HTTP_RETURN_BAD_REQUEST;
			tcp_dump_rx_packet();
			return;
		}	
	}

	//READ THE FILENAME
	data_buffer[1] = data_buffer[2] = data_buffer[3] = 0x00;
	waiting_for_forward_slash = 1;					//Name must start with a forward slash
	filename_next_character = 0;
	file_extension_next_character = 0;
	reading_filename = 1;
	inputs_present = 0;								//Default to there being no arguments appearing after the filename

	while (1)
	{
		//data_buffer[0] has the next byte

		if (data_buffer[0] == ' ')
		{
			//-----------------------------------
			//----- SPACE - END OF FILENAME -----
			//-----------------------------------
			break;
		}
		else if (data_buffer[0] == '?')
		{
			//-----------------------------------------------------
			//----- '?' - END OF FILENAME AND START OF INPUTS -----
			//-----------------------------------------------------
			inputs_present = 1;
			break;
		}
		else if (data_buffer[0] == '%')
		{
			//------------------------------------------------------------
			//----- START OF HEXADECIMAL REPRESENTED ASCII CHARACTER -----
			//------------------------------------------------------------
			//2 hex bytes will follow
		}
		else if (data_buffer[1] == '%')
		{
			//---------------------------------------------------------------------
			//----- 1ST CHARACTER OF HEXADECIMAL REPRESENTED ASCII CHARACTER  -----
			//---------------------------------------------------------------------
			//1 more hex bytes will follow
		}
		else if (data_buffer[2] == '%')
		{
			//--------------------------------------------------------------------
			//----- 2ND CHARACTER OF HEXADECIMAL REPRESENTED ASCII CHARACTER -----
			//--------------------------------------------------------------------
			//Convert it to ASCII
			if (!waiting_for_forward_slash)		//Don't add to string if we are waiting on the next forward slash to appear
			{
				if (reading_filename)
				{
					if (filename_next_character < HTTP_MAX_FILENAME_LENGTH)
						filename[filename_next_character++] = convert_ascii_hex_to_byte(data_buffer[1], data_buffer[0]);
				}	
				else
				{
					if (file_extension_next_character < 3)			//If extension is > 3 characters simply ignore any additional characters as we only deal with 3 character extensions
						file_extension[file_extension_next_character++] = convert_ascii_hex_to_byte(data_buffer[1], data_buffer[0]);
				}
			}
		}
		else if (data_buffer[0] == '/')
		{
			//------------------------------
			//----- '/' FORWARD SLASH  -----
			//------------------------------
			if ((!reading_filename) || (waiting_for_forward_slash))
			{
				//We're waiting on a forward slash or we thought we we're reading the file extension so assume that what we've actually
				//just read is a leading domain name.  Reset as we're now reading the start of the filename
				waiting_for_forward_slash = 0;
				filename_next_character = 0;
				file_extension_next_character = 0;
				reading_filename = 1;
			}	
			else
			{
				//Add to the filename
				if (filename_next_character < HTTP_MAX_FILENAME_LENGTH)
					filename[filename_next_character++] = data_buffer[0];
			}
		}
		else if (data_buffer[0] == '.')
		{
			//----------------------------------------------------
			//----- '.' - MOVE TO EXTENSION PART OF FILENAME -----
			//----------------------------------------------------
			if (reading_filename)
			{
				//We are reading the file name so now move to the file extension
				reading_filename = 0;
				file_extension_next_character = 0;
			}
			else
			{
				//We are already reading the file extension so assume that what we've just read is part of a full domain name that the client has preceeded the
				//filename with.  The filename will appear later so flag that we're waiting for the next forward slash character which will be the start of the
				//actual filename.
				waiting_for_forward_slash = 1;
			}	
		}
		else
		{
			//--------------------------
			//----- NEXT CHARACTER -----
			//--------------------------
			if (!waiting_for_forward_slash)		//Don't add to string if we are waiting on the next forward slash to appear
			{
				if (reading_filename)
				{
					if (filename_next_character < HTTP_MAX_FILENAME_LENGTH)
						filename[filename_next_character++] = data_buffer[0];
				}	
				else
				{
					if (file_extension_next_character < 3)			//If extension is > 3 characters simply ignore any additional characters as we only deal with 3 character extensions
						file_extension[file_extension_next_character++] = data_buffer[0];
				}	
			}
		}

		//----- READ NEXT BYTE -----
		if (http_read_next_byte_to_buffer(&data_buffer[0]) == 0)
		{
			//ERROR - NOT ENOUGH BYTES IN PACKET
			http_socket[socket_number].sm_http_state = HTTP_RETURN_BAD_REQUEST;
			tcp_dump_rx_packet();
			return;
		}

	} //while (1)


	if ((filename_next_character == 0) && (file_extension_next_character == 0) && (!waiting_for_forward_slash))
	{
		//--------------------------------------------------------------
		//----- FILENAME WAS BLANK - USE THE DEFAULT ROOT FILENAME -----
		//--------------------------------------------------------------
		filename[0] = 'i';
		filename[1] = 'n';
		filename[2] = 'd';
		filename[3] = 'e';
		filename[4] = 'x';
		filename_next_character = 5;
		file_extension[0] = 'h';
		file_extension[1] = 't';
		file_extension[2] = 'm';
		file_extension_next_character = 3;
	}
	
	//----- ADD TERMINATING NULL TO THE FILE NAME -----
	if (filename_next_character < HTTP_MAX_FILENAME_LENGTH)
		filename[filename_next_character++] = 0x00;
	else
		filename[HTTP_MAX_FILENAME_LENGTH - 1] = 0x00;

	//----- CHECK FILENAME IS VALID -----
	if ((filename_next_character <= 1) || (file_extension_next_character != 3) || (waiting_for_forward_slash))
	{
		http_socket[socket_number].sm_http_state = HTTP_RETURN_BAD_REQUEST;
		tcp_dump_rx_packet();
		return;
	}	

	//--------------------------------------------------------------------
	//----- ENSURE FILENAME IS LOWER CASE TO REMOVE CASE SENSITIVITY -----
	//--------------------------------------------------------------------
	convert_string_to_lower_case(&filename[0]);
	file_extension[0] = convert_character_to_lower_case(file_extension[0]);
	file_extension[1] = convert_character_to_lower_case(file_extension[1]);
	file_extension[2] = convert_character_to_lower_case(file_extension[2]);

	//------------------------------------------------------------------------
	//----- CALL USER APPLICATION IN CASE IT WANTS TO REJECT THE REQUEST -----
	//------------------------------------------------------------------------
	#ifdef HTTP_AUTHORISE_REQUEST_FUNCTION
		if (!HTTP_AUTHORISE_REQUEST_FUNCTION(&filename[0], &file_extension[0], http_socket[socket_number].tcp_socket_id))
		{
			//----- USER APPLICATION HAS REJECTED THIS REQUEST - SEND 400 BAD REQUEST RESPONSE -----
			http_socket[socket_number].sm_http_state = HTTP_RETURN_BAD_REQUEST;
			tcp_dump_rx_packet();
			return;
		}
	#endif


	//---------------------------------------------------
	//---------------------------------------------------
	//----- GET ANY INPUTS THAT FOLLOW THE FILENAME -----
	//---------------------------------------------------
	//---------------------------------------------------
	//Inputs appear following a '?' character and are seperated by the '&' character
	//A request with inputs may look like this for example:- GET /index.htm?variable1=25&varaible2=1280&string1=Hello+World HTTP/1.0<CR><LF>
	//Any spaces will have been converted to '+' so we need to convert back and any occurances of %xx represent a single hex byte

	if ((inputs_present) && (request_type == HTTP_REQUEST_TYPE_GET))		//Inputs after the filename are only valid for GET requests
	{	
		//We call a seperate function to process the inputs as the post method requires the same handling later in the packet
		if (http_process_inputs(&filename[0], &file_extension[0], socket_number, 0) == 0)
		{
			//ERROR - NOT ENOUGH BYTES IN PACKET
			http_socket[socket_number].sm_http_state = HTTP_RETURN_BAD_REQUEST;
			tcp_dump_rx_packet();
			return;
		}
	}


	//----------------------------
	//----------------------------
	//----- READ ANY HEADERS -----
	//----------------------------
	//----------------------------
	//We generally ignore headers as we don't need to learn specific information about the client.
	//However if the client is sending us data we need to obtain the Content-Length value to determin the length of the data.

	#ifdef HTTP_ACCEPT_POST_REQUESTS
		if (request_type == HTTP_REQUEST_TYPE_POST)
		{
			//-----------------------------------------------------
			//----- THIS IS A POST REQUEST SO PROCESS HEADERS -----
			//-----------------------------------------------------
		
			//----- DUMP THIS REQUEST IF ANY OTHER SOCKET IS CURRENTLY PROCESSING A POST REQUEST -----
			//We only permit one socket to be receiving a multi packet post request at any one time, to avoid large ram buffering requirements
			for (b_count = 0; b_count < HTTP_NO_OF_AVAILABLE_SOCKETS; b_count++)
			{
				if ((http_socket[b_count].sm_http_state == HTTP_PROCESSING_POST) && (b_count != socket_number))
				{
					http_socket[socket_number].sm_http_state = HTTP_RETURN_SERVICE_UNAVAILABLE;
					tcp_dump_rx_packet();
					return;
				}
			}
			
			//SET DEFAULT VALUES
			http_post_content_bytes_remaining = 0xffffffff;		//Default to maximum possible in case no value is given
			http_post_is_multipart_type = 0;					//Default to not a multipart POST type
		
			//----- READ EACH LINE -----
			http_post_leading_spaces_passed = 0;			//Ignore any leading spaces
			http_post_line_length = 0;
			http_post_boundary_length = 100;				//Values from 100 to 108 are used as flags to search for the string "boundary=" before the actual boundary value
			http_post_boundary_string[69] = 0x00;			//Value of 0x00 in the last byte of the boundary array is used as a flag that the boundary string is no yet all stored (0x00 is an illegal boundary string value)
			
			while (1)
			{
				if (tcp_read_next_rx_byte(&data_buffer[0]) == 0)
				{
					//ERROR - NOT ENOUGH BYTES IN PACKET
					http_socket[socket_number].sm_http_state = HTTP_RETURN_BAD_REQUEST;
					tcp_dump_rx_packet();
					return;
				}	
	
				//----- CHECK FOR END OF LEADING SPACES -----
				if (data_buffer[0] != ' ')
					http_post_leading_spaces_passed = 1;
	
				//----- ADD THE NEW BYTE TO LINE BUFFER -----
				if ((http_post_leading_spaces_passed) && (data_buffer[0] != 0x0a) && (data_buffer[0] != 0x0d))		//Ignore carriage return (0x0a) & line feed (0x0d)
				{
					if (http_post_line_length < HTTP_MAX_POST_LINE_LENGTH)		//We only read the first # bytes of very long lines
					{
						http_post_line_buffer[http_post_line_length] = data_buffer[0];
						http_post_line_length++;
					}
				}
				
				//----- LOOK FOR THE BOUNDARY PARAMETER -----
				//(we do this seperatly here as the boundary line length may well exceede HTTP_MAX_POST_LINE_LENGTH)
				if (http_post_boundary_string[69] == 0x00)			//Do this if we haven't got all of the boundary string yet
				{
					data = convert_character_to_lower_case(data_buffer[0]);
					switch (http_post_boundary_length)
					{
					//----- LOOKING FOR BOUNDARY PARAMETER STRING -----
					case 100:
						if (data == 'b')
							http_post_boundary_length++;
						break;
					case 101:
						if (data == 'o')
							http_post_boundary_length++;
						else
							http_post_boundary_length = 100;			//Doesn't match reset to start looking again
						break;
					case 102:
						if (data == 'u')
							http_post_boundary_length++;
						else
							http_post_boundary_length = 100;			//Doesn't match reset to start looking again
						break;
					case 103:
						if (data == 'n')
							http_post_boundary_length++;
						else
							http_post_boundary_length = 100;			//Doesn't match reset to start looking again
						break;
					case 104:
						if (data == 'd')
							http_post_boundary_length++;
						else
							http_post_boundary_length = 100;			//Doesn't match reset to start looking again
						break;
					case 105:
						if (data == 'a')
							http_post_boundary_length++;
						else
							http_post_boundary_length = 100;			//Doesn't match reset to start looking again
						break;
					case 106:
						if (data == 'r')
							http_post_boundary_length++;
						else
							http_post_boundary_length = 100;			//Doesn't match reset to start looking again
						break;
					case 107:
						if (data == 'y')
							http_post_boundary_length++;
						else
							http_post_boundary_length = 100;			//Doesn't match reset to start looking again
						break;
					case 108:
						if (data == '=')
							http_post_boundary_length = 0;				//This is the boundary value marker - the next byte is the start of the boundary string
						else
							http_post_boundary_length = 100;
						
						break;
					
					//----- STORING THE BOUNDARY VALUE -----
					default:
						if ((data_buffer[0] >= '!') && (data_buffer[0] <= '~'))
						{
							//NEXT BYTE OF THE BOUNDARY STRING
							if (http_post_boundary_length < 70)
							{
								http_post_boundary_string[http_post_boundary_length] = data_buffer[0];
								http_post_boundary_length++;
							}
						}
						else
						{
							//----- END OF THE BOUNDARY STRING -----
							if (http_post_boundary_string[69] == 0x00)		//Clear string not stored yet marker if its not been used to store the actual boundary (if boundary is actually 70 bytes long - won't necessarily be)
								http_post_boundary_string[69] = 0x01;		//Non 0x00 value = boundary value stored
						}
					} //switch (http_post_boundary_length)
				} //if (http_post_boundary_string[69] == 0x00)

				
				//CHECK FOR END OF LINE
				if (data_buffer[0] == 0x0a)			//<LF> (must be last byte of a line)
				{
					//------------------------------------------------------------------
					//----- END OF A LINE - CHECK FOR HEADERS WE ARE INTERESTED IN -----
					//------------------------------------------------------------------
					
					//ADD NULL TERMINATION TO STRING
					if (http_post_line_length < HTTP_MAX_POST_LINE_LENGTH)
						http_post_line_buffer[http_post_line_length] = 0x00;
					else
						http_post_line_buffer[HTTP_MAX_POST_LINE_LENGTH - 1] = 0x00;
					
					
					//----- LOOK FOR BLANK LINE -----
					if (http_post_line_length == 0)
						break;							//A blank line marks the end of the headers
					
					//----- LOOK FOR CONTENT LENGTH -----
					p_string = find_string_in_string_no_case (&http_post_line_buffer[0], http_content_length);
					if (p_string)
					{
						p_string += (sizeof(http_content_length) - 1);		//Move pointer to 1st character after the string
						while (*p_string == ' ')							//Move past any leading spaces
							p_string++;
						http_post_content_bytes_remaining = convert_ascii_to_dword(p_string);
					}	

					//----- LOOK FOR MULTIPART/FORM DATA OPTION -----
					p_string = find_string_in_string_no_case (&http_post_line_buffer[0], http_multipart_form_data);
					if (p_string)
					{
						http_post_is_multipart_type = 1;
					}	

	
					//<<<<< LOOK FOR ANY OTHER HEADER TYPES HERE
	
	
					//----- SETUP FOR READ OF THE NEXT LINE -----
					http_post_leading_spaces_passed = 0;			//Ignore any leading spaces
					http_post_line_length = 0;
				}
			} //while (1)
		}
		//else
		//{
			//------------------------------------------------
			//----- NOT A POST REQUEST SO IGNORE HEADERS -----
			//------------------------------------------------
		//}
	#endif //#ifdef HTTP_ACCEPT_POST_REQUESTS


	#ifdef HTTP_ACCEPT_POST_REQUESTS
		if (request_type == HTTP_REQUEST_TYPE_POST)
		{
			//-----------------------------------------------------------------------
			//----- THIS IS A POST REQUEST WITH MESSAGE BODY CONTENT TO BE READ -----
			//-----------------------------------------------------------------------
			
			//Store the filename of this request
			for (b_count = 0; b_count < HTTP_MAX_FILENAME_LENGTH; b_count++)
				http_post_filename[b_count] = filename[b_count];
		
			http_post_file_extension[0] = file_extension[0];
			http_post_file_extension[1] = file_extension[1];
			http_post_file_extension[2] = file_extension[2];

			//SETUP FOR NEW POST MESSAGE BODY
			http_post_line_length = 0;				//Setup for a new line
			http_post_reading_data = 0xff;			//0xff = not got to first multipart boundary yet
			http_post_boundary_compare_length = 2;	//Not matched any of the boundary yet but we have just passed the <CR><LF> so actually 1st 2 bytes of potential boundary have matched

			//-------------------------------------------
			//----- PROCESS ANY DATA IN THIS PACKET -----
			//-------------------------------------------
			//(Browsers may include the posted data after the headers or they may start the data with the start of a new TCP packet)
			http_process_post_message_body(socket_number);

			if (http_post_content_bytes_remaining)
			{
				//------------------------------------------------------------------------------------------------
				//----- THERE IS MORE CONTENT TO BE RECEIVED FOR THIS POST REQUEST IN SUBSEQUENT TCP PACKETS -----
				//------------------------------------------------------------------------------------------------
				//Exit now and flag new socket state as still receiving a post request (this will also block other post requests until its complete)
				http_socket[socket_number].sm_http_state = HTTP_PROCESSING_POST;
				tcp_dump_rx_packet();
			}			
			return;	
		}
	#endif

	
	//------------------------------------
	//----- DUMP THE RECEIVED PACKET -----
	//------------------------------------
	tcp_dump_rx_packet();

	//-------------------------
	//----- SEND RESPONSE -----
	//-------------------------
	http_setup_response(socket_number, request_type, &filename[0], &file_extension[0]);
}



//*****************************************
//*****************************************
//********** SETUP HTTP RESPONSE **********
//*****************************************
//*****************************************
void http_setup_response (BYTE socket_number, BYTE request_type, BYTE *request_filename, BYTE *request_file_extension)
{
	BYTE file_count;
	BYTE b_count;
	BYTE filename_matches;
	BYTE next_character;
	CONSTANT BYTE *p_file;


	#ifdef HTTP_USING_FILING_SYSTEM
		//-------------------------------------------------------------------------
		//-------------------------------------------------------------------------
		//----- WE ARE USING AN EXTERNAL FILING SYSTEM TO STORE OUR WEB FILES -----
		//-------------------------------------------------------------------------
		//-------------------------------------------------------------------------

		//----- CALL EXTERNAL FUNCTION TO FIND THE REQUESTED FILE -----
		if (!HTTP_EXTERNAL_FILE_FIND_FILE(request_filename, request_file_extension, &http_socket[socket_number].response_bytes_remaining, &http_socket[socket_number].response_next_byte_address))
		{
			//----- UNABLE TO FIND THE REQUESTED FILE -----
			//Return a 404 not found error
			http_socket[socket_number].sm_http_state = HTTP_RETURN_NOT_FOUND;
			return;
		}
	
		//NUMBER OF BYTES REMAINING WILL HAVE BEEN WRITTEN BY FUNCTION CALL ABOVE
		//http_socket[socket_number].response_bytes_remaining =
		
		//ADDRESS POINTER OF THE FILE DATA WILL HAVE BEEN WRITTEN BY FUNCTION CALL ABOVE (this variable is for external system to use as required)
		//http_socket[socket_number].response_next_byte_address = 
	
	#else
		//-----------------------------------------------------------------------
		//-----------------------------------------------------------------------
		//----- WE ARE USING C HEADER OR BINARY FILE TO STORE OUR WEB FILES -----
		//-----------------------------------------------------------------------
		//-----------------------------------------------------------------------

		//-----------------------------------
		//----- FIND THE FILE TO RETURN -----
		//-----------------------------------
		filename_matches = 0;
		for (file_count = 0; file_count < (sizeof(http_filenames) / sizeof(BYTE*)); file_count++)
		{
			//----- DOES REQUESTED FILE MATCH THIS FILE? -----
			p_file = http_filenames[file_count];	//Set to first character of this next server file
			b_count = 0;							//Reset back to first character of requested file
			filename_matches = 1;					//Default to this filename matches
			
			//CHECK THE FILENAME
			while (*p_file != 0x00)
			{
				//Get next character of the request filename
				next_character = request_filename[b_count++];
				
				//The '/' character is converted to "_" in our internal filenames to make the C compiler friendly
				if (next_character == '/')
					next_character = '_';
	
				//Exit if we're reached the end of the request filename
				if (next_character == 0x00)
				{
					b_count--;
					break;
				}
				
				if (next_character != *p_file++)			//Does it match this current server file?
				{
					//Doesn't match - move onto next server file
					filename_matches = 0;
					break;
				}	
			}
			if (filename_matches)
			{
				//Check we matched all the way to the end of the requested filename
				if (request_filename[b_count] != 0x00)
					filename_matches = 0;
			
				//The '.' character before the file extension is converted to "_" in our internal filenames to make the C compiler friendly
				if (*p_file++ != '_')
					filename_matches = 0;
				
				//CHECK THE FILE EXTENSION
				if (*p_file++ != request_file_extension[0])
					filename_matches = 0;
				if (*p_file++ != request_file_extension[1])
					filename_matches = 0;
				if (*p_file++ != request_file_extension[2])
					filename_matches = 0;
				
				//Check for nulll terminator at end of our filename 
				if (*p_file++ != 0x00)
					filename_matches = 0;
	
				if (filename_matches)
				{
					//----- THIS IS THE FILE -----
					break;
				}
			}		
		}
		
		if (!filename_matches)
		{
			//---------------------------------------------
			//----- UNABLE TO FIND THE REQUESTED FILE -----
			//---------------------------------------------
			//Return a 404 not found error
			http_socket[socket_number].sm_http_state = HTTP_RETURN_NOT_FOUND;
			return;
		}
	
		//-----------------------------------
		//----- GET THE NUMBER OF BYTES -----
		//-----------------------------------
		//Length is stored in the 4 bytes after the null termianted filename
		http_socket[socket_number].response_bytes_remaining = ((DWORD)*p_file++) << 24;
		http_socket[socket_number].response_bytes_remaining |= ((DWORD)*p_file++) << 16;
		http_socket[socket_number].response_bytes_remaining |= ((DWORD)*p_file++) << 8;
		http_socket[socket_number].response_bytes_remaining |= (DWORD)*p_file++;
	
		//--------------------------------------------
		//----- GET THE ADDRESS OF THE FILE DATA -----
		//--------------------------------------------
		#ifdef HTTP_USING_C_FILES
			//For C file storage the data start address is the next byte
			http_socket[socket_number].response_next_byte_address = p_file;
		#endif
		#ifdef HTTP_USING_BINARY_FILES
			//For binary file storage the file data start address is in the next 4 bytes
			http_socket[socket_number].response_next_byte_address = ((DWORD)*p_file++) << 24;
			http_socket[socket_number].response_next_byte_address |= ((DWORD)*p_file++) << 16;
			http_socket[socket_number].response_next_byte_address |= ((DWORD)*p_file++) << 8;
			http_socket[socket_number].response_next_byte_address |= (DWORD)*p_file++;
		#endif

	#endif


	//-----------------------------------------------------------------------
	//----- DETERMINE THE FILE TYPE READY FOR WHEN THE HEADERS ARE SENT -----
	//-----------------------------------------------------------------------
	http_socket[socket_number].response_content_type = HTTP_CONTENT_UNKNOWN;		//Default to unknown

	p_file = &http_content_type_file_extensions[0];
	for (b_count = 0; b_count < (sizeof(http_content_type_file_extensions) / sizeof(CONSTANT BYTE)); b_count++)
	{
		if ((request_file_extension[0] == p_file[0]) && (request_file_extension[1] == p_file[1]) && (request_file_extension[2] == p_file[2]))
		{
			http_socket[socket_number].response_content_type = p_file[3];
			break;
		}
		p_file += 4;		//Move to next entry
	}

	//---------------------------------
	//----- FLAG TO SEND RESPONSE -----
	//---------------------------------
	//Now that we have all of the information we need let the seperate response function do the actual response as it may take
	//several tries before the nic is ready to start transmitting.
	
	if (request_type == HTTP_REQUEST_TYPE_HEAD)
	{
		//Only return the headers that would normally be returned, not the message body
		http_socket[socket_number].sm_http_state = HTTP_START_HEAD_RESPONSE;
	}
	else
	{
		//Do a normal response
		http_socket[socket_number].sm_http_state = HTTP_START_RESPONSE;
	}

	http_socket[socket_number].file_bytes_sent_last_time = 0xffff;		//Flag that response has not been started yet
}



//****************************************
//****************************************
//********** SEND HTTP RESPONSE **********
//****************************************
//****************************************
//Call tcp_is_socket_ready_to_tx_new_packet() before calling this function
//Call with:
//	resend_last_packet = 1 if we need to resend the last packet sent, = 0 if we need to send the next packet
void http_transmit_next_response_packet (BYTE socket_number, BYTE resend_last_packet)
{
	BYTE general_string[15];		//Big enough to hold a DWORD value + an additional byte
	BYTE *p_string;
	BYTE next_byte;
	WORD bytes_sent;
	WORD file_offset;
	BYTE dynamic_variable_name_next_character = 0xff;

#ifdef HTTP_DYNAMIC_DATA_FUNCTION
	BYTE dynamic_variable_name[HTTP_MAX_OUTPUT_VARIABLE_NAME_LENGTH];
#endif

#ifdef HTTP_USING_C_FILES
	CONSTANT BYTE *p_rom_file;
#endif


	//tcp_is_socket_ready_to_tx_new_packet() has been called
	//----- THE LAST TCP PACKET HAS BEEN ACKNOWLEDGED OR IS NEEDING A RESEND AND THE NIC IS READY TO TRANSMIT A NEW PACKET -----

	if (resend_last_packet)
	{
		//-------------------------------------------
		//----- RE-SEND THE LAST PACKET WE SENT -----
		//-------------------------------------------
		
		//AS WE ONLY UPDATE THE RESPONSE REGISTERS ONCE resend_last_packet = 0 (TCP ACK HAS BEEN RECEIVED TO THE LAST PACKET) WE DON'T NEED TO
		//DO ANYTHING HERE AS EVERTHING IS STILL SETUP READY TO SEND THE SAME PACKET AGAIN

	}
	else
	{
		//--------------------------------
		//----- SEND THE NEXT PACKET -----
		//--------------------------------
		if (http_socket[socket_number].file_bytes_sent_last_time == 0xffff)
		{
			//----- WE HAVE NOT STARTED THE RESPONSE YET -----

		}
		else if (http_socket[socket_number].file_bytes_sent_last_time == 0)
		{
			//----- NO FILE DATA WAS SENT LAST TIME - JUST THE RESPONSE HEADERS -----
			if (http_socket[socket_number].sm_http_state == HTTP_START_HEAD_RESPONSE)
			{
				//THE CLIENT REQUEST WAS FOR HEADERS ONLY SO WE ARE COMPLETE
				http_socket[socket_number].sm_http_state = HTTP_CONNECTED_TO_CLIENT;
				return;
			}
			else
			{
				//START SENDING THE FILE DATA
				http_socket[socket_number].sm_http_state = HTTP_CONTINUE_RESPONSE;
			}
		}
		else
		{
			//----- MOVE TO NEXT BLOCK OF DATA TO SEND -----
			//Subtract bytes sent last time from the number of bytes remaining to be sent
			if (http_socket[socket_number].response_bytes_remaining > (DWORD)http_socket[socket_number].file_bytes_sent_last_time)
				http_socket[socket_number].response_bytes_remaining -= http_socket[socket_number].file_bytes_sent_last_time;
			else
				http_socket[socket_number].response_bytes_remaining = 0;
			
			//Adjust the address of the next byte to send
			http_socket[socket_number].response_next_byte_address += http_socket[socket_number].file_bytes_sent_last_time;
				
			if (http_socket[socket_number].response_bytes_remaining == 0)
			{
				//-----------------------------------------
				//----- ALL OF THE FILE HAS BEEN SENT -----
				//-----------------------------------------
				if (http_socket[socket_number].response_content_type == HTTP_CONTENT_TEXT_HTML)
					http_socket[socket_number].sm_http_state = HTTP_CLOSE_CONNECTION;			//Content length isn't sent for .htm files so we must close the connection to mark the end of the file
				else
					http_socket[socket_number].sm_http_state = HTTP_CONNECTED_TO_CLIENT;
				return;
			}
		}
	}
	
	http_socket[socket_number].file_bytes_sent_last_time = 0;	//Reset bytes sent last time counter (used to handle possible TCP packet re-send)


	//--------------------
	//----- SETUP TX -----
	//--------------------
	
	tcp_setup_socket_tx(http_socket[socket_number].tcp_socket_id);	//We know nic is ready to transmit

	if ((http_socket[socket_number].sm_http_state == HTTP_START_HEAD_RESPONSE) || (http_socket[socket_number].sm_http_state == HTTP_START_RESPONSE))
	{
		//---------------------------------------------------
		//----- SEND FIRST PACKET OF RESPONSE - HEADERS -----
		//---------------------------------------------------

		//----- FIRST PACKET JUST CONTAINS HEADERS - NO FILE DATA -----
		//(This is important as if we have to re-send the packet we rely on http_socket[socket_number].file_bytes_sent_last_time = 0
		//to indicate that the last packet was the headers and not file data)


		//----- SEND RESPONSE CODE  -----
		http_send_const_string(http_response_200);

		//----- SEND CONTENT TYPE -----
		http_send_const_string(http_content_type);
		tcp_write_next_byte(' ');
		switch (http_socket[socket_number].response_content_type)
		{
		case HTTP_CONTENT_TEXT_PLAIN:
			http_send_const_string(http_response_text_plain);
			break;
			
		case HTTP_CONTENT_TEXT_HTML:
			http_send_const_string(http_response_text_html);
			break;

		case HTTP_CONTENT_IMAGE_GIF:
			http_send_const_string(http_response_image_gif);
			break;

		case HTTP_CONTENT_IMAGE_JPEG:
			http_send_const_string(http_response_image_jpeg);
			break;

		case HTTP_CONTENT_IMAGE_PNG:
			http_send_const_string(http_response_image_png);
			break;

		case HTTP_CONTENT_TEXT_XML:
			http_send_const_string(http_response_text_xml);
			break;

		case HTTP_CONTENT_TEXT_CSS:
			http_send_const_string(http_response_text_css);
			break;

		case HTTP_CONTENT_AUDIO_XWAVE:
			http_send_const_string(http_response_audio_xwave);
			break;

		default:
			http_send_const_string(http_response_application_octetstream);		//File type we don't know - browser will work it out from the file type it requested
			break;
		}

		//----- SEND CONTENT LENGTH -----
		//Length of message body in bytes.  In HTTP, it SHOULD be sent whenever the message's length can be determined prior to being
		//transferred.  If content length isn't included then the closing of the connection determines the length of the message body.
		if (http_socket[socket_number].response_content_type != HTTP_CONTENT_TEXT_HTML)		//We don't send for .htm pages as the length will change if the page include dynamic content
		{
			http_send_const_string(http_content_length);

			general_string[0] = ' ';
			p_string = convert_dword_to_ascii(http_socket[socket_number].response_bytes_remaining, &general_string[1]);
			*p_string++ = 0x0d;	//<CR>
			*p_string++ = 0x0a;	//<LF>
			tcp_write_array(&general_string[0], (WORD)(p_string - &general_string[0]));
		}

		//----- SEND END OF HEADERS -----
		//A blank line marks end of headers.  File data follows immediately after
		tcp_write_next_byte(0x0d);	//<CR>
		tcp_write_next_byte(0x0a);	//<LF>
	}
	else
	{
		//--------------------------------------------------
		//----- SEND NEXT FILE DATA PACKET OF RESPONSE -----
		//--------------------------------------------------
		#ifdef HTTP_USING_C_FILES
			//HTTP files stored in program memory - setup pointer
			p_rom_file = http_socket[socket_number].response_next_byte_address;
		#endif

		bytes_sent = 0;
		file_offset = 0;
		while (bytes_sent < (MAX_TCP_DATA_LEN - 100))		//-100 to allow for dynamic content that may appear right at the end of a packet
		{
			//----- CHECK FOR END OF FILE -----
			if (http_socket[socket_number].file_bytes_sent_last_time >= http_socket[socket_number].response_bytes_remaining)
				break;

			//----- GET NEXT BYTE -----
			#ifdef HTTP_USING_C_FILES
				next_byte = *p_rom_file++;
			#endif
			#ifdef HTTP_USING_BINARY_FILES
				next_byte = HTTP_BINARY_FILE_NEXT_BYTE(http_socket[socket_number].response_next_byte_address + (DWORD)file_offset++);
			#endif
			#ifdef HTTP_USING_FILING_SYSTEM
				next_byte = HTTP_EXTERNAL_FILE_NEXT_BYTE(http_socket[socket_number].response_next_byte_address + (DWORD)file_offset++);
			#endif
			
			http_socket[socket_number].file_bytes_sent_last_time++;

			//----- CHECK FOR DYNAMIC CONTENT -----
			#ifdef HTTP_DYNAMIC_DATA_FUNCTION
			if ((http_socket[socket_number].response_content_type == HTTP_CONTENT_TEXT_HTML) && (next_byte == '~'))
			{
				//----- TILDE '~' CHARACTER MARKS START OF A DYNAMIC DATA VARIABLE NAME -----
				dynamic_variable_name_next_character = 0;
			}
			else if (dynamic_variable_name_next_character != 0xff)
			{
				//----- WE ARE READING VARIABLE NAME -----
				if (next_byte == '-')
				{
					//----- HYPHEN '-' MARKS END OF VARIABLE NAME - OUTPUT DYNAMIC DATA ------
					
					//Add null terminator to variable name string
					if (dynamic_variable_name_next_character < HTTP_MAX_OUTPUT_VARIABLE_NAME_LENGTH)
						dynamic_variable_name[dynamic_variable_name_next_character++] = 0x00;
					else
						dynamic_variable_name[dynamic_variable_name_next_character - 1] = 0x00;
					
					//Call user application function
					p_string = HTTP_DYNAMIC_DATA_FUNCTION(&dynamic_variable_name[0], http_socket[socket_number].tcp_socket_id);
					
					//Output returned string as part of this packet
					dynamic_variable_name_next_character = 0;		//Use this variable to limit output to max 100 characters
					while ((*p_string != 0x00) && (dynamic_variable_name_next_character < 100))
					{
						tcp_write_next_byte(*p_string++);
						bytes_sent++;
						
						dynamic_variable_name_next_character++;
					}
					
					//All done - flag that we're no longer reading a variable name
					dynamic_variable_name_next_character = 0xff;
					continue;			//Jump back to the beginning of the while loop
				}
				else
				{
					//----- ADD NEXT CHARCTER TO VARIABLE NAME ----
					if (dynamic_variable_name_next_character < HTTP_MAX_OUTPUT_VARIABLE_NAME_LENGTH)
					{
						dynamic_variable_name[dynamic_variable_name_next_character++] = next_byte;
					}
				}
			}
			#endif

			//----- ADD BYTE TO PACKET -----
			if (dynamic_variable_name_next_character == 0xff)		//Don't output byte if we are currently reading a dynamic data variable name
			{
				tcp_write_next_byte(next_byte);
				bytes_sent++;
			}

		}
		//----- THIS PACKET FULL OR END OF FILE REACHED -----
		
	}

	//---------------------------
	//----- SEND THE PACKET -----
	//---------------------------
	tcp_socket_tx_packet(http_socket[socket_number].tcp_socket_id);
}



//**********************************************
//**********************************************
//********** SEND HTTP ERROR RESPONSE **********
//**********************************************
//**********************************************
//Call tcp_is_socket_ready_to_tx_new_packet() before calling this function
//Call with:
//	resend_last_packet = 1 if we need to resend the last packet sent, = 0 if we need to send the next packet
void http_transmit_error_response (BYTE socket_number, BYTE resend_last_packet)
{

	//tcp_is_socket_ready_to_tx_new_packet() has been called
	//----- THE LAST TCP PACKET HAS BEEN ACKNOWLEDGED OR IS NEEDING A RESEND AND THE NIC IS READY TO TRANSMIT A NEW PACKET -----

	if (resend_last_packet)
	{
		//-------------------------------------------
		//----- RE-SEND THE LAST PACKET WE SENT -----
		//-------------------------------------------
		
		//AS WE ONLY UPDATE THE RESPONSE REGISTERS ONCE resend_last_packet = 0 (TCP ACK HAS BEEN RECEIVED TO THE LAST PACKET) WE DON'T NEED TO
		//DO ANYTHING HERE AS EVERTHING IS STILL SETUP READY TO SEND THE SAME PACKET AGAIN
	}
	else
	{
		//---------------------------
		//----- SEND THE PACKET -----
		//---------------------------
		if (http_socket[socket_number].file_bytes_sent_last_time == 0xfffe)
		{
			//----- THE RESPONSE WAS SENT LAST TIME -----
			//We have now confirmed that the response was sent (ack must have been received back) to exit this mode
			http_socket[socket_number].file_bytes_sent_last_time = 0;
			http_socket[socket_number].sm_http_state = HTTP_CLOSE_CONNECTION;
			return;
		}
		else
		{
			//----- WE HAVE NOT STARTED THE RESPONSE YET -----

		}
	}
	
	//--------------------
	//----- SETUP TX -----
	//--------------------
	tcp_setup_socket_tx(http_socket[socket_number].tcp_socket_id);		//We know nic is ready to transmit

	//----- SEND RESPONSE CODE AND DEFAULT HEADERS -----
	switch (http_socket[socket_number].sm_http_state)
	{
	case HTTP_RETURN_NOT_FOUND:
		//404- NOT FOUND
		http_send_const_string(http_response_404_not_found);
		break;

	case HTTP_RETURN_NOT_IMPLEMENTED:
		//501 - REQUEST IS NOT IMPLEMENTED
		http_send_const_string(http_response_501_not_implemented);
		break;

	case HTTP_RETURN_SERVICE_UNAVAILABLE:
		//503 - SERVICE UNAVAILABLE
		http_send_const_string(http_response_503_unavailable);
		break;

	case HTTP_RETURN_BAD_REQUEST:
	default:
		//400 - BAD REQUEST
		http_send_const_string(http_response_400_bad_request);
		break;
	}
			
	//----- SEND THE PACKET -----
	tcp_socket_tx_packet(http_socket[socket_number].tcp_socket_id);

	http_socket[socket_number].file_bytes_sent_last_time = 0xfffe;		//Special flag value to indicate that we have sent the reponse
}



#ifdef HTTP_ACCEPT_POST_REQUESTS			//Exclude the following functions if not acepting POST to reduce program memory space
//************************************************************
//************************************************************
//********** PROCESS HTTP POST REQUEST MESSAGE BODY **********
//************************************************************
//************************************************************
//This is a seperate fucntion as post requests may span more than 1 TCP packet so this function is called each time.
//We know that each packet will be in sequence as the TCP driver requires TCP clients to only send us 1 packet at a time
void http_process_post_message_body (BYTE socket_number)
{
	BYTE b_temp;
	BYTE next_byte;
	BYTE *p_string;
	BYTE header_found;


	if (!http_post_is_multipart_type)
	{
		//----------------------------------------------------------
		//----------------------------------------------------------
		//----- POST IS APPLICATION/X-WWW-FORM-URLENCODED TYPE -----
		//----------------------------------------------------------
		//----------------------------------------------------------
		//This type of POST request is the same as a GET request except that the inputs are contained here in the message body.
		//Process using the same function that GET uses
		
		//----- PROCESS INPUTS -----
		b_temp = http_process_inputs(&http_post_filename[0], &http_post_file_extension[0], socket_number, 1);

		//----- SET OUR STATUS BASED ON PROCESS INPUT FUNCTION RESPOSNE -----
		switch(b_temp)
		{
		case 1:		//All the inputs we're processed and next byte is start of next section
			http_post_content_bytes_remaining = 0;										//All inputs are done
			http_socket[socket_number].sm_http_state = HTTP_CONNECTED_TO_CLIENT;		//Return the socket to the normal state - response will be sent
			break;
		
		case 2:		//End of packet reached and there are more bytes to follow
			if (http_post_content_bytes_remaining == 0)									//Make sure there are more bytes to follow
				http_socket[socket_number].sm_http_state = HTTP_CONNECTED_TO_CLIENT;
			break;

		case 0:		//There we're no more bytes in packet or inputs are malformed (error)
		case 3:		//End of packet reached and there are more bytes to follow but we can't handle as an input is split across a packet boundary
		default:
			http_post_content_bytes_remaining = 0;
			http_socket[socket_number].sm_http_state = HTTP_RETURN_BAD_REQUEST;
			break;
		}
	}
	else
	{
		//--------------------------------------------
		//--------------------------------------------
		//----- POST IS MULTIPART/FORM-DATA TYPE -----
		//--------------------------------------------
		//--------------------------------------------

		if ((http_post_boundary_length == 0) || (http_post_boundary_length > 70))
		{
			//----- ERROR - BOUNDARY VALUE WASN'T FOUND WITH "multipart/form-data" HEADER -----
			http_post_content_bytes_remaining = 0;
			http_socket[socket_number].sm_http_state = HTTP_RETURN_BAD_REQUEST;
			goto http_process_post_message_body_exit;
		}

		//------------------------------------------------
		//----- READ ALL OF THIS PACKET BYTE BY BYTE -----
		//------------------------------------------------
		while (1)
		{
			//--------------------------
			//----- READ NEXT BYTE -----
			//--------------------------
			if (tcp_read_next_rx_byte(&next_byte) == 0)
			{
				//NO MORE BYTES IN THIS PACKET - WE'LL COME BACK AND CONTINUE WHEN WE GET THE NEXT PACKET
				break;
			}

			if (http_post_content_bytes_remaining)
				http_post_content_bytes_remaining--;


			if (http_post_reading_data == 0)
			{
				//-----------------------------------------
				//-----------------------------------------
				//----- READING NEXT MULTIPART HEADER -----
				//-----------------------------------------
				//-----------------------------------------

				//----- CHECK FOR FINAL BOUNDARY MARKER -----
				if (http_post_line_length == 0xfe)			//http_post_line_length is set to 0xfe on boundary match when receiving the multipart data.  If
				{											//it is followed immediately by '--' then this is the final boundary marker - end of message.
					if (next_byte == '-')
						http_post_line_length = 0xff;		//First match
					else
						http_post_line_length = 1;			//Doesn't match - set to a dummy value to ensure we don't indicate a blank line
				}
				else if (http_post_line_length == 0xff)		//http_post_line_length is set to 0xff above on boundary match plus a single '-' when receiving the multipart data. If
				{											//it is followed immediately by another '-' then this is the final boundary marker - end of message.
					if (next_byte == '-')
					{
						//------------------------------------------------------------
						//----- FINAL BOUNDARY MARKER - END OF MULTIPART MESSAGE -----
						//------------------------------------------------------------
						http_post_content_bytes_remaining =0;
						http_socket[socket_number].sm_http_state = HTTP_CONNECTED_TO_CLIENT;		//Return the socket to the normal state (response will automatically be sent at end of this function)
						break;
					}
					else
					{
						http_post_line_length = 1;			//Doesn't match - set to a dummy value to ensure we don't indicate a blank line
					}
				}

				//----- CHECK FOR END OF LEADING SPACES -----
				if (next_byte != ' ')
					http_post_leading_spaces_passed = 1;
	
				//----- ADD THE NEW BYTE TO LINE BUFFER -----
				if ((http_post_leading_spaces_passed) && (next_byte != 0x0a) && (next_byte != 0x0d))		//Ignore carriage return (0x0a) & line feed (0x0d)
				{
					if (http_post_line_length < HTTP_MAX_POST_LINE_LENGTH)		//We only read the first # bytes of very long lines
					{
						http_post_line_buffer[http_post_line_length] = next_byte;
						http_post_line_length++;
					}
				}
	
				//----- CHECK FOR END OF LINE OR PRE PARAMETER -----
				if ((next_byte == 0x0a) || (next_byte == ';'))			//<LF> (must be last byte of a line), ';' marks pre optional parameters on a line
				{
					//----------------------------------------------------------------
					//----- END OF LINE - CHECK FOR HEADERS WE ARE INTERESTED IN -----
					//----------------------------------------------------------------
					
					//ADD NULL TERMINATION TO STRING
					if (http_post_line_length < HTTP_MAX_POST_LINE_LENGTH)
						http_post_line_buffer[http_post_line_length] = 0x00;
					else
						http_post_line_buffer[HTTP_MAX_POST_LINE_LENGTH - 1] = 0x00;
					
					//Convert to lower case to remove case sensitivity
					convert_string_to_lower_case(&http_post_line_buffer[0]);
					
					//----- LOOK FOR BLANK LINE -----
					if (http_post_line_length == 0)			//(A blank line marks the end of the headers)
					{
						//---------------------------------------------------------------
						//----- END OF HEADERS - MOVE TO READING THIS SECTIONS DATA -----
						//---------------------------------------------------------------
						http_post_boundary_compare_length = 0;		//Reset boundary compare counter
						http_post_reading_data = 1;					//We are no longer reading headers.  We are now reading this sections data
						http_post_decode_count = 0;					//Reset working variables
						http_post_decoder_buffer[0] = 0;
						http_post_decoder_buffer[1] = 0;
						http_post_decoder_buffer[2] = 0;
					}
					else
					{
						header_found = 0;			//Default to not found a match yet (as a check 'name' will match the 'filename' header we use this and check 'filename' before 'name'
						
						//----- LOOK FOR content-disposition -----
						//Value will be 'form-data' or 'file'
						if (!header_found)
							header_found = http_process_post_check_for_multipart_header(http_content_disposition, (BYTE)sizeof(http_content_disposition), socket_number);	//Will be sent to user function if found

						//----- LOOK FOR filename -----
						//Name of the file when content-disposition = file (note that the client is not requried to provide this, but usually will)
						if (!header_found)
							header_found = http_process_post_check_for_multipart_header(http_filename_equals, (BYTE)sizeof(http_filename_equals), socket_number);	//Will be sent to user function if found

						//----- LOOK FOR name -----
						//name of the corresponding form control when content-disposition = form-data
						if (!header_found)
							header_found = http_process_post_check_for_multipart_header(http_name_equals, (BYTE)sizeof(http_name_equals), socket_number);		//Will be sent to user function if found
						
						//----- LOOK FOR content-type -----
						//Value dependant on the content.  e.g. text/plain, image/gif, etc
						//If not present then you must assume content-type = text/plain; charset=us-ascii
						if (!header_found)
							header_found = http_process_post_check_for_multipart_header(http_content_type, (BYTE)sizeof(http_content_type), socket_number);		//Will be sent to user function if found

						//----- LOOK FOR content-transfer-encoding -----
						//Encoding of the data.  Will be: "7bit", "8bit", "binary", "quoted-printable" or "base64"
						//If not present then you must assume: Content-Transfer-Encoding = 7bit
						if (!header_found)
						{
							p_string = find_string_in_string_no_case(&http_post_line_buffer[0], http_content_transfer_encoding);
							if (p_string)
							{
								//"7bit", "8bit", "binary" are all non encoded (don't require decoding), so we only need to look for "quoted-printable" & "base64"
								p_string = find_string_in_string_no_case(&http_post_line_buffer[0], http_quoted_printable);
								if (p_string)
								{
									//Data will be quoted printable encoded
									http_post_encoding = HTTP_ENCODING_QUOTED_PRINTABLE;
								}
								p_string = find_string_in_string_no_case(&http_post_line_buffer[0], http_base64);
								if (p_string)
								{
									//Data will be base64 encoded
									http_post_encoding = HTTP_ENCODING_BASE64;
								}
								//No need for else as we have defaulted to HTTP_ENCODING_NONE in case the content-transfer-encoding: header is not found
							}
						}
		
						//---------------------------------------
						//----- SETUP FOR READ OF NEXT LINE -----
						//---------------------------------------
						http_post_leading_spaces_passed = 0;			//Ignore any leading spaces
						http_post_line_length = 0;
						
						//If we've actually only reached the pre parameters marker then ensure rest of line can't be
						//detected as a end of headers blank line, by creating a dummy first character
						if (next_byte == ';')
						{
							http_post_line_buffer[0] = ';';
							http_post_line_length = 1;
						}
					}
				}
			}
			else
			{
				//--------------------------------------
				//--------------------------------------
				//----- READING THE MULTIPART DATA -----
				//--------------------------------------
				//--------------------------------------
				//0xff = not got to first multipart boundary yet
				//0x01 = reading data area of this multipart section of message
	
				//---------------------------------------
				//----- LOOK FOR MULTIPART BOUNDARY -----
				//---------------------------------------
				//We have to always be looking for a multipart boundary.  As we send data to the user applicaiton byte by byte if we start seeing data that could
				//possibly be the boundary we have to not send the data to the user application, but if we later find that it was not the boundary we need to
				//then send all of the bytes that had been witheld from being sent.
				
				//----- DETERMINE THE NEXT BYTE WE ARE LOOKING FOR -----
				if (http_post_boundary_compare_length == 0)			//http_post_boundary_compare_length = the number of bytes we have matched so far
					b_temp = 0x0d;		//<CR>						//Boundaries start with '<CR><LF>--' then the stored boundary string obtained from the message headers (up to 70 bytes)
				else if (http_post_boundary_compare_length == 1)
					b_temp = 0x0a;		//<LF>
				else if (http_post_boundary_compare_length == 2)
					b_temp = '-';
				else if (http_post_boundary_compare_length == 3)
					b_temp = '-';
				else
					b_temp = http_post_boundary_string[http_post_boundary_compare_length - 4];
				
				//----- COMPARE IT TO THE NEW RECEIVED BYTE -----
				if (next_byte == b_temp)
				{
					//----- NEXT BYTE MATCHES -----
					http_post_boundary_compare_length++;
					if (http_post_boundary_compare_length == (http_post_boundary_length + 4))
					{
						//------------------------------------------------------------------------
						//----- MULTIPART BOUNDARY MATCH - MOVE TO READING NEXT PART HEADERS -----
						//------------------------------------------------------------------------
						
						//Call user application function when we reach the end of a multipart section
						HTTP_POST_LAST_MULTIPART_DONE_FUNCTION();
						
						//Setup to read next section headers
						http_post_reading_data = 0;					//We are no longer reading data.  We are now reading the next headers section (after any
																	//trailing white space which we must ignore)
						http_post_leading_spaces_passed = 1;
						http_post_line_length = 0xfe;				//Special value for headers handler to look for trailing '--' which would mark the final boundary marker
						http_post_encoding = HTTP_ENCODING_NONE;	//We must assume "Content-Transfer-Encoding: 7BIT" if no header is supplied
					}
				}
				else
				{
					//----- NO MATCH -----
					//IF WE HAD STARTED STORING A BOUNDARY MATCH THEN NOW SEND THE PREVIOUS BYTES THAT WE'RE NOT PASSED TO THE USER FUNCTION
					if ((http_post_reading_data != 0xff) && (http_post_boundary_compare_length))		//Don't pass on if we're looking for the 1st multipart boundary
					{
						p_string = &http_post_boundary_string[0];
						if (http_post_boundary_compare_length)
						{
							http_process_post_decode_next_data_byte(0x0d);	//<CR>
							http_post_boundary_compare_length--;
						}
						if (http_post_boundary_compare_length)
						{
							http_process_post_decode_next_data_byte(0x0a);	//<LF>
							http_post_boundary_compare_length--;
						}
						if (http_post_boundary_compare_length)
						{
							http_process_post_decode_next_data_byte('-');
							http_post_boundary_compare_length--;
						}
						if (http_post_boundary_compare_length)
						{
							http_process_post_decode_next_data_byte('-');
							http_post_boundary_compare_length--;
						}
						while (http_post_boundary_compare_length)
						{
							http_process_post_decode_next_data_byte(*p_string++);
							http_post_boundary_compare_length--;
						}
						//http_post_boundary_compare_length has been reset to 0
					}
				}

				//---------------------------------------------------------
				//----- DECODE AND SEND THIS BYTE TO USER APPLICATION -----
				//---------------------------------------------------------
				if (http_post_reading_data == 1)		//Don't process value if we're looking for the 1st multipart boundary or if we're changing to reading headers
				{
					if (http_post_boundary_compare_length == 0)				//Don't send if this may be a multipart boundary (will get sent above if it turns out not to be a boundary)
						http_process_post_decode_next_data_byte(next_byte);
				}

			}

			if (http_post_content_bytes_remaining == 0)
			{
				//----- POST REQUEST COMPLETE -----
				//(This is unlikely to be our exit point as we will detect end of message by final boundary marker, but do this check in case)
				http_socket[socket_number].sm_http_state = HTTP_CONNECTED_TO_CLIENT;		//Return the socket to the normal state (response will automatically be sent at end of this function)
				break;
			}
		} //while (1)

	}


http_process_post_message_body_exit:
	//---------------------------
	//----- DUMP THE PACKET -----
	//---------------------------
	tcp_dump_rx_packet();

	//-------------------------------------
	//----- SEND RESPONSE IF ALL DONE -----
	//-------------------------------------
	if (http_post_content_bytes_remaining == 0)
	{
		if (http_socket[socket_number].sm_http_state == HTTP_CONNECTED_TO_CLIENT)		//(Don't do if we are sending an error response instead)
			http_setup_response(socket_number, HTTP_REQUEST_TYPE_POST, &http_post_filename[0], &http_post_file_extension[0]);
	}

}



//********************************************************
//********************************************************
//********** CHECK FOR MULTIPART HEADER ON LINE **********
//********************************************************
//********************************************************
//A seperate function as we need to call multiple times to check for several headers on each line
//Returns 1 if header found, 0 if not
BYTE http_process_post_check_for_multipart_header (CONSTANT BYTE *header_to_find, BYTE string_length, BYTE socket_number)
{
	BYTE *p_string;
	BYTE b_count;
	
	
	p_string = find_string_in_string_no_case(&http_post_line_buffer[0], header_to_find);
	if (p_string)
	{
		//String was found
		
		//Get the start position of the value
		p_string += (string_length - 1);	//Move pointer to 1st character after the string
		while ((*p_string == ' ') || (*p_string == 0x22))	//Move past any leading spaces or '"' (0x22)
			p_string++;
		
		//Null terminate the value
		b_count = p_string - http_post_line_buffer;
		while (1)
		{
			if (b_count >= HTTP_MAX_POST_LINE_LENGTH)		//Check for run out of space on line
			{
				http_post_line_buffer[b_count - 1] = 0x00;
				break;
			}
			if (
			(http_post_line_buffer[b_count] == ' ') ||
			(http_post_line_buffer[b_count] == 0x0d) ||		//<CR>
			(http_post_line_buffer[b_count] == 0x22) ||		//'"'
			(http_post_line_buffer[b_count] == ';') ||
			(http_post_line_buffer[b_count] == 0x00)		//Null
			)
			{
				http_post_line_buffer[b_count] = 0x00;		//Replace with terminating null before passing to user function
				break;
			}
			b_count++;
		}
		//Send header to user function (it may want the alue and this also indicates the start of a new multpart data section
		HTTP_POST_MULTIPART_HEADER_FUNCTION(header_to_find, p_string, http_post_filename, http_post_file_extension, socket_number);
		return(1);
	}
	return(0);
}



//*********************************************************************
//*********************************************************************
//********** DECODE MULTIPART DATA AND PASS TO USER FUNCTION **********
//*********************************************************************
//*********************************************************************
//This function is passed each byte of a multipart section data area and decodes them if necessary before passing the
//originally sent data to the user application
void http_process_post_decode_next_data_byte (BYTE data)
{
	DWORD_VAL dw_temp;


	if (http_post_encoding == HTTP_ENCODING_NONE)
	{
		//-----------------------
		//-----------------------
		//----- NO ENCODING -----
		//-----------------------
		//-----------------------
		//content-transfer-encoding: 7bit, 8bit and binary are all non encoded (don't require decoding)

		//Send byte to user application
		HTTP_POST_MULTIPART_NEXT_BYTE_FUNCTION(data);
	}
	else if (http_post_encoding == HTTP_ENCODING_BASE64)
	{
		//--------------------------
		//--------------------------
		//----- BASE64 ENCODED -----
		//--------------------------
		//--------------------------
		//content-transfer-encoding: base64

		//Base64 uses an efficient 8bit to 7 bit conversion but without the encoded data being human readable.
		//It uses a 65 character subset of US-ASCII, with 6 bits represented per printable character. The 65th
		//character '=' is used as a special marker.  Each 24 bits (3 bytes) of the data being encoded is represented
		//by 4 characters, resulting in approximately 33% larger size overall.
		//Working from left to right, the next 3 bytes are joined together to form 24 bits and this is then split up
		//into 4 groups of 6 bits, with each 6 bit group represented by a character from the base64 alphabet:
		//	0  'A'		16 'Q'		32 'g'		48 'w'		PAD '='
		//	1  'B'		17 'R'		33 'h'		49 'x'
		//	2  'C'		18 'S'		34 'i'		50 'y'
		//	3  'D'		19 'T'		35 'j'		51 'z'
		//	4  'E'		20 'U'		36 'k'		52 '0'
		//	5  'F'		21 'V'		37 'l'		53 '1'
		//	6  'G'		22 'W'		38 'm'		54 '2'
		//	7  'H'		23 'X'		39 'n'		55 '3'
		//	8  'I'		24 'Y'		40 'o'		56 '4'
		//	9  'J'		25 'Z'		41 'p'		57 '5'
		//	10 'K'		26 'a'		42 'q'		58 '6'
		//	11 'L'		27 'b'		43 'r'		59 '7'
		//	12 'M'		28 'c'		44 's'		60 '8'
		//	13 'N'		29 'd'		45 't'		61 '9'
		//	14 'O'		30 'e'		46 'u'		62 '+'
		//	15 'P'		31 'f'		47 'v'		63 '/'
		//Rules:
		//When decoding any character not included in the base64 alphabet must be ignored.
		//The encoded output must have line lengths of no more than 76 characters.
		//If there are less than 24 bits available at the end of the encoded data then zero bits must be added to the right
		//of the final bits to reach the next 6 bit group boundary.  The '=' character is then added to make up any
		//remaining characters of the last 4 character group.  This allows 3 posibilities.  Where there are 24 bits to make
		//up the final 4 character output there will be no '=' padding character.  Where there are only 16 bits there will
		//be 3 characters followed by 1 '=' padding character.  Where there are only 8 bits there will be 2 characters
		//followed by 2 '=' padding characters.

		if (
		((data >= 'A') && (data <= 'Z')) ||
		((data >= 'a') && (data <= 'z')) ||
		((data >= '0') && (data <= '9')) ||
		(data == '+') ||
		(data == '/') ||
		(data == '=')
		)
		{
			if (http_post_decode_count < 3)
			{
				//--------------------------------------------------------
				//----- STORE NEXT CHARACTER OF 4 CHARACTER SEQUENCE -----
				//--------------------------------------------------------
				http_post_decoder_buffer[http_post_decode_count] = data;
				http_post_decode_count++;
			}
			else
			{
				//------------------------------------------------------------
				//----- 4TH CHARACTER OF A 4 CHARACTER GROUP - DO DECODE -----
				//------------------------------------------------------------

				//Convert back to the 24 bit value
				dw_temp.Val = ((DWORD)http_convert_base64_to_bits(http_post_decoder_buffer[0]) << 18);
				dw_temp.Val |= ((DWORD)http_convert_base64_to_bits(http_post_decoder_buffer[1]) << 12);
				dw_temp.Val |= ((DWORD)http_convert_base64_to_bits(http_post_decoder_buffer[2]) << 6);
				dw_temp.Val |= (DWORD)http_convert_base64_to_bits(data);

				//Send bytes to user application
				HTTP_POST_MULTIPART_NEXT_BYTE_FUNCTION(dw_temp.v[2]);
				if (http_post_decoder_buffer[2] != '=')					//Check for special padding character
				{
					HTTP_POST_MULTIPART_NEXT_BYTE_FUNCTION(dw_temp.v[1]);
					if (data != '=')									//Check for special padding character
					{
						HTTP_POST_MULTIPART_NEXT_BYTE_FUNCTION(dw_temp.v[0]);
					}
				}

				//Reset for next group of 4 bytes
				http_post_decode_count = 0;
			}
		}
	}	
	else if (http_post_encoding == HTTP_ENCODING_QUOTED_PRINTABLE)
	{
		//------------------------------------
		//------------------------------------
		//----- QUOTED PRINTABLE ENCODED -----
		//------------------------------------
		//------------------------------------
		//content-transfer-encoding: quoted-printable

		//Quoted-Printable encoding is intended to represent data that largely consists of bytes that correspond to
		//printable characters in the US-ASCII character set (i.e. useful where much of the data being encoded is
		//text in a language that uses one of the Latin charsets.  The output is a compromise between readability
		//and reliability in transport. 
		//Encoding Rules:
		//	=xx		Where xx is a 2 digit hexadecimal value (uppercase A-F only ? lowercase is not allowed).
		//			for example:
		//			=3D		indicates a byte with a value of decimal 61
		//	'!' to '<' and '>' to '~'
		//			Literal representation of characters from decimal value 33 to 60 inclusive, and 62 through 126
		//			inclusive.  Bytes with these values may be represented as the US-ASCII characters which correspond
		//			to those values, but they do not have to (for instance =xx could be used instead).
		//	<SP> and <TAB>
		//			White Space bytes with values of 9 and 32 may be represented as the US-ASCII TAB (HT) and SPACE
		//			characters respectively, but not at the end of an encoded line.  (they must be followed by a
		//			printable character on that line.  For example an "=" at the end of an encoded line (indicating
		//			a soft line break) may follow one or more TAB (HT) or SPACE characters.  When decoding a
		//			Quoted-Printable body, any trailing white space on a line must be deleted.
		//	Line Breaks
		//			A line break in a text body, represented as a <CR><LF> sequence in the text, must be represented by
		//			a <CR><LF> line break in the Quoted-Printable encoding (i.e. the same, but the encoder must not use
		//			a different encoding for textual data).  Encoding of non text (binary) data must use "=0D=0A" instead.
		//	'=' Soft line break
		//			An encoded line must not be more than 76 characters long.  The equal character (0x3D) as the last
		//			printable character on an encoded line indicates a soft line break in the encoded text.  There will
		//			be a CRLF following it.  The 76 character limit does not count the trailing <CR><LF>, but counts all
		//			other characters, including any equal signs.

		if (data == 0x09)		//<HT>
		{
			//------------------------------------
			//----- WHITESPACE TAB CHARACTER -----
			//------------------------------------
			//This whitespace character is only valid if followed by a printable character.  Store a count of the number
			//of tabs and only include them if we get another printable character before the end of the line.
			
			http_post_decoder_buffer[1]++;				//We use this varaible to store a count of the number of tabs in this decoding mode
		}
		else if (data == ' ')
		{
			//--------------------------------------
			//----- WHITESPACE SPACE CHARACTER -----
			//--------------------------------------
			//This whitespace character is only valid if followed by a printable character.  Store a count of the number
			//of spaces and only include them if we get another printable character before the end of the line.

			http_post_decoder_buffer[2]++;				//We use this varaible to store a count of the number of spaces in this decoding mode
		}
		else if (data == 0x0d)	//<CR>
		{
			//---------------------------
			//----- CARRIAGE RETURN -----
			//---------------------------
			//As a <LF> Line Feed character can only occur after the carriage return with this encoding we simply check for <CR>
			//and don't bother checking for <LF>.

			//Send bytes to user application
			if (http_post_decode_count != 1)
			{
				//NOT A SOFT LINE BREAK SO SEND <CR><LF>
				HTTP_POST_MULTIPART_NEXT_BYTE_FUNCTION(0x0d);		//<CR>
				HTTP_POST_MULTIPART_NEXT_BYTE_FUNCTION(0x0a);		//<LF>
			}

			//Reset for a new line
			http_post_decoder_buffer[1] = 0;		//Clear any stored whitespace tab characters
			http_post_decoder_buffer[2] = 0;		//Clear any stored whitespace space characters
			http_post_decode_count = 0;
		}
		else if ((data >= '!') && (data <= '~'))
		{
			//-------------------------------
			//----- PRINTABLE CHARACTER -----
			//-------------------------------
			
			//------------------------------------------------------------------------------------------------------------------
			//----- SEND ANY STORED WHITE SPACE CHARACTERS THAT WE'RE WAITING ON THE NEXT PRINTABLE CHARACTER TO BE VALID ------
			//------------------------------------------------------------------------------------------------------------------
			while (http_post_decoder_buffer[1])				//We use this varaible to store a count of the number of tabs in this decoding mode
			{
				//TAB CHARACTER
				HTTP_POST_MULTIPART_NEXT_BYTE_FUNCTION(0x09);		//<HT>
				http_post_decoder_buffer[1] --;
			}
			while (http_post_decoder_buffer[2])				//We use this varaible to store a count of the number of spaces in this decoding mode
			{
				//SPACE CHARACTER
				HTTP_POST_MULTIPART_NEXT_BYTE_FUNCTION(' ');
				http_post_decoder_buffer[2] --;
			}			
			
			if (data == '=')
			{
				//------------------------------------------------------
				//----- START OF A HEXADECIMAL VALUE OR SOFT BREAK -----
				//------------------------------------------------------
				http_post_decode_count = 1;					//Flag that waiting for first hexadecimal character (if this is actually a soft break '=' it will be dealt with by the <CR> detect above)
			}
			else if (http_post_decode_count == 1)
			{
				//------------------------------------------
				//----- GET 1ST CHARACTER OF HEX VALUE -----
				//------------------------------------------
				http_post_decoder_buffer[0] = data;
				http_post_decode_count = 2;					//Flag that waiting for second hexadecimal character 
			}
			else if (http_post_decode_count == 2)
			{
				//------------------------------------------
				//----- GET 2ND CHARACTER OF HEX VALUE -----
				//------------------------------------------
				
				//Send bytes to user application
				HTTP_POST_MULTIPART_NEXT_BYTE_FUNCTION(convert_ascii_hex_to_byte(http_post_decoder_buffer[0], data));
				
				http_post_decode_count = 0;
			}
			else
			{
				//--------------------------------------------
				//----- GET STANDARD PRINTABLE CHARACTER -----
				//--------------------------------------------

				//Send bytes to user application
				HTTP_POST_MULTIPART_NEXT_BYTE_FUNCTION(data);
			}
		}
	}
}



//*************************************************************
//*************************************************************
//********** CONVERT BASE64 CHARACTER TO 6 BIT VALUE **********
//*************************************************************
//*************************************************************
//The calling function must have checked the character is part of the base64 alphabet.
BYTE http_convert_base64_to_bits (BYTE character)
{
	if (character == '+')
		return(62);
	else if (character == '/')
		return(63);
	else if (character <= '9')
		return((character - '0') + 52);
	else if (character == '=')
		return(0);
	else if (character <= 'Z')
		return(character - 'A');
	else
		return((character - 'a') + 26);
}
#endif //#ifdef HTTP_ACCEPT_POST_REQUESTS



//********************************************************
//********************************************************
//********** PROCESS HTTP REQUEST PACKET INPUTS **********
//********************************************************
//********************************************************
//This seperate function is used as GET and POST application/x-www-form-urlencoded requests require the same processing
//but at different points in a HTTP request packet
//Returns:
//	0 There we're no more bytes in packet or inputs are malformed (error - tcp_dump_rx_packet() required)
//	1 All the inputs we're processed and next byte is start of next section (get or post)
//	2 End of packet reached and there are more bytes to follow (post method only - tcp_dump_rx_packet() required)
//	3 End of packet reached and there are more bytes to follow but we can't handle as an input is split across a packet boundary  (post method only - tcp_dump_rx_packet() required)
BYTE http_process_inputs (BYTE *requested_filename, BYTE *requested_file_extension, BYTE socket_number, BYTE request_is_post_method)
{
	BYTE data_buffer[4];
	BYTE input_name[HTTP_MAX_INPUT_NAME_LENGTH];
	BYTE input_value[HTTP_MAX_INPUT_VALUE_LENGTH];
	BYTE input_name_next_character;
	BYTE input_value_next_character;
	BYTE reading_input_name;


	data_buffer[1] = data_buffer[2] = data_buffer[3] = 0x00;
	input_name_next_character = 0;
	input_value_next_character = 0;
	reading_input_name = 1;

	while (1)
	{
		//----- READ NEXT BYTE -----
		if (http_read_next_byte_to_buffer(&data_buffer[0]) == 0)
		{
			//NO MORE BYTES IN PACKET
			if (request_is_post_method)
			{
				//----- DOING POST REQUEST - THERE ARE MORE PACKETS -----
				if ((input_name_next_character == 0) && (input_value_next_character == 0))
				{
					//We are not part way through an input so if there is another packet we are fine to process it
					return(2);
				}
				else
				{
					//We are part way through an input - we don't handle this as for large input requirements use the multipart POST method
					return(3);
				}
			}
			else
			{
				//----- DOING GET REQUEST - ERROR THERE SHOULD BE MORE BYTES AFTER THE INPUTS -----
				return(0);
			}
		}

		#ifdef HTTP_ACCEPT_POST_REQUESTS
			if (request_is_post_method)
			{
				//----- DOING POST - DECREMENT CONTENT LENGTH IF IN USE -----
				if (http_post_content_bytes_remaining)
					http_post_content_bytes_remaining--;
			}
		#endif

		if ((data_buffer[0] == ' ') || (data_buffer[0] == '&'))
		{
			//------------------------------------------------------------------------------------------------
			//----- JUST COMPELTED AN INPUT VALUE PARAMETER - SEND IT TO THE MAIN APPLICATION TO PROCESS -----
			//------------------------------------------------------------------------------------------------

			#ifdef HTTP_PROCESS_INPUT_FUNCTION	//Don't do if no prcess input has been provided
				if (input_name_next_character)	//Process the preceeding input
				{
					//Add terminating nulls to the strings
					if (input_name_next_character < HTTP_MAX_INPUT_NAME_LENGTH)
						input_name[input_name_next_character++] = 0x00;
					else
						input_name[input_name_next_character - 1] = 0x00;

					if (input_value_next_character < HTTP_MAX_INPUT_VALUE_LENGTH)
						input_value[input_value_next_character++] = 0x00;
					else
						input_value[input_value_next_character - 1] = 0x00;

					//Call the application function that handles http form inputs
					HTTP_PROCESS_INPUT_FUNCTION(&input_name[0], &input_value[0], requested_filename, requested_file_extension, http_socket[socket_number].tcp_socket_id);
				}	
			#endif
			
			//Reset for next input
			reading_input_name = 1;
			input_name_next_character = 0;
			input_value_next_character = 0;
		}

		if (data_buffer[0] == ' ')
		{
			//----- SPACE - END OF INPUTS -----
			//Input has already been sent to main application to process as desired
			break;
		}	
		else if (data_buffer[0] == '&')
		{
			//----- '&' START OF NEXT INPUT -----
			//Input has already been sent to main application to process as desired
		}
		else if (data_buffer[0] == '%')
		{
			//----- START OF HEXADECIMAL REPRESENTED ASCII CHARACTER -----
			//2 hex bytes will follow
		}
		else if (data_buffer[1] == '%')
		{
			//----- 1ST CHARACTER OF HEXADECIMAL REPRESENTED ASCII CHARACTER  -----
			//1 more hex bytes will follow
		}
		else if (data_buffer[2] == '%')
		{
			//----- 2ND CHARACTER OF HEXADECIMAL REPRESENTED ASCII CHARACTER - CONVERT IT TO ASCII -----
			if (reading_input_name)
			{
				if (input_name_next_character < HTTP_MAX_INPUT_NAME_LENGTH)
					input_name[input_name_next_character++] = convert_ascii_hex_to_byte(data_buffer[1], data_buffer[0]);
			}	
			else
			{
				if (input_value_next_character < HTTP_MAX_INPUT_VALUE_LENGTH)
					input_value[input_value_next_character++] = convert_ascii_hex_to_byte(data_buffer[1], data_buffer[0]);
			}
		}
		else if (data_buffer[0] == '=')
		{
			//----- '=' CHARACTER - MOVE TO VALUE PART OF INPUT -----
			if (reading_input_name)
			{
				reading_input_name = 0;
				input_value_next_character = 0;
			}
			else
			{
				//Error - we we're already reading the input value
				return(0);
			}	
		}
		else
		{
			//----- NEXT CHARACTER -----
			
			//Convert any '+' characters back to space characters
			if (data_buffer[0] == '+')
				data_buffer[0] = ' ';
			
			//Add to string
			if (reading_input_name)
			{
				if (input_name_next_character < HTTP_MAX_INPUT_NAME_LENGTH)
					input_name[input_name_next_character++] = data_buffer[0];
			}	
			else
			{
				if (input_value_next_character < HTTP_MAX_INPUT_VALUE_LENGTH)
					input_value[input_value_next_character++] = data_buffer[0];
			}
		}
	} //while (1)
	
	return(1);
}



//*****************************************************************
//*****************************************************************
//********** READ NEXT BYTE AND ADD IT TO ROLLING BUFFER **********
//*****************************************************************
//*****************************************************************
//Call with pointer to the start of a 4 byte buffer
//The existing bytes in the buffer are moved down 1 and the new byte is added to the first positon.
//Returns 1 is read OK, or 0 if no more bytes available
BYTE http_read_next_byte_to_buffer (BYTE *data_buffer)
{

	//SHIFT BUFFER DOWN READY FOR NEXT BYTE
	data_buffer[3] = data_buffer[2];
	data_buffer[2] = data_buffer[1];
	data_buffer[1] = data_buffer[0];

	if (tcp_read_next_rx_byte(data_buffer) == 0)
		return(0);					//Error - not enough bytes in packet
	
	return(1);
}



//******************************************
//******************************************
//********** SEND CONSTANT STRING **********
//******************************************
//******************************************
void http_send_const_string (CONSTANT BYTE *string_to_send)
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





