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
//NXP LPC2365 (BUILT IN NETWORK INTERFACE CONTROLLER) WITH KSZ8001




//##################################
//##################################
//########## USING DRIVER ##########
//##################################
//##################################

//Check this header file for defines to setup and any usage notes
//Configure the IO pins as requried in your applications initialisation.



//#######################
//##### MAC ADDRESS #####
//#######################
//The MAC address needs to be provided by the driver during initialisation.


//For further information please see the project technical manual





//*****************************
//*****************************
//********** DEFINES **********
//*****************************
//*****************************
#ifndef NIC_C_INIT		//Do only once the first time this file is used
#define	NIC_C_INIT


//----------------------------------------------
//----- DEFINE TARGET COMPILER & PROCESSOR -----
//----------------------------------------------
//(ONLY 1 SHOULD BE INCLUDED, COMMENT OUT OTHERS)
#define	NIC_USING_CROSSWORKS_ARM_COMPILER
//<< add other compiler types here





#ifdef NIC_USING_CROSSWORKS_ARM_COMPILER
//#########################################
//#########################################
//##### USING CROSSWORKS ARM COMPILER #####
//#########################################
//#########################################


//----------------------
//----- IO DEFINES -----
//----------------------

//CONTROL PINS:-
#define NIC_PHY_RST(state)		(state ? (FIO3SET = 0x04000000) : (FIO3CLR = 0x04000000))		//0 = Reset (minimum of 50 us pulse is required)
#define NIC_PHY_PD(state)		(state ? (FIO3SET = 0x02000000) : (FIO3CLR = 0x02000000))		//0 = Power Down
#define NIC_PHY_INT_PIN			(FIO2PIN & 0x0800)												//0 = Irq

	



//################################################
//################################################
//##### END OF USING CROSSWORKS ARM COMPILER #####
//################################################
//################################################
#endif		//#ifdef NIC_USING_CROSSWORKS_ARM_COMPILER




//----- NIC LED OUTPUTS -----
//0x0
//	LED3 <- collision
//	LED2 <- full duplex
//	LED1 <- speed
//	LED0 <- link/activity
//0x1
//	LED3 <- activity
//	LED2 <- full duplex/collision
//	LED1 <- speed
//	LED0 <- link
//0x2
//	LED3 <- activity
//	LED2 <- full duplex
//	LED1 <- 100Mbps link
//	LED0 <- 10Mbps link
#define NIC_LEDS_FUNCTION				0x0



//------------------------
//----- USER OPTIONS -----
//------------------------

//----- ETHERNET SPEED TO USE -----
#define	NIC_INIT_SPEED						0	//0 = allow speed 10 / 100 Mbps, 1 = force speed to 10 Mbps

#define	PROCESS_NIC_CALLS_BEFORE_DUMP_RX	5	//The number of times the nic_check_for_rx may be called with a received packet waiting to be processed before it
												//just gets dumped.  This is a backup in case an application function has opened a socket but doesn't process a
												//received packet for that socket for some reason.



//------------------------------------
//----- ETHERNET INTERFACE SETUP -----
//------------------------------------
#define NIC_MAX_FRAME_LENGTH        1522			//Max Ethernet Frame Size



//RX AND TX DESCRIPTOR AND STATUS DEFINITIONS

//MEMORY BUFFER CONFIGURATION FOR 16kB OF ETHERNET RAM
//We use a fragment (block) size of 1536 (0x600) as this will accept a complete ethernet packet.  We could use a smaller size but we'd then have to 
//deal with packets spanning multiple blocks and wraparound issues.  We have 16kB so this seems unessary and avoids additional complexity.
//Memory usage:
//	(NIC_FRAGMENT_BUFFERS_SIZE x (NIC_NUM_OF_RX_FRAGMENT_BUFFERS + NIC_NUM_OF_TX_FRAGMENT_BUFFERS)) = 15360		//Data buffers
//	32 x (NIC_NUM_OF_RX_FRAGMENT_BUFFERS + NIC_NUM_OF_TX_FRAGMENT_BUFFERS) = 320								//Descriptors etc
//Total of 15680 bytes used or a total 16384 available
#define NIC_NUM_OF_RX_FRAGMENT_BUFFERS				8			//Number of receive fragment buffers (minimum 2)
#define NIC_NUM_OF_TX_FRAGMENT_BUFFERS				2			//Number of transmit fragment buffers (minimum 2)
#define NIC_FRAGMENT_BUFFERS_SIZE					1536		//Size of each fragment (block) in bytes

//ETHERNET 16kB RAM MEMORY AREA
//MAC_BASE = 0xFFE00000 (to 0xFFE0 3FFF)
//Descriptors have to be aligned to the 32 bit boundary
#define NIC_RAM_BASE_ADDRESS				0x7FE00000																	//0x7FE00000
#define	NIC_RX_DESCRIPTOR_BASE_ADDRESS		NIC_RAM_BASE_ADDRESS														//0x7FE00000
#define NIC_RX_STATUS_BASE_ADDRESS			(NIC_RAM_BASE_ADDRESS + (NIC_NUM_OF_RX_FRAGMENT_BUFFERS * 8))				//0x7FE00040
#define NIC_TX_DESCRIPTOR_BASE_ADDRESS		(NIC_RX_STATUS_BASE_ADDRESS + (NIC_NUM_OF_RX_FRAGMENT_BUFFERS * 8))			//0x7FE00080
#define NIC_TX_STATUS_BASE_ADDRESS			(NIC_TX_DESCRIPTOR_BASE_ADDRESS + (NIC_NUM_OF_TX_FRAGMENT_BUFFERS * 8))		//0x7FE00090
#define NIC_RX_BUFFER_BASE_ADDRESS			(NIC_TX_STATUS_BASE_ADDRESS + (NIC_NUM_OF_TX_FRAGMENT_BUFFERS * 4))			//0x7FE00098
#define NIC_TX_BUFFER_BASE_ADDRESS			(NIC_RX_BUFFER_BASE_ADDRESS + (NIC_NUM_OF_RX_FRAGMENT_BUFFERS * NIC_FRAGMENT_BUFFERS_SIZE))		//0x7FE03098
																														//0x7FE03C98

#define NIC_RX_DESCRIPTOR_PACKET(i)			(*(DWORD*)(NIC_RX_DESCRIPTOR_BASE_ADDRESS + (8 * i)))
#define NIC_RX_DESCRIPTOR_CONTROL(i)		(*(DWORD*)(NIC_RX_DESCRIPTOR_BASE_ADDRESS + 4 + (8 * i)))
#define NIC_RX_STATUS_INFO(i)				(*(DWORD*)(NIC_RX_STATUS_BASE_ADDRESS + (8 * i)))							//Receive status arrays need to be aligned on 8 byte boundaries
#define NIC_RX_STATUS_HASHCRC(i)			(*(DWORD*)(NIC_RX_STATUS_BASE_ADDRESS + 4 + (8 * i)))
#define NIC_TX_DESCRIPTOR_PACKET(i)			(*(DWORD*)(NIC_TX_DESCRIPTOR_BASE_ADDRESS + (8 * i)))
#define NIC_TX_DESCRIPTOR_CONTROL(i)		(*(DWORD*)(NIC_TX_DESCRIPTOR_BASE_ADDRESS + 4 + (8 * i)))
#define NIC_TX_STATUS_INFO(i)				(*(DWORD*)(NIC_TX_STATUS_BASE_ADDRESS + (4 * i)))
#define NIC_RX_BUFFER(i)					(NIC_RX_BUFFER_BASE_ADDRESS + (NIC_FRAGMENT_BUFFERS_SIZE * i))
#define NIC_TX_BUFFER(i)					(NIC_TX_BUFFER_BASE_ADDRESS + (NIC_FRAGMENT_BUFFERS_SIZE * i))




//----- KSZ8001 PHY REGISTERS -----
#define NIC_PHY_REG_BASIC_CTRL_REG					0x00			//Basic Control Register
#define NIC_PHY_BASIC_STATUS_REG					0x01			//Basic Status Register
#define NIC_PHY_IDENTIFIER_1						0x02			//PHY Identifier 1
#define NIC_PHY_IDENTIFIER_2						0x03			//PHY Identifier 2
#define NIC_PHY_AUTO_NEG_ADVERT_REG					0x04			//Auto-Negotiation Advertisement Register
#define NIC_PHY_AUTO_NEG_LINK_PARTNER_ABILITY_REG	0x05			//Auto-Negotiation Link Partner Ability Register
#define NIC_PHY_AUTO_NEG_EXPANSION_REG				0x06			//Auto-Negotiation Expansion Register
#define NIC_PHY_AUTO_NEG_NEXT_PAGE_REG				0x07			//Auto-Negotiation Next Page Register
#define NIC_PHY_LINK_PARTNER_NEXT_PAGE_ABILITY		0x08			//Link Partner Next Page Ability
#define NIC_PHYRXER_COUNTER_REG						0x15			//RXER Counter Register
#define NIC_PHY_IRQ_CTRL_STATUS_REG					0x1B			//Interrupt Control/Status Register
#define NIC_PHY_LINKMD_CTRL_STATUS_REG				0x1D			//LinkMD Control/Status Register
#define	NIC_PHY_CTRL_REG							0x1e			//PHY Control Register
#define	NIC_PHY_100BASE_TX_CTRL_REG					0x1f			//100BASE-TX PHY Control Register

#define NIC_MII_WRITE_TIMEOUT						0x00060000
#define NIC_MII_READ_TIMEOUT						0x00060000

#define	NIC_PHY_IRQ_CTRL_STATUS_VALUE				0x0500			//Enable Link Down Interrupt, Enable Link up Interrupt





//----- NIC IS TX READY STATE MACHINE -----
typedef enum _NIC_IS_TX_READY_STATE
{
	SM_NIC_IS_TX_RDY_IDLE,
	SM_NIC_IS_TX_RDY_ALLOCATED
} NIC_IS_TX_READY_STATE;


//----- DATA TYPE DEFINITIONS -----

typedef struct _ETHERNET_HEADER
{
	MAC_ADDR		destination_mac_address;
	MAC_ADDR		source_mac_address;
	WORD_VAL		type;
} ETHERNET_HEADER;
#define	ETHERNET_HEADER_LENGTH		14		//Can't use sizeof as some compilers will add pad bytes within the struct


#endif		//NIC_C_INIT





//*******************************
//*******************************
//********** FUNCTIONS **********
//*******************************
//*******************************
#ifdef NIC_C
//-----------------------------------
//----- INTERNAL ONLY FUNCTIONS -----
//-----------------------------------
void nic_write_phy_register (BYTE address, WORD data);
WORD nic_read_phy_register (BYTE address);


//-----------------------------------------
//----- INTERNAL & EXTERNAL FUNCTIONS -----
//-----------------------------------------
//(Also defined below as extern)
void nic_initialise (BYTE init_config);
WORD nic_check_for_rx (void);
void nic_setup_read_data (void);
BYTE nic_read_next_byte (BYTE *data);
BYTE nic_read_array (BYTE *array_buffer, WORD array_length);
void nic_move_pointer (WORD move_pointer_to_ethernet_byte);
BYTE nic_read_link_setup (void);
void nic_rx_dump_packet (void);
BYTE nic_setup_tx (void);
void write_eth_header_to_nic (MAC_ADDR *remote_mac_address, WORD ethernet_packet_type);
void nic_write_next_byte (BYTE data);
void nic_write_array (BYTE *array_buffer, WORD array_length);
void nic_write_tx_word_at_location (WORD byte_address, WORD data);
void nix_tx_packet (void);
BYTE nic_ok_to_do_tx (void);


#else
//------------------------------
//----- EXTERNAL FUNCTIONS -----
//------------------------------
extern void nic_initialise (BYTE init_config);
extern WORD nic_check_for_rx (void);
extern void nic_setup_read_data (void);
extern BYTE nic_read_next_byte (BYTE *data);
extern BYTE nic_read_array (BYTE *array_buffer, WORD array_length);
extern void nic_move_pointer (WORD move_pointer_to_ethernet_byte);
extern BYTE nic_read_link_setup (void);
extern void nic_rx_dump_packet (void);
extern BYTE nic_setup_tx (void);
extern void write_eth_header_to_nic (MAC_ADDR *remote_mac_address, WORD ethernet_packet_type);
extern void nic_write_next_byte (BYTE data);
extern void nic_write_array (BYTE *array_buffer, WORD array_length);
extern void nic_write_tx_word_at_location (WORD byte_address, WORD data);
extern void nix_tx_packet (void);
extern BYTE nic_ok_to_do_tx (void);


#endif




//****************************
//****************************
//********** MEMORY **********
//****************************
//****************************
#ifdef NIC_C
//--------------------------------------------
//----- INTERNAL ONLY MEMORY DEFINITIONS -----
//--------------------------------------------
DWORD nic_tx_start_buffer;
BYTE *nic_tx_buffer_next_byte;
DWORD nic_rx_start_buffer;
BYTE *nic_rx_buffer_next_byte;
BYTE nic_read_next_byte_get_word;
WORD nic_tx_next_byte_next_data_word;
WORD nic_rx_packet_total_ethernet_bytes;
BYTE auto_negotation_disabled;


//--------------------------------------------------
//----- INTERNAL & EXTERNAL MEMORY DEFINITIONS -----
//--------------------------------------------------
//(Also defined below as extern)
BYTE nic_is_linked;
BYTE nic_speed_is_100mbps;
WORD nic_rx_bytes_remaining;
WORD nic_tx_len;
BYTE nic_rx_packet_waiting_to_be_dumped;


#else
//---------------------------------------
//----- EXTERNAL MEMORY DEFINITIONS -----
//---------------------------------------
extern BYTE nic_is_linked;
extern BYTE nic_speed_is_100mbps;
extern WORD nic_rx_bytes_remaining;
extern WORD nic_tx_len;
extern BYTE nic_rx_packet_waiting_to_be_dumped;


#endif








