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
//SNTP (SIMPLE NETWORK TIME PROTOCOL) C CODE HEADER FILE



//##################################
//##################################
//########## USING DRIVER ##########
//##################################
//##################################

//#########################
//##### GET SNTP TIME #####
//#########################
/*
	//----- START SNTP GET TIME PROCESS -----
	sntp_get_time();


//----- SNTP GET TIME EVENTS FUNCTION -----
//We have set this function to be called in eth-sntp.h when we trigger the SNTP client.  Its called once when the SNTP request
//gets sent (after DNS and ARP lookup has occurred, in case we want to start a tight timeout timer) and once when the response
//is received.
void sntp_send_receive_handler (BYTE event, DWORD sntp_seconds_value)
{

	if (event == 1)
	{
		//----- JUST SENT SNTP REQUEST -----

	}
	else if (event == 2)
	{
		//----- JUST RECEIVED SNTP RESPONSE -----
		//sntp_seconds_value is SNTP server time in seconds

	}
}
*/



//For further information please see the project technical manual




//********************************
//********************************
//********** DO DEFINES **********
//********************************
//********************************
#ifndef ETH_SNTP_INIT		//Do only once the first time this file is used
#define	ETH_SNTP_INIT


//----- USER APPLICATION FUNCTION TO BE CALLED WHEN SNTP REQUEST IS SENT AND RESPONSE RECEIVED -----
#define	SNTP_USER_AP_FUNCTION			sntp_send_receive_handler
//Your function definition needs to be:
//	void sntp_send_receive_handler (BYTE event, DWORD sntp_seconds_value)
//event:
//	1 = sntp request has just been sent (allows you application to implemnt a timer to reject sntp responses that take too long if desired
//	2 = sntp response has just been received.  sntp_seconds_value contains the response value in seconds.
//	3 = sntp response failed (probably due to a timeout).
//You need to #include the file that contains your function definition at the top of eth-sntp.c for the compiler


//GENERAL
#define	SNTP_GENERAL_WAIT_TIMEOUT_x100MS		50
#define	SNTP_REFERENCE_EPOCH					0		//The server returns the time in seconds past a known reference epoch.  Default to NTP default of January 1, 1900, 00:00:00

//UDP PORTS
#define	SNTP_REMOTE_PORT					123

//SNTP TASK STATE MACHINE STATES
typedef enum _SNTP_STATE
{
    SM_SNTP_IDLE,
    SM_SNTP_GET_TIME,
    SM_SNTP_WAITING_DNS_RESPONSE,
    SM_SNTP_SEND_ARP_REQUEST,
    SM_SNTP_WAIT_FOR_ARP_RESPONSE,
    SM_SNTP_OPEN_UDP_SOCKET,
    SM_SNTP_TX,
    SM_SNTP_WAIT_FOR_RESPONSE,
    SM_SNTP_FAILED
} SNTP_STATE;

#endif


//----- SNTP SERVER TO CONNECT TO -----
#ifdef ETH_SNTP_C
CONSTANT BYTE sntp_server_domain_name[] = {"pool.ntp.org"};
#endif


//*******************************
//*******************************
//********** FUNCTIONS **********
//*******************************
//*******************************
#ifdef ETH_SNTP_C
//-----------------------------------
//----- INTERNAL ONLY FUNCTIONS -----
//-----------------------------------


//-----------------------------------------
//----- INTERNAL & EXTERNAL FUNCTIONS -----
//-----------------------------------------
//(Also defined below as extern)
BYTE sntp_get_time (void);
BYTE sntp_is_get_time_active (void);
void process_sntp (void);


#else
//------------------------------
//----- EXTERNAL FUNCTIONS -----
//------------------------------
extern BYTE sntp_get_time (void);
extern BYTE sntp_is_get_time_active (void);
extern void process_sntp (void);


#endif




//****************************
//****************************
//********** MEMORY **********
//****************************
//****************************
#ifdef ETH_SNTP_C
//--------------------------------------------
//----- INTERNAL ONLY MEMORY DEFINITIONS -----
//--------------------------------------------
BYTE sm_sntp;
BYTE sntp_100ms_timeout_timer;


//--------------------------------------------------
//----- INTERNAL & EXTERNAL MEMORY DEFINITIONS -----
//--------------------------------------------------
//(Also defined below as extern)


#else
//---------------------------------------
//----- EXTERNAL MEMORY DEFINITIONS -----
//---------------------------------------


#endif






