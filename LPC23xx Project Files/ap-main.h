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
//LPC23xx SAMPLE PROJECT C CODE HEADER FILE





#include "eth-http.h"		//Include for HTTP_MAX_INPUT_VALUE_LENGTH define used below


//*****************************
//*****************************
//********** DEFINES **********
//*****************************
//*****************************
#ifndef MAIN_C_INIT				//(Include this section only once for each source file that includes this header file)
#define	MAIN_C_INIT

//------------------------
//----- USER DEFINES -----
//------------------------



//------------------------
//----- USER DEFINES -----
//------------------------



//----------------------
//----- IO DEFINES -----
//----------------------
#define	LED_A_ON(state)					(state ? (FIO2SET = 0x00000004) : (FIO2CLR = 0x00000004))
#define	LED_B_ON(state)					(state ? (FIO2SET = 0x00000008) : (FIO2CLR = 0x00000008))



//--------------------------
//----- SWITCH DEFINES -----
//--------------------------
#define	SWITCH_A_PRESSED			switches_debounced & 0x01
#define	SWITCH_B_PRESSED			switches_debounced & 0x02

#define	SWITCH_A_NEW_PRESS			switches_new & 0x01
#define	SWITCH_B_NEW_PRESS			switches_new & 0x02


//STATES USED BY OUR DEMO SOCKET STATE MACHINES
typedef enum _CONNECTION_STATE
{
	SM_IDLE,
	SM_LOOKUP_DNS,
	SM_OPEN_SOCKET,
	SM_PROCESS_SOCKET,
	SM_WAIT_FOR_CONNECTION,
	SM_TX_PACKET,
	SM_PROCESS_CONNECTION,
	SM_TX_RESPONSE,
	SM_WAIT_FOR_RESPONSE,
	SM_REQUEST_DISCONNECT,
	SM_WAIT_FOR_DISCONNECT,
	SM_CLOSE_SOCKET,
	SM_COMMS_COMPLETE,
	SM_COMMS_FAILED,
	SM_TRIGGER_GET_EMAIL,
	SM_GETTING_EMAIL,
	SM_TRIGGER_SEND_EMAIL,
	SM_SENDING_EMAIL,
} CONNECTION_STATE;


#endif







//*******************************
//*******************************
//********** FUNCTIONS **********
//*******************************
//*******************************
#ifdef MAIN_C
//-----------------------------------
//----- INTERNAL ONLY FUNCTIONS -----
//-----------------------------------
void initialise (void);
void timer0_irq_handler (void);
void read_switches (void);
void process_user_mode (void);
void process_demo_udp_server (void);
void process_demo_udp_client(BYTE start_comms);
void process_demo_tcp_server (void);
void process_demo_tcp_client (BYTE start_comms);


//-----------------------------------------
//----- INTERNAL & EXTERNAL FUNCTIONS -----
//-----------------------------------------
//(Also defined below as extern)
BYTE process_http_authorise_request (BYTE *requested_filename, BYTE *requested_filename_extension, BYTE tcp_socket_number);
BYTE *process_http_dynamic_data (BYTE *variable_name, BYTE tcp_socket_number);
void process_http_inputs (BYTE *input_name, BYTE *input_value, BYTE *requested_filename, BYTE *requested_filename_extension, BYTE tcp_socket_number);
void process_http_multipart_form_header (const BYTE *input_name, BYTE *input_value, BYTE *requested_filename, BYTE *requested_filename_extension, BYTE tcp_socket_number);
void process_http_multipart_form_data (BYTE data);
void process_http_multipart_form_last_section_done (void);
BYTE process_pop3_received_email_line (BYTE status, BYTE *string, BYTE *sender_email_address);
BYTE provide_smtp_next_data_byte (BYTE sending_email_body, BYTE start_of_new_tcp_packet, WORD resend_move_back_bytes, BYTE* next_byte);
void sntp_send_receive_handler (BYTE event, DWORD sntp_seconds_value);


#else
//------------------------------
//----- EXTERNAL FUNCTIONS -----
//------------------------------
extern BYTE process_http_authorise_request (BYTE *requested_filename, BYTE *requested_filename_extension, BYTE tcp_socket_number);
extern BYTE *process_http_dynamic_data (BYTE *variable_name, BYTE tcp_socket_number);
extern void process_http_inputs (BYTE *input_name, BYTE *input_value, BYTE *requested_filename, BYTE *requested_filename_extension, BYTE tcp_socket_number);
extern void process_http_multipart_form_header (const BYTE *input_name, BYTE *input_value, BYTE *requested_filename, BYTE *requested_filename_extension, BYTE tcp_socket_number);
extern void process_http_multipart_form_data (BYTE data);
extern void process_http_multipart_form_last_section_done (void);
extern BYTE process_pop3_received_email_line (BYTE status, BYTE *string, BYTE *sender_email_address);
extern BYTE provide_smtp_next_data_byte (BYTE sending_email_body, BYTE start_of_new_tcp_packet, WORD resend_move_back_bytes, BYTE* next_byte);
extern void sntp_send_receive_handler (BYTE event, DWORD sntp_seconds_value);


#endif






//****************************
//****************************
//********** MEMORY **********
//****************************
//****************************
#ifdef MAIN_C
//--------------------------------------------
//----- INTERNAL ONLY MEMORY DEFINITIONS -----
//--------------------------------------------
BYTE read_switches_flag;
WORD user_mode_10ms_timer;
BYTE udp_client_socket_timeout_timer;
BYTE tcp_server_socket_timeout_timer;
BYTE tcp_client_socket_timeout_timer;
WORD send_email_body_next_byte;
WORD send_email_file_next_byte;


//--------------------------------------------------
//----- INTERNAL & EXTERNAL MEMORY DEFINITIONS -----
//--------------------------------------------------
//(Also defined below as extern)
BYTE switches_debounced = 0;
BYTE switches_new = 0;
WORD general_use_10ms_timer;
WORD general_use_100ms_timer;
BYTE our_pop3_server[HTTP_MAX_INPUT_VALUE_LENGTH] = {0x00};
BYTE our_pop3_username[HTTP_MAX_INPUT_VALUE_LENGTH] = {0x00};
BYTE our_pop3_password[HTTP_MAX_INPUT_VALUE_LENGTH] = {0x00};
BYTE our_smtp_server[HTTP_MAX_INPUT_VALUE_LENGTH] = {0x00};
BYTE our_smtp_username[HTTP_MAX_INPUT_VALUE_LENGTH] = {0x00};
BYTE our_smtp_password[HTTP_MAX_INPUT_VALUE_LENGTH] = {0x00};
BYTE our_smtp_to[HTTP_MAX_INPUT_VALUE_LENGTH] = {0x00};
BYTE our_smtp_from[HTTP_MAX_INPUT_VALUE_LENGTH] = {0x00};
BYTE our_smtp_subject[HTTP_MAX_INPUT_VALUE_LENGTH] = {0x00};


#else
//---------------------------------------
//----- EXTERNAL MEMORY DEFINITIONS -----
//---------------------------------------
extern BYTE switches_debounced;
extern BYTE switches_new;
extern WORD general_use_10ms_timer;
extern WORD general_use_100ms_timer;
extern BYTE our_pop3_server[HTTP_MAX_INPUT_VALUE_LENGTH];
extern BYTE our_pop3_username[HTTP_MAX_INPUT_VALUE_LENGTH];
extern BYTE our_pop3_password[HTTP_MAX_INPUT_VALUE_LENGTH];
extern BYTE our_smtp_server[HTTP_MAX_INPUT_VALUE_LENGTH];
extern BYTE our_smtp_username[HTTP_MAX_INPUT_VALUE_LENGTH];
extern BYTE our_smtp_password[HTTP_MAX_INPUT_VALUE_LENGTH];
extern BYTE our_smtp_to[HTTP_MAX_INPUT_VALUE_LENGTH];
extern BYTE our_smtp_from[HTTP_MAX_INPUT_VALUE_LENGTH];
extern BYTE our_smtp_subject[HTTP_MAX_INPUT_VALUE_LENGTH];


#endif




//*****************************
//*****************************
//********** STRINGS **********
//*****************************
//*****************************
#ifdef MAIN_C				//(Defined only by associated C file)

CONSTANT BYTE html_name_pop3_server[] = {"pop3_server"};
CONSTANT BYTE html_name_pop3_username[] = {"pop3_username"};
CONSTANT BYTE html_name_pop3_password[] = {"pop3_password"};
CONSTANT BYTE html_name_pop3_check_mail[] = {"pop3_check_mail"};
CONSTANT BYTE html_name_smtp_server[] = {"smtp_server"};
CONSTANT BYTE html_name_smtp_username[] = {"smtp_username"};
CONSTANT BYTE html_name_smtp_password[] = {"smtp_password"};
CONSTANT BYTE html_name_smtp_to[] = {"smtp_to"};
CONSTANT BYTE html_name_smtp_from[] = {"smtp_from"};
CONSTANT BYTE html_name_smtp_subject[] = {"smtp_subject"};
CONSTANT BYTE html_name_smtp_send_mail[] = {"smtp_send_mail"};

CONSTANT BYTE default_pop3_server[] = {"mail.some_domain.com"};	//These can't be longer than HTTP_MAX_INPUT_VALUE_LENGTH
CONSTANT BYTE default_pop3_username[] = {"me@some_domain.com"};
CONSTANT BYTE default_pop3_password[] = {"my_password"};

CONSTANT BYTE default_smtp_server[] = {"mail.some_domain.com"};	//These can't be longer than HTTP_MAX_INPUT_VALUE_LENGTH
CONSTANT BYTE default_smtp_username[] = {"me@some_domain.com"};
CONSTANT BYTE default_smtp_password[] = {"my_password"};
CONSTANT BYTE default_smtp_to[] = {"someone@some_domain.com"};
CONSTANT BYTE default_smtp_from[] = {"me@some_domain.com"};
CONSTANT BYTE default_smtp_subject[] = {"Hello from EC Device"};

#endif






