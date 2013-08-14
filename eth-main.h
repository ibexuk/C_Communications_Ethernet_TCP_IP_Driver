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
//MAIN STACK FUNCTIONS C CODE HEADER FILE





//##################################
//##################################
//########## USING DRIVER ##########
//##################################
//##################################

//################################################
//##### IN THE MAIN FILE OF YOUR APPLICATION #####
//################################################
/*
#include "eth-main.h"
#include "eth-nic.h"
#include "eth-dhcp.h"
*/


//###################################################
//##### IN YOUR APPLICATION INITIALISE FUNCTION #####
//###################################################
/*
	//----- SET OUR ETHENET UNIQUE MAC ADDRESS -----
	our_mac_address.v[0] = 0;		//MSB
	our_mac_address.v[1] = 80;
	our_mac_address.v[2] = 194;
	our_mac_address.v[3] = 80;
	our_mac_address.v[4] = 16;
	our_mac_address.v[5] = 50;		//LSB

	//----- INITIALISE ETHERNET -----
	tcp_ip_initialise();
*/


//#########################################
//##### IN YOUR APPLICATION MAIN LOOP #####
//#########################################
/*
	//----- PROCESS ETHERNET STACK -----
	tcp_ip_process_stack();
*/	



//###################################################
//##### ADD TO YOUR APPLICATION HEARTBEAT TIMER #####
//###################################################
/*
	//-----------------------------
	//----- HERE EVERY 10 mSec -----
	//-----------------------------
	
	//----- ETHERNET GENERAL TIMER -----
	ethernet_10ms_clock_timer_working++;
*/


//########################
//##### IN THIS FILE #####
//########################
//Select the requried stack functions you want to use from the defines below


//#############################################################
//##### REGISTERS THAT MAY BE USEFUL FOR YOUR APPLICATION #####
//#############################################################
//nic_is_linked						//= 1 if nic is linked, 0 otherwise
//nic_linked_and_ip_address_valid	//= 1 if we are linked and have valid IP settings (either manually or from a DHCP server), 0 otherwise
//
//Usage example to determin if are we ready to communicate:
//	if (nic_linked_and_ip_address_valid)



//For further information please see the project technical manual






//*****************************
//*****************************
//********** DEFINES **********
//*****************************
//*****************************
#ifndef ETH_MAIN_C_INIT		//(Do only once)
#define	ETH_MAIN_C_INIT




//------------------------------------
//----- CONFIGURE ETHERNET STACK -----		<<<<< SETUP FOR A NEW APPLICATION
//------------------------------------
//----- ICMP -----
//(Used for ping)
#define STACK_USE_ICMP				//(Comment out if not required)

//----- DHCP -----
//(Used to allow us to automatically obtain an IP address, subnet mask and gateway address from a local network DHCP server)
#define STACK_USE_DHCP				//(Comment out if not required)

//----- DNS -----
//(Used to allow us to find out the IP address of remote devices from their domain name)
#define	STACK_USE_DNS				//(Comment out if not required)

//----- EMAIL -----
#define	STACK_USE_POP3				//(Comment out if not required)
#define	STACK_USE_SMTP				//(Comment out if not required)

//----- UDP -----
#define STACK_USE_UDP				//(Comment out if not required)

//----- TCP -----
#define STACK_USE_TCP				//(Comment out if not required)

//----- NETBIOS -----
#define	STACK_USE_NETBIOS			//(Comment out if not required)

//----- HTTP -----
#define STACK_USE_HTTP				//(Comment out if not required)
//#define	STACK_USE_HTTP_CLIENT		//(Comment out if not required)

//----- SNTP -----
#define STACK_USE_SNTP				//(Comment out if not required)


//----- GENERATE ERROR MESSAGES IF INCOMPATIBLE OPTIONS ARE SELECTED -----
#if defined(STACK_USE_DHCP) && !defined(STACK_USE_UDP)
	#error Ethernet config error - If DHCP is enabled then UDP must also be enabled
#endif

#if defined(STACK_USE_DNS) &&!defined(STACK_USE_UDP)
	#error Ethernet config error - If DNS is enabled then UDP must also be enabled
#endif

#if defined(STACK_USE_POP3) &&!defined(STACK_USE_TCP)
	#error Ethernet config error - If POP3 is enabled then TCP must also be enabled
#endif

#if defined(STACK_USE_POP3) &&!defined(STACK_USE_DNS)
	#error Ethernet config error - If POP3 is enabled then DNS must also be enabled
#endif

#if defined(STACK_USE_SMTP) &&!defined(STACK_USE_TCP)
	#error Ethernet config error - If SMTP is enabled then TCP must also be enabled
#endif

#if defined(STACK_USE_SMTP) &&!defined(STACK_USE_DNS)
	#error Ethernet config error - If SMTP is enabled then DNS must also be enabled
#endif

#if defined(STACK_USE_NETBIOS) &&!defined(STACK_USE_UDP)
	#error Ethernet config error - If NetBIOS is enabled then UDP must also be enabled
#endif


#if defined(STACK_USE_HTTP) &&!defined(STACK_USE_TCP)
	#error Ethernet config error - If HTTP is enabled then TCP must also be enabled
#endif


//----- ETHERNET PACKET DEFINITIONS -----
#define	ETHERNET_HARDWARE_TYPE			0x0001
#define	ETHERNET_TYPE_ARP				0x0806
#define ETHERNET_TYPE_IP				0x0800




//----- STACK STATE MACHINE STATES -----
typedef enum _ETH_STACK_STATE
{
	SM_ETH_STACK_IDLE,
	SM_ETH_STACK_ARP,
	SM_ETH_STACK_IP,
	SM_ETH_STACK_ICMP,
	SM_ETH_STACK_ICMP_REPLY,
	SM_ETH_STACK_UDP,
	SM_ETH_STACK_TCP

} ETH_STACK_STATE;



//---------------------------------
//---------------------------------
//----- DATA TYPE DEFINITIONS -----
//---------------------------------
//---------------------------------

//MAC ADDRESS:
typedef struct _MAC_ADDR
{
    BYTE		v[6];
} MAC_ADDR;
#define		MAC_ADDR_LENGTH		6		//Use this define instead of sizeof to deal with compilers that pad this size to a 32 bit boundary (8 bytes)

//IP ADDRESS:
typedef union _IP_ADDR
{
    BYTE        v[4];
    DWORD       Val;
} IP_ADDR;
#define		IP_ADDR_LENGTH		4

//REMOTE DEVICE INFO:
typedef struct _DEVICE_INFO
{
    MAC_ADDR    mac_address;
    IP_ADDR     ip_address;
} DEVICE_INFO;


#endif		//ETH_MAIN_C_INIT




//*******************************
//*******************************
//********** FUNCTIONS **********
//*******************************
//*******************************
#ifdef ETH_MAIN_C
//-----------------------------------
//----- INTERNAL ONLY FUNCTIONS -----
//-----------------------------------


//-----------------------------------------
//----- INTERNAL & EXTERNAL FUNCTIONS -----
//-----------------------------------------
//(Also defined below as extern)
void tcp_ip_initialise (void);
void tcp_ip_process_stack (void);
WORD swap_word_bytes (WORD data);
DWORD swap_dword_bytes (DWORD data);
BYTE convert_character_to_lower_case (BYTE character);
BYTE convert_character_to_upper_case (BYTE character);
BYTE* convert_string_to_lower_case (BYTE *string_to_convert);
BYTE* find_character_in_string (BYTE *examine_string, BYTE character);
BYTE* find_string_in_string_no_case (BYTE *examine_string, CONSTANT BYTE *looking_for_string);
BYTE* copy_ram_string_to_ram_string (BYTE *destination_string, BYTE *source_string);
WORD convert_ascii_to_integer (BYTE *source_string);
DWORD convert_ascii_to_dword (BYTE *source_string);
BYTE* convert_word_to_ascii (WORD value, BYTE *dest_string);
BYTE* convert_dword_to_ascii (DWORD value, BYTE *dest_string);
BYTE convert_ascii_hex_to_byte (BYTE high_char, BYTE low_char);


#else
//------------------------------
//----- EXTERNAL FUNCTIONS -----
//------------------------------
extern void tcp_ip_initialise (void);
extern void tcp_ip_process_stack (void);
extern WORD swap_word_bytes (WORD data);
extern DWORD swap_dword_bytes (DWORD data);
extern BYTE convert_character_to_lower_case (BYTE character);
extern BYTE convert_character_to_upper_case (BYTE character);
extern BYTE* convert_string_to_lower_case (BYTE *string_to_convert);
extern BYTE* find_character_in_string (BYTE *examine_string, BYTE character);
extern BYTE* find_string_in_string_no_case (BYTE *examine_string, CONSTANT BYTE *looking_for_string);
extern BYTE* copy_ram_string_to_ram_string (BYTE *destination_string, BYTE *source_string);
extern WORD convert_ascii_to_integer (BYTE *source_string);
extern DWORD convert_ascii_to_dword (BYTE *source_string);
extern BYTE* convert_word_to_ascii (WORD value, BYTE *dest_string);
extern BYTE* convert_dword_to_ascii (DWORD value, BYTE *dest_string);
extern BYTE convert_ascii_hex_to_byte (BYTE high_char, BYTE low_char);


#endif




//****************************
//****************************
//********** MEMORY **********
//****************************
//****************************
#ifdef ETH_MAIN_C
//--------------------------------------------
//----- INTERNAL ONLY MEMORY DEFINITIONS -----
//--------------------------------------------
BYTE sm_ethernet_stack;


//--------------------------------------------------
//----- INTERNAL & EXTERNAL MEMORY DEFINITIONS -----
//--------------------------------------------------
//(Also defined below as extern)
MAC_ADDR our_mac_address;
IP_ADDR our_ip_address;
IP_ADDR our_subnet_mask;
IP_ADDR our_gateway_ip_address;
BYTE nic_linked_and_ip_address_valid = 0;
DWORD ethernet_10ms_clock_timer;
DWORD ethernet_10ms_clock_timer_working;


#else
//---------------------------------------
//----- EXTERNAL MEMORY DEFINITIONS -----
//---------------------------------------
extern MAC_ADDR our_mac_address;
extern IP_ADDR our_ip_address;
extern IP_ADDR our_subnet_mask;
extern IP_ADDR our_gateway_ip_address;
extern BYTE nic_linked_and_ip_address_valid;
extern DWORD ethernet_10ms_clock_timer;
extern DWORD ethernet_10ms_clock_timer_working;

#endif








