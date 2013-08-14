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
//SMTP (SIMPLE MAIL TRANSFER PROTOCOL) C CODE HEADER FILE





//##################################
//##################################
//########## USING DRIVER ##########
//##################################
//##################################

//############################
//##### TO SEND AN EMAIL #####
//############################
/*
	static WORD send_email_body_next_byte;
	static DWORD send_email_file_next_byte;

	//----- START EMAIL SEND -----
	//If SMTP_USING_CONST_ROM_SETTINGS is commented out load the byte arrays your using for the following string defines:
	SMTP_SERVER_STRING
	SMTP_USERNAME_STRING
	SMTP_PASSWORD_STRING
	SMTP_TO_STRING
	SMTP_SENDER_STRING
	SMTP_SUBJECT_STRING

	send_email_body_next_byte = 0;
	send_email_file_next_byte = 0;
	email_start_send(1, 1);		//Trigger send email.  Use authenticated login, include a file attachment


//---------------------------------------------------------------------------
//----- FUNCTION TO PROVIDE EACH BYTE OF EMAIL BODY AND FILE ATTACHMENT -----
//---------------------------------------------------------------------------
BYTE provide_smtp_next_data_byte (BYTE sending_email_body, BYTE start_of_new_tcp_packet, WORD resend_move_back_bytes, BYTE* next_byte)
{
	if (sending_email_body)
	{
		//-------------------------------------------
		//----- PROVIDE NEXT BYTE OF EMAIL BODY -----
		//-------------------------------------------
		if (start_of_new_tcp_packet)
		{
			//LAST PACKET CONFIMRED AS SENT - IF WE NEED TO STORE VALUES FOR THE NEXT PACKET OF DATA TO BE SENT DO IT NOW

		}
		if (resend_move_back_bytes)
		{
			//WE NEED TO RESEND THE LAST PACKET
			if (resend_move_back_bytes >= send_email_body_next_byte)		//(Should always be true but check in case of error)
				send_email_body_next_byte -= resend_move_back_bytes;
		}
		
		//GET NEXT BYTE OF TEXT
		if (send_email_body_text_array[send_email_body_next_byte] != 0x00)
		{
			//NEXT BYTE TO SEND
			*next_byte = send_email_body_text_array[send_email_body_next_byte++];
			return(1);
		}
		else
		{
			//ALL BYTES SENT
			return(0);
		}
	}
	else
	{
		//------------------------------------------------------
		//----- PROVIDE NEXT BYTE OF EMAIL FILE ATTACHMENT -----
		//------------------------------------------------------
		if (start_of_new_tcp_packet)
		{
			//LAST PACKET CONFIMRED AS SENT - IF WE NEED TO STORE VALUES FOR THE NEXT PACKET OF DATA TO BE SENT DO IT NOW

		}
		if (resend_move_back_bytes)
		{
			//WE NEED TO RESEND THE LAST PACKET
			if (resend_move_back_bytes >= send_email_file_next_byte)		//(Should always be true but check in case of error)
				send_email_file_next_byte -= resend_move_back_bytes;
		}
		
		//GET NEXT BYTE OF FILE ATTACHMENT
		if (our_file_to_send_size > send_email_file_next_byte)
		{
			//NEXT BYTE TO SEND
			*next_byte = our_file_to_send_array[send_email_file_next_byte++];
			return(1);
		}
		else
		{
			//ALL BYTES SENT
			return(0);
		}
	}
}

*/


//#######################################################
//##### TO INDICATE SEND EMAIL PROGRESS TO THE USER #####
//#######################################################
/*
	//Include the DO_SMTP_PROGRESS_STRING define to enable these messages
	//Edit the strings in eth-smtp.h as desired
	if (email_is_send_active())
	{
		//----- WE ARE SENDING AN EMAIL -----
		//CHECK FOR UPDATE USER DISPLAY
		if (smtp_email_progress_string_update)
		{
			//SEND EMAIL STATUS HAS CHANGED - UPDATE DISPLAY
			smtp_email_progress_string_update = 0;
			//... = &smtp_email_progress_string[0];			//Display the status string to the user on our screen
		}
	}
*/


//For further information please see the project technical manual









//********************************
//********************************
//********** DO DEFINES **********
//********************************
//********************************
#ifndef ETH_SMTP_C_INIT		//Do only once the first time this file is used
#define	ETH_SMTP_C_INIT


#include "eth-main.h"
#include "eth-tcp.h"


//----- USE USER INTERFACE PROGRESS STRINGS? -----
//(See later in this file for details)
#define	DO_SMTP_PROGRESS_STRING						//Comment out if not required.  The strings may optionally be used to provide the
													//user with the current state of the receive mail process

//----- EMAIL ATTACHMENT FILENAME -----
#define	EMAIL_ATTACHMENT_FILENAME	"test_file.txt"	//Comment out if not using send email file attachment (will reduce program memory size)

//----- FUNCTION TO PROCESS RECEIVED EMAILS -----
//The function name in your application that will be called to get each byte
#define	SMTP_GET_NEXT_DATA_BYTE_FUNCTION		provide_smtp_next_data_byte
//Your function definition needs to be:
//	BYTE my_function_name (BYTE sending_email_body, BYTE start_of_new_tcp_packet, WORD resend_move_back_bytes, BYTE* next_byte);
//Returns:
//	1 = more bytes to follow
//	0 = no more bytes (last byte returned was the final byte for email body or file attachment)
//sending_email_body
//	1 = Sending email body text (email message)
//	0 = Sending file attachment
//start_of_new_tcp_packet
//	1 = This is start of a new TCP packet so previously sent data has been recevied and will not need to be resent
//resend_move_back_bytes
//	0 = send next byte
//	> 0 = we need to resend the last packet so move back this many bytes (up to 1400) and start again from that position
//	Note that resend will not jump back from file attachment to email body - once file attachment is started then email body will have been confirmed as received
//next_byte
//	Next byte of email body or file attachment to send
//You need to #include the file that contains the function definition at the top of eth-smtp.c for the compiler

//----- EMAIL CONFIGURATION -----
#define	EMAIL_REPLY_TO_STRING_LEN					36		//Max number of characters including terminating null
															//Smaller value to reduce ram requirement, but ensure its big enough to hold the maximum expected email address.
#define	SMTP_RECEIVE_MESSAGE_STRING_MAX_LEN			8		//Max number of characters including terminating null

//TIMEOUTS
#define	EMAIL_DO_DNS_TIMEOUT_x100MS					250
#define	EMAIL_DO_ARP_TIMEOUT_x100MS					250
#define	EMAIL_WAIT_TCP_CONNECTION_TIMEOUT_x100MS	350
#define	EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS	600		//This needs to be long to deal with servers that are busy and take a long time to give a welcom emessage etc.
															//If too short then we can FIN a connection and start a new connection, but the new connection will never work
															//as each time the server will have cached the last FIN request and when it finally responds with its welcome
															//message it will then process the previous FIN request and close the conneciton.
//TCP PORTS
#define	SMTP_REMOTE_PORT					25

//SMTP TASK STATE MACHINE STATES
typedef enum _SMTP_STATE
{
    SM_SMTP_IDLE,
    SM_SMTP_SEND_EMAIL,
    SM_SMTP_WAITING_DNS_RESPONSE,
    SM_SMTP_SEND_DNS_ARP_REQUEST,
    SM_SMTP_WAIT_FOR_ARP_RESPONSE,
    SM_SMTP_OPEN_TCP_CONNECTION,
    SM_SMTP_WAIT_FOR_TCP_CONNECTION,
    SM_SMTP_WAIT_FOR_SMTP_SERVER_GREETING,
    SM_SMTP_SEND_SMTP_HELO,
    SM_SMTP_WAIT_FOR_SMTP_HELO_RESPONSE,
    SM_SMTP_SEND_AUTH_LOGIN_COMMAND,
    SM_SMTP_WAIT_FOR_SMTP_AUTH_LOGIN_RESPONSE,
    SM_SMTP_SEND_AUTH_USERNAME,
    SM_SMTP_WAIT_FOR_SMTP_AUTH_USERNAME_RESPONSE,
    SM_SMTP_SEND_AUTH_PASSWORD,
    SM_SMTP_WAIT_FOR_SMTP_AUTH_PASSWORD_RESPONSE,
    SM_SMTP_SEND_MAIL_FROM_COMMAND,
    SM_SMTP_WAIT_FOR_SMTP_MAIL_FROM_RESPONSE,
    SM_SMTP_SEND_RCPT_TO,
    SM_SMTP_WAIT_FOR_SMTP_RCPT_TO_RESPONSE,
    SM_SMTP_SEND_DATA_COMMAND,
    SM_SMTP_WAIT_FOR_SMTP_DATA_RESPONSE,
    SM_SMTP_SEND_EMAIL_HEADER,
    SM_SMTP_WAIT_EMAIL_HEADER_ACK,
    SM_SMTP_SEND_SMTP_EMAIL_BODY,
    SM_SMTP_WAIT_FOR_SMTP_EMAIL_BODY_REPONSE,
    SM_SMTP_DO_QUIT,
    SM_SMTP_WAIT_FOR_SMTP_QUIT_RESPONSE,
    SM_SMTP_FAILED
} SMTP_STATE;

//SEND EMAIL BODY STATE MACHINE STATES
typedef enum _SEND_EMAIL_BODY_STATE
{
	SM_SEND_EMAIL_DATA_MIME_HEADER,
	SM_SEND_EMAIL_BODY_ASCII,
	SM_SEND_EMAIL_BODY_MIME_FILE_HEADER_1,
	SM_SEND_EMAIL_BODY_MIME_FILE_HEADER_2,
	SM_SEND_EMAIL_BODY_MIME_FILE_HEADER_3,
	SM_SEND_EMAIL_BODY_MIME_FILE_DATA,
	SM_SEND_EMAIL_BODY_MIME_END_OF_BODY,
	SM_SEND_EMAIL_BODY_DONE
} SEND_EMAIL_BODY_STATE;

#endif



//---------------------------------
//----- SMTP ACCOUNT SETTINGS -----
//---------------------------------
#ifdef ETH_SMTP_C

//----- SERVER SETTINGS -----
//USE HARD CODED ROM STRINGS OR VARAIBLE STRINGS?
//#define	SMTP_USING_CONST_ROM_SETTINGS				//Comment out if using variables to store the settings

#ifdef SMTP_USING_CONST_ROM_SETTINGS
	//DEFINE HARD CODED CONSTANT STRINGS IF USED
	CONSTANT BYTE SMTP_SERVER_STRING[] = {"pop.yahoo.co.uk"};
	CONSTANT BYTE SMTP_USERNAME_STRING[] = {"embedded-code"};
	CONSTANT BYTE SMTP_PASSWORD_STRING[] = {"ourpassword"};
	CONSTANT BYTE SMTP_TO_STRING[] = {"you@myurl.com"};
	CONSTANT BYTE SMTP_SENDER_STRING[] = {"me@myurl.com"};
	CONSTANT BYTE SMTP_SUBJECT_STRING[] = {"Hello from my embedded device"};
#else
	//DEFINE VARIABLE STRINGS IF BEING USED - MUST BE BYTE ARRAYS AND NULL TERMINATED (0X00 REQUIRED AFTER LAST CHARACTER - IF YOU FORGET THIS EXPECT HORRIBLE BUGS!)
	#define	SMTP_SERVER_STRING			our_smtp_server
	#define	SMTP_USERNAME_STRING		our_smtp_username
	#define	SMTP_PASSWORD_STRING		our_smtp_password
	#define	SMTP_TO_STRING				our_smtp_to
	#define	SMTP_SENDER_STRING			our_smtp_from
	#define	SMTP_SUBJECT_STRING			our_smtp_subject
#endif

//----- USER INTERFACE PROGRESS STRINGS -----
//If enabled the driver will load the pointer smtp_email_progress_string_pointer with each string and set smtp_email_progress_string_update each time the status changes
//You may edit the strings as desired
#ifdef DO_SMTP_PROGRESS_STRING
CONSTANT BYTE smtp_email_progress_string_null[] = {" "};
CONSTANT BYTE smtp_email_progress_string_wait_smtp_dns_resp[] = {"Getting SMTP DNS"};
CONSTANT BYTE smtp_email_progress_string_wait_smtp_arp_resp[] = {"Getting SMTP ARP"};
CONSTANT BYTE smtp_email_progress_string_wait_smtp_tcp_conn[] = {"Getting SMTP TCP con"};	
CONSTANT BYTE smtp_email_progress_string_wait_smtp_greeting[] = {"Waiting SMTP hello"};
CONSTANT BYTE smtp_email_progress_string_wait_smtp_helo[] = {"Sent SMTP HELO"};
CONSTANT BYTE smtp_email_progress_string_wait_smtp_auth_login[] = {"Sent SMTP login req"};
CONSTANT BYTE smtp_email_progress_string_wait_smtp_username[] = {"Sent SMTP username"};
CONSTANT BYTE smtp_email_progress_string_wait_smtp_password[] = {"Sent SMTP password"};
CONSTANT BYTE smtp_email_progress_string_wait_smtp_from[] = {"Sent SMTP from addr"};
CONSTANT BYTE smtp_email_progress_string_wait_smtp_to[] = {"Sent SMTP to addr"};
CONSTANT BYTE smtp_email_progress_string_wait_smtp_data[] = {"Sent SMTP data req"};
CONSTANT BYTE smtp_email_progress_string_sending_smtp_data[] = {"Sending email data"};
CONSTANT BYTE smtp_email_progress_string_wait_smtp_data_resp[] = {"Sent SMTP email end"};
CONSTANT BYTE smtp_email_progress_string_wait_smtp_data_accepted[] = {"SMTP email accepted"};
CONSTANT BYTE smtp_email_progress_string_wait_smtp_quit[] = {"Logging off SMTP"};
CONSTANT BYTE smtp_email_progress_string_complete[] = {"Send email complete"};
CONSTANT BYTE smtp_email_progress_string_failed[] = {"Send email failed!"};
#endif

#endif



//*******************************
//*******************************
//********** FUNCTIONS **********
//*******************************
//*******************************
#ifdef ETH_SMTP_C
//-----------------------------------
//----- INTERNAL ONLY FUNCTIONS -----
//-----------------------------------
BYTE send_email_get_next_byte (BYTE *data, BYTE resend_flags);
void email_convert_3_bytes_to_base64 (BYTE *source, BYTE *dest, BYTE len);
BYTE email_convert_byte_to_base64_alphabet (BYTE data);
void email_return_smtp_url (BYTE *string_pointer, BYTE max_length);
void email_return_smtp_username (BYTE *string_pointer, BYTE max_length);
void email_return_smtp_password (BYTE *string_pointer, BYTE max_length);
void email_return_smtp_to (BYTE *string_pointer, BYTE max_length);
void email_return_smtp_sender (BYTE *string_pointer, BYTE max_length);
void email_return_smtp_subject (BYTE *string_pointer, BYTE max_length);


//-----------------------------------------
//----- INTERNAL & EXTERNAL FUNCTIONS -----
//-----------------------------------------
//(Also defined below as extern)
BYTE email_start_send (BYTE use_authenticated_login, BYTE include_file_attachment);
BYTE email_is_send_active (void);
void process_smtp (void);


#else
//------------------------------
//----- EXTERNAL FUNCTIONS -----
//------------------------------
extern BYTE email_start_send (BYTE use_authenticated_login, BYTE include_file_attachment);
extern BYTE email_is_send_active (void);
extern void email_process_smtp (void);


#endif




//****************************
//****************************
//********** MEMORY **********
//****************************
//****************************
#ifdef ETH_SMTP_C
//--------------------------------------------
//----- INTERNAL ONLY MEMORY DEFINITIONS -----
//--------------------------------------------
BYTE sm_send_email_data;
BYTE smtp_tcp_socket;
DEVICE_INFO smtp_server_node;
WORD smtp_100ms_timeout_timer;
BYTE smtp_receive_message_string[SMTP_RECEIVE_MESSAGE_STRING_MAX_LEN];
BYTE smtp_receive_message_string_len;
BYTE smtp_use_authenticated_login;
BYTE smtp_include_file_attachment;
WORD send_email_body_byte_number;


//--------------------------------------------------
//----- INTERNAL & EXTERNAL MEMORY DEFINITIONS -----
//--------------------------------------------------
//(Also defined below as extern)
BYTE sm_smtp = SM_SMTP_IDLE;

#ifdef DO_SMTP_PROGRESS_STRING
CONSTANT BYTE *smtp_email_progress_string_pointer;
BYTE smtp_email_progress_string_update;
#endif


#else
//---------------------------------------
//----- EXTERNAL MEMORY DEFINITIONS -----
//---------------------------------------
extern BYTE sm_smtp;

#ifdef DO_SMTP_PROGRESS_STRING
extern CONSTANT BYTE *smtp_email_progress_string_pointer;
extern BYTE smtp_email_progress_string_update;
#endif


#endif




//*******************************************
//*******************************************
//********** MIME CONSTANT STRINGS **********
//*******************************************
//*******************************************
#ifdef ETH_SMTP_C

//The MIME header used just following the standard email body header:
CONSTANT BYTE mime_header_for_text_string[] = "MIME-Version: 1.0\r\n"
											"Content-Type: multipart/mixed; boundary=\"MIME-BOUNDARY58\"\r\n"
											"\r\n"
											"--MIME-BOUNDARY58\r\n"
											"Content-Type: text/plain;\r\n"
											"Content-Transfer-Encoding: 7bit\r\n"
											"\r\n";		// \r\n is <CR> <LF>, \" to include the " character, "MIME-BOUNDARY58" is any string that is guaranteed not to appear within the email ascii text
														//Include a blank line at the end to indicate the start of the email body


//Then the MIME header for the file attachment:
CONSTANT BYTE mime_body_file_header_string_1[] = "\r\n"
											"\r\n"
											"--MIME-BOUNDARY58\r\n"
											"Content-Type: text/plain;\r\n"
											"Content-Transfer-Encoding: base64\r\n"
											"Content-Disposition: attachment; filename=\"";		//File name inserted after this and before remainder
CONSTANT BYTE mime_body_file_header_string_2[] = "\"\r\n"
											"\r\n";

//Then the file data

//Then the end of mime marker
CONSTANT BYTE mime_body_end_of_mime_string[] = "\r\n"
											"\r\n"
											"--MIME-BOUNDARY58--\r\n"
											"\r\n";

#endif


