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
//Microchip PIC32 (BUILT IN NETWORK INTERFACE CONTROLLER) C CODE HEADER FILE




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
#define	NIC_USING_MICROCHIP_C32_COMPILER
//<< add other compiler types here



#ifdef NIC_USING_MICROCHIP_C32_COMPILER
//########################################
//########################################
//##### USING MICROCHIP C32 COMPILER #####
//########################################
//########################################


//----------------------
//----- IO DEFINES -----
//----------------------


//CONFIGURATION BITS:-
//Set as follows
//	#pragma config FETHIO = OFF				//Default Ethernet IO Pins (ON=Default, OFF=Alternate)
//	#pragma config FMIIEN = OFF				//MII enabled (OFF=RMII enabled)

//CONTROL PINS:-
#define NIC_PHY_RST(state)		(state ? mPORTDSetBits(BIT_2) : mPORTDClearBits(BIT_2))			//0 = Reset (minimum of 50 us pulse is required)
#define NIC_PHY_PD(state)		(state ? mPORTDSetBits(BIT_3) : mPORTDClearBits(BIT_3))			//0 = Power Down
#define NIC_PHY_INT_PIN			mPORTBReadBits(BIT_0) 											//0 = Irq



//###############################################
//###############################################
//##### END OF USING MICROCHIP C32 COMPILER #####
//###############################################
//###############################################
#endif		//#ifdef NIC_USING_MICROCHIP_C32_COMPILER




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
//deal with packets spanning multiple blocks and wraparound issues.
//Memory usage:
//	(NIC_FRAGMENT_BUFFERS_SIZE x (NIC_NUM_OF_RX_FRAGMENT_BUFFERS + NIC_NUM_OF_TX_FRAGMENT_BUFFERS)) = 15360		//Data buffers
//	32 x (NIC_NUM_OF_RX_FRAGMENT_BUFFERS + NIC_NUM_OF_TX_FRAGMENT_BUFFERS) = 320								//Descriptors etc
//Total of 15680 bytes used of a total 16384 available
#define NIC_NUM_OF_RX_FRAGMENT_BUFFERS				8			//Number of receive fragment buffers (minimum 2)
#define NIC_NUM_OF_TX_FRAGMENT_BUFFERS				2			//Number of transmit fragment buffers (minimum 2)
#define NIC_FRAGMENT_BUFFERS_SIZE					1536		//Size of each fragment (block) in bytes.  Must be a multiple of 16.  Needs to be large enough to
																//store any received packet.

//----- DESCRIPTOR HEADER -----
typedef union
{
	struct
	{
		unsigned				: 7;
		unsigned eown			: 1;
		unsigned npv			: 1;
		unsigned				: 7;
		unsigned byte_count		: 11;
		unsigned				: 3;
		unsigned eop			: 1;
		unsigned sop			: 1;
	};
	unsigned int	Val;
}NIC_DESCRIPTOR_HEADER;

//----- DESCRIPTOR TX STATUS BITS -----
typedef union
{
	struct
	{
		WORD	total_bytes_transmitted;				//Total bytes transmitted
		unsigned tx_control_frame			: 1;		//Control frame transmitted
		unsigned tx_pause_control_frame		: 1;		//Pause control frame transmitted
		unsigned tx_backpressure_applied	: 1;		//Transmit backpressure applied
		unsigned tx_vlan					: 1;		//Transmit VLAN tagged frame
		unsigned							: 12;
		WORD 	byte_count;								//Transmit bytes count
		unsigned collision_count			: 4;		//Transmit collision count
		unsigned crc_error					: 1;		//Transmit CRC error
		unsigned length_error				: 1;		//Transmit length check error
		unsigned length_out_of_range		: 1;		//Transmit length out of range
		unsigned tx_done					: 1;		//Transmit done
		unsigned multicast					: 1;		//Transmit multicast
		unsigned broadcast					: 1;		//Transmit broadcast
		unsigned defer						: 1;		//Transmit packet defer
		unsigned excessive_defer			: 1;		//Transmit excessive packet defer
		unsigned max_collision				: 1;		//Transmit maximum collision - packet aborted
		unsigned late_collision				: 1;		//Transmit late collision
		unsigned giant						: 1;		//Transmit giant frame (set when pktSz>MaxFrameSz && HugeFrameEn==0)
		unsigned underrun					: 1;		//Transmit underrun - system failed to transfer all of packet
	}__attribute__ ((__packed__));
	unsigned long long		Val;		// status is 2 words always
}NIC_DESCRIPTOR_TX_STATUS;	// transmitted packet status

//----- DESCRIPTOR RX STATUS BITS -----
typedef union
{
	struct 
	{
		unsigned packet_checksum			:16;		//Packet payload checksum
		unsigned							: 8;
		unsigned runt_packet				: 1;		//Runt packet received
		unsigned not_unicast_not_multicast	: 1;		//Unicast, not me packet
		unsigned hash_table_match			: 1;		//Hash table match
		unsigned magic_packet_match			: 1;		//Magic packet match
		unsigned pattern_match_match		: 1;		//Pattern match match
		unsigned unicast_match				: 1;		//Unicast match
		unsigned broadcast_match			: 1;		//Broadcast match
		unsigned multicast_match			: 1;		//Multicast match
		unsigned rx_bytes_count				:16;		//Received bytes
		unsigned prev_long_drop_event		: 1;		//Packet previously ignored
		unsigned prev_seen_dv				: 1;		//Rx data valid event previously seen
		unsigned prev_carrier				: 1;		//Carrier event previously seen
		unsigned rx_code_violation			: 1;		//Rx code violation
		unsigned crc_error					: 1;		//CRC error in packet
		unsigned length_check_error			: 1;		//Receive length check error
		unsigned length_out_of_range		: 1;		//Receive length out of range
		unsigned rx_ok						: 1;		//Receive OK
		unsigned multicast_address_valid	: 1;		//Multicast packet
		unsigned broadcast_address_valid	: 1;		//Broadcast packet
		unsigned dribble_nibble				: 1;		//Dribble nibble
		unsigned rx_control_frame			: 1;		//Control frame received
		unsigned rx_pause_control_frame		: 1;		//Pause control frame received
		unsigned rx_unknown_op_code			: 1;		//Received unsupported code
		unsigned rx_vlan_type_detected		: 1;		//Received VLAN tagged frame
		unsigned 							: 1;
	}__attribute__ ((__packed__));
	unsigned long long		Val;				//Status is 2 words always	
}NIC_DESCRIPTOR_RX_STATUS;	// received packet status

//----- TX DESCRIPTOR -----
typedef struct 
{
	volatile NIC_DESCRIPTOR_HEADER		header;						//Header bits
	BYTE								*p_data_buffer_address;		//Data buffer start address
	volatile NIC_DESCRIPTOR_TX_STATUS	status;						//Transmit packet status
	unsigned int						next_ethernet_descriptor;	//Next descriptor address (header.nvp = 1)
}__attribute__ ((__packed__)) NIC_TX_BUFFER_DESCRIPTOR;

//----- RX DESCRIPTOR -----
typedef struct 
{
	volatile NIC_DESCRIPTOR_HEADER		header;						//Header bits
	BYTE								*p_data_buffer_address;		//Data buffer start address
	volatile NIC_DESCRIPTOR_RX_STATUS	status;						//Receive packet status
	unsigned int						next_ethernet_descriptor;	//Next descriptor address (header.nvp = 1)
}__attribute__ ((__packed__)) NIC_RX_BUFFER_DESCRIPTOR;




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
BYTE nic_tx_current_buffer;
BYTE *nic_tx_buffer_next_byte;
BYTE nic_rx_current_buffer;
BYTE *nic_rx_buffer_next_byte;
BYTE nic_read_next_byte_get_word;
WORD nic_tx_next_byte_next_data_word;
WORD nic_rx_packet_total_ethernet_bytes;
BYTE auto_negotation_disabled;

NIC_TX_BUFFER_DESCRIPTOR nic_tx_descriptor[NIC_NUM_OF_TX_FRAGMENT_BUFFERS] __attribute__((aligned(4)));
NIC_RX_BUFFER_DESCRIPTOR nic_rx_descriptor[NIC_NUM_OF_RX_FRAGMENT_BUFFERS] __attribute__((aligned(4)));
BYTE nic_tx_data_buffer[NIC_NUM_OF_TX_FRAGMENT_BUFFERS][NIC_FRAGMENT_BUFFERS_SIZE] __attribute__((aligned(4)));
BYTE nic_rx_data_buffer[NIC_NUM_OF_RX_FRAGMENT_BUFFERS][NIC_FRAGMENT_BUFFERS_SIZE] __attribute__((aligned(4)));

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








