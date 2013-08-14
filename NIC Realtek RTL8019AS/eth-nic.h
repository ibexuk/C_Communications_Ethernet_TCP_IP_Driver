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
//REALTEK RTL8019AS (NETWORK INTERFACE CONTROLLER) C CODE HEADER FILE




//##################################
//##################################
//########## USING DRIVER ##########
//##################################
//##################################

//Check this header file for defines to setup and any usage notes
//Configure the IO pins as requried in your applications initialisation.

//#######################
//##### BUSY SIGNAL #####
//#######################
//The Realtek RTL8019AS has a busy pin (iochrdy) that must be monitored to pause operation if it needs to insert a wait.
//A simple method of doing this, if your microcontroller / processor does not provide such functionality, is to connect
//the signal to an interrupt input that will be triggered when the RTL8019AS pulls the pin low.  In the interrupt handler
//simply monitor the pin and don't exit until it returns high again.

//#########################
//##### PROVIDE DELAY #####
//#########################
//Ensure the nic_delay_1ms() will provide a 1mS delay

//######################
//##### NIC EEPROM #####
//######################
//This driver assumes that the nic does not have an eeprom connected because in typical applicaitons eeprom is avaialble elsewhere.  The
//MAC address is instead provided by the driver during initialisation.
//Note that the DO pin of where the eeprom would be connected to the nic must be pulled low.


//For further information please see the project technical manual




//*****************************
//*****************************
//********** DEFINES **********
//*****************************
//*****************************
#ifndef NIC_C_INIT		//Do only once the first time this file is used
#define	NIC_C_INIT

#include "eth-main.h"

//----------------------------------------------
//----- DEFINE TARGET COMPILER & PROCESSOR -----
//----------------------------------------------
//(ONLY 1 SHOULD BE INCLUDED, COMMENT OUT OTHERS)
#define	NIC_USING_MICROCHIP_C18_COMPILER
//#define	NIC_USING_MICROCHIP_C30_COMPILER
//#define	NIC_USING_MICROCHIP_C32_COMPILER
//<< add other compiler types here


//------------------------
//----- USER OPTIONS -----
//------------------------

//----- ETHERNET SPEED TO USE -----
#define	NIC_INIT_SPEED				0			//1 = force speed to 10 Mbps (only option available for this IC)



#ifdef NIC_USING_MICROCHIP_C18_COMPILER
//########################################
//########################################
//##### USING MICROCHIP C18 COMPILER #####
//########################################
//########################################

//----------------------
//----- IO DEFINES -----
//----------------------
//PORTS:-
#define NIC_DATA_BUS_IP				PORTD				//D7:0 data bus read register
#define NIC_DATA_BUS_OP(data)		LATD = data			//D7:0 data bus write register (same as read register if microcontroller / processor doesn't have separate registers for input and output)
#define NIC_DATA_BUS_TRIS(state)	TRISD = state		//D7:0 data bus input / output register (bit state 0 = output, 1 = input)

//CONTROL PINS:-
#define NIC_WR(state)			LATAbits.LATA1 = state		//0 = write
#define NIC_RD(state)			LATAbits.LATA0 = state		//0 = write
#define	NIC_ADDR_0(state)		LATCbits.LATC0 = state
#define	NIC_ADDR_1(state)		LATCbits.LATC1 = state
#define	NIC_ADDR_2(state)		LATCbits.LATC2 = state
#define	NIC_ADDR_3(state)		LATCbits.LATC3 = state
#define	NIC_ADDR_4(state)		LATCbits.LATC4 = state
#define	NIC_RESET_PIN(state)	LATAbits.LATA2 = state


//-------------------------
//----- DELAY DEFINES -----
//-------------------------
//Resetting the nic requires minimum setup time.  This define should cause a delay of at least 800nS
#define NIC_DELAY_800NS()			Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop()		//('Nop();' is a single cycle null instruction for the C18 compiler, include multiple times if required)

//Depending on the speed of your processor and bus you may need to use a delay when reading and writing to and from the nic.
//Use this define to do this.
#define NIC_DELAY_READ_WRITE()		Nop(); Nop();		//Use to provide time for data bus read or write to stabilise


//###############################################
//###############################################
//##### END OF USING MICROCHIP C18 COMPILER #####
//###############################################
//###############################################
#endif		//#ifdef NIC_USING_MICROCHIP_C18_COMPILER



#ifdef NIC_USING_MICROCHIP_C30_COMPILER
//########################################
//########################################
//##### USING MICROCHIP C30 COMPILER #####
//########################################
//########################################

//----------------------
//----- IO DEFINES -----
//----------------------
//PORTS:-
#define NIC_DATA_BUS_IP				(BYTE)(PORTD & 0x00ff)						//D7:0 data bus read register
#define NIC_DATA_BUS_OP(data)		LATD &= 0xff00; LATD |= (WORD)data			//D7:0 data bus write register (same as read register if microcontroller / processor doesn't have separate registers for input and output)
#define NIC_DATA_BUS_TRIS(state)	TRISD &= 0xff00; TRISD |= (WORD)state		//D7:0 data bus input / output register (bit state 0 = output, 1 = input)


//CONTROL PINS:-
#define NIC_WR(state)			_LATA1 = state			//0 = write
#define NIC_RD(state)			_LATA0 = state			//0 = write
#define	NIC_ADDR_0(state)		_LATE0 = state
#define	NIC_ADDR_1(state)		_LATE1 = state
#define	NIC_ADDR_2(state)		_LATE2 = state
#define	NIC_ADDR_3(state)		_LATE3 = state
#define	NIC_ADDR_4(state)		_LATE4 = state
#define	NIC_RESET_PIN(state)	_LATA2 = state


//-------------------------
//----- DELAY DEFINES -----
//-------------------------
//Resetting the nic requires minimum setup time.  This define should cause a delay of at least 800nS
#define NIC_DELAY_800NS()			Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop();
									//('Nop();' is a single cycle null instruction for the C30 compiler, include multiple times if required)

//Depending on the speed of your processor and bus you may need to use a delay when reading and writing to and from the nic.
//Use this define to do this.
#define NIC_DELAY_READ_WRITE()		Nop(); Nop();		//Use to provide time for data bus read or write to stabilise

//###############################################
//###############################################
//##### END OF USING MICROCHIP C30 COMPILER #####
//###############################################
//###############################################
#endif		//#ifdef NIC_USING_MICROCHIP_C30_COMPILER


#ifdef NIC_USING_MICROCHIP_C32_COMPILER
//########################################
//########################################
//##### USING MICROCHIP C32 COMPILER #####
//########################################
//########################################

//----------------------
//----- IO DEFINES -----
//----------------------
//PORTS:-
#define NIC_DATA_BUS_IP				(BYTE)(mPORTDRead() & 0x000000ff)												//D7:0 data bus read register
#define NIC_DATA_BUS_OP(data)		mPORTDSetBits((DWORD)data); mPORTDClearBits((DWORD)~data)						//D7:0 data bus write register (same as read register if microcontroller / processor doesn't have separate registers for input and output)
#define NIC_DATA_BUS_TRIS(state)	mPORTDSetPinsDigitalIn((DWORD)state); mPORTDSetPinsDigitalOut((DWORD)~state)	//D7:0 data bus input / output register (bit state 0 = output, 1 = input)

//CONTROL PINS:-
#define NIC_WR(state)			(state ? mPORTASetBits(BIT_1) : mPORTAClearBits(BIT_1))		//0 = write
#define NIC_RD(state)			(state ? mPORTASetBits(BIT_0) : mPORTAClearBits(BIT_0))		//0 = write
#define	NIC_ADDR_0(state)		(state ? mPORTESetBits(BIT_0) : mPORTEClearBits(BIT_0))
#define	NIC_ADDR_1(state)		(state ? mPORTESetBits(BIT_1) : mPORTEClearBits(BIT_1))
#define	NIC_ADDR_2(state)		(state ? mPORTESetBits(BIT_2) : mPORTEClearBits(BIT_2))
#define	NIC_ADDR_3(state)		(state ? mPORTESetBits(BIT_3) : mPORTEClearBits(BIT_3))
#define	NIC_ADDR_4(state)		(state ? mPORTESetBits(BIT_4) : mPORTEClearBits(BIT_4))
#define	NIC_RESET_PIN(state)	(state ? mPORTASetBits(BIT_2) : mPORTAClearBits(BIT_2))


//-------------------------
//----- DELAY DEFINES -----
//-------------------------
//Resetting the nic requires minimum setup time.  This define should cause a delay of at least 800nS
#define NIC_DELAY_800NS()			Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop();
									//('Nop();' is a single cycle null instruction for the C32 compiler, include multiple times if required)

//Depending on the speed of your processor and bus you may need to use a delay when reading and writing to and from the nic.
//Use this define to do this.
#define NIC_DELAY_READ_WRITE()		Nop(); Nop(); Nop(); Nop(); Nop();		//Use to provide time for data bus read or write to stabilise


//###############################################
//###############################################
//##### END OF USING MICROCHIP C32 COMPILER #####
//###############################################
//###############################################
#endif		//#ifdef NIC_USING_MICROCHIP_C32_COMPILER



//GENERAL DEFINES
#define	PROCESS_NIC_CALLS_BEFORE_DUMP_RX	5	//The number of times the nic_check_for_rx may be called with a received packet waiting to be processed before it
												//just gets dumped.  This is a backup in case an application function has opened a socket but doesn't process a
												//received packet for that socket for some reason.



//----- NIC REGISTER DEFINITIONS -----
//Page 0
#define NIC_REG_CR					0x00
#define NIC_REG_PSTART				0x01
#define NIC_REG_PSTOP				0x02
#define NIC_REG_BNDRY				0x03
#define NIC_REG_TPSR				0x04
#define NIC_REG_TSR					0x04
#define NIC_REG_NCR					0x05
#define NIC_REG_TBCR0				0x05
#define NIC_REG_TBCR1				0x06
#define NIC_REG_ISR					0x07
#define NIC_REG_RSAR0				0x08
#define NIC_REG_CRDA0				0x08
#define NIC_REG_RSAR1				0x09
#define NIC_REG_CRDA1				0x09
#define NIC_REG_RBCR0				0x0a
#define NIC_REG_RBCR1				0x0b
#define NIC_REG_RSR					0x0c
#define NIC_REG_RCR					0x0c
#define NIC_REG_TCR					0x0d
#define NIC_REG_CNTR0				0x0d
#define NIC_REG_DCR					0x0e
#define NIC_REG_CNTR1				0x0e
#define NIC_REG_IMR					0x0f
#define NIC_REG_CNTR2				0x0f
#define NIC_REG_REM_DMA				0x10
#define NIC_REG_RESET				0x1f

//Page 1
#define NIC_REG_PAR0				0x01
#define NIC_REG_PAR1				0x02
#define NIC_REG_PAR2				0x03
#define NIC_REG_PAR3				0x04
#define NIC_REG_PAR4				0x05
#define NIC_REG_PAR5				0x06
#define NIC_REG_CURR				0x07
#define NIC_REG_MAR0				0x08
#define NIC_REG_MAR1				0x09
#define NIC_REG_MAR2				0x0a
#define NIC_REG_MAR3				0x0b
#define NIC_REG_MAR4				0x0c
#define NIC_REG_MAR5				0x0d
#define NIC_REG_MAR6				0x0e
#define NIC_REG_MAR7				0x0f

//Page3
#define NIC_REG_9346CR				0x01
#define NIC_REG_CONFIG0				0x03
#define NIC_REG_CONFIG1				0x04
#define NIC_REG_CONFIG2				0x05
#define NIC_REG_CONFIG3				0x06

//Memory allocation
#define NIC_TX_START_LOC_IN_RING	0x40
#define NIC_RX_START_LOC_IN_RING	0x46
#define NIC_RX_END_LOC_IN_RING		0x60


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
void nic_delay_ms (BYTE delay_ms);
static void nic_write_address(WORD address);
static void nic_write (WORD address, BYTE data);
static BYTE nic_read (WORD address);
void nic_reset (void);
void nic_delay_ms (BYTE delay_ms);
void nic_rx_overflow_clear (void);


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
WORD nic_rx_packet_total_ethernet_bytes;
BYTE nic_rx_next_packet_pointer_value;
BYTE boundary_register;


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








