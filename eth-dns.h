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
//DNS (DOMAIN NAME SYSTEM) C CODE HEADER FILE


//##################################
//##################################
//########## USING DRIVER ##########
//##################################
//##################################
//DNS requests are automatically dealt with by the stack, for instance when connecting to a POP3 or SMTP server.
//No user application intervention is required.  The following example is shown in case you want to use DNS for a
//specific reason in your application.

/*
IP_ADDR dns_resolved_ip_address;

	//----- START NEW DNS QUERY -----
	if (!do_dns_query(temp_string, QNS_QUERY_TYPE_HOST))
	{
		//DNS query not currently available (already doing a query) - try again next time
	}

	//----- IS DNS QUERY IS COMPLETE? -----
	//(Do this periodically until complete)
	dns_resolved_ip_address = check_dns_response();
	if (dns_resolved_ip_address.Val == 0xffffffff)
	{
		//DNS QUERY FAILED
		//(Timed out or invalid response)
	}
	else if (dns_resolved_ip_address.Val)
	{
		//DNS QUERY SUCESSFUL
		my_remote_device_ip_address.Val = dns_resolved_ip_address.Val;		//Store the IP address
	}
	else
	{
		//DNS NOT YET COMPLETE
	}
*/


//For further information please see the project technical manual




//********************************
//********************************
//********** DO DEFINES **********
//********************************
//********************************
#ifndef ETH_DNS_C_INIT		//Do only once the first time this file is used
#define	ETH_DNS_C_INIT


#include "eth-main.h"
#include "eth-udp.h"


#define	DNS_GATEWAY_DO_ARP_TIMEOUT_x100MS	40
#define	DNS_DO_REQUEST_TIMEOUT_x100MS		100		//Allow time for a router to connect to the internet if necessary

#define	DNS_MAX_URL_LENGTH					64

//UDP PORTS
#define	DNS_CLIENT_PORT						53
#define	DNS_SERVER_PORT						53

//QUERY TYPE VALUES
#define	QNS_QUERY_TYPE_HOST		1
#define	QNS_QUERY_TYPE_MX		15

typedef enum _SM_DNS
{
    SM_DNS_IDLE,
    SM_DNS_WAITING_TO_SEND,
    SM_DNS_WAITING_FOR_ARP_RESPONSE,
    SM_DNS_SEND_REQUEST,
    SM_DNS_WAITING_FOR_DNS_RESPONSE,
    SM_DNS_SUCCESS,
    SM_DNS_FAILED
} SM_DNS;

#endif







//*******************************
//*******************************
//********** FUNCTIONS **********
//*******************************
//*******************************
#ifdef ETH_DNS_C
//-----------------------------------
//----- INTERNAL ONLY FUNCTIONS -----
//-----------------------------------
void dns_send_request (BYTE *requested_domain_name, BYTE qtype);
BYTE dns_check_response(BYTE *requested_domain_name, BYTE qtype, IP_ADDR *resolved_ip_address);


//-----------------------------------------
//----- INTERNAL & EXTERNAL FUNCTIONS -----
//-----------------------------------------
//(Also defined below as extern)
BYTE do_dns_query (BYTE *url_pointer, BYTE dns_type);
IP_ADDR check_dns_response (void);
void process_dns (void);


#else
//------------------------------
//----- EXTERNAL FUNCTIONS -----
//------------------------------
extern BYTE do_dns_query (BYTE *url_pointer, BYTE dns_type);
extern IP_ADDR check_dns_response (void);
extern void process_dns (void);


#endif




//****************************
//****************************
//********** MEMORY **********
//****************************
//****************************
#ifdef ETH_DNS_C
//--------------------------------------------
//----- INTERNAL ONLY MEMORY DEFINITIONS -----
//--------------------------------------------
BYTE dns_100ms_timeout_timer;
BYTE  dns_state = SM_DNS_IDLE;
BYTE dns_udp_socket = UDP_INVALID_SOCKET;
DEVICE_INFO DNSServerNode;
BYTE dns_requested_qtype;
IP_ADDR dns_resolved_ip_address;
BYTE dns_requested_url[DNS_MAX_URL_LENGTH];


//--------------------------------------------------
//----- INTERNAL & EXTERNAL MEMORY DEFINITIONS -----
//--------------------------------------------------
//(Also defined below as extern)


#else
//---------------------------------------
//----- EXTERNAL MEMORY DEFINITIONS -----
//---------------------------------------


#endif





