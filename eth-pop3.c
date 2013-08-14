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
//POP3 (POST OFFICE PROTOCOL) C CODE FILE


#include "main.h"					//Global data type definitions (see https://github.com/ibexuk/C_Generic_Header_File )
#define	ETH_POP3_C
#include "eth-pop3.h"
#undef	ETH_POP3_C

#include "eth-main.h"
#include "eth-tcp.h"
#include "eth-arp.h"
#include "eth-nic.h"

#ifdef STACK_USE_DNS
#include "eth-dns.h"
#endif

#ifdef STACK_USE_SMTP
#include "eth-smtp.h"
#endif


#include "ap-main.h"			//Your application file that includes the definition for the function used to process
								//received pop3 emails and the variables for the POP3 server strings if used


//--------------------------------------------------------------------------
//----- GENERATE COMPILE ERROR IF POP3 HAS NOT BEEN DEFINED TO BE USED -----
//--------------------------------------------------------------------------
#ifndef STACK_USE_POP3
#error POP3 file is included in project but not defined to be used - remove file from project to reduce code size.
#endif






//*******************************
//*******************************
//********** GET EMAIL **********
//*******************************
//*******************************
//Returns:-
//	0 = unable to start process at the current time
//	1 = mail receive process started
BYTE email_start_receive (void)
{
	
	//----- CHECK WE ARE CONNECTED -----
	if (!nic_linked_and_ip_address_valid)
		return(0);

	//----- DON'T GET EMAIL WHILE WE'RE SENDING EMAIL -----
	#ifdef STACK_USE_SMTP
		if (email_is_send_active())
			return(0);
	#endif

	//----- CHECK WE'RE NOT ALREADY RECEIVING EMAIL -----
	if (email_is_receive_active())
		return(0);

	//----- START RECEIVE EMAIL PROCESS -----
	pop3_100ms_timeout_timer = EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS;
	sm_pop3 = SM_POP3_GET_EMAIL;						//Trigger state machine to do the get email process

	//UPDATE USER DISPLAYED STRING IF IN USE
	#ifdef DO_POP3_PROGRESS_STRING
		pop3_email_progress_string_pointer = pop3_email_progress_string_null;
		pop3_email_progress_string_update = 0;
	#endif

	return(1);
}



//********************************************************
//********************************************************
//********** ARE WE CURRRENTLY RECEIVING EMAIL? **********
//********************************************************
//********************************************************
//Returns 1 is receive is active, 0 if not
BYTE email_is_receive_active (void)
{

	if (sm_pop3 == SM_POP3_IDLE)
		return(0);
	else
		return(1);
}



//***************************************
//***************************************
//********** PROCESS GET EMAIL **********
//***************************************
//***************************************
//This function is called reguarly by tcp_ip_process_stack
void email_process_pop3 (void)
{
	static WORD number_of_emails_in_pop3_box;
	static WORD get_emails_next_email_number;
	static BYTE pop3_10ms_clock_timer_last;
	static BYTE get_email_all_fields_done;
	BYTE received_byte;
	BYTE loop_count;
	BYTE temp_string[64];
	BYTE b_temp;
	BYTE *string_pointer;
	IP_ADDR dns_resolved_ip_address;


	//-----------------------------------
	//----- CHECK FOR UPDATE TIMERS -----
	//-----------------------------------
	if ((BYTE)((BYTE)(ethernet_10ms_clock_timer & 0x000000ff) - pop3_10ms_clock_timer_last) >= 10)
	{
		pop3_10ms_clock_timer_last = (BYTE)(ethernet_10ms_clock_timer & 0x000000ff);
		
		//OUR TIMEOUT TIMER
		if (pop3_100ms_timeout_timer)
			pop3_100ms_timeout_timer--;
	}

	
	if ((sm_pop3 != SM_POP3_IDLE) && (sm_pop3 != SM_POP3_FAILED))
	{
		//----------------------------------------------------
		//----- RECEIVE IS ACTIVE - DO BACKGROUND CHECKS -----
		//----------------------------------------------------
		
		//CHECK FOR RESPONSE TIMEOUT
		if (pop3_100ms_timeout_timer == 0)
			goto process_get_email_tcp_failed;
		
		//CHECK FOR LOST ETHERNET CONNECTION
		if (!nic_is_linked)
			goto process_get_email_tcp_failed;
	}

	

	switch(sm_pop3)
	{
	case SM_POP3_IDLE:
		//----------------
		//----------------
		//----- IDLE -----
		//----------------
		//----------------

		break;


	case SM_POP3_GET_EMAIL:
		//---------------------
		//---------------------
		//----- GET EMAIL -----
		//---------------------
		//---------------------

		//GET THE IP ADDRESS OF THE POP3 SERVER
		email_return_pop3_url(&temp_string[0], sizeof(temp_string));

		if (!do_dns_query(temp_string, QNS_QUERY_TYPE_HOST))
		{
			//DNS query not currently available - probably already doing a query - try again next time
			break;
		}
		
		//WAIT FOR DNS TO COMPLETE OUR QUERY
		sm_pop3 = SM_POP3_WAITING_DNS_RESPONSE;
		pop3_100ms_timeout_timer = EMAIL_DO_DNS_TIMEOUT_x100MS;

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_POP3_PROGRESS_STRING
			pop3_email_progress_string_pointer = pop3_email_progress_string_wait_pop3_dns_resp;
			pop3_email_progress_string_update = 1;
		#endif

		break;


	case SM_POP3_WAITING_DNS_RESPONSE:
		//------------------------------------------------
		//------------------------------------------------
		//----- WAITING FOR POP3 SERVER DNS RESPONSE -----
		//------------------------------------------------
		//------------------------------------------------

		//SEE IF DNS RESPONSE HAS BEEN RECEIVED
		dns_resolved_ip_address = check_dns_response();

		if (dns_resolved_ip_address.Val == 0xffffffff)
		{
			//----- DNS QUERY FAILED -----
			sm_pop3 = SM_POP3_FAILED;
			break;
		}

		if (dns_resolved_ip_address.Val)
		{
			//----- DNS QUERY SUCESSFUL -----
			//Store the IP address
			pop3_server_node.ip_address.Val = dns_resolved_ip_address.Val;

			sm_pop3 = SM_POP3_SEND_ARP_REQUEST;
		}
		break;	
			

	case SM_POP3_SEND_ARP_REQUEST:
		//------------------------------------------------------------
		//------------------------------------------------------------
		//----- SEND ARP REQUEST FOR THE DNS RETURNED IP ADDRESS -----
		//------------------------------------------------------------
		//------------------------------------------------------------
		
		//Do ARP query to get the MAC address of the server or our gateway
		if (!arp_resolve_ip_address(&pop3_server_node.ip_address))
		{
			//ARP request cannot be sent right now - try again next time
			break;
		}
		pop3_100ms_timeout_timer = EMAIL_DO_ARP_TIMEOUT_x100MS;
		sm_pop3 = SM_POP3_WAIT_FOR_ARP_RESPONSE;

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_POP3_PROGRESS_STRING
			pop3_email_progress_string_pointer = pop3_email_progress_string_wait_pop3_arp_resp;
			pop3_email_progress_string_update = 1;
		#endif

		break;


	case SM_POP3_WAIT_FOR_ARP_RESPONSE:
		//---------------------------------
		//---------------------------------
		//----- WAIT FOR ARP RESPONSE -----
		//---------------------------------
		//---------------------------------
		//Wait for the ARP response
		if (arp_is_resolve_complete(&pop3_server_node.ip_address, &pop3_server_node.mac_address))
		{
			//----- ARP RESPONSE RECEVIED -----
			sm_pop3 = SM_POP3_OPEN_TCP_CONNECTION;
		}
		break;


	case SM_POP3_OPEN_TCP_CONNECTION:
		//-------------------------------
		//-------------------------------
		//----- OPEN TCP CONNECTION -----
		//-------------------------------
		//-------------------------------

		//----- CREATE A TCP CONNECTION TO THE POP3 SERVER AND WAIT FOR THE GREETING MESSAGE -----
		if (pop3_tcp_socket != TCP_INVALID_SOCKET)			//We shouldn't have a socket currently, but make sure
			tcp_close_socket(pop3_tcp_socket);

		pop3_tcp_socket = tcp_connect_socket(&pop3_server_node, POP3_REMOTE_PORT);
		
		if (pop3_tcp_socket == TCP_INVALID_SOCKET)
		{
			//No TCP sockets currently available - try again next time
			break;
		}

		pop3_100ms_timeout_timer = EMAIL_WAIT_TCP_CONNECTION_TIMEOUT_x100MS;
		sm_pop3 = SM_POP3_WAIT_FOR_TCP_CONNECTION;

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_POP3_PROGRESS_STRING
			pop3_email_progress_string_pointer = pop3_email_progress_string_wait_pop3_tcp_conn;
			pop3_email_progress_string_update = 1;
		#endif

		break;


	case SM_POP3_WAIT_FOR_TCP_CONNECTION:
		//--------------------------------------
		//--------------------------------------
		//----- WAITING FOR TCP CONNECTION -----
		//--------------------------------------
		//--------------------------------------
		//Has connection been made?
		if (tcp_is_socket_connected(pop3_tcp_socket))
		{
			sm_pop3 = SM_POP3_WAIT_FOR_POP3_SERVER_GREETING;
			pop3_100ms_timeout_timer = EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS;

			//UPDATE USER DISPLAYED STRING IF IN USE
			#ifdef DO_POP3_PROGRESS_STRING
				pop3_email_progress_string_pointer = pop3_email_progress_string_wait_pop3_greeting;
				pop3_email_progress_string_update = 1;
			#endif
		}

		break;


	case SM_POP3_WAIT_FOR_POP3_SERVER_GREETING:
		//-----------------------------------------------------------
		//-----------------------------------------------------------
		//----- WAITING FOR POP3 SERVER GREETING TO BE RECEIVED -----
		//-----------------------------------------------------------
		//-----------------------------------------------------------
		
		//Check for packet received
		if (tcp_check_socket_for_rx(pop3_tcp_socket))
		{
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte == '+')
			{
				//RESPONSE IS '+OK'
				tcp_dump_rx_packet();				//Dump the remainder of the recevied packet

				sm_pop3 = SM_POP3_SEND_POP3_USERNAME;
			}
			else
			{
				//RESPONSE IS NOT '+OK' - ERROR
				goto process_get_email_tcp_failed;
			}
	    }
		break;


	case SM_POP3_SEND_POP3_USERNAME:
		//------------------------------
		//------------------------------
		//----- SEND POP3 USERNAME -----
		//------------------------------
		//------------------------------
		if (!tcp_setup_socket_tx(pop3_tcp_socket))
		{
			//Can't send currently - try again next time
			break;
		}
		//Send the username
		tcp_write_next_byte('U');
		tcp_write_next_byte('S');
		tcp_write_next_byte('E');
		tcp_write_next_byte('R');
		tcp_write_next_byte(' ');

		//Get the pop3 username
		email_return_pop3_username(&temp_string[0], sizeof(temp_string));

		for (loop_count = 0; temp_string[loop_count] != 0x00; loop_count++)
			tcp_write_next_byte(temp_string[loop_count]);	

		tcp_write_next_byte('\r');		//CR
		tcp_write_next_byte('\n');		//LF

		tcp_socket_tx_packet(pop3_tcp_socket);				//Send the packet

		sm_pop3 = SM_POP3_WAIT_FOR_POP3_USER_RESPONSE;
		pop3_100ms_timeout_timer = EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS;

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_POP3_PROGRESS_STRING
			pop3_email_progress_string_pointer = pop3_email_progress_string_wait_pop3_user;
			pop3_email_progress_string_update = 1;
		#endif

		break;


	case SM_POP3_WAIT_FOR_POP3_USER_RESPONSE:
		//--------------------------------------------
		//--------------------------------------------
		//----- WAITING FOR RESPONSE TO USERNAME -----
		//--------------------------------------------
		//--------------------------------------------

		//DO WE NEED TO RE-SEND THE LAST PACKET?
		//(TCP requires resending of packets if they are not acknowledged and to avoid requiring a large RAM buffer the application needs to remember the last packet sent on a socket
		//so it can be resent if requried - If your application has ram available to store a copy of the last sent packet then the tcp driver could be modified to use it instead).
		if (tcp_does_socket_require_resend_of_last_packet(pop3_tcp_socket))
		{
			sm_pop3 = SM_POP3_SEND_POP3_USERNAME;
			break;
		}

		//CHECK FOR PACKET RECEIVED
		if (tcp_check_socket_for_rx(pop3_tcp_socket))
		{
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte == '+')
			{
				//RESPONSE IS '+OK'
				tcp_dump_rx_packet();				//Dump the remainder of the received packet

				sm_pop3 = SM_POP3_SEND_POP3_PASSWORD;
			}
			else
			{
				//RESPONSE IS NOT '+OK' - ERROR
				goto process_get_email_tcp_failed;
			}
	    }
		break;


	case SM_POP3_SEND_POP3_PASSWORD:
		//------------------------------
		//------------------------------
		//----- SEND POP3 PASSWORD -----
		//------------------------------
		//------------------------------
		if (!tcp_setup_socket_tx(pop3_tcp_socket))
		{
			//Can't send currently - try again next time
			break;
		}

		//Send the username
		tcp_write_next_byte('P');
		tcp_write_next_byte('A');
		tcp_write_next_byte('S');
		tcp_write_next_byte('S');
		tcp_write_next_byte(' ');

		//Get the pop3 password
		email_return_pop3_password(&temp_string[0], sizeof(temp_string));

		for (loop_count = 0; temp_string[loop_count] != 0x00; loop_count++)
			tcp_write_next_byte(temp_string[loop_count]);	

		tcp_write_next_byte('\r');		//CR
		tcp_write_next_byte('\n');		//LF

		tcp_socket_tx_packet(pop3_tcp_socket);				//Send the packet

		sm_pop3 = SM_POP3_WAIT_FOR_POP3_PASS_RESPONSE;
		pop3_100ms_timeout_timer = EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS;

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_POP3_PROGRESS_STRING
			pop3_email_progress_string_pointer = pop3_email_progress_string_wait_pop3_password;
			pop3_email_progress_string_update = 1;
		#endif

		break;


	case SM_POP3_WAIT_FOR_POP3_PASS_RESPONSE:
		//--------------------------------------------
		//--------------------------------------------
		//----- WAITING FOR RESPONSE TO PASSWORD -----
		//--------------------------------------------
		//--------------------------------------------

		//DO WE NEED TO RE-SEND THE LAST PACKET?
		if (tcp_does_socket_require_resend_of_last_packet(pop3_tcp_socket))
		{
			sm_pop3 = SM_POP3_SEND_POP3_PASSWORD;
			break;
		}

		//CHECK FOR PACKET RECEIVED
		if (tcp_check_socket_for_rx(pop3_tcp_socket))
		{
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte == '+')
			{
				//RESPONSE IS '+OK' - SEND STAT TO GET NUMBER OF EMAILS IN THE POP3 BOX
				tcp_dump_rx_packet();				//Dump the remainder of the recevied packet

				sm_pop3 = SM_POP3_SEND_POP3_STAT_REQUEST;
			}
			else
			{
				//RESPONSE IS NOT '+OK' - ERROR
				goto process_get_email_tcp_failed;
			}
	    }
		break;


	case SM_POP3_SEND_POP3_STAT_REQUEST:
		//----------------------------------
		//----------------------------------
		//----- SEND POP3 STAT REQUEST -----
		//----------------------------------
		//----------------------------------
		if (!tcp_setup_socket_tx(pop3_tcp_socket))
		{
			//Can't send currently - try again next time
			break;
		}

		//Send the command
		tcp_write_next_byte('S');
		tcp_write_next_byte('T');
		tcp_write_next_byte('A');
		tcp_write_next_byte('T');

		tcp_write_next_byte('\r');		//CR
		tcp_write_next_byte('\n');		//LF

		tcp_socket_tx_packet(pop3_tcp_socket);				//Send the packet

		sm_pop3 = SM_POP3_WAIT_FOR_POP3_STAT_RESPONSE;
		pop3_100ms_timeout_timer = EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS;

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_POP3_PROGRESS_STRING
			pop3_email_progress_string_pointer = pop3_email_progress_string_wait_pop3_stat;
			pop3_email_progress_string_update = 1;
		#endif

		break;


	case SM_POP3_WAIT_FOR_POP3_STAT_RESPONSE:
		//--------------------------------------------------
		//--------------------------------------------------
		//----- WAITING FOR RESPONSE TO 'STAT' REQUEST -----
		//--------------------------------------------------
		//--------------------------------------------------

		//DO WE NEED TO RE-SEND THE LAST PACKET?
		if (tcp_does_socket_require_resend_of_last_packet(pop3_tcp_socket))
		{
			sm_pop3 = SM_POP3_SEND_POP3_STAT_REQUEST;
			break;
		}

		//CHECK FOR PACKET RECEIVED
		if (tcp_check_socket_for_rx(pop3_tcp_socket))
		{
			//Check for '+OK ' at start of line
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte != '+')
				goto process_get_email_tcp_failed;
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte != 'O')
				goto process_get_email_tcp_failed;
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte != 'K')
				goto process_get_email_tcp_failed;
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte != ' ')
				goto process_get_email_tcp_failed;

			//Get the number of messages waiting (up to 4 digits)
			
			tcp_read_next_rx_byte(&received_byte);
			temp_string[0] = received_byte;
			temp_string[1] = 0x00;
			temp_string[2] = 0x00;
			temp_string[3] = 0x00;
			temp_string[4] = 0x00;

			tcp_read_next_rx_byte(&received_byte);
			if (received_byte != ' ')
			{
				temp_string[1] = received_byte;

				tcp_read_next_rx_byte(&received_byte);
				if (received_byte != ' ')
				{
					temp_string[2] = received_byte;

					tcp_read_next_rx_byte(&received_byte);
					if (received_byte != ' ')
					{
						temp_string[3] = received_byte;
					}
				}
			}

			number_of_emails_in_pop3_box = convert_ascii_to_integer(&temp_string[0]);

			tcp_dump_rx_packet();				//Dump the remainder of the recevied packet

			get_emails_next_email_number = 1;
			sm_pop3 = SM_POP3_WAIT_GET_NEXT_EMAIL;
		}

		break;


	case SM_POP3_WAIT_GET_NEXT_EMAIL:
		//------------------------------
		//------------------------------
		//----- REQUEST NEXT EMAIL -----
		//------------------------------
		//------------------------------
		if (get_emails_next_email_number > number_of_emails_in_pop3_box)
		{
			//----- ALL EMAILS RETREIVED - QUIT -----
			sm_pop3 = SM_POP3_DO_QUIT;
			break;
		}

		//----- GET NEXT EMAIL -----
		if (!tcp_setup_socket_tx(pop3_tcp_socket))
		{
			//Can't send currently - try again next time
			break;
		}

		//Send the command
		tcp_write_next_byte('R');
		tcp_write_next_byte('E');
		tcp_write_next_byte('T');
		tcp_write_next_byte('R');
		tcp_write_next_byte(' ');
		
		//Send the message number
		convert_word_to_ascii(get_emails_next_email_number, &temp_string[0]);
		for (loop_count = 0; loop_count < 6; loop_count++)
		{
			if (temp_string[loop_count] != 0x00)
				tcp_write_next_byte(temp_string[loop_count]);
			else
				break;
		}
		tcp_write_next_byte('\r');		//CR
		tcp_write_next_byte('\n');		//LF

		tcp_socket_tx_packet(pop3_tcp_socket);				//Send the packet

		get_emails_next_email_number++;
        pop3_receive_message_string_len = 0;
        get_email_all_fields_done = 0;
		pop3_100ms_timeout_timer = EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS;
		sm_pop3 = SM_POP3_WAIT_GET_NEXT_EMAIL_RESPONSE;

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_POP3_PROGRESS_STRING
			pop3_email_progress_string_pointer = pop3_email_progress_string_wait_pop3_header;
			pop3_email_progress_string_update = 1;
		#endif

		break;


	case SM_POP3_WAIT_GET_NEXT_EMAIL_RESPONSE:
		//-----------------------------------------------------
		//-----------------------------------------------------
		//----- WAITING FOR RESPONSE TO GET EMAIL REQUEST -----
		//-----------------------------------------------------
		//-----------------------------------------------------

		//DO WE NEED TO RE-SEND THE LAST PACKET?
		if (tcp_does_socket_require_resend_of_last_packet(pop3_tcp_socket))
		{
			get_emails_next_email_number--;
			sm_pop3 = SM_POP3_WAIT_GET_NEXT_EMAIL;
			break;
		}

		//CHECK FOR PACKET RECEIVED
		if (tcp_check_socket_for_rx(pop3_tcp_socket))
		{
			//Check for '+OK ' at start of line
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte != '+')
				goto process_get_email_tcp_failed;
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte != 'O')
				goto process_get_email_tcp_failed;
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte != 'K')
				goto process_get_email_tcp_failed;
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte != ' ')
				goto process_get_email_tcp_failed;

			//Don't dump the packet - leave it for the next state to process as this packet may contain lines from the header
			sm_pop3 = SM_POP3_WAIT_GET_NEXT_EMAIL_RECEIVE;
		}

		//break;	//Fall into next state


	case SM_POP3_WAIT_GET_NEXT_EMAIL_RECEIVE:
		//-------------------------------------
		//-------------------------------------
		//----- RECEIVE THE CURRENT EMAIL -----
		//-------------------------------------
		//-------------------------------------
		//Check for packet received
		if (tcp_check_socket_for_rx(pop3_tcp_socket))
		{
			//Remember that the response may span over more than 1 packet, especially if the header is large due to routing
			
			while (tcp_read_next_rx_byte(&received_byte))			//Get next byte, returns 0 if end of packet
			{
				//----- RECEIVE EACH LINE -----
			    if (received_byte != '\r')						//Do until we get the CR character			
			    {
				    //Store each character, truncating any lines that don't fit in our string array
			        if ((pop3_receive_message_string_len < (POP3_RECEIVE_MESSAGE_STRING_MAX_LEN - 1)) && (received_byte != '\n'))		//Don't include the line feed character
						pop3_receive_message_string[pop3_receive_message_string_len++] = received_byte;
			    }
			    else
			    {
				    //----------------------------------------------------
				    //----- CR CHARACTER RECEIVED - PROCESS THE LINE -----
				    //----------------------------------------------------
			        pop3_receive_message_string[pop3_receive_message_string_len] = 0x00;			//Add null termination to the string

					//----- LOOK FOR THE 'FROM' FIELD -----
					if (find_string_in_string_no_case(&pop3_receive_message_string[0], &pop3_reply_to_string[0]))	//Does this line contain this string?
					{
						//--------------------------------
						//----- 'FROM' EMAIL ADDRESS -----
						//--------------------------------
						string_pointer = find_character_in_string(&pop3_receive_message_string[0], '<');
						if (string_pointer != 0)
						{
							string_pointer++;
							for (b_temp = 0; b_temp < (POP3_REPLY_TO_STRING_LEN - 1); b_temp++)
							{
								if (*string_pointer == '>')
								{
									//ALL OF REPLY EMAIL ADDRESS HAS BEEN COPIED
									email_reply_to_string[b_temp] = 0x00;					//Add null termination

									get_email_all_fields_done |= 0x01;						//Flag that we have the reply address
									break;
								}
								//Store next character
								email_reply_to_string[b_temp] = *string_pointer++;
							}
						}
					}

					//----- LOOK FOR THE 'SUBJECT' FIELD -----
					string_pointer = find_string_in_string_no_case(&pop3_receive_message_string[0], &pop3_subject_string[0]);	//Does this line contain this string?
					if (string_pointer)
					{
						//-------------------------
						//----- EMAIL SUBJECT -----
						//-------------------------
						
						//Pass to user application to process
						POP3_PROCESS_RECEIVED_EMAIL_LINE_FUNCTION(0, string_pointer + sizeof(pop3_subject_string), &email_reply_to_string[0]);
						get_email_all_fields_done |= 0x02;						//Flag that we have the subject
					}
					//----- LOOK FOR THE END OF HEADERS MARKER -----
					if ((pop3_receive_message_string_len == 0))
					{
						//---------------------------------
						//----- END OF HEADERS MARKER -----
						//---------------------------------
						get_email_all_fields_done |= 0x04;						//Flag that we have moved to the email body
					}
					//----- LOOK FOR EMAIL BODY -----
					else if (get_email_all_fields_done == 0x07)
					{
						//-----------------------------------
						//----- NEXT LINE OF EMAIL BODY -----
						//-----------------------------------

						//Pass to user application to process
						POP3_PROCESS_RECEIVED_EMAIL_LINE_FUNCTION(1, &pop3_receive_message_string[0], &email_reply_to_string[0]);
					}

					//----- LOOK FOR THE END OF MESSAGE MARKER -----			        
					if ((pop3_receive_message_string_len == 1) && (pop3_receive_message_string[0] == '.'))
					{
						//---------------------------------
						//----- END OF MESSAGE MARKER -----
						//---------------------------------

						//Flag end of email to user application
						if (POP3_PROCESS_RECEIVED_EMAIL_LINE_FUNCTION(2, 0x00, &email_reply_to_string[0]))
						{
							//Delete email, which will return to the getting of the next email
							sm_pop3 = SM_POP3_DO_DELETE;
						}
						else
						{
							//User does not want this email deleted
							sm_pop3 = SM_POP3_WAIT_GET_NEXT_EMAIL;
						}
					}

			        //Reset ready for the next line
			        pop3_receive_message_string_len = 0;
			    }
			}

			tcp_dump_rx_packet();				//Dump the remainder of the recevied packet

		}

		break;


	case SM_POP3_DO_DELETE:
		//------------------------------------
		//------------------------------------
		//----- DELETE THE CURRENT EMAIL -----
		//------------------------------------
		//------------------------------------
		//----- GET NEXT EMAIL HEADER -----
		if (!tcp_setup_socket_tx(pop3_tcp_socket))
		{
			//Can't send currently - try again next time
			break;
		}
		//Send the command
		tcp_write_next_byte('D');
		tcp_write_next_byte('E');
		tcp_write_next_byte('L');
		tcp_write_next_byte('E');
		tcp_write_next_byte(' ');
			
		//Send the message number
		convert_word_to_ascii((get_emails_next_email_number - 1), &temp_string[0]);
		for (loop_count = 0; loop_count < 6; loop_count++)
		{
			if (temp_string[loop_count] != 0x00)
				tcp_write_next_byte(temp_string[loop_count]);
			else
				break;
		}

		tcp_write_next_byte('\r');		//CR
		tcp_write_next_byte('\n');		//LF

		tcp_socket_tx_packet(pop3_tcp_socket);				//Send the packet

		pop3_100ms_timeout_timer = EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS;
		sm_pop3 = SM_POP3_DO_DELETE_RESPONSE;

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_POP3_PROGRESS_STRING
			pop3_email_progress_string_pointer = pop3_email_progress_string_wait_pop3_delete;
			pop3_email_progress_string_update = 1;
		#endif

		break;


	case SM_POP3_DO_DELETE_RESPONSE:
		//------------------------------------
		//------------------------------------
		//----- WAIT FOR DELETE RESPONSE -----
		//------------------------------------
		//------------------------------------

		//DO WE NEED TO RE-SEND THE LAST PACKET?
		if (tcp_does_socket_require_resend_of_last_packet(pop3_tcp_socket))
		{
			sm_pop3 = SM_POP3_DO_DELETE;
			break;
		}

		//CHECK FOR PACKET RECEIVED
		if (tcp_check_socket_for_rx(pop3_tcp_socket))
		{
			//Check for '+OK ' at start of line
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte != '+')
				goto process_get_email_tcp_failed;
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte != 'O')
				goto process_get_email_tcp_failed;
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte != 'K')
				goto process_get_email_tcp_failed;
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte != ' ')
				goto process_get_email_tcp_failed;

			tcp_dump_rx_packet();				//Dump the remainder of the recevied packet

			pop3_100ms_timeout_timer = EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS;
			
			sm_pop3 = SM_POP3_WAIT_GET_NEXT_EMAIL;
		}
		break;


	case SM_POP3_DO_QUIT:
		//------------------------------------------
		//------------------------------------------
		//----- QUIT CONNECTION TO POP3 SERVER -----
		//------------------------------------------
		//------------------------------------------
		if (!tcp_setup_socket_tx(pop3_tcp_socket))
		{
			//Can't send currently - try again next time
			break;
		}
		
		//Send the username
		tcp_write_next_byte('Q');
		tcp_write_next_byte('U');
		tcp_write_next_byte('I');
		tcp_write_next_byte('T');

		tcp_write_next_byte('\r');		//CR
		tcp_write_next_byte('\n');		//LF

		tcp_socket_tx_packet(pop3_tcp_socket);				//Send the packet

		sm_pop3 = SM_POP3_WAIT_FOR_POP3_QUIT_RESPONSE;
		pop3_100ms_timeout_timer = EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS;

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_POP3_PROGRESS_STRING
			pop3_email_progress_string_pointer = pop3_email_progress_string_wait_pop3_quit;
			pop3_email_progress_string_update = 1;
		#endif

		break;


	case SM_POP3_WAIT_FOR_POP3_QUIT_RESPONSE:
		//----------------------------------
		//----------------------------------
		//----- WAIT FOR QUIT RESPONSE -----
		//----------------------------------
		//----------------------------------

		//DO WE NEED TO RE-SEND THE LAST PACKET?
		if (tcp_does_socket_require_resend_of_last_packet(pop3_tcp_socket))
		{
			sm_pop3 = SM_POP3_DO_QUIT;
			break;
		}

		//CHECK FOR PACKET RECEIVED
		if (tcp_check_socket_for_rx(pop3_tcp_socket))
		{
			//----- ALL DONE - DUMP THE TCP CONNECTION AND RETURN TO IDLE MODE -----
			tcp_request_disconnect_socket(pop3_tcp_socket);
			sm_pop3 = SM_POP3_IDLE;

			//UPDATE USER DISPLAYED STRING IF IN USE
			#ifdef DO_POP3_PROGRESS_STRING
				pop3_email_progress_string_pointer = pop3_email_progress_string_complete;
				pop3_email_progress_string_update = 1;
			#endif
		}
		break;

	case SM_POP3_FAILED:
		//------------------------------------
		//------------------------------------
		//----- GET EMAIL PROCESS FAILED -----
		//------------------------------------
		//------------------------------------

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_POP3_PROGRESS_STRING
			pop3_email_progress_string_pointer = pop3_email_progress_string_failed;
			pop3_email_progress_string_update = 1;
		#endif

		sm_pop3 = SM_POP3_IDLE;

		break;
		
	}


	return;

process_get_email_tcp_failed:
//----- FAILED DURING TCP TRANSFER - CLOSE CONNECTION AND CHANGE STATE TO ERROR -----
	if ((sm_pop3 != SM_POP3_GET_EMAIL) && (sm_pop3 != SM_POP3_WAIT_FOR_ARP_RESPONSE) && (sm_pop3 != SM_POP3_WAITING_DNS_RESPONSE))
	{
		tcp_dump_rx_packet();								//Dump the remainder of any received packet
		tcp_request_disconnect_socket(pop3_tcp_socket);		//Close the connection
	}

	sm_pop3 = SM_POP3_FAILED;

	return;

}





//********************************************
//********************************************
//********** RETURN POP3 SERVER URL **********
//********************************************
//********************************************
//(e.g. pop3.domain.com)
void email_return_pop3_url (BYTE *string_pointer, BYTE max_length)
{
	BYTE count = 0;
	#ifdef POP3_USING_CONST_ROM_SETTINGS
	CONSTANT BYTE *p_source_string;				//Using hard coded constant strings
	#else
	BYTE *p_source_string;							//Using variable strings
	#endif

	p_source_string = &POP3_SERVER_STRING[0];
	do
	{
		//Check for overflow
		if (++count == max_length)
		{
			*string_pointer++ = 0x00;
			return;
		}

		//Copy next byte
		*string_pointer++ = *p_source_string;
	}
	while (*p_source_string++ != 0x00);
	
}



//******************************************
//******************************************
//********** RETURN POP3 USERNAME **********
//******************************************
//******************************************
void email_return_pop3_username (BYTE *string_pointer, BYTE max_length)
{
	BYTE count = 0;
	#ifdef POP3_USING_CONST_ROM_SETTINGS
	CONSTANT BYTE *p_source_string;				//Using hard coded constant strings
	#else
	BYTE *p_source_string;							//Using variable strings
	#endif

	p_source_string = &POP3_USERNAME_STRING[0];
	do
	{
		//Check for overflow
		if (++count == max_length)
		{
			*string_pointer++ = 0x00;
			return;
		}
		
		//Copy next byte
		*string_pointer++ = *p_source_string;
	}
	while (*p_source_string++ != 0x00);
	
}



//******************************************
//******************************************
//********** RETURN POP3 PASSWORD **********
//******************************************
//******************************************
void email_return_pop3_password (BYTE *string_pointer, BYTE max_length)
{
	BYTE count = 0;
	#ifdef POP3_USING_CONST_ROM_SETTINGS
	CONSTANT BYTE *p_source_string;				//Using hard coded constant strings
	#else
	BYTE *p_source_string;							//Using variable strings
	#endif

	p_source_string = &POP3_PASSWORD_STRING[0];
	do
	{
		//Check for overflow
		if (++count == max_length)
		{
			*string_pointer++ = 0x00;
			return;
		}

		//Copy next byte
		*string_pointer++ = *p_source_string;
	}
	while (*p_source_string++ != 0x00);
	
}









