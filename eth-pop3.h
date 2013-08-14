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
//POP3 (POST OFFICE PROTOCOL) C CODE HEADER FILE




//##################################
//##################################
//########## USING DRIVER ##########
//##################################
//##################################


//#############################
//##### TO RECEIVE EMAILS #####
//#############################
/*
	//-------------------------------
	//----- START EMAIL RECEIVE -----
	//-------------------------------
	//If POP3_USING_CONST_ROM_SETTINGS is commented out load the byte arrays your using for the following string defines:
	POP3_SERVER_STRING
	POP3_USERNAME_STRING
	POP3_PASSWORD_STRING

	email_start_receive();		//Trigger receiving email


//----------------------------------------------------------------
//----- FUNCTION TO PROCESS EACH LINE OF EACH RECEIVED EMAIL -----
//----------------------------------------------------------------
BYTE process_pop3_received_email_line (BYTE status, BYTE *string, BYTE *sender_email_address)
{
	BYTE count;
	BYTE *p_dest;

	if (status == 0)
	{
		//----- START OF A NEW EMAIL - THIS IS THE SUBJECT -----
		//Process as desired

		return(0);
	}
	else if (status == 1)
	{
		//----- NEXT LINE OF THIS EMAIL -----
		//Process as desired

		return(0);
	}
	else
	{
		//----- END OF EMAIL -----
		return(1);		//0x01 = delete email, 0x00 = don't delete this email
	}
}


*/


//##########################################################
//##### TO INDICATE RECEIVE EMAIL PROGRESS TO THE USER #####
//##########################################################
/*
	//Include the DO_POP3_PROGRESS_STRING define to enable these messages
	//Edit the strings in eth-pop3.h as desired
	if (email_is_receive_active())
	{
		//----- WE ARE RECEIVING EMAIL -----
		//CHECK FOR UPDATE USER DISPLAY
		if (pop3_email_progress_string_update)
		{
			//RECEIVE EMAIL STATUS HAS CHANGED - UPDATE DISPLAY
			pop3_email_progress_string_update = 0;
			//... = &pop3_email_progress_string[0];			//Display the status null terminated string to the user on our screen
		}
	}
*/


//For further information please see the project technical manual




//********************************
//********************************
//********** DO DEFINES **********
//********************************
//********************************
#ifndef ETH_POP3_C_INIT		//Do only once the first time this file is used
#define	ETH_POP3_C_INIT


#include "eth-main.h"
#include "eth-tcp.h"

//----- USE USER INTERFACE PROGRESS STRINGS? -----
//(See later in this file for details)
#define	DO_POP3_PROGRESS_STRING							//Comment out if not required.  The strings may optionally be used to provide the
														//user with the current state of the send mail process

//----- FUNCTION TO PROCESS RECEIVED EMAILS -----
//The function name in your application that will be called with each line of received emails
#define	POP3_PROCESS_RECEIVED_EMAIL_LINE_FUNCTION		process_pop3_received_email_line

//Your function definition needs to be:
//	BYTE my_function_name (BYTE status, BYTE *string, BYTE *sender_email_address)
//status:
//	0 = this is the start of a new email received.  The string contains the contents of the subject line.  Return value is don't care
//	1 = this is the next line of the email body (excluding the trailing <CR><LF>).  Return value is don't care.
//	2 = End of email.  string is null.  Return 1 to delete email, return 0 to leave email in mailbox.
//sender_email_address
//	Pointer to string included to allow your function to identify the sender and optionally store the sender if you will reply with an email.
//You need to #include the file that contains your function definition at the top of eth-pop3.c for the compiler

//----- EMAIL CONFIGURATION -----
#define	POP3_RECEIVE_MESSAGE_STRING_MAX_LEN			81	//Max length of each received line in an email header or body (including terminating null) (extra characters will be dumped)
														//Smaller value to reduce ram requirement, bigger value to avoid issues with truncated lines.
#define	POP3_REPLY_TO_STRING_LEN					36	//Max length of the 'reply to' email address (the senders email) (including terminating null)
														//Smaller value to reduce ram requirement, but ensure its big enough to hold the maximum expected email address.

//TIMEOUTS
#define	EMAIL_DO_DNS_TIMEOUT_x100MS					250
#define	EMAIL_DO_ARP_TIMEOUT_x100MS					250
#define	EMAIL_WAIT_TCP_CONNECTION_TIMEOUT_x100MS	350
#define	EMAIL_WAIT_SERVER_MESSAGE_TIMEOUT_x100MS	600	//This needs to be long to deal with servers that are busy and take a long time to give a welcome message etc.
														//If too short then we can FIN a connection and start a new connection, but the new connection will never work
														//as each time the server will have cached the last FIN request and when it finally responds with its welcome
														//message it will then process the previous FIN request and close the conneciton.
//TCP PORTS
#define	POP3_REMOTE_PORT					110

//POP3 TASK STATE MACHINE STATES
typedef enum _POP3_STATE
{
    SM_POP3_IDLE,
    SM_POP3_GET_EMAIL,
    SM_POP3_WAITING_DNS_RESPONSE,
    SM_POP3_SEND_ARP_REQUEST,
    SM_POP3_WAIT_FOR_ARP_RESPONSE,
    SM_POP3_OPEN_TCP_CONNECTION,
    SM_POP3_WAIT_FOR_TCP_CONNECTION,
    SM_POP3_WAIT_FOR_POP3_SERVER_GREETING,
    SM_POP3_SEND_POP3_USERNAME,
    SM_POP3_WAIT_FOR_POP3_USER_RESPONSE,
    SM_POP3_SEND_POP3_PASSWORD,
    SM_POP3_WAIT_FOR_POP3_PASS_RESPONSE,
    SM_POP3_SEND_POP3_STAT_REQUEST,
    SM_POP3_WAIT_FOR_POP3_STAT_RESPONSE,
    SM_POP3_WAIT_GET_NEXT_EMAIL,
    SM_POP3_WAIT_GET_NEXT_EMAIL_RESPONSE,
    SM_POP3_WAIT_GET_NEXT_EMAIL_RECEIVE,
    SM_POP3_DO_DELETE,
    SM_POP3_DO_DELETE_RESPONSE,
    SM_POP3_DO_QUIT,
    SM_POP3_WAIT_FOR_POP3_QUIT_RESPONSE,
    SM_POP3_FAILED
} POP3_STATE;

#endif


//---------------------------------
//----- POP3 ACCOUNT SETTINGS -----
//---------------------------------
#ifdef ETH_POP3_C

//----- SERVER SETTINGS -----
//USE HARD CODED ROM STRINGS OR VARAIBLE STRINGS?
//#define	POP3_USING_CONST_ROM_SETTINGS				//Comment out if using variables to store the settings

#ifdef POP3_USING_CONST_ROM_SETTINGS
	//DEFINE HARD CODED CONSTANT STRINGS IF USED
	CONSTANT BYTE POP3_SERVER_STRING[] = {"pop.yahoo.co.uk"};
	CONSTANT BYTE POP3_USERNAME_STRING[] = {"embedded-code"};
	CONSTANT BYTE POP3_PASSWORD_STRING[] = {"ourpassword"};
#else
	//DEFINE VARIABLE STRINGS IF BEING USED - MUST BE BYTE ARRAYS AND NULL TERMINATED (0X00 REQUIRED AFTER LAST CHARACTER - IF YOU FORGET THIS EXPECT HORRIBLE BUGS!)
	#define	POP3_SERVER_STRING 			our_pop3_server
	#define	POP3_USERNAME_STRING		our_pop3_username
	#define	POP3_PASSWORD_STRING		our_pop3_password
#endif


//----- USER INTERFACE PROGRESS STRINGS -----
//If enabled the driver will load the pointer pop3_email_progress_string_pointer with each string and set pop3_email_progress_string_update each time the status changes
//You may edit the strings as desired
#ifdef DO_POP3_PROGRESS_STRING
CONSTANT BYTE pop3_email_progress_string_null[] = {" "};
CONSTANT BYTE pop3_email_progress_string_wait_pop3_dns_resp[] = {"Getting POP3 DNS"};
CONSTANT BYTE pop3_email_progress_string_wait_pop3_arp_resp[] = {"Getting POP3 ARP"};
CONSTANT BYTE pop3_email_progress_string_wait_pop3_tcp_conn[] = {"Getting POP3 TCP con"};
CONSTANT BYTE pop3_email_progress_string_wait_pop3_greeting[] = {"Waiting POP3 hello"};
CONSTANT BYTE pop3_email_progress_string_wait_pop3_user[] = {"Sent POP3 username"};
CONSTANT BYTE pop3_email_progress_string_wait_pop3_password[] = {"Sent POP3 password"};
CONSTANT BYTE pop3_email_progress_string_wait_pop3_stat[] = {"Waiting POP3 count"};
CONSTANT BYTE pop3_email_progress_string_wait_pop3_header[] = {"Reading emails"};
CONSTANT BYTE pop3_email_progress_string_wait_pop3_delete[] = {"Deleting email to me"};
CONSTANT BYTE pop3_email_progress_string_wait_pop3_quit[] = {"Logging off POP3"};
CONSTANT BYTE pop3_email_progress_string_complete[] = {"Receive email complete"};
CONSTANT BYTE pop3_email_progress_string_failed[] = {"Receive email failed!"};
#endif

//DRIVER STRINGS
CONSTANT BYTE pop3_reply_to_string[] = {"from:"};			//Use 'from:' as this must occur in an email header whereas 'reply-to:' does not always occur
CONSTANT BYTE pop3_subject_string[] = {"subject:"};
#endif


//*******************************
//*******************************
//********** FUNCTIONS **********
//*******************************
//*******************************
#ifdef ETH_POP3_C
//-----------------------------------
//----- INTERNAL ONLY FUNCTIONS -----
//-----------------------------------
void email_return_pop3_url (BYTE *string_pointer, BYTE max_length);
void email_return_pop3_username (BYTE *string_pointer, BYTE max_length);
void email_return_pop3_password (BYTE *string_pointer, BYTE max_length);


//-----------------------------------------
//----- INTERNAL & EXTERNAL FUNCTIONS -----
//-----------------------------------------
//(Also defined below as extern)
BYTE email_start_receive (void);
BYTE email_is_receive_active (void);
void email_process_pop3 (void);


#else
//------------------------------
//----- EXTERNAL FUNCTIONS -----
//------------------------------
extern BYTE email_start_receive (void);
extern BYTE email_is_receive_active (void);
extern void email_process_pop3 (void);


#endif



//****************************
//****************************
//********** MEMORY **********
//****************************
//****************************
#ifdef ETH_POP3_C
//--------------------------------------------
//----- INTERNAL ONLY MEMORY DEFINITIONS -----
//--------------------------------------------
BYTE pop3_tcp_socket = TCP_INVALID_SOCKET;
DEVICE_INFO pop3_server_node;
WORD pop3_100ms_timeout_timer;
BYTE pop3_receive_message_string[POP3_RECEIVE_MESSAGE_STRING_MAX_LEN];
BYTE pop3_receive_message_string_len;


//--------------------------------------------------
//----- INTERNAL & EXTERNAL MEMORY DEFINITIONS -----
//--------------------------------------------------
//(Also defined below as extern)
BYTE sm_pop3 = SM_POP3_IDLE;
BYTE email_reply_to_string[POP3_REPLY_TO_STRING_LEN];

#ifdef DO_POP3_PROGRESS_STRING
CONSTANT BYTE *pop3_email_progress_string_pointer;
BYTE pop3_email_progress_string_update;
#endif


#else
//---------------------------------------
//----- EXTERNAL MEMORY DEFINITIONS -----
//---------------------------------------
extern BYTE sm_pop3;
extern BYTE email_reply_to_string[POP3_REPLY_TO_STRING_LEN];

#ifdef DO_POP3_PROGRESS_STRING
extern CONSTANT BYTE *pop3_email_progress_string_pointer;
extern BYTE pop3_email_progress_string_update;
#endif


#endif






