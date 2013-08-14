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
//SNTP (SIMPLE NETWORK TIME PROTOCOL) C CODE FILE


#include "main.h"					//Global data type definitions (see https://github.com/ibexuk/C_Generic_Header_File )
#define	ETH_SNTP_C
#include "eth-sntp.h"
#undef	ETH_SNTP_C

#include "eth-main.h"
#include "eth-udp.h"
#include "eth-arp.h"
#include "eth-nic.h"

#ifdef STACK_USE_DNS
#include "eth-dns.h"
#endif



//--------------------------------------------------------------------------
//----- GENERATE COMPILE ERROR IF SNTP HAS NOT BEEN DEFINED TO BE USED -----
//--------------------------------------------------------------------------
#ifndef STACK_USE_SNTP
#error SNTP file is included in project but not defined to be used - remove file from project to reduce code size.
#endif



#include "ap-main.h"			//Your application file that includes the definition for the function SNTP_USER_AP_FUNCTION



//******************************
//******************************
//********** GET TIME **********
//******************************
//******************************
//Returns:-
//	0 = unable to start process at the current time
//	1 = get time process started
BYTE sntp_get_time (void)
{

	//----- CHECK WE ARE CONNECTED -----
	if (!nic_linked_and_ip_address_valid)
		return(0);


	//----- CHECK WE'RE NOT ALREADY GETTING THE TIME -----
	if (sntp_is_get_time_active())
		return(0);

	//----- START RECEIVE PROCESS -----
	sntp_100ms_timeout_timer = SNTP_GENERAL_WAIT_TIMEOUT_x100MS;
	sm_sntp = SM_SNTP_GET_TIME;						//Trigger state machine to do the get time process

	return(1);
}



//******************************************************
//******************************************************
//********** ARE WE CURRRENTLY GETTING TIME ? **********
//******************************************************
//******************************************************
//Returns 1 is receive is active, 0 if not
BYTE sntp_is_get_time_active (void)
{

	if (sm_sntp == SM_SNTP_IDLE)
		return(0);
	else
		return(1);
}



//**********************************
//**********************************
//********** PROCESS SNTP **********
//**********************************
//**********************************
//This function is called reguarly by tcp_ip_process_stack
void process_sntp (void)
{
	static BYTE sntp_10ms_clock_timer_last;
	static DEVICE_INFO sntp_server_info;
	static BYTE sntp_udp_socket = UDP_INVALID_SOCKET;
	CONSTANT BYTE *p_const_string;
	BYTE domain_name[sizeof(sntp_server_domain_name)];
	BYTE count;
	BYTE data;
	DWORD_VAL sntp_time_seconds;
	

	//-----------------------------------
	//----- CHECK FOR UPDATE TIMERS -----
	//-----------------------------------
	if ((BYTE)((BYTE)(ethernet_10ms_clock_timer & 0x000000ff) - sntp_10ms_clock_timer_last) >= 10)
	{
		sntp_10ms_clock_timer_last = (BYTE)(ethernet_10ms_clock_timer & 0x000000ff);
		
		//OUR TIMEOUT TIMER
		if (sntp_100ms_timeout_timer)
			sntp_100ms_timeout_timer--;
	}

	if ((sm_sntp != SM_SNTP_IDLE) && (sm_sntp != SM_SNTP_FAILED))
	{
		//----------------------------------------------------
		//----- RECEIVE IS ACTIVE - DO BACKGROUND CHECKS -----
		//----------------------------------------------------
		
		//CHECK FOR RESPONSE TIMEOUT
		if (sntp_100ms_timeout_timer == 0)
			sm_sntp = SM_SNTP_FAILED;
		
		if (!nic_linked_and_ip_address_valid)
		{
			//WE ARE NOT CONNECTED OR DO NOT YET HAVE AN IP ADDRESS
			sm_sntp = SM_SNTP_FAILED;
		
			//Ensure our socket is closed if we have just lost the Ethernet connection
			udp_close_socket(&sntp_udp_socket);
			
			return;										//Exit as we can't do anything without a connection
		}
	}
	

	switch(sm_sntp)
	{
	case SM_SNTP_IDLE:
		//----------------
		//----------------
		//----- IDLE -----
		//----------------
		//----------------

		break;


	case SM_SNTP_GET_TIME:
		//--------------------
		//--------------------
		//----- GET TIME -----
		//--------------------
		//--------------------

		//GET THE IP ADDRESS OF THE SNTP SERVER
		count = 0;
		p_const_string = &sntp_server_domain_name[0];
		do
		{
			//Check for overflow
			if (count >= sizeof(domain_name))
			{
				domain_name[count - 1] = 0x00;
				break;
			}

			//Copy next byte
			domain_name[count++] = *p_const_string;
		}
		while (p_const_string++ != 0x00);
	
		if (!do_dns_query(&domain_name[0], QNS_QUERY_TYPE_HOST))
		{
			//DNS query not currently available - probably already doing a query - try again next time
			break;
		}
		
		//WAIT FOR DNS TO COMPLETE OUR QUERY
		sm_sntp = SM_SNTP_WAITING_DNS_RESPONSE;
		sntp_100ms_timeout_timer = SNTP_GENERAL_WAIT_TIMEOUT_x100MS;

		break;


	case SM_SNTP_WAITING_DNS_RESPONSE:
		//------------------------------------------------
		//------------------------------------------------
		//----- WAITING FOR SNTP SERVER DNS RESPONSE -----
		//------------------------------------------------
		//------------------------------------------------

		//SEE IF DNS RESPONSE HAS BEEN RECEIVED
		sntp_server_info.ip_address = check_dns_response();

		if (sntp_server_info.ip_address.Val == 0xffffffff)
		{
			//----- DNS QUERY FAILED -----
			sm_sntp = SM_SNTP_FAILED;
			break;
		}

		if (sntp_server_info.ip_address.Val)
		{
			//----- DNS QUERY SUCESSFUL -----
			sm_sntp = SM_SNTP_SEND_ARP_REQUEST;
		}
		break;	
			

	case SM_SNTP_SEND_ARP_REQUEST:
		//------------------------------------------------------------
		//------------------------------------------------------------
		//----- SEND ARP REQUEST FOR THE DNS RETURNED IP ADDRESS -----
		//------------------------------------------------------------
		//------------------------------------------------------------
		
		//Do ARP query to get the MAC address of the server or our gateway
		if (!arp_resolve_ip_address(&sntp_server_info.ip_address))
		{
			//ARP request cannot be sent right now - try again next time
			break;
		}
		sntp_100ms_timeout_timer = SNTP_GENERAL_WAIT_TIMEOUT_x100MS;
		sm_sntp = SM_SNTP_WAIT_FOR_ARP_RESPONSE;

		break;


	case SM_SNTP_WAIT_FOR_ARP_RESPONSE:
		//---------------------------------
		//---------------------------------
		//----- WAIT FOR ARP RESPONSE -----
		//---------------------------------
		//---------------------------------
		//Wait for the ARP response
		if (arp_is_resolve_complete(&sntp_server_info.ip_address, &sntp_server_info.mac_address))
		{
			//----- ARP RESPONSE RECEVIED -----
			sm_sntp = SM_SNTP_OPEN_UDP_SOCKET;
		}
		break;


	case SM_SNTP_OPEN_UDP_SOCKET:
		//---------------------------
		//---------------------------
		//----- OPEN UDP SOCKET -----
		//---------------------------
		//---------------------------
		sntp_udp_socket = udp_open_socket(&sntp_server_info, (WORD)SNTP_REMOTE_PORT, (WORD)SNTP_REMOTE_PORT);
		if (sntp_udp_socket != UDP_INVALID_SOCKET)
		{
			sm_sntp = SM_SNTP_TX;
			break;
		}
		//Could not open a socket - none currently available - keep trying
		break;


	case SM_SNTP_TX:
		//---------------------------
		//---------------------------
		//----- TX SNTP REQUEST -----
		//---------------------------
		//---------------------------

		//SETUP TX
		if (!udp_setup_tx(sntp_udp_socket))
		{
			//Can't tx right now - try again next time
			break;
		}

		//----- FLAGS [2] -----
		//LI (Bits 7:6)
		//	Leap Indicator (warning of an impending leap second to be inserted/deleted in the last minute of the current day)
		//VN (Bits 5:3)
		//	NTP/SNTP version number, currently 4
		//Mode (Bits 2:0)
		//	Protocol mode (3 = client, 4 = server)
   		udp_write_next_byte((0x04 << 3) | 0x03);

		//----- STRATUM [1] -----
		//This field is significant only in SNTP server messages
   		udp_write_next_byte(0x00);

		//----- POLL INTERVAL [1] -----
		//This field is significant only in SNTP server messages
   		udp_write_next_byte(0x04);

		//----- PRECISION [1] -----
		//This field is significant only in SNTP server messages
   		udp_write_next_byte(0x00);
		
		//----- ROOT DEAY [4] -----
		//signed fixed-point number of the total roundtrip delay to the primary reference source, in seconds with the fraction point between bits 15 and 16.
		//This field is significant only in server messages
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
		
		//----- ROOT DISPERSION [4] -----
   		//unsigned fixed-point number indicating the maximum error due to the clock frequency tolerance, in seconds with the fraction point between bits 15 and 16.
   		//This field is significant only in server messages
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);

		//----- REFERENCE IDENTIFIER [4] -----
		//Bitstring identifying the particular reference source.  This field is significant only in server messages
		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);

		//----- REFERENCE TIMESTAMP [8] -----
		//The time the system clock was last set or corrected, in 64-bit timestamp format.
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);

		//----- ORIGINATE TIMESTAMP [8] -----
		//The time at which the request departed the client for the server, in 64-bit timestamp format.
   		udp_write_next_byte((SNTP_REFERENCE_EPOCH & 0xff000000) >> 24);
   		udp_write_next_byte((SNTP_REFERENCE_EPOCH & 0x00ff0000) >> 16);
   		udp_write_next_byte((SNTP_REFERENCE_EPOCH & 0x0000ff00) >> 8);
   		//udp_write_next_byte(SNTP_REFERENCE_EPOCH & 0x000000ff);
   		udp_write_next_byte(0x01);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);

		//----- RECEIVE TIMESTAMP [8] -----
		//The time at which the request arrived at the server or the reply arrived at the client, in 64-bit timestamp format.
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);

		//----- TRANSMIT TIMESTAMP [8] -----
		//The time at which the request departed the client or the reply departed the server, in 64-bit timestamp format.
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);
   		udp_write_next_byte(0x00);

		//SEND THE PACKET
		udp_tx_packet();

		//CALL USER APPLICATION FUNCTION AT MOMENT REQUEST IS SENT (in case user ap wan'ts to implement a tight timeout)
		SNTP_USER_AP_FUNCTION(1, 0);

		sntp_100ms_timeout_timer = SNTP_GENERAL_WAIT_TIMEOUT_x100MS;
		sm_sntp = SM_SNTP_WAIT_FOR_RESPONSE;
		break;

	case SM_SNTP_WAIT_FOR_RESPONSE:
		//-----------------------------
		//-----------------------------
		//----- WAIT FOR RESPONSE -----
		//-----------------------------
		//-----------------------------
		if (udp_check_socket_for_rx(sntp_udp_socket))
		{
			//SOCKET HAS RECEIVED A PACKET - PROCESS IT


			//----- FLAGS [2] -----
			//LI (Bits 7:6)
			//	Leap Indicator (warning of an impending leap second to be inserted/deleted in the last minute of the current day)
			//VN (Bits 5:3)
			//	NTP/SNTP version number, currently 4
			//Mode (Bits 2:0)
			//	Protocol mode (3 = client, 4 = server)
	   		udp_read_next_rx_byte(&data);
	
			//----- STRATUM [1] -----
			//This field is significant only in SNTP server messages
	   		udp_read_next_rx_byte(&data);
	
			//----- POLL INTERVAL [1] -----
			//This field is significant only in SNTP server messages
	   		udp_read_next_rx_byte(&data);
	
			//----- PRECISION [1] -----
			//This field is significant only in SNTP server messages
	   		udp_read_next_rx_byte(&data);
			
			//----- ROOT DEAY [4] -----
			//signed fixed-point number of the total roundtrip delay to the primary reference source, in seconds with the fraction point between bits 15 and 16.
			//This field is significant only in server messages
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
			
			//----- ROOT DISPERSION [4] -----
	   		//unsigned fixed-point number indicating the maximum error due to the clock frequency tolerance, in seconds with the fraction point between bits 15 and 16.
	   		//This field is significant only in server messages
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	
			//----- REFERENCE IDENTIFIER [4] -----
			//Bitstring identifying the particular reference source.  This field is significant only in server messages
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	
			//----- REFERENCE TIMESTAMP [8] -----
			//The time the system clock was last set or corrected, in 64-bit timestamp format.
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	
			//----- ORIGINATE TIMESTAMP [8] -----
			//The time at which the request departed the client for the server, in 64-bit timestamp format.
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	
			//----- RECEIVE TIMESTAMP [8] -----
			//The time at which the request arrived at the server or the reply arrived at the client, in 64-bit timestamp format.
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	
			//----- TRANSMIT TIMESTAMP [8] -----
			//The time at which the request departed the client or the reply departed the server, in 64-bit timestamp format.
			if (!udp_read_next_rx_byte(&sntp_time_seconds.v[3]))
				sm_sntp = SM_SNTP_FAILED;						//Error - no more bytes in rx packet
			if (!udp_read_next_rx_byte(&sntp_time_seconds.v[2]))
				sm_sntp = SM_SNTP_FAILED;						//Error - no more bytes in rx packet
			if (!udp_read_next_rx_byte(&sntp_time_seconds.v[1]))
				sm_sntp = SM_SNTP_FAILED;						//Error - no more bytes in rx packet
			if (!udp_read_next_rx_byte(&sntp_time_seconds.v[0]))
				sm_sntp = SM_SNTP_FAILED;						//Error - no more bytes in rx packet
	   		
	   		udp_read_next_rx_byte(&data);						//If the fraction > 500mS then round up
	   		if (data & 0x80)
	   			sntp_time_seconds.Val++;
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
	   		udp_read_next_rx_byte(&data);
			
			if (sntp_time_seconds.Val && (sm_sntp != SM_SNTP_FAILED))		//If value is 0x00 then its invalid
			{
				//CALL USER APPLICATION FUNCTION AT MOMENT RESPONSE IS RECEIVED (to reduce latency to a minimum)
				SNTP_USER_AP_FUNCTION(2, sntp_time_seconds.Val);
			}

			//DUMP THE PACKET
			udp_dump_rx_packet();
			
			//EXIT
			udp_close_socket(&sntp_udp_socket);
			sm_sntp = SM_SNTP_IDLE;
		}
		
		break;


	case SM_SNTP_FAILED:
		//-----------------------------------
		//-----------------------------------
		//----- GET TIME PROCESS FAILED -----
		//-----------------------------------
		//-----------------------------------
		sm_sntp = SM_SNTP_IDLE;

		//CALL USER APPLICATION FUNCTION TO NOTIFY IT OF THE FAILURE
		SNTP_USER_AP_FUNCTION(3, 0);

		break;
		
	}

	return;
}




	









