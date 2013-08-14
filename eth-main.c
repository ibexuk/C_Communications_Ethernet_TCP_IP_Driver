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
//MAIN STACK FUNCTIONS C CODE FILE


#include "main.h"					//Global data type definitions (see https://github.com/ibexuk/C_Generic_Header_File )
#define	ETH_MAIN_C
#include "eth-main.h"
#undef	ETH_MAIN_C

#include "eth-nic.h"
#include "eth-arp.h"
#include "eth-ip.h"

#ifdef STACK_USE_ICMP
#include "eth-icmp.h"
#endif

#ifdef STACK_USE_UDP
#include "eth-udp.h"
#endif

#ifdef STACK_USE_TCP
#include "eth-tcp.h"
#endif

#ifdef STACK_USE_DHCP
#include "eth-dhcp.h"
#endif

#ifdef STACK_USE_HTTP
#include "eth-http.h"
#endif

#ifdef STACK_USE_HTTP_CLIENT
#include "eth-http-client.h"
#endif

#ifdef STACK_USE_POP3
#include "eth-pop3.h"
#endif

#ifdef STACK_USE_SMTP
#include "eth-smtp.h"
#endif

#ifdef STACK_USE_DNS
#include "eth-dns.h"
#endif

#ifdef STACK_USE_NETBIOS
#include "eth-netbios.h"
#endif

#ifdef STACK_USE_SNTP
#include "eth-sntp.h"
#endif


//*****************************************
//*****************************************
//********** INITIALISE ETHERNET **********
//*****************************************
//*****************************************
//(This function must be called before any of the stack or its component routines are used.
void tcp_ip_initialise (void)
{


	//----- INITIALSIE THE NIC -----
	nic_initialise(NIC_INIT_SPEED);				//Speed value sets either 10/100Mbps or 10Mbps speed for nic's support this option


	//----- INITIALISE ARP -----
	arp_initialise();


	//----- INITIALISE UDP -----
	#ifdef STACK_USE_UDP
		udp_initialise();
	#endif


	//----- INITIALISE DHCP -----
	#ifdef STACK_USE_DHCP
		dhcp_initialise();
	#else
		nic_linked_and_ip_address_valid = 1;				//No DHCP in use so flag that our IP address is OK
	#endif


	//----- INITIALISE TCP -----
	#if defined(STACK_USE_TCP)
		tcp_initialise();
	#endif


	//----- INITIALISE HTTP -----
	#if defined(STACK_USE_HTTP)
		http_initialise();
	#endif


	//----- INITIALISE THE STACK -----
	sm_ethernet_stack = SM_ETH_STACK_IDLE;
}




//***************************************
//***************************************
//********** PROCESS THE STACK **********
//***************************************
//***************************************
//This state machine checks for new incoming packets, and routes them to appropriate stack functions.  It also performs timed operations.
//This function must be called called reguarly (typically as part of the main loop of your application).
void tcp_ip_process_stack (void)
{
	static DEVICE_INFO remote_device_info;
	static MAC_ADDR rx_destination_mac_address;
	static WORD rx_len;
    static WORD data_remaining_bytes;
	static IP_ADDR destination_ip_address;
	BYTE ip_protocol;
	WORD_VAL rx_ethernet_packet_type;
	BYTE process_stack_again;

#ifdef STACK_USE_ICMP
	static BYTE icmp_data_buffer[ICMP_MAX_DATA_LENGTH];
	static WORD icmp_id;
	static WORD icmp_sequence;
#endif



	//-----------------------------------------------
	//----- GET THE CURRENT ETHERNET TIMER TIME -----
	//-----------------------------------------------
	ethernet_10ms_clock_timer = ethernet_10ms_clock_timer_working;
	//CHECK THAT TIMER HASN'T JUST CHANGED IN WHICH CASE WE COULD HAVE READ A CORRUPT TIME AS THE VALUE IS PROBABLY CHANGED IN AN INTERRUPT
	while (ethernet_10ms_clock_timer != ethernet_10ms_clock_timer_working)
		ethernet_10ms_clock_timer = ethernet_10ms_clock_timer_working;


	//------------------------------------------------
	//----- IF NIC IS NOT LINKED RESET THE STACK -----
	//------------------------------------------------
	if (!nic_is_linked)
		sm_ethernet_stack = SM_ETH_STACK_IDLE;

	//-------------------------------------------------------------
	//----- IF DHCP IS NOT INCLUDED THEN DO DHCP GENERAL FLAG -----
	//-------------------------------------------------------------
	#ifndef STACK_USE_DHCP
		if (nic_is_linked)
			nic_linked_and_ip_address_valid = 1;
		else
			nic_linked_and_ip_address_valid = 0;
	#endif

	//------------------------------------------
	//----- CHECK FOR DUMP RECEIVED PACKET -----
	//------------------------------------------
	//(Backup in case an application function has opened a socket but doesn't process a received packet for that socket for some reason)
	if (nic_rx_packet_waiting_to_be_dumped)
	{
		nic_rx_packet_waiting_to_be_dumped--;
		if (nic_rx_packet_waiting_to_be_dumped == 0)
		{
			#ifdef STACK_USE_UDP
				udp_rx_packet_is_waiting_to_be_processed = 0;
			#endif
			
			#ifdef STACK_USE_TCP
				tcp_rx_packet_is_waiting_to_be_processed = 0;
			#endif
			
			nic_rx_dump_packet();
		}
	}


	//-----------------------------
	//-----------------------------
	//----- PROCESS THE STACK -----
	//-----------------------------
	//-----------------------------
	process_stack_again = 1;
	while(process_stack_again)
	{
		process_stack_again = 0;			//Default to no need to run through the stack checks again for a subsequent task

        switch(sm_ethernet_stack)
        {
		case SM_ETH_STACK_IDLE:
			//-----------------------------
        	//-----------------------------
        	//----- THE STACK IS IDLE -----
        	//-----------------------------
        	//-----------------------------

			//----- CHECK FOR A PACKET RECEIVED (AND DO ANY OTHER NIC BACKGROUND TASKS) -----
			//(This must happen reguarly)
			rx_len = nic_check_for_rx();
	
			if (rx_len == 0)
			{
				//-----------------------------------------------
				//----- THERE IS NO RECEIVED PACKET WAITING -----
				//-----------------------------------------------

				break;
			}
			else
			{
	            //-------------------------------------
				//----- NIC HAS RECEIVED A PACKET -----
				//-------------------------------------
				//(The nic function has read the packet length from the nic IC and the next byte to read is the first byte of the data area)
				
				process_stack_again = 1;				//Process the stack again for this received packet or if this packet is dumped for any other packet that may be waiting

				//----- GET THE ETHERNET HEADER -----
				//Get the destination mac address [6]
				nic_read_next_byte(&rx_destination_mac_address.v[0]);
				nic_read_next_byte(&rx_destination_mac_address.v[1]);
				nic_read_next_byte(&rx_destination_mac_address.v[2]);
				nic_read_next_byte(&rx_destination_mac_address.v[3]);
				nic_read_next_byte(&rx_destination_mac_address.v[4]);
				nic_read_next_byte(&rx_destination_mac_address.v[5]);

				//Get the source mac address [6]
				nic_read_next_byte(&remote_device_info.mac_address.v[0]);
				nic_read_next_byte(&remote_device_info.mac_address.v[1]);
				nic_read_next_byte(&remote_device_info.mac_address.v[2]);
				nic_read_next_byte(&remote_device_info.mac_address.v[3]);
				nic_read_next_byte(&remote_device_info.mac_address.v[4]);
				nic_read_next_byte(&remote_device_info.mac_address.v[5]);

				//Get the header type [2]
				nic_read_next_byte(&rx_ethernet_packet_type.v[1]);
				nic_read_next_byte(&rx_ethernet_packet_type.v[0]);
				

				//----- DECIDE WHAT TO DO FROM THE PACKET TYPE VALUE -----
				if (rx_ethernet_packet_type.Val == 0x0806)
				{
					//-------------------------
					//----- PACKET IS ARP -----
					//-------------------------
	                sm_ethernet_stack = SM_ETH_STACK_ARP;
				}
				else if (rx_ethernet_packet_type.Val == 0x0800)
				{
					//------------------------
					//----- PACKET IS IP -----
					//------------------------
	                sm_ethernet_stack = SM_ETH_STACK_IP;
				}

				//<<<< ADD ANY OTHER ETHERNET PACKET TYPES HERE
			
				else
				{
					//----------------------------------
					//----- PACKET TYPE IS UNKNOWN -----
					//----------------------------------
					nic_rx_dump_packet();
				}
				break;
			}


		case SM_ETH_STACK_ARP:
			//---------------------------------------
			//---------------------------------------
			//----- PROCESS RECEIVED ARP PACKET -----
			//---------------------------------------
			//---------------------------------------
			if (arp_process_rx())
			{
				//----- ARP PACKET HAS BEEN PROCESSED -----
				sm_ethernet_stack = SM_ETH_STACK_IDLE;
			}
			//else
			//{
				//----- ARP PACKET HAS BEEN READ BUT ARP COULDN'T TRANSMIT A RESPONSE RIGHT NOW - WE'LL KEEP CALLING ARP_PROCESS_RX UNTIL IT CAN -----
			//}
			nic_rx_dump_packet();		//Ensure packet has been dumped
			break;


		case SM_ETH_STACK_IP:
			//--------------------------------------
			//--------------------------------------
			//----- PROCESS RECEIVED IP PACKET -----
			//--------------------------------------
			//--------------------------------------
			
			//----- GET THE IP HEADER -----
			if (ip_get_header(&destination_ip_address, &remote_device_info, &ip_protocol, &data_remaining_bytes))
			{
				//-------------------------------
				//----- IP HEADER RETREIVED -----
				//-------------------------------
				process_stack_again = 1;		//Process the stack again for this packet once we've set the state to handle it below
				
				if (ip_protocol == IP_PROTOCOL_ICMP)
				{
					//----- PACKET IS ICMP -----
					sm_ethernet_stack = SM_ETH_STACK_ICMP;
				}

				#ifdef STACK_USE_UDP
				else if (ip_protocol == IP_PROTOCOL_UDP)
				{
					//----- PACKET IS UDP -----
					sm_ethernet_stack = SM_ETH_STACK_UDP;
				}
				#endif

				#ifdef STACK_USE_TCP
				else if (ip_protocol == IP_PROTOCOL_TCP)
				{
					//----- PACKET IS TCP -----
					sm_ethernet_stack = SM_ETH_STACK_TCP;
				}
				#endif

				//<<<<< ADD ANY OTHER IP PROTOCOL TYPES HERE

				else
				{
					//----- UNKNOWN IP PACKET TYPE -----
					nic_rx_dump_packet();
					process_stack_again = 0;
					sm_ethernet_stack = SM_ETH_STACK_IDLE;
				}
			}
			else
			{
				//-------------------------
				//----- BAD IP HEADER -----
				//-------------------------
				//(The packet has already been dumped by ip_get_header)
				sm_ethernet_stack = SM_ETH_STACK_IDLE;
			}
			break;


		case SM_ETH_STACK_ICMP:
			//----------------------------------------
			//----------------------------------------
			//----- PROCESS RECEIVED ICMP PACKET -----
			//----------------------------------------
			//----------------------------------------
			//(ICMP is used by 'ping')
			#ifdef STACK_USE_ICMP
				//Check the packet is for us (broadcast is not allowed)
				if (destination_ip_address.Val == our_ip_address.Val)
				{
					//Process the packet as long as its length is OK
					if (data_remaining_bytes <= (ICMP_MAX_DATA_LENGTH + ICMP_HEADER_LENGTH))
					{
						if (icmp_process_received_echo_request(&icmp_id, &icmp_sequence, &icmp_data_buffer[0], data_remaining_bytes))
						{
							//----- VALID ICMP ECHO REQUEST RECEIVED -----
							process_stack_again = 1;
							sm_ethernet_stack = SM_ETH_STACK_ICMP_REPLY;			//(Will be processed when loop runs again)
						}
						else
						{
							//The request was not valid
							sm_ethernet_stack = SM_ETH_STACK_IDLE;
						}
					}
					else
					{
						//Packet is too long
						sm_ethernet_stack = SM_ETH_STACK_IDLE;
					}
				}
				else
				{
					//Packet was not sent to our IP address so ignore it
					sm_ethernet_stack = SM_ETH_STACK_IDLE;
				}
			#else
				sm_ethernet_stack = SM_ETH_STACK_IDLE;
			#endif
				
				//Dump the packet if it hasn't been already
				nic_rx_dump_packet();
				break;


		case SM_ETH_STACK_ICMP_REPLY:
			//--------------------------------
			//--------------------------------
			//----- SEND ICMP ECHO REPLY -----
			//--------------------------------
			//--------------------------------
			#ifdef STACK_USE_ICMP
				if (nic_setup_tx())
				{
					//----- WE CAN TRANSMIT OUR RESPONSE NOW -----
					icmp_send_packet(&remote_device_info,ICMP_ECHO_REPLY, &icmp_data_buffer[0], (data_remaining_bytes - ICMP_HEADER_LENGTH),
									&icmp_id, &icmp_sequence);
	
					sm_ethernet_stack = SM_ETH_STACK_IDLE;
					break;;
				}
				else
				{
					//----- CAN'T TX OUR RESPONSE NOW - TRY AGAIN NEXT TIME -----
					break;
				}
			#else
				sm_ethernet_stack = SM_ETH_STACK_IDLE;
			#endif
	
			break;


        case SM_ETH_STACK_UDP:
			//---------------------------------------
			//---------------------------------------
			//----- PROCESS RECEIVED UDP PACKET -----
			//---------------------------------------
			//---------------------------------------
			#ifdef STACK_USE_UDP
				if (udp_process_rx(&remote_device_info, &destination_ip_address, data_remaining_bytes))
				{
					//----- UDP PACKET HAS BEEN PROCESSED -----
					sm_ethernet_stack = SM_ETH_STACK_IDLE;
				}
				//else
				//{
					//----- UDP PACKET IS PENDING PROCESSING BY THE APPLICATION PROCESS USING THE SOCKET -----
					//Wait until the packet has been processed and dumped
				//}
				process_stack_again = 0;
				break;
			#else
				sm_ethernet_stack = SM_ETH_STACK_IDLE;
			#endif
			
			break;


        case SM_ETH_STACK_TCP:
			//---------------------------------------
			//---------------------------------------
			//----- PROCESS RECEIVED TCP PACKET -----
			//---------------------------------------
			//---------------------------------------
			#ifdef STACK_USE_TCP
				if (tcp_process_rx(&remote_device_info, &destination_ip_address, data_remaining_bytes))
				{
					//----- TCP PACKET HAS BEEN PROCESSED -----
					sm_ethernet_stack = SM_ETH_STACK_IDLE;
				}
				//else
				//{
					//----- TCP PACKET IS PENDING PROCESSING BY THE APPLICATION PROCESS USING THE SOCKET -----
					//Wait until the packet has been processed and dumped
				//}
				process_stack_again = 0;
				break;
			#else
				sm_ethernet_stack = SM_ETH_STACK_IDLE;
			#endif

			break;






		//-----------------------------------------
		//-----------------------------------------
		//----- END OF STACK SWITCH STATEMENT -----
		//-----------------------------------------
		//-----------------------------------------
		}	//switch(sm_ethernet_stack)

		//----------------------------------
		//----------------------------------
		//----- END OF WHILE STATEMENT -----
		//----------------------------------
		//----------------------------------
	}	//while(process_stack_again)



	//-----------------------------
	//-----------------------------
	//----- DO DHCP PROCESSES -----
	//-----------------------------
	//-----------------------------
	//DHCP needs to be called periodically to deal with lease renewal
	#ifdef STACK_USE_DHCP
	   process_dhcp();
	#endif


	//----------------------------
	//----------------------------
	//----- DO TCP PROCESSES -----
	//----------------------------
	//----------------------------
	//TCP NEEDS TO BE PROCESSED REGUARLY TO DEAL WITH TIMEOUTS ETC
	#ifdef STACK_USE_TCP
		process_tcp();
	#endif


	//-----------------------------
	//-----------------------------
	//----- DO HTTP PROCESSES -----
	//-----------------------------
	//-----------------------------
	//HTTP NEEDS TO BE PROCESSED REGUARLY TO DEAL WITH BACKGROUND TASKS
	#ifdef STACK_USE_HTTP
		process_http();
	#endif

	#ifdef STACK_USE_HTTP_CLIENT
		process_http_client();
	#endif


	//------------------------------
	//------------------------------
	//----- DO EMAIL PROCESSES -----
	//------------------------------
	//------------------------------
	//EMAIL NEEDS TO BE PROCESSED REGUARLY TO SERVICE ANY SEND OR RECEIVE OF EMAIL
	#ifdef STACK_USE_POP3
		email_process_pop3();
	#endif

	#ifdef STACK_USE_SMTP
		email_process_smtp();
	#endif


	//----------------------------
	//----------------------------
	//----- DO DNS PROCESSES -----
	//----------------------------
	//----------------------------
	//DNS NEEDS TO BE PROCESSED REGUARLY TO SERVICE ANY ACTIVE DNS REQUESTS
	#ifdef STACK_USE_DNS
		process_dns();
	#endif


	//--------------------------------
	//--------------------------------
	//----- DO NETBIOS PROCESSES -----
	//--------------------------------
	//--------------------------------
	//NETBIOS NEEDS TO BE PROCESSED REGUARLY TO SERVICE ANY ACTIVE INCOMING NETBIOS REQUESTS
	#ifdef STACK_USE_NETBIOS
		process_netbios_nameservice();
	#endif


	//-----------------------------
	//-----------------------------
	//----- DO SNTP PROCESSES -----
	//-----------------------------
	//-----------------------------
	//SNTP NEEDS TO BE PROCESSED REGUARLY TO SERVICE ANY ACTIVE SNTP REQUESTS
	#ifdef STACK_USE_SNTP
		process_sntp();
	#endif


}








//******************************************
//******************************************
//********** SWAP BYTES IN A WORD **********
//******************************************
//******************************************
WORD swap_word_bytes (WORD data)
{
	WORD_VAL new_data;
	BYTE b_temp;

	new_data.Val = data;
    b_temp = new_data.v[1];
    new_data.v[1] = new_data.v[0];
    new_data.v[0] = b_temp;

    return (new_data.Val);
}



//*******************************************
//*******************************************
//********** SWAP BYTES IN A DWORD **********
//*******************************************
//*******************************************
DWORD swap_dword_bytes (DWORD data)
{
	DWORD_VAL old_data;
	DWORD_VAL new_data;
	
	old_data.Val = data;
	
	new_data.v[0] = old_data.v[3];
	new_data.v[1] = old_data.v[2];
	new_data.v[2] = old_data.v[1];
	new_data.v[3] = old_data.v[0];

    return (new_data.Val);
}


//************************************************
//************************************************
//********** CONVERT BYTE TO LOWER CASE **********
//************************************************
//************************************************
BYTE convert_character_to_lower_case (BYTE character)
{
	if ((character >= 'A') && (character <= 'Z'))
		return(character + 0x20);
	else
		return(character);
}	



//************************************************
//************************************************
//********** CONVERT BYTE TO UPPER CASE **********
//************************************************
//************************************************
BYTE convert_character_to_upper_case (BYTE character)
{
	if ((character >= 'a') && (character <= 'z'))
		return(character - 0x20);
	else
		return(character);
}	



//**************************************************
//**************************************************
//********** CONVERT STRING TO LOWER CASE **********
//**************************************************
//**************************************************
//Returns pointer to the character that is currently the terminating null
BYTE* convert_string_to_lower_case (BYTE *string_to_convert)
{
	while (*string_to_convert != 0x00)
	{
		if ((*string_to_convert >= 'A') && (*string_to_convert <= 'Z'))
			*string_to_convert++ += 0x20;
		else
			string_to_convert++;
	}
	return(string_to_convert);	
}



//*********************************************************************
//*********************************************************************
//********** FIND FIRST OCCURANCE OF CHARACTER WITHIN STRING **********
//*********************************************************************
//*********************************************************************
//Looks for the first occurance of a character within a variable string
//Returns pointer to the character or null
BYTE* find_character_in_string (BYTE *examine_string, BYTE character)
{
	while (*examine_string != 0x00)
	{
		if (*examine_string++ == character)
		{
			//----- CHARACTER FOUND -----
			return (examine_string - 1);
		}
	}	

	//----- NOT FOUND - RETURN NULL -----
	return(0);
}



//********************************************************************
//********************************************************************
//********** FIND FIRST OCCURANCE OF STRING WITHIN A STRING **********
//********************************************************************
//********************************************************************
//Looks for the first occurance of a constant string within a variable string
//Returns character number of first character when found, or 0 if not found
//Compare is case insensitive
BYTE* find_string_in_string_no_case (BYTE *examine_string, CONSTANT BYTE *looking_for_string)
{
	BYTE number_of_characters_matched = 0;
	BYTE character1;
	BYTE character2;
	
	while (*examine_string != 0x00)
	{
		//----- CHECK NEXT CHARACTER -----
		character1 = *examine_string++;
		if ((character1 >= 'a') && (character1 <= 'z'))	//Convert to uppercase if lowercase
			character1 -= 0x20;
		
		character2 = *looking_for_string;
		if ((character2 >= 'a') && (character2 <= 'z'))	//Convert to uppercase if lowercase
			character2 -= 0x20;
		
		if (character1 == character2)
		{
			//----- THIS CHARACTER MATCHES -----
			looking_for_string++;
			number_of_characters_matched++;
			
			if (*looking_for_string == 0x00)
			{
				//----- GOT TO NULL OF STRING TO FIND - SUCCESS - STRING FOUND -----
				return(examine_string - number_of_characters_matched);
			}
		}
		else if (number_of_characters_matched)
		{
			looking_for_string -= number_of_characters_matched;		//Return string being looked for pointer back to start - the match was not completed
			number_of_characters_matched = 0;
		}	
	}
	
	//----- NOT FOUND -----
	return(0);

}



//***************************************
//***************************************
//********** COPY A RAM STRING **********
//***************************************
//***************************************
//Returns pointer to the terminating null character of the destination string
BYTE* copy_ram_string_to_ram_string (BYTE *destination_string, BYTE *source_string)
{
	while (*source_string != 0x00)
	{
		*destination_string++ = *source_string++;
	}
	
	*destination_string = 0x00;		//Add terminating null
	
	return(destination_string);
}



//***************************************************
//***************************************************
//********** CONVERT ASCII TEXT TO INTEGER **********
//***************************************************
//***************************************************
WORD convert_ascii_to_integer (BYTE *source_string)
{
	WORD value = 0;
	WORD value_last = 0;
	
	while ((*source_string >= '0') && (*source_string <= '9'))
	{
		value_last = value;
		
		value *= 10;
		value += (*source_string++ - 0x30);
		
		if (value_last > value)
			return(0xffff);				//Value is > 65535
	}
	return (value);
}


//*************************************************
//*************************************************
//********** CONVERT ASCII TEXT TO DWORD **********
//*************************************************
//*************************************************
DWORD convert_ascii_to_dword (BYTE *source_string)
{
	DWORD value = 0;
	DWORD value_last = 0;
	
	while ((*source_string >= '0') && (*source_string <= '9'))
	{
		value_last = value;
		
		value *= 10;
		value += (*source_string++ - 0x30);
		
		if (value_last > value)
			return(0xffff);				//Value is > 0xffffffff
	}
	return (value);
}


//************************************************
//************************************************
//********** CONVERT WORD TO ASCII TEXT **********
//************************************************
//************************************************
//Returns pointer to the character that is currently the terminating null
BYTE* convert_word_to_ascii (WORD value, BYTE *dest_string)
{
	WORD value_working;
	BYTE string_length = 0;
	BYTE b_count;
	BYTE character;

	if (value == 0)
		*dest_string++ = '0';

	while (value)
	{
		//We have another character to add so shift string right ready for it
		for (b_count = 0; b_count < string_length; b_count++)
		{
			dest_string--;
			character = *dest_string++;
			*dest_string-- = character;
		}
		
		value_working = value / 10;
		*dest_string++ = (value - (value_working * 10)) + 0x30;
		dest_string += string_length;
		
		value = value_working;
		string_length++;
	}
	
	*dest_string = 0x00;		//Add the terminating null
	return(dest_string);
}



//*************************************************
//*************************************************
//********** CONVERT DWORD TO ASCII TEXT **********
//*************************************************
//*************************************************
//Returns pointer to the character that is currently the terminating null
BYTE* convert_dword_to_ascii (DWORD value, BYTE *dest_string)
{
	DWORD value_working;
	BYTE string_length = 0;
	BYTE b_count;
	BYTE character;

	if (value == 0)
		*dest_string++ = '0';

	while (value)
	{
		//We have another character to add so shift string right ready for it
		for (b_count = 0; b_count < string_length; b_count++)
		{
			dest_string--;
			character = *dest_string++;
			*dest_string-- = character;
		}
		
		value_working = value / 10;
		*dest_string++ = (value - (value_working * 10)) + 0x30;
		dest_string += string_length;
		
		value = value_working;
		string_length++;
	}
	
	*dest_string = 0x00;		//Add the terminating null
	return(dest_string);
}

//***************************************************************
//***************************************************************
//********** CONVERT 2 CHARACTERS OF HEX ASCII TO BYTE **********
//***************************************************************
//***************************************************************
//Case insensitive
BYTE convert_ascii_hex_to_byte (BYTE high_char, BYTE low_char)
{
	BYTE value = 0;
	
	//Ensure character are uppercase
	high_char = convert_character_to_upper_case(high_char);
	low_char = convert_character_to_upper_case(low_char);
	
	if (high_char > '9')
		value = (high_char - ('A' - 10)) << 4;
	else
		value = (high_char - '0') << 4;

	if (low_char > '9')
		value |= (low_char - ('A' - 10));
	else
		value |= (low_char - '0');
	
	return(value);
}
