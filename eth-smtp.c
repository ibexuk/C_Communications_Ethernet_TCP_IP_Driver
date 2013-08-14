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
//SMTP (SIMPLE MAIL TRANSFER PROTOCOL) C CODE FILE


#include "main.h"					//Global data type definitions (see https://github.com/ibexuk/C_Generic_Header_File )
#define	ETH_SMTP_C
#include "eth-smtp.h"
#undef	ETH_SMTP_C

#include "eth-main.h"
#include "eth-tcp.h"
#include "eth-arp.h"
#include "eth-nic.h"

#ifdef STACK_USE_DNS
#include "eth-dns.h"
#endif

#ifdef STACK_USE_POP3
#include "eth-pop3.h"
#endif


#include "ap-main.h"			//Your application file that includes the definition for the function used to process
								//received pop3 emails and the variables for the SMTP server strings if used


//--------------------------------------------------------------------------
//----- GENERATE COMPILE ERROR IF SMTP HAS NOT BEEN DEFINED TO BE USED -----
//--------------------------------------------------------------------------
#ifndef STACK_USE_SMTP
#error SMTP file is included in project but not defined to be used - remove file from project to reduce code size.
#endif




//********************************
//********************************
//********** SEND EMAIL **********
//********************************
//********************************
//Returns:-
//	0 = unable to start process at the current time
//	1 = mail receive process started
BYTE email_start_send (BYTE use_authenticated_login, BYTE include_file_attachment)
{	
	
	//----- CHECK WE ARE CONNECTED -----
	if (!nic_linked_and_ip_address_valid)
		return(0);

	//----- DON'T SEND EMAIL WHILE WE'RE RECEIVING EMAIL -----
	#ifdef STACK_USE_POP3
		if (email_is_receive_active())
			return(0);
	#endif

	//----- CHECK WE'RE NOT ALREADY SENDING EMAIL -----
	if (email_is_send_active())
		return(0);

	//----- FLAG IF WE'RE TO DO AN AUTHENTICATED LOGIN -----
	if (use_authenticated_login)
		smtp_use_authenticated_login = 1;			//1 = use SMTP authenticated login (username and password requried), 0 = normal login
	else
		smtp_use_authenticated_login = 0;

	//----- FLAG IF WE'RE GOING TO INCLUDE A FILE ATTACHMENT -----
	if (include_file_attachment)
		smtp_include_file_attachment = 1;			//1 = use SMTP authenticated login (username and password requried), 0 = normal login
	else
		smtp_include_file_attachment = 0;


	//----- START SEND EMAIL PROCESS -----
	send_email_body_byte_number = 0xffff;
	sm_send_email_data = SM_SEND_EMAIL_DATA_MIME_HEADER;
	smtp_100ms_timeout_timer = EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS;
	sm_smtp = SM_SMTP_SEND_EMAIL;						//Trigger state machine to do the send email process

	//UPDATE USER DISPLAYED STRING IF IN USE
	#ifdef DO_SMTP_PROGRESS_STRING
		smtp_email_progress_string_pointer = smtp_email_progress_string_null;
		smtp_email_progress_string_update = 0;
	#endif

	return(1);
}



//******************************************************
//******************************************************
//********** ARE WE CURRRENTLY SENDING EMAIL? **********
//******************************************************
//******************************************************
//Returns 1 is receive is active, 0 if not
BYTE email_is_send_active (void)
{
	if (sm_smtp == SM_SMTP_IDLE)
		return(0);
	else
		return(1);
}



//****************************************
//****************************************
//********** PROCESS SEND EMAIL **********
//****************************************
//****************************************
//This function is called reguarly by tcp_ip_process_stack
void email_process_smtp (void)
{
	static BYTE smtp_10ms_clock_timer_last;
	BYTE received_byte;
	BYTE loop_count;
	WORD count;
	BYTE b_temp;
	BYTE b_temp1;
	BYTE base64_source[3];
	BYTE base64_converted[4];
	BYTE temp_string[64];
	IP_ADDR dns_resolved_ip_address;
	BYTE resend_flag;



	//-----------------------------------
	//----- CHECK FOR UPDATE TIMERS -----
	//-----------------------------------
	if ((BYTE)((BYTE)(ethernet_10ms_clock_timer & 0x000000ff) - smtp_10ms_clock_timer_last) >= 10)
	{
		smtp_10ms_clock_timer_last = (BYTE)(ethernet_10ms_clock_timer & 0x000000ff);
		
		//OUT TIMEOUT TIMER
		if (smtp_100ms_timeout_timer)
			smtp_100ms_timeout_timer--;
	}

	if ((sm_smtp != SM_SMTP_IDLE) && (sm_smtp != SM_SMTP_FAILED))
	{
		//-------------------------------------------------
		//----- SEND IS ACTIVE - DO BACKGROUND CHECKS -----
		//-------------------------------------------------
		
		//CHECK FOR RESPONSE TIMEOUT
		if (smtp_100ms_timeout_timer == 0)
			goto process_send_email_tcp_failed;

		//CHECK FOR LOST ETHERNET CONNECTION
		if (!nic_is_linked)
			goto process_send_email_tcp_failed;
	}

	switch(sm_smtp)
	{
	case SM_SMTP_IDLE:
		//----------------
		//----------------
		//----- IDLE -----
		//----------------
		//----------------
		
		break;


	case SM_SMTP_SEND_EMAIL:
		//----------------------
		//----------------------
		//----- SEND EMAIL -----
		//----------------------
		//----------------------

		//GET THE IP ADDRESS OF THE SMTP SERVER
		email_return_smtp_url(&temp_string[0], sizeof(temp_string));

		if (!do_dns_query(temp_string, QNS_QUERY_TYPE_HOST))
		{
			//DNS query not available - try again next time
			break;
		}

		sm_smtp = SM_SMTP_WAITING_DNS_RESPONSE;
		smtp_100ms_timeout_timer = EMAIL_DO_DNS_TIMEOUT_x100MS;

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_SMTP_PROGRESS_STRING
			smtp_email_progress_string_pointer = smtp_email_progress_string_wait_smtp_dns_resp;
			smtp_email_progress_string_update = 1;
		#endif

		break;


	case SM_SMTP_WAITING_DNS_RESPONSE:
		//------------------------------------------------
		//------------------------------------------------
		//----- WAITING FOR SMTP SERVER DNS RESPONSE -----
		//------------------------------------------------
		//------------------------------------------------

		//SEE IF DNS RESPONSE HAS BEEN RECEIVED
		dns_resolved_ip_address = check_dns_response();

		if (dns_resolved_ip_address.Val == 0xffffffff)
		{
			//----- DNS QUERY FAILED -----
			sm_smtp = SM_SMTP_FAILED;
			break;
		}

		if (dns_resolved_ip_address.Val)
		{
			//----- DNS QUERY SUCESSFUL -----
			
			//Store the IP address
			smtp_server_node.ip_address.Val = dns_resolved_ip_address.Val;
			
			sm_smtp = SM_SMTP_SEND_DNS_ARP_REQUEST;
		}
		
		break;


	case SM_SMTP_SEND_DNS_ARP_REQUEST:
		//------------------------------------------------------------
		//------------------------------------------------------------
		//----- SEND ARP REQUEST FOR THE DNS RETURNED IP ADDRESS -----
		//------------------------------------------------------------
		//------------------------------------------------------------

		//Do ARP query to get the MAC address of the server or our gateway
		if (!arp_resolve_ip_address(&smtp_server_node.ip_address))
		{
			//ARP request cannot be sent right now - try again next time
			break;
		}
	
		//ARP REQUEST WAS SENT
		smtp_100ms_timeout_timer = EMAIL_DO_ARP_TIMEOUT_x100MS;
		sm_smtp = SM_SMTP_WAIT_FOR_ARP_RESPONSE;

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_SMTP_PROGRESS_STRING
			smtp_email_progress_string_pointer = smtp_email_progress_string_wait_smtp_arp_resp;
			smtp_email_progress_string_update = 1;
		#endif
		
		break;


	case SM_SMTP_WAIT_FOR_ARP_RESPONSE:
		//---------------------------------
		//---------------------------------
		//----- WAIT FOR ARP RESPONSE -----
		//---------------------------------
		//---------------------------------
		//Wait for the ARP response
		if (arp_is_resolve_complete(&smtp_server_node.ip_address, &smtp_server_node.mac_address))
		{
			//----- ARP RESPONSE RECEVIED -----
			sm_smtp = SM_SMTP_OPEN_TCP_CONNECTION;
		}

		break;


	case SM_SMTP_OPEN_TCP_CONNECTION:
		//-------------------------------
		//-------------------------------
		//----- OPEN TCP CONNECTION -----
		//-------------------------------
		//-------------------------------

		//CREATE A TCP CONNECTION TO THE SMTP SERVER AND WAIT FOR THE GREETING MESSAGE
		if (smtp_tcp_socket != TCP_INVALID_SOCKET)			//We shouldn't have a socket currently, but make sure
			tcp_close_socket(smtp_tcp_socket);
		
		smtp_tcp_socket = tcp_connect_socket(&smtp_server_node, SMTP_REMOTE_PORT);
		
		if (smtp_tcp_socket == TCP_INVALID_SOCKET)
		{
			//No socket available - try again next time
			break;
		}

		smtp_100ms_timeout_timer = EMAIL_WAIT_TCP_CONNECTION_TIMEOUT_x100MS;
		sm_smtp = SM_SMTP_WAIT_FOR_TCP_CONNECTION;

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_SMTP_PROGRESS_STRING
			smtp_email_progress_string_pointer = smtp_email_progress_string_wait_smtp_tcp_conn;
			smtp_email_progress_string_update = 1;
		#endif

		break;


	case SM_SMTP_WAIT_FOR_TCP_CONNECTION:
		//--------------------------------------
		//--------------------------------------
		//----- WAITING FOR TCP CONNECTION -----
		//--------------------------------------
		//--------------------------------------

		//Has connection been made?
		if (tcp_is_socket_connected(smtp_tcp_socket))
		{
			sm_smtp = SM_SMTP_WAIT_FOR_SMTP_SERVER_GREETING;
			smtp_100ms_timeout_timer = EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS;

			//UPDATE USER DISPLAYED STRING IF IN USE
			#ifdef DO_SMTP_PROGRESS_STRING
				smtp_email_progress_string_pointer = smtp_email_progress_string_wait_smtp_greeting;
				smtp_email_progress_string_update = 1;
			#endif

		}

		break;


	case SM_SMTP_WAIT_FOR_SMTP_SERVER_GREETING:
		//-----------------------------------------------------------
		//-----------------------------------------------------------
		//----- WAITING FOR SMTP SERVER GREETING TO BE RECEIVED -----
		//-----------------------------------------------------------
		//-----------------------------------------------------------
		//Check for packet received
		if (tcp_check_socket_for_rx(smtp_tcp_socket))
		{
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte == '2')				//A leading 2 character indicates the server is happy (3 used for data transfers, 4 & 5 = error)
			{
				//RESPONSE IS GOOD
				tcp_dump_rx_packet();				//Dump the remainder of the recevied packet

				sm_smtp = SM_SMTP_SEND_SMTP_HELO;
				
			}
			else
			{
				//RESPONSE IS NOT GOOD - ERROR
				goto process_send_email_tcp_failed;
			}
	    }

		break;


	case SM_SMTP_SEND_SMTP_HELO:
		//---------------------
		//---------------------
		//----- SEND HELO -----
		//---------------------
		//---------------------
		if (!tcp_setup_socket_tx(smtp_tcp_socket))
		{
			//Can't send currently - try again next time
			break;
		}
		
		//Send the hello
		if (smtp_use_authenticated_login)
		{
			//Special command for authenticated login
			tcp_write_next_byte('E');
			tcp_write_next_byte('H');
		}
		else
		{
			//Normal command
			tcp_write_next_byte('H');
			tcp_write_next_byte('E');
		}
		tcp_write_next_byte('L');
		tcp_write_next_byte('O');
		tcp_write_next_byte(' ');
		tcp_write_next_byte('E');		//EMB is our computer name (can be anything)
		tcp_write_next_byte('M');
		tcp_write_next_byte('B');

		tcp_write_next_byte('\r');		//CR
		tcp_write_next_byte('\n');		//LF

		tcp_socket_tx_packet(smtp_tcp_socket);			//Send the packet

		sm_smtp = SM_SMTP_WAIT_FOR_SMTP_HELO_RESPONSE;
		smtp_100ms_timeout_timer = EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS;

		//Reset ready for the next line of response
		smtp_receive_message_string_len = 0;

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_SMTP_PROGRESS_STRING
			smtp_email_progress_string_pointer = smtp_email_progress_string_wait_smtp_helo;
			smtp_email_progress_string_update = 1;
		#endif

		break;


	case SM_SMTP_WAIT_FOR_SMTP_HELO_RESPONSE:
		//----------------------------------------
		//----------------------------------------
		//----- WAITING FOR RESPONSE TO HELO -----
		//----------------------------------------
		//----------------------------------------

		//DO WE NEED TO RE-SEND THE LAST PACKET?
		//(TCP requires resending of packets if they are not acknowledged and to avoid requiring a large RAM buffer the application needs to remember the last packet sent on a socket
		//so it can be resent if requried - If your application has ram available to store a copy of the last sent packet then the tcp driver could be modified to use it instead).
		if (tcp_does_socket_require_resend_of_last_packet(smtp_tcp_socket))
		{
			sm_smtp = SM_SMTP_SEND_SMTP_HELO;
			break;
		}

		//CHECK FOR PACKET RECEIVED
		if (tcp_check_socket_for_rx(smtp_tcp_socket))
		{
			//----- PACKET RECEIVED -----
			while (tcp_read_next_rx_byte(&received_byte))			//Get next byte, returns 0 if end of packet
			{
				//----- RECEIVE EACH LINE -----
				//(EHLO EXTENDED SMTP responses are multiline)
			    if (received_byte != '\r')							//Do until we get the CR character			
			    {
			        if (smtp_receive_message_string_len < SMTP_RECEIVE_MESSAGE_STRING_MAX_LEN - 1 && received_byte != '\n')		//Don't include the line feed character
						smtp_receive_message_string[smtp_receive_message_string_len++] = received_byte;
			    }
			    else
			    {
				    //----------------------------------------------------
				    //----- CR CHARACTER RECEIVED - PROCESS THE LINE -----
				    //----------------------------------------------------
					//receive_message_string[receive_message_string_len] = 0x00;			//Add null termination to the string
					//strlwr(&receive_message_string[0]);									//Convert string to lower case

					if (smtp_receive_message_string[0] == '2')				//A leading 2 character indicates the server is happy (3 used for data transfers, 4 & 5 = error)
					{
						//RESPONSE IS GOOD
						if (smtp_use_authenticated_login)
						{
							//----- USING EXTENDED SMTP - CHECK FOR FURTHER RESPONSE DUE -----
							if (smtp_receive_message_string[3] == '-')				//A trailing '-' instead of a space indicates that another response will follow
							{
								//GET ADDITIONAL RESPONSE(S)
								//(Either in this packet or in additional [ackets)
			
						        //Reset ready for the next line
						        smtp_receive_message_string_len = 0;
			
								continue;
							}
						}
				
						//----- IF NOT DOING AUTHENTICATED LOGIN THEN SETUP TO START SENDING EMAIL -----
						//----- IF DOING AUTHENTICATED LOGIN THEN SETUP TO DO LOGIN -----
						if (smtp_use_authenticated_login)
						{
							//DOING AUTHENTICATED LOGIN
							sm_smtp = SM_SMTP_SEND_AUTH_LOGIN_COMMAND;
						}
						else
						{
							//DOING NORMAL NON-AUTHENTICATED LOGIN
							sm_smtp = SM_SMTP_SEND_MAIL_FROM_COMMAND;
						}
						
					}
					else
					{
						//----- RESPONSE IS NOT GOOD - ERROR -----
						goto process_send_email_tcp_failed;
					}

			        //Reset ready for the next line
			        smtp_receive_message_string_len = 0;
			    }

			}
			//Packet processed - dump it
			tcp_dump_rx_packet();
	    }

		break;



	case SM_SMTP_SEND_AUTH_LOGIN_COMMAND:
		//------------------------------------------------------
		//------------------------------------------------------
		//----- SEND MAIL SMTP AUTHENTICATED LOGIN COMMAND -----
		//------------------------------------------------------
		//------------------------------------------------------
		//(We only support AUTH LOGIN - other AUTH methods are available and a server is not requried to support all types, however AUTH LOGIN is typical)
		if (!tcp_setup_socket_tx(smtp_tcp_socket))
		{
			//Can't send currently - try again next time
			break;
		}

		//Send MAIL FROM
		tcp_write_next_byte('A');
		tcp_write_next_byte('U');
		tcp_write_next_byte('T');
		tcp_write_next_byte('H');
		tcp_write_next_byte(' ');
		tcp_write_next_byte('L');
		tcp_write_next_byte('O');
		tcp_write_next_byte('G');
		tcp_write_next_byte('I');
		tcp_write_next_byte('N');
		tcp_write_next_byte('\r');		//CR
		tcp_write_next_byte('\n');		//LF

		tcp_socket_tx_packet(smtp_tcp_socket);				//Send the packet

		sm_smtp = SM_SMTP_WAIT_FOR_SMTP_AUTH_LOGIN_RESPONSE;
		smtp_100ms_timeout_timer = EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS;

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_SMTP_PROGRESS_STRING
			smtp_email_progress_string_pointer = smtp_email_progress_string_wait_smtp_auth_login;
			smtp_email_progress_string_update = 1;
		#endif

		break;


	case SM_SMTP_WAIT_FOR_SMTP_AUTH_LOGIN_RESPONSE:
		//------------------------------------------------------
		//------------------------------------------------------
		//----- WAITING FOR RESPONSE TO AUTH LOGIN COMMAND -----
		//------------------------------------------------------
		//------------------------------------------------------

		//DO WE NEED TO RE-SEND THE LAST PACKET?
		if (tcp_does_socket_require_resend_of_last_packet(smtp_tcp_socket))
		{
			sm_smtp = SM_SMTP_SEND_AUTH_LOGIN_COMMAND;
			break;
		}

		//CHECK FOR PACKET RECEIVED
		if (tcp_check_socket_for_rx(smtp_tcp_socket))
		{
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte == '3')				//A leading 3 character indicates the server is in data transfer mode, 4 & 5 = error)
			{
				//RESPONSE IS GOOD - SEND THE LOGIN USERNAME
				tcp_dump_rx_packet();				//Dump the remainder of the recevied packet

				sm_smtp = SM_SMTP_SEND_AUTH_USERNAME;
			}
			else
			{
				//RESPONSE IS NOT GOOD - ERROR
				goto process_send_email_tcp_failed;
			}
	    }

		break;


	case SM_SMTP_SEND_AUTH_USERNAME:
		//------------------------------
		//------------------------------
		//----- SEND AUTH USERNAME -----
		//------------------------------
		//------------------------------
		if (!tcp_setup_socket_tx(smtp_tcp_socket))
		{
			//Can't send currently - try again next time
			break;
		}

		//Get the username
		email_return_smtp_username(&temp_string[0], sizeof(temp_string));

		//Write the string
		b_temp = 0;
		while (b_temp < 32)
		{
			//Write the next 3 bytes as Base64 encoded
			if (temp_string[b_temp] != 0)
			{
				base64_source[0] = temp_string[b_temp++];
				b_temp1 = 1;
			}
			else
			{
				break;			//All characters done
			}

			if (temp_string[b_temp] != 0)
			{
				base64_source[1] = temp_string[b_temp++];
				b_temp1 = 2;
			}

			if (temp_string[b_temp] != 0)
			{
				base64_source[2] = temp_string[b_temp++];
				b_temp1 = 3;
			}

			//Convert to 4 bytes of base 64
			email_convert_3_bytes_to_base64(&base64_source[0], &base64_converted[0], b_temp1);

			tcp_write_next_byte(base64_converted[0]);
			tcp_write_next_byte(base64_converted[1]);
			tcp_write_next_byte(base64_converted[2]);
			tcp_write_next_byte(base64_converted[3]);
			if (b_temp1 < 3)
				break;			//All characters done
		}

		tcp_write_next_byte('\r');		//CR
		tcp_write_next_byte('\n');		//LF

		tcp_socket_tx_packet(smtp_tcp_socket);				//Send the packet

		sm_smtp = SM_SMTP_WAIT_FOR_SMTP_AUTH_USERNAME_RESPONSE;
		smtp_100ms_timeout_timer = EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS;

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_SMTP_PROGRESS_STRING
			smtp_email_progress_string_pointer = smtp_email_progress_string_wait_smtp_username;
			smtp_email_progress_string_update = 1;
		#endif

		break;


	case SM_SMTP_WAIT_FOR_SMTP_AUTH_USERNAME_RESPONSE:
		//---------------------------------------------------------
		//---------------------------------------------------------
		//----- WAITING FOR RESPONSE TO AUTH USERNAME COMMAND -----
		//---------------------------------------------------------
		//---------------------------------------------------------

		//DO WE NEED TO RE-SEND THE LAST PACKET?
		if (tcp_does_socket_require_resend_of_last_packet(smtp_tcp_socket))
		{
			sm_smtp = SM_SMTP_SEND_AUTH_USERNAME;
			break;
		}

		//CHECK FOR PACKET RECEIVED
		if (tcp_check_socket_for_rx(smtp_tcp_socket))
		{
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte == '3')				//A leading 3 character indicates the server is in data transfer mode, 4 & 5 = error)
			{
				//RESPONSE IS GOOD - SEND THE LOGIN PASSWORD
				tcp_dump_rx_packet();				//Dump the remainder of the recevied packet

				sm_smtp = SM_SMTP_SEND_AUTH_PASSWORD;
			}
			else
			{
				//RESPONSE IS NOT GOOD - ERROR
				goto process_send_email_tcp_failed;
			}
	    }

		break;


	case SM_SMTP_SEND_AUTH_PASSWORD:
		//------------------------------
		//------------------------------
		//----- SEND AUTH PASSWORD -----
		//------------------------------
		//------------------------------
		if (!tcp_setup_socket_tx(smtp_tcp_socket))
		{
			//Can't send currently - try again next time
			break;
		}

		//Get the password
		email_return_smtp_password(&temp_string[0], sizeof(temp_string));
		
		//Write the string
		b_temp = 0;
		while (b_temp < 32)
		{
			//Write the next 3 bytes as Base64 encoded
			if (temp_string[b_temp] != 0)
			{
				base64_source[0] = temp_string[b_temp++];
				b_temp1 = 1;
			}
			else
			{
				break;			//All characters done
			}
			
			if (temp_string[b_temp] != 0)
			{
				base64_source[1] = temp_string[b_temp++];
				b_temp1 = 2;
			}
			
			if (temp_string[b_temp] != 0)
			{
				base64_source[2] = temp_string[b_temp++];
				b_temp1 = 3;
			}

			//Convert to 4 bytes of base 64
			email_convert_3_bytes_to_base64(&base64_source[0], &base64_converted[0], b_temp1);

			tcp_write_next_byte(base64_converted[0]);
			tcp_write_next_byte(base64_converted[1]);
			tcp_write_next_byte(base64_converted[2]);
			tcp_write_next_byte(base64_converted[3]);
			if (b_temp1 < 3)
				break;			//All characters done
		}

		tcp_write_next_byte('\r');		//CR
		tcp_write_next_byte('\n');		//LF

		tcp_socket_tx_packet(smtp_tcp_socket);				//Send the packet

		sm_smtp = SM_SMTP_WAIT_FOR_SMTP_AUTH_PASSWORD_RESPONSE;
		smtp_100ms_timeout_timer = EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS;

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_SMTP_PROGRESS_STRING
			smtp_email_progress_string_pointer = smtp_email_progress_string_wait_smtp_password;
			smtp_email_progress_string_update = 1;
		#endif

		break;


	case SM_SMTP_WAIT_FOR_SMTP_AUTH_PASSWORD_RESPONSE:
		//---------------------------------------------------------
		//---------------------------------------------------------
		//----- WAITING FOR RESPONSE TO AUTH PASSWORD COMMAND -----
		//---------------------------------------------------------
		//---------------------------------------------------------

		//DO WE NEED TO RE-SEND THE LAST PACKET?
		if (tcp_does_socket_require_resend_of_last_packet(smtp_tcp_socket))
		{
			sm_smtp = SM_SMTP_SEND_AUTH_PASSWORD;
			break;
		}

		//CHECK FOR PACKET RECEIVED
		if (tcp_check_socket_for_rx(smtp_tcp_socket))
		{
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte == '2')				//A leading 2 character indicates the server is happy (3 used for data transfers, 4 & 5 = error)
			{
				//RESPONSE IS GOOD
				tcp_dump_rx_packet();				//Dump the remainder of the recevied packet
	
				//Login complete - now send email as normal
				sm_smtp = SM_SMTP_SEND_MAIL_FROM_COMMAND;
			}
			else
			{
				//RESPONSE IS NOT GOOD - ERROR
				goto process_send_email_tcp_failed;
			}
	    }

		break;


	case SM_SMTP_SEND_MAIL_FROM_COMMAND:
		//----------------------------------
		//----------------------------------
		//----- SEND MAIL FROM COMMAND -----
		//----------------------------------
		//----------------------------------
		if (!tcp_setup_socket_tx(smtp_tcp_socket))
		{
			//Can't send currently - try again next time
			break;
		}

		//Send MAIL FROM
		tcp_write_next_byte('M');
		tcp_write_next_byte('A');
		tcp_write_next_byte('I');
		tcp_write_next_byte('L');
		tcp_write_next_byte(' ');
		tcp_write_next_byte('F');
		tcp_write_next_byte('R');
		tcp_write_next_byte('O');
		tcp_write_next_byte('M');
		tcp_write_next_byte(':');
		tcp_write_next_byte(' ');
		tcp_write_next_byte('<');

		//Get the from address
		email_return_smtp_sender(&temp_string[0], sizeof(temp_string));

		for (loop_count = 0; temp_string[loop_count] != 0x00; loop_count++)
		{
			tcp_write_next_byte(temp_string[loop_count]);	
		}

		tcp_write_next_byte('>');
		tcp_write_next_byte('\r');		//CR
		tcp_write_next_byte('\n');		//LF

		tcp_socket_tx_packet(smtp_tcp_socket);				//Send the packet

		sm_smtp = SM_SMTP_WAIT_FOR_SMTP_MAIL_FROM_RESPONSE;
		smtp_100ms_timeout_timer = EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS;

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_SMTP_PROGRESS_STRING
			smtp_email_progress_string_pointer = smtp_email_progress_string_wait_smtp_from;
			smtp_email_progress_string_update = 1;
		#endif

		break;


	case SM_SMTP_WAIT_FOR_SMTP_MAIL_FROM_RESPONSE:
		//---------------------------------------------
		//---------------------------------------------
		//----- WAITING FOR RESPONSE TO MAIL FROM -----
		//---------------------------------------------
		//---------------------------------------------

		//DO WE NEED TO RE-SEND THE LAST PACKET?
		if (tcp_does_socket_require_resend_of_last_packet(smtp_tcp_socket))
		{
			sm_smtp = SM_SMTP_SEND_MAIL_FROM_COMMAND;
			break;
		}

		//CHECK FOR PACKET RECEIVED
		if (tcp_check_socket_for_rx(smtp_tcp_socket))
		{
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte == '2')				//A leading 2 character indicates the server is happy (3 used for data transfers, 4 & 5 = error)
			{
				//RESPONSE IS GOOD - SEND THE RCPT TO FIELD
				tcp_dump_rx_packet();				//Dump the remainder of the recevied packet

				sm_smtp = SM_SMTP_SEND_RCPT_TO;
			}
			else
			{
				//RESPONSE IS NOT GOOD - ERROR
				goto process_send_email_tcp_failed;
			}
	    }

		break;


	case SM_SMTP_SEND_RCPT_TO:
		//--------------------------------
		//--------------------------------
		//----- SEND MAIL TO COMMAND -----
		//--------------------------------
		//--------------------------------
		if (!tcp_setup_socket_tx(smtp_tcp_socket))
		{
			//Can't send currently - try again next time
			break;
		}
		
		//Send MAIL FROM
		tcp_write_next_byte('R');
		tcp_write_next_byte('C');
		tcp_write_next_byte('P');
		tcp_write_next_byte('T');
		tcp_write_next_byte(' ');
		tcp_write_next_byte('T');
		tcp_write_next_byte('O');
		tcp_write_next_byte(':');
		tcp_write_next_byte(' ');
		tcp_write_next_byte('<');
		

		//Get the to address
		email_return_smtp_to(&temp_string[0], sizeof(temp_string));

		for (loop_count = 0; temp_string[loop_count] != 0x00; loop_count++)
		{
			tcp_write_next_byte(temp_string[loop_count]);	
		}

		tcp_write_next_byte('>');
		tcp_write_next_byte('\r');		//CR
		tcp_write_next_byte('\n');		//LF

		tcp_socket_tx_packet(smtp_tcp_socket);				//Send the packet

		sm_smtp = SM_SMTP_WAIT_FOR_SMTP_RCPT_TO_RESPONSE;
		smtp_100ms_timeout_timer = EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS;

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_SMTP_PROGRESS_STRING
			smtp_email_progress_string_pointer = smtp_email_progress_string_wait_smtp_to;
			smtp_email_progress_string_update = 1;
		#endif

		break;


	case SM_SMTP_WAIT_FOR_SMTP_RCPT_TO_RESPONSE:
		//--------------------------------------------
		//--------------------------------------------
		//----- WAITING FOR RESPONSE TO RCPT TO  -----
		//--------------------------------------------
		//--------------------------------------------
		//(Once the response is recevied we could continue sending RCPT TO fields for additional recipients if we wanted to)

		//DO WE NEED TO RE-SEND THE LAST PACKET?
		if (tcp_does_socket_require_resend_of_last_packet(smtp_tcp_socket))
		{
			sm_smtp = SM_SMTP_SEND_RCPT_TO;
			break;
		}

		//CHECK FOR PACKET RECEIVED
		if (tcp_check_socket_for_rx(smtp_tcp_socket))
		{
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte == '2')				//A leading 2 character indicates the server is happy (3 used for data transfers, 4 & 5 = error)
			{
				//RESPONSE IS GOOD - SEND DATA COMMAND TO START THE EMAIL TRANSFER
				tcp_dump_rx_packet();				//Dump the remainder of the recevied packet

				sm_smtp = SM_SMTP_SEND_DATA_COMMAND;
			}
			else
			{
				//RESPONSE IS NOT GOOD - ERROR
				goto process_send_email_tcp_failed;
			}
	    }

		break;


	case SM_SMTP_SEND_DATA_COMMAND:
		//-----------------------------
		//-----------------------------
		//----- SEND DATA COMMAND -----
		//-----------------------------
		//-----------------------------
		if (!tcp_setup_socket_tx(smtp_tcp_socket))
		{
			//Can't send currently - try again next time
			break;
		}

		//Send the command
		tcp_write_next_byte('D');
		tcp_write_next_byte('A');
		tcp_write_next_byte('T');
		tcp_write_next_byte('A');

		tcp_write_next_byte('\r');		//CR
		tcp_write_next_byte('\n');		//LF

		tcp_socket_tx_packet(smtp_tcp_socket);				//Send the packet

		sm_smtp = SM_SMTP_WAIT_FOR_SMTP_DATA_RESPONSE;
		smtp_100ms_timeout_timer = EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS;

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_SMTP_PROGRESS_STRING
			smtp_email_progress_string_pointer = smtp_email_progress_string_wait_smtp_data;
			smtp_email_progress_string_update = 1;
		#endif

		break;


	case SM_SMTP_WAIT_FOR_SMTP_DATA_RESPONSE:
		//------------------------------------------------
		//------------------------------------------------
		//----- WAITING FOR RESPONSE TO DATA REQUEST -----
		//------------------------------------------------
		//------------------------------------------------

		//DO WE NEED TO RE-SEND THE LAST PACKET?
		if (tcp_does_socket_require_resend_of_last_packet(smtp_tcp_socket))
		{
			sm_smtp = SM_SMTP_SEND_DATA_COMMAND;
			break;
		}

		//CHECK FOR PACKET RECEIVED
		if (tcp_check_socket_for_rx(smtp_tcp_socket))
		{
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte == '3')				//A leading 3 character indicates the server is in data transfer mode, 4 & 5 = error)
			{
				//RESPONSE IS GOOD
				tcp_dump_rx_packet();				//Dump the remainder of the received packet

				sm_smtp = SM_SMTP_SEND_EMAIL_HEADER;
			}
			else
			{
				//RESPONSE IS NOT GOOD - ERROR
				goto process_send_email_tcp_failed;
			}
	    }
	    break;


	case SM_SMTP_SEND_EMAIL_HEADER:
		//---------------------------------
		//---------------------------------
		//----- SEND THE EMAIL HEADER -----
		//---------------------------------
		//---------------------------------
		if (!tcp_setup_socket_tx(smtp_tcp_socket))
		{
			//Can't send currently - try again next time
			break;
		}
			
		//SEND THE 'FROM' FIELD ON ITS OWN LINE
		tcp_write_next_byte('F');
		tcp_write_next_byte('R');
		tcp_write_next_byte('O');
		tcp_write_next_byte('M');
		tcp_write_next_byte(':');
		tcp_write_next_byte(' ');
		tcp_write_next_byte('<');

		//Get the from address
		email_return_smtp_sender(&temp_string[0], sizeof(temp_string));

		for (loop_count = 0; temp_string[loop_count] != 0x00; loop_count++)
		{
			tcp_write_next_byte(temp_string[loop_count]);	
		}
		tcp_write_next_byte('>');
		tcp_write_next_byte('\r');		//CR
		tcp_write_next_byte('\n');		//LF

		//SEND THE 'TO' FIELD ON ITS OWN LINE
		tcp_write_next_byte('T');
		tcp_write_next_byte('O');
		tcp_write_next_byte(':');
		tcp_write_next_byte(' ');
		tcp_write_next_byte('<');
		
		//Get to address
		email_return_smtp_to(&temp_string[0], sizeof(temp_string));

		for (loop_count = 0; temp_string[loop_count] != 0x00; loop_count++)
		{
			tcp_write_next_byte(temp_string[loop_count]);	
		}
		
		tcp_write_next_byte('>');
		tcp_write_next_byte('\r');		//CR
		tcp_write_next_byte('\n');		//LF

		//SEND THE 'SUBJECT' FIELD ON ITS OWN LINE
		tcp_write_next_byte('S');
		tcp_write_next_byte('u');
		tcp_write_next_byte('b');
		tcp_write_next_byte('j');
		tcp_write_next_byte('e');
		tcp_write_next_byte('c');
		tcp_write_next_byte('t');
		tcp_write_next_byte(':');
		tcp_write_next_byte(' ');

		//Get the subject
		email_return_smtp_subject(&temp_string[0], sizeof(temp_string));

		for (loop_count = 0; temp_string[loop_count] != 0x00; loop_count++)
		{
			tcp_write_next_byte(temp_string[loop_count]);	
		}

		tcp_write_next_byte('\r');		//CR
		tcp_write_next_byte('\n');		//LF

		tcp_socket_tx_packet(smtp_tcp_socket);				//Send the packet

		sm_smtp = SM_SMTP_WAIT_EMAIL_HEADER_ACK;
		smtp_100ms_timeout_timer = EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS;

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_SMTP_PROGRESS_STRING
			smtp_email_progress_string_pointer = smtp_email_progress_string_sending_smtp_data;
			smtp_email_progress_string_update = 1;
		#endif

		break;


	case SM_SMTP_WAIT_EMAIL_HEADER_ACK:
		//--------------------------------------------------------
		//--------------------------------------------------------
		//----- WAIT FOR TCP ACK FOR THE EMAIL HEADER PACKET -----
		//--------------------------------------------------------
		//--------------------------------------------------------

		//DO WE NEED TO RE-SEND THE LAST PACKET?
		if (tcp_does_socket_require_resend_of_last_packet(smtp_tcp_socket))
		{
			sm_smtp = SM_SMTP_SEND_EMAIL_HEADER;
			break;
		}

		//CHECK FOR TCP ACK RETURNED BY SERVER
		if (tcp_is_socket_ready_to_tx_new_packet(smtp_tcp_socket))
		{
			sm_smtp = SM_SMTP_SEND_SMTP_EMAIL_BODY;
		}
		break;


	case SM_SMTP_SEND_SMTP_EMAIL_BODY:
		//-------------------------------
		//-------------------------------
		//----- SEND THE EMAIL BODY -----
		//-------------------------------
		//-------------------------------
		//(This state may be called more than once depending on the length of the email)

		//DO WE NEED TO RE-SEND THE LAST PACKET?
		//(TCP requires resending of packets if they are not acknowledged and to avoid requiring a large RAM buffer the application needs to remember the last packet sent on a socket
		//so it can be resent if requried - If your application has ram available to store a copy of the last sent packet then the tcp driver could be modified to use it instead).
		resend_flag = 0x01;					//Default to this is first byte of a new packet - functions should copy working registers in case a resend is needed
		if (tcp_does_socket_require_resend_of_last_packet(smtp_tcp_socket))
		{
			resend_flag = 0x02;				//Flag that functions should restore the copies of working registers as this packet is going to be a re-send
		}

		//----------------------------------------
		//----------------------------------------
		//----- GET NEXT BLOCK OF EMAIL DATA -----
		//----------------------------------------
		//----------------------------------------
		if (!tcp_setup_socket_tx(smtp_tcp_socket))
		{
			//Can't send currently - try again next time
			break;
		}
		
		for (count = 0; count < 1400; count++)			//Max 1500 bytes in an IP datagram.  Min IP header is 20 bytes, min TCP header is 20 bytes.  Limit to 1400 bytes to provide some slack
		{
			//CALL SEPERATE FUNCTION TO GET THE NEXT DATA BYTE TO SEND
			b_temp = send_email_get_next_byte(&received_byte, resend_flag);
			if (b_temp)
			{
				//Send next byte
				tcp_write_next_byte(received_byte);
				
				if (b_temp == 2)
					break;								//Moving from email body to file attachment - start a new packet to make life simple if we have to do a resend
			}
			else
			{	
				//NO MORE BYTES TO SEND
				//End of email marker
				tcp_write_next_byte('\r');		//CR
				tcp_write_next_byte('\n');		//LF
				tcp_write_next_byte('.');
				tcp_write_next_byte('\r');		//CR
				tcp_write_next_byte('\n');		//LF
	
				smtp_100ms_timeout_timer = EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS;
				sm_smtp = SM_SMTP_WAIT_FOR_SMTP_EMAIL_BODY_REPONSE;

				//UPDATE USER DISPLAYED STRING IF IN USE
				#ifdef DO_SMTP_PROGRESS_STRING
					smtp_email_progress_string_pointer = smtp_email_progress_string_wait_smtp_data_resp;
					smtp_email_progress_string_update = 1;
				#endif

				break;
			}
			resend_flag = 0;
		}

		tcp_socket_tx_packet(smtp_tcp_socket);				//Send the packet

		if (sm_smtp != SM_SMTP_WAIT_FOR_SMTP_EMAIL_BODY_REPONSE)
		{
			smtp_100ms_timeout_timer = EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS;
			sm_smtp = SM_SMTP_SEND_SMTP_EMAIL_BODY;					//Repeate this state until all of email body is sent (there is no SMTP server acknowledgement until the email body is complete)
		}

		//There is not response in this part of the transfer - just keep sending the data until we're done

		break;


	case SM_SMTP_WAIT_FOR_SMTP_EMAIL_BODY_REPONSE:
		//----------------------------------------------
		//----------------------------------------------
		//----- WAITING FOR RESPONSE TO EMAIL DATA -----
		//----------------------------------------------
		//----------------------------------------------
		//(A response from the server is only send once all of the email has been sent and the special <CR><LF>.<CR><LF> end of message has been sent)
		//Check for packet received
		if (tcp_check_socket_for_rx(smtp_tcp_socket))
		{
			tcp_read_next_rx_byte(&received_byte);
			if (received_byte == '2')				//A leading 2 character indicates the server is is happy (3 used for data transfers, 4 & 5 = error)
			{
				tcp_dump_rx_packet();				//Dump the remainder of the received packet
				sm_smtp = SM_SMTP_DO_QUIT;

				//UPDATE USER DISPLAYED STRING IF IN USE
				#ifdef DO_SMTP_PROGRESS_STRING
					smtp_email_progress_string_pointer = smtp_email_progress_string_wait_smtp_data_accepted;
					smtp_email_progress_string_update = 1;
				#endif

				break;
			}
			else
			{
				//RESPONSE IS NOT GOOD - ERROR
				goto process_send_email_tcp_failed;
			}
		}
		break;


	case SM_SMTP_DO_QUIT:
		//------------------------------------------
		//------------------------------------------
		//----- QUIT CONNECTION TO SMTP SERVER -----
		//------------------------------------------
		//------------------------------------------
		if (!tcp_setup_socket_tx(smtp_tcp_socket))
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

		tcp_socket_tx_packet(smtp_tcp_socket);				//Send the packet

		sm_smtp = SM_SMTP_WAIT_FOR_SMTP_QUIT_RESPONSE;
		smtp_100ms_timeout_timer = EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS;

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_SMTP_PROGRESS_STRING
			smtp_email_progress_string_pointer = smtp_email_progress_string_wait_smtp_quit;
			smtp_email_progress_string_update = 1;
		#endif

		break;


	case SM_SMTP_WAIT_FOR_SMTP_QUIT_RESPONSE:
		//----------------------------------
		//----------------------------------
		//----- WAIT FOR QUIT RESPONSE -----
		//----------------------------------
		//----------------------------------

		//DO WE NEED TO RE-SEND THE LAST PACKET?
		if (tcp_does_socket_require_resend_of_last_packet(smtp_tcp_socket))
		{
			sm_smtp = SM_SMTP_DO_QUIT;
			break;
		}

		//CHECK FOR PACKET RECEIVED
		if (tcp_check_socket_for_rx(smtp_tcp_socket))
		{
			//----- ALL DONE - DUMP THE TCP CONNECTION AND RETURN TO IDLE MODE -----
			tcp_request_disconnect_socket(smtp_tcp_socket);
			sm_smtp = SM_SMTP_IDLE;

			//UPDATE USER DISPLAYED STRING IF IN USE
			#ifdef DO_SMTP_PROGRESS_STRING
				smtp_email_progress_string_pointer = smtp_email_progress_string_complete;
				smtp_email_progress_string_update = 1;
			#endif
		}

		break;


	case SM_SMTP_FAILED:
		//-------------------------------------
		//-------------------------------------
		//----- SEND EMAIL PROCESS FAILED -----
		//-------------------------------------
		//-------------------------------------

		//UPDATE USER DISPLAYED STRING IF IN USE
		#ifdef DO_SMTP_PROGRESS_STRING
			smtp_email_progress_string_pointer = smtp_email_progress_string_failed;
			smtp_email_progress_string_update = 1;
		#endif
		
		sm_smtp = SM_SMTP_IDLE;

		break;
		
	}

	return;

process_send_email_tcp_failed:
//-----------------------------------------------------------------------------------
//----- FAILED DURING TCP TRANSFER - CLOSE CONNECTION AND CHANGE STATE TO ERROR -----
//-----------------------------------------------------------------------------------

	if ((sm_smtp != SM_SMTP_SEND_EMAIL) && (sm_smtp != SM_SMTP_WAIT_FOR_ARP_RESPONSE) && (sm_smtp != SM_SMTP_WAITING_DNS_RESPONSE))
	{
		tcp_dump_rx_packet();							//Dump the remainder of any received packet
		tcp_request_disconnect_socket(smtp_tcp_socket);	//Close connection
	}

	sm_smtp = SM_SMTP_FAILED;

	return;

}






//****************************************************************
//****************************************************************
//********** SEND EMAIL PROVIDE NEXT BYTE OF EMAIL BODY **********
//****************************************************************
//****************************************************************
//Once the connection has been establised to the smtp server and the email header sent this function will be called for each
//byte to be included in the email body.
//The first call will be the start of the line following the 'subject' field of the header.
//Additional header lines may be added if requried (i.e. MIME header if attaching a file to the email)
//To start sending the email body you must first send a blank line to indicate the end of the header and the beginning of the
//email body

//Returns:
//	2 = there are more bytes to send but start a new packet (we ensure a new packet is started before sending a file attachment for simplicity)
//	1 = there are more bytes to send
//	0 = the last byte has already been sent and there is no more data.
//The calling function will add the <CR><LF>.<CR><LF> end of email marker
//
//resend_flags:
//	0x00	Get next byte as normal.
//	0x01	The byte being requested is the first byte of a new TCP packet.  The previous packet was sucessfully sent.
//	0x02 	The byte being requested is the first byte of a re-send of the last packet.
//			This is requried as TCP requires resending of packets if they are not acknowledged and to avoid requiring a large RAM buffer the
//			application needs to remember the last packet sent on a socket so it can be re-sent if requried.
//			(If your application has ram available to store a copy of the last sent packet then the tcp driver could be modified to use it instead).

BYTE send_email_get_next_byte(BYTE *data, BYTE resend_flags)
{


	static BYTE mime_base64_byte[4];
	static BYTE mime_line_character_count;
	static BYTE sm_send_email_data_copy;
	static WORD send_email_body_byte_number_copy;
	static BYTE mime_line_character_count_copy;
	static BYTE mime_base64_byte_copy[4];
	static WORD resend_possible_move_back_bytes;
	static WORD resend_move_back_bytes;
	static BYTE file_attachment_last_byte_sent;
	static BYTE get_data_started_new_tcp_packet;
	BYTE b_temp;
	BYTE bytes_to_encode[3];
	BYTE bytes_to_encode_len;

	
	//TO SEND A BASIC ASCII EMAIL:-
	//Send a blank line "\r\n"
	//Send the email text, using "\r\n" on the end of each line, with a maximum of 1000 charcters permitted per line (RFC821)
	//Finally return with 0x00 so that the SMTP function adds the end of email marker and finishes sending the email

	//TO SEND A MIME EMAIL WITH AN ATTACHMENT:-
	//Send mime_header_for_text_string
	//Send email_body_message_string or any ascii text to be displayed in the email body
	//Send mime_body_for_file_string
	//Send the file encoded in base64 format
	//Send mime_body_end_of_mime_string
	//Finally return with 0x00 so that the SMTP function adds the end of email marker and finishes sending the email


	if (resend_flags == 0x01)
	{
		//-------------------------------------------------------------------------------
		//----- COPY ALL WORKING REGISTERS AS THIS IS THE START OF A NEW TCP PACKET -----
		//-------------------------------------------------------------------------------
		sm_send_email_data_copy = sm_send_email_data;
		send_email_body_byte_number_copy = send_email_body_byte_number;
		mime_line_character_count_copy = mime_line_character_count;
		mime_base64_byte_copy[0] = mime_base64_byte[0];
		mime_base64_byte_copy[1] = mime_base64_byte[1];
		mime_base64_byte_copy[2] = mime_base64_byte[2];
		mime_base64_byte_copy[3] = mime_base64_byte[3];
		resend_possible_move_back_bytes = 0;
		resend_move_back_bytes = 0;
		get_data_started_new_tcp_packet = 1;

	}
	else if (resend_flags == 0x02)
	{
		//-------------------------------------------------------------------------------------
		//----- RESTORE ALL WORKING REGISTERS AS THIS IS A RE-SEND OF THE LAST TCP PACKET -----
		//-------------------------------------------------------------------------------------
		sm_send_email_data = sm_send_email_data_copy;
		send_email_body_byte_number = send_email_body_byte_number_copy;
		mime_line_character_count = mime_line_character_count_copy;
		mime_base64_byte[0] = mime_base64_byte_copy[0];
		mime_base64_byte[1] = mime_base64_byte_copy[1];
		mime_base64_byte[2] = mime_base64_byte_copy[2];
		mime_base64_byte[3] = mime_base64_byte_copy[3];
		resend_move_back_bytes = resend_possible_move_back_bytes;
		resend_possible_move_back_bytes = 0;
		get_data_started_new_tcp_packet = 0;
		file_attachment_last_byte_sent = 0;
	}


send_email_get_next_byte_redo:
	send_email_body_byte_number++;

	switch(sm_send_email_data)
	{
	case SM_SEND_EMAIL_DATA_MIME_HEADER:
		//------------------------------------------------------------------------------
		//----- THE MIME HEADER USED JUST FOLLOWING THE STANDARD EMAIL BODY HEADER -----
		//------------------------------------------------------------------------------
		*data = mime_header_for_text_string[(WORD)send_email_body_byte_number];

		if (mime_header_for_text_string[(WORD)send_email_body_byte_number + 1] == 0x00)
		{
			send_email_body_byte_number = 0xffff;
			sm_send_email_data = SM_SEND_EMAIL_BODY_ASCII;

			resend_possible_move_back_bytes = 0;
		}
		return(1);

	case SM_SEND_EMAIL_BODY_ASCII:
		//--------------------------------------------------------
		//----- ASCII TEXT TO BE DISPLAYED IN THE EMAIL BODY -----
		//--------------------------------------------------------

		//----- GET NEXT BYTE FROM USER APLICATION -----
		b_temp = SMTP_GET_NEXT_DATA_BYTE_FUNCTION(1, get_data_started_new_tcp_packet, resend_move_back_bytes, data);
		resend_move_back_bytes = 0;
		get_data_started_new_tcp_packet = 0;
		if (b_temp)
		{
			//----- WE HAVE THE NEXT BYTE TO SEND -----
			resend_possible_move_back_bytes++;
			return(1);
		}
		else
		{
			//----- NO MORE BYTES TO SEND -----
			send_email_body_byte_number = 0xffff;

			#ifdef EMAIL_ATTACHMENT_FILENAME
				if (smtp_include_file_attachment)
					sm_send_email_data = SM_SEND_EMAIL_BODY_MIME_FILE_HEADER_1;		//User flagged that a file would be included when send email was origianlly called
				else
					sm_send_email_data = SM_SEND_EMAIL_BODY_MIME_END_OF_BODY;		//Not using send an attachment
			#else
				sm_send_email_data = SM_SEND_EMAIL_BODY_MIME_END_OF_BODY;			//Not using send attachment functionality
			#endif
			
			goto send_email_get_next_byte_redo;
		}


#ifdef EMAIL_ATTACHMENT_FILENAME			//Don't include file attachment code if this functionality is not being used
	case SM_SEND_EMAIL_BODY_MIME_FILE_HEADER_1:
		//--------------------------------------------------------
		//----- START OF MIME HEADER FOR THE FILE ATTACHMENT -----
		//--------------------------------------------------------
		*data = mime_body_file_header_string_1[(WORD)send_email_body_byte_number];

		if (mime_body_file_header_string_1[(WORD)send_email_body_byte_number + 1] == 0x00)
		{
			send_email_body_byte_number = 0xffff;
			mime_line_character_count = 0xff;
			sm_send_email_data = SM_SEND_EMAIL_BODY_MIME_FILE_HEADER_2;
		}
		return(1);


	case SM_SEND_EMAIL_BODY_MIME_FILE_HEADER_2:
		//-----------------------------------------------------------
		//----- FILENAME OF MIME HEADER FOR THE FILE ATTACHMENT -----
		//-----------------------------------------------------------
		//(This includes the file name)
		*data = EMAIL_ATTACHMENT_FILENAME[(WORD)send_email_body_byte_number];

		if (EMAIL_ATTACHMENT_FILENAME[(WORD)send_email_body_byte_number + 1] == 0x00)
		{
			send_email_body_byte_number = 0xffff;
			mime_line_character_count = 0xff;
			sm_send_email_data = SM_SEND_EMAIL_BODY_MIME_FILE_HEADER_3;
		}
		return(1);


	case SM_SEND_EMAIL_BODY_MIME_FILE_HEADER_3:
		//------------------------------------------------------
		//----- END OF MIME HEADER FOR THE FILE ATTACHMENT -----
		//------------------------------------------------------
		*data = mime_body_file_header_string_2[(WORD)send_email_body_byte_number];

		if (mime_body_file_header_string_2[(WORD)send_email_body_byte_number + 1] == 0x00)
		{
			send_email_body_byte_number = 0xffff;
			mime_line_character_count = 0xff;
			sm_send_email_data = SM_SEND_EMAIL_BODY_MIME_FILE_DATA;

			resend_possible_move_back_bytes = 0;
			file_attachment_last_byte_sent = 0;
			return(2);
		}
		return(1);


	case SM_SEND_EMAIL_BODY_MIME_FILE_DATA:
		//-----------------------------------
		//----- THE FILE DATA IN BASE64 -----
		//-----------------------------------

		//MAXIMUM PERMITTED LINE LENGTH IS 76 CHARACTERS
		mime_line_character_count++;
		if (mime_line_character_count == 74)
		{
			send_email_body_byte_number--;
			*data = '\r';
			return (1);
		}
		else if (mime_line_character_count == 75)
		{
			send_email_body_byte_number--;
			mime_line_character_count = 0xff;
			*data = '\n';
			return (1);
		}

		
		if ((send_email_body_byte_number & 0x0003) == 0)
		{
			//----- GET NEW BLOCK OF 3 BYTES READY TO BE ENCODED -----
			bytes_to_encode_len = 0;
			while (bytes_to_encode_len < 3)
			{
				if (file_attachment_last_byte_sent)
					break;

				//----- GET NEXT FILE BYTE FROM USER APLICATION -----
				b_temp = SMTP_GET_NEXT_DATA_BYTE_FUNCTION(0, get_data_started_new_tcp_packet, resend_move_back_bytes, &bytes_to_encode[bytes_to_encode_len]);
				resend_move_back_bytes = 0;
				get_data_started_new_tcp_packet = 0;
				if (b_temp)
				{
					//------ WE HAVE NEXT BYTE TO SEND -----
					resend_possible_move_back_bytes++;
					bytes_to_encode_len++;
				}
				else
				{
					//----- THERE ARE NO MORE BYTES TO SEND -----
					file_attachment_last_byte_sent = 1;
				}
			}

			if (file_attachment_last_byte_sent && (bytes_to_encode_len == 0))
			{
				//----- NO MORE FILE BYTES TO SEND -----
				send_email_body_byte_number = 0xffff;
				sm_send_email_data = SM_SEND_EMAIL_BODY_MIME_END_OF_BODY;
				goto send_email_get_next_byte_redo;
			}

			//----- CONVERT TO 4 BYTES OF BASE 64 -----
			email_convert_3_bytes_to_base64(&bytes_to_encode[0], &mime_base64_byte[0], bytes_to_encode_len);
		}


		//----- RETURN WITH THE NEXT ENCODED BYTE -----
		if ((send_email_body_byte_number & 0x0003) == 0)
		{
			*data = mime_base64_byte[0];
			return(1);
		}
		else if ((send_email_body_byte_number & 0x0003) == 1)
		{
			*data = mime_base64_byte[1];
			return(1);
		}
		else if ((send_email_body_byte_number & 0x0003) == 2)
		{
			*data = mime_base64_byte[2];
			return(1);
		}
		else
		{
			*data = mime_base64_byte[3];
			return(1);
		}
#endif //#ifdef EMAIL_ATTACHMENT_FILENAME

	case SM_SEND_EMAIL_BODY_MIME_END_OF_BODY:
		//------------------------------
		//----- END OF MIME MARKER -----
		//------------------------------
		*data = mime_body_end_of_mime_string[send_email_body_byte_number];

		if (mime_body_end_of_mime_string[send_email_body_byte_number + 1]  == 0x00)
		{
			send_email_body_byte_number = 0xffff;
			sm_send_email_data = SM_SEND_EMAIL_BODY_DONE;
		}
		return(1);

	case SM_SEND_EMAIL_BODY_DONE:
		//--------------------------------
		//----- NO MORE DATA TO SEND -----
		//--------------------------------
		return (0);
	
	}

	return(0);
}



//***************************************************************
//***************************************************************
//********** CONVERT 3 BYTES TO 4 BYTE BASE 64 ENCODED **********
//***************************************************************
//***************************************************************
//source = start of 3 byte array containing the 3 bytes to be converted
//dest = start of 4 byte array that the encoded bytes will be written to
void email_convert_3_bytes_to_base64 (BYTE *source, BYTE *dest, BYTE len)
{

	//Base64 uses an efficient 8bit to 7 bit conversion but without the encoded data being human readable.
	//It uses a 65 character subset of US-ASCII, with 6 bits represented per printable character. The 65th
	//character '=' is used as a special marker. The characters are selected so as to be universally representable,
	//and they exclude characters with particular significance to SMTP and MIME (e.g. ".", CR, LF, "-").
	//Each 24 bits (3 bytes) of the data being encoded is represented
	//by 4 characters, resulting in approximately 33% larger size overall.
	//Working from left to right, the next 3 bytes to send are joined together to form 24 bits and this is then split up
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

	dest[0] = 0;
	dest[1] = 0;
	dest[2] = 0;
	dest[3] = 0;

	dest[0] |= ((source[0] >> 2) & 0x3f);
	dest[1] |= ((source[0] << 4) & 0x30);

	if (len == 1)
	{
		//ONLY 1 BYTE TO BE ENCODED
		dest[2] = 0xff;				//Flag to set byte to '=' (for null)
		dest[3] = 0xff;				//Flag to set byte to '=' (for null)
	}
	else
	{
		// > 1 BYTE TO BE ENCODED
		dest[1] |= ((source[1] >> 4) & 0x0f);
		dest[2] |= ((source[1] << 2) & 0x3c);

		if (len == 2)
		{
			//ONLY 2 BYTES TO BE ENCODED
			dest[3] = 0xff;			//Flag to set byte to '=' (for null)
		}
		else
		{
			//ALL 3 BYTES TO BE ENCODED
			dest[2] |= ((source[2] >> 6) & 0x03);
			dest[3] |= (source[2] & 0x3f);
		}
	}

	dest[0] = email_convert_byte_to_base64_alphabet(dest[0]);
	dest[1] = email_convert_byte_to_base64_alphabet(dest[1]);
	dest[2] = email_convert_byte_to_base64_alphabet(dest[2]);
	dest[3] = email_convert_byte_to_base64_alphabet(dest[3]);
}



//************************************************************
//************************************************************
//********** SEND EMAIL CONVERT CHAR TO BASE64 CHAR **********
//************************************************************
//************************************************************
BYTE email_convert_byte_to_base64_alphabet (BYTE data)
{
	if (data < 26)
		return (data + 'A');
	else if (data < 52)
		return (data + 'a' - 26);
	else if (data < 62)
		return (data + '0' - 52);
	else if (data == 62)
		return ('+');
	else if (data == 63)
		return ('/');
	else
		return ('=');
}



//********************************************
//********************************************
//********** RETURN SMTP SERVER URL **********
//********************************************
//********************************************
void email_return_smtp_url (BYTE *string_pointer, BYTE max_length)
{
	BYTE count = 0;
	#ifdef SMTP_USING_CONST_ROM_SETTINGS
	CONSTANT BYTE *p_source_string;				//Using hard coded constant strings
	#else
	BYTE *p_source_string;							//Using variable strings
	#endif

	p_source_string = &SMTP_SERVER_STRING[0];
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
//********** RETURN SMTP USERNAME **********
//******************************************
//******************************************
//(Only used if authorised login is used)
void email_return_smtp_username (BYTE *string_pointer, BYTE max_length)
{
	BYTE count = 0;
	#ifdef SMTP_USING_CONST_ROM_SETTINGS
	CONSTANT BYTE *p_source_string;				//Using hard coded constant strings
	#else
	BYTE *p_source_string;							//Using variable strings
	#endif

	p_source_string = &SMTP_USERNAME_STRING[0];
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
//********** RETURN SMTP PASSWORD **********
//******************************************
//******************************************
//(Only used if authorised login is used)
void email_return_smtp_password (BYTE *string_pointer, BYTE max_length)
{
	BYTE count = 0;
	#ifdef SMTP_USING_CONST_ROM_SETTINGS
	CONSTANT BYTE *p_source_string;				//Using hard coded constant strings
	#else
	BYTE *p_source_string;							//Using variable strings
	#endif

	p_source_string = &SMTP_PASSWORD_STRING[0];
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



//********************************************
//********************************************
//********** RETURN SMTP TO ADDRESS **********
//********************************************
//********************************************
//(The email to be used as the destination address)
void email_return_smtp_to (BYTE *string_pointer, BYTE max_length)
{
	BYTE count = 0;
	#ifdef SMTP_USING_CONST_ROM_SETTINGS
	CONSTANT BYTE *p_source_string;				//Using hard coded constant strings
	#else
	BYTE *p_source_string;							//Using variable strings
	#endif

	p_source_string = &SMTP_TO_STRING[0];
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



//**********************************************
//**********************************************
//********** RETURN SMTP FROM ADDRESS **********
//**********************************************
//**********************************************
//(The email to be used as the from address)
void email_return_smtp_sender (BYTE *string_pointer, BYTE max_length)
{
	BYTE count = 0;
	#ifdef SMTP_USING_CONST_ROM_SETTINGS
	CONSTANT BYTE *p_source_string;				//Using hard coded constant strings
	#else
	BYTE *p_source_string;							//Using variable strings
	#endif

	p_source_string = &SMTP_SENDER_STRING[0];
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



//*****************************************
//*****************************************
//********** RETURN SMTP SUBJECT **********
//*****************************************
//*****************************************
void email_return_smtp_subject (BYTE *string_pointer, BYTE max_length)
{
	BYTE count = 0;
	#ifdef SMTP_USING_CONST_ROM_SETTINGS
	CONSTANT BYTE *p_source_string;
	#else
	BYTE *p_source_string;
	#endif

	p_source_string = &SMTP_SUBJECT_STRING[0];
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



