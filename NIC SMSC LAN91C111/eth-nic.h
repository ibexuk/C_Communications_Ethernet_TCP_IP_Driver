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
//SMSC LAN91C111 (NETWORK INTERFACE CONTROLLER) C CODE HEADER FILE




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
//The LAN91C111 has a busy pin (ARDY) that must be monitored to pause operation if it needs to insert a wait.
//A simple method of doing this, if your microcontroller / processor does not provide such functionality, is to connect
//the signal to an interrupt input that will be triggered when the LAN91C111 pulls the pin low.  In the interrupt handler
//simply monitor the pin and don't exit until it returns high again.

//#########################
//##### PROVIDE DELAY #####
//#########################
//Ensure the nic_delay_ms() will provide a 1mS delay

//######################
//##### NIC EEPROM #####
//######################
//This driver assumes that the nic does not have an eeprom connected because in typical applicaitons eeprom is available elsewhere.  The
//MAC address is instead provided by the driver during initialisation.


//For further information please see the project technical manual




//----- HARDWARE CONNECTIONS IN 8 BIT MODE -----
//nBE3:2	Tie to VCC
//nBE1		Idles high, bring low to output the low byte +1 on D15:8 (D7:0 not driven)
//nBE0		Idles high, bring low to output the low byte on D7:0 (D15:8 not driven)
//D31:16	No connection
//D15:0		D8 links to D0, D9 to D1 etc to form an 8 bit bi-directional data bus
//AEN		Tie to GND
//A15:10	Tie to GND
//A9:8		Tie to VCC
//A7:4		Tie to GND
//A3:1		Connect to processor - used to select register to address
//
//----- HARDWARE CONNECTIONS IN 16 BIT MODE -----
//nBE3:2	Tie to VCC
//nBE1:0	Tie to GND (Enables D15:0)
//D31:16	No connection
//D15:0		16 bit bi-directional data bus
//AEN		Tie to GND
//A15:10	Tie to GND
//A9:8		Tie to VCC
//A7:4		Tie to GND
//A3:1		Connect to processor - used to select register to address



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
#define	NIC_USING_MICROCHIP_C18_COMPILER
//#define	NIC_USING_MICROCHIP_C30_COMPILER
//#define	NIC_USING_MICROCHIP_C32_COMPILER
//<< add other compiler types here


//------------------------
//----- USER OPTIONS -----
//------------------------

//----- ETHERNET SPEED TO USE -----
#define	NIC_INIT_SPEED				0			//1 = force speed to 10 Mbps, 0 = use 10/100Mbps



#ifdef NIC_USING_MICROCHIP_C18_COMPILER
//########################################
//########################################
//##### USING MICROCHIP C18 COMPILER #####
//########################################
//########################################

#define	NIC_BUS_WIDTH_IS_8_BIT							//USING A 8 BIT DATA BUS

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
#define	NIC_ADDR_1(state)		LATCbits.LATC0 = state
#define	NIC_ADDR_2(state)		LATCbits.LATC1 = state
#define	NIC_ADDR_3(state)		LATCbits.LATC2 = state
#define	NIC_RESET_PIN(state)	LATAbits.LATA2 = state

//INPUT PINS:-
#define	NIC_INTR0_IRQ			PORTBbits.RB0				//This pin is used as a signal input rather than an interrupt, so does not need to be connected to
															//an interrupt input on the microcontroller / processor if processing of the stack will simply be
															//carried out constantly from the user application main loop

#ifdef NIC_BUS_WIDTH_IS_8_BIT
#define	NIC_NBE0_LOW_NBE1_HIGH()	LATCbits.LATC3 = 0; LATCbits.LATC4 = 1;		//Done as a single define to allow easy use of 1 processor pin with an external
#define	NIC_NBE0_HIGH_NBE1_LOW()	LATCbits.LATC3 = 1; LATCbits.LATC4 = 0;		//inverter or 2 processor pins
#endif

//-------------------------
//----- DELAY DEFINES -----
//-------------------------
//Resetting the nic requires minimum setup time.  This define should cause a delay of at least 100nS
#define NIC_DELAY_100NS()			Nop()		//('Nop();' is a single cycle null instruction for the C18 compiler, include multiple times if required)

//Depending on the speed of your processor and bus you may need to use a delay when reading and writing to and from the nic.
//Also used to allow the nic wait (ARDY) irq to occur if its going to (max 10nS)
//Use this define to do this.
#define NIC_DELAY_READ_WRITE()		Nop(); Nop()		//Use to provide time for data bus read or write to stabilise

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

//#define	NIC_BUS_WIDTH_IS_8_BIT						//USING A 8 BIT DATA BUS
#define	NIC_BUS_WIDTH_IS_16_BIT							//USING A 16 BIT DATA BUS (Only 1 should be defined)

//----------------------
//----- IO DEFINES -----
//----------------------

//PORTS:-
#ifdef NIC_BUS_WIDTH_IS_8_BIT
#define NIC_DATA_BUS_IP				(BYTE)(PORTD & 0x00ff)					//D7:0 data bus read register
#define NIC_DATA_BUS_OP(data)		LATD &= 0xff00; LATD |= (WORD)data		//D7:0 data bus write register (same as read register if microcontroller / processor doesn't have separate registers for input and output)
#define NIC_DATA_BUS_TRIS(state)	TRISD &= 0xff00; TRISD |= (WORD)state	//D7:0 data bus input / output register (bit state 0 = output, 1 = input)
#endif

#ifdef NIC_BUS_WIDTH_IS_16_BIT
#define NIC_DATA_BUS_IP				PORTD									//D15:0 data bus read register
#define NIC_DATA_BUS_OP(data)		LATD = (WORD)data						//D15:0 data bus write register (same as read register if microcontroller / processor doesn't have separate registers for input and output)
#define NIC_DATA_BUS_TRIS(state)	TRISD = (WORD)state						//D15:0 data bus input / output register (bit state 0 = output, 1 = input)
#endif

//CONTROL PINS:-
#define NIC_WR(state)			_LATF8 = state			//0 = write
#define NIC_RD(state)			_LATF7 = state			//0 = write
#define	NIC_ADDR_1(state)		_LATF0 = state
#define	NIC_ADDR_2(state)		_LATF1 = state
#define	NIC_ADDR_3(state)		_LATF2 = state
#define	NIC_RESET_PIN(state)	_LATG0 = state

//INPUT PINS:-
#define	NIC_INTR0_IRQ			_RA15						//This pin is used as a signal input rather than an interrupt, so does not need to be connected to
															//an interrupt input on the microcontroller / processor if processing of the stack will simply be
															//carried out constantly from the user application main loop

#ifdef NIC_BUS_WIDTH_IS_8_BIT
#define	NIC_NBE0_LOW_NBE1_HIGH()	_LATF3 = 0; _LATF4 = 1;		//Done as a single define to allow easy use of 1 processor pin with an external
#define	NIC_NBE0_HIGH_NBE1_LOW()	_LATF3 = 1; _LATF4 = 0;		//inverter or 2 processor pins
#endif

//-------------------------
//----- DELAY DEFINES -----
//-------------------------
//Resetting the nic requires minimum setup time.  This define should cause a delay of at least 100nS
#define NIC_DELAY_100NS()			Nop(); Nop(); Nop()		//('Nop();' is a single cycle null instruction for the C18 compiler, include multiple times if required)

//Depending on the speed of your processor and bus you may need to use a delay when reading and writing to and from the nic.
//Also used to allow the nic wait (ARDY) irq to occur if its going to (max 10nS)
//Use this define to do this.
#define NIC_DELAY_READ_WRITE()		Nop(); Nop(); Nop(); Nop()		//Use to provide time for data bus read or write to stabilise

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

#define	NIC_BUS_WIDTH_IS_16_BIT							//USING A 16 BIT DATA BUS (Only 1 should be defined)

//----------------------
//----- IO DEFINES -----
//----------------------

//PORTS:-
#define NIC_DATA_BUS_IP				(WORD)(mPORTDRead() & 0x0000ffff)												//D15:0 data bus read register
#define NIC_DATA_BUS_OP(data)		mPORTDSetBits((DWORD)data); mPORTDClearBits((DWORD)~data)						//D15:0 data bus write register (same as read register if microcontroller / processor doesn't have separate registers for input and output)
#define NIC_DATA_BUS_TRIS(state)	mPORTDSetPinsDigitalIn((DWORD)state); mPORTDSetPinsDigitalOut((DWORD)~state)	//D15:0 data bus input / output register (bit state 0 = output, 1 = input)

//CONTROL PINS:-
#define NIC_WR(state)			(state ? mPORTFSetBits(BIT_8) : mPORTFClearBits(BIT_8))		//0 = write
#define NIC_RD(state)			(state ? mPORTFSetBits(BIT_7) : mPORTFClearBits(BIT_7))		//0 = write
#define	NIC_ADDR_1(state)		(state ? mPORTFSetBits(BIT_0) : mPORTFClearBits(BIT_0))
#define	NIC_ADDR_2(state)		(state ? mPORTFSetBits(BIT_1) : mPORTFClearBits(BIT_1))
#define	NIC_ADDR_3(state)		(state ? mPORTFSetBits(BIT_2) : mPORTFClearBits(BIT_2))
#define	NIC_RESET_PIN(state)	(state ? mPORTGSetBits(BIT_0) : mPORTGClearBits(BIT_0))



//INPUT PINS:-
#define	NIC_INTR0_IRQ			mPORTAReadBits(BIT_15)		//This pin is used as a signal input rather than an interrupt, so does not need to be connected to
															//an interrupt input on the microcontroller / processor if processing of the stack will simply be
															//carried out constantly from the user application main loop
	
//-------------------------
//----- DELAY DEFINES -----
//-------------------------
//Resetting the nic requires minimum setup time.  This define should cause a delay of at least 100nS
#define NIC_DELAY_100NS()			Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop()		//('Nop();' is a single cycle null instruction for the C18 compiler, include multiple times if required)

//Depending on the speed of your processor and bus you may need to use a delay when reading and writing to and from the nic.
//Also used to allow the nic wait (ARDY) irq to occur if its going to (max 10nS)
//Use this define to do this.
#define NIC_DELAY_READ_WRITE()		Nop(); Nop(); Nop(); Nop(); Nop(); Nop()		//Use to provide time for data bus read or write to stabilise


//###############################################
//###############################################
//##### END OF USING MICROCHIP C32 COMPILER #####
//###############################################
//###############################################
#endif		//#ifdef NIC_USING_MICROCHIP_C32_COMPILER




//----- NIC LED OUTPUTS -----
//(0 = LINK, 2 = 10Mbps link detected, 3 = full duplex mode enabled, 4 = TX or RX packet occured, 5 = 100Mbps link detected, 6 = rx packet occured, 7 = tx packet occured
#define NIC_LED_A_FUNCTION			0			//(Has to be link as this pin is read back to determin link status and to trigger an irq on link status change)		
#define NIC_LED_B_FUNCTION			4			//Can be any function



//GENERAL DEFINES
//ETHERNET SPEED TO USE
#define	NIC_INIT_SPEED				0			//0 = allow speed 10 / 100 Mbps, 1 = force speed to 10 Mbps

#define	PROCESS_NIC_CALLS_BEFORE_DUMP_RX	5	//The number of times the nic_check_for_rx may be called with a received packet waiting to be processed before it
												//just gets dumped.  This is a backup in case an application function has opened a socket but doesn't process a
												//received packet for that socket for some reason.


//----- NIC REGISTER DEFINITIONS -----
//Bank 0
#define NIC_REG_TCR						0x0300
#define NIC_REG_EPH_STATUS				0x0302
#define NIC_REG_RCR						0x0304
#define NIC_REG_COUNTER					0x0306
#define NIC_REG_MIR						0x0308
#define NIC_REG_RPCR					0x030a
#define NIC_REG_RESERVED				0x030c
#define NIC_REG_BANK					0x030e
//Bank 1
#define NIC_REG_CONFIG					0x0300
#define NIC_REG_BASE					0x0302
#define NIC_REG_IA0_1					0x0304
#define NIC_REG_IA2_3					0x0306
#define NIC_REG_IA4_5					0x0308
#define NIC_REG_GEN_PURPOSE				0x030a
#define NIC_REG_CONTROL					0x030c
//Bank 2
#define NIC_REG_MMU_COMMAND				0x0300
#define NIC_REG_PNR						0x0302	//(Little endian!)
#define NIC_REG_FIFO_PORTS				0x0304
#define NIC_REG_POINTER					0x0306
#define NIC_REG_DATA					0x0308
#define NIC_REG_DATA_H					0x030a
#define NIC_REG_INTERRUPT				0x030c	//(Little endian!)
//Bank 3
#define NIC_REG_MT0_1					0x0300
#define NIC_REG_MT2_3					0x0302
#define NIC_REG_MT4_5					0x0304
#define NIC_REG_MT6_7					0x0306
#define NIC_REG_MGMT					0x0308
#define NIC_REG_REVISION				0x030a
#define NIC_REG_ERCV					0x030c

//Register constants:-
#define NIC_CONST_IRQ_REGISTER			0x3300	//EPH INT, TX INT & RCV INT ENABLED, all others disabled
#define NIC_CONST_TX_CTRL_REGISTER		0x0401	//(SWFDUP (full duplex) off, EHP_LOOP off, STP_SQET off, FDUPLX (disable ignore own packets)off, MON_CSN (monitor carrier during tx) on, NO CRC off, Padd frames off, TX enable on)
#define NIC_CONST_RX_CTRL_REGISTER		0x2300	//(Abort enable on, strip CRC on, receive on, All Multicast Frames off, Receive all frames off)
#define NIC_CONST_PPCR_REGISTER			(0x0800 | (NIC_LED_A_FUNCTION << 5) | (NIC_LED_B_FUNCTION << 2))		//(Speed and duplex auto negotiation on)


//----- NIC PHY REGISTERS -----
#define	NIC_PHY_CONTROL						0
#define	NIC_PHY_STATUS						1
#define	NIC_PHY_ID0							2
#define	NIC_PHY_ID1							3
#define	NIC_PHY_AUTO_NEG_ADVERTISEMENT		4
#define	NIC_PHY_AUTO_NEG_REM_END_CAPABILITY	5
#define	NIC_PHY_CONFIG1						16
#define	NIC_PHY_CONFIG2						17
#define	NIC_PHY_STATUS_OUTPUT				18
#define	NIC_PHY_MASK						19



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
static void nic_write_address(WORD address);
static void nic_write (WORD address, WORD data);
static WORD nic_read (WORD address);
void nic_reset (void);
void nic_delay_ms (BYTE delay_ms);
void nic_rx_overflow_clear (void);
static void nic_write_phy_0 (void);
static void nic_write_phy_1 (void);
static void nic_write_phy_z (void);
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
BYTE nic_read_next_byte_get_word;
WORD nic_tx_next_byte_next_data_word;
WORD nic_rx_packet_total_ethernet_bytes;


//--------------------------------------------------
//----- INTERNAL & EXTERNAL MEMORY DEFINITIONS -----
//--------------------------------------------------
//(Also defined below as extern)
BYTE nic_is_linked;
BYTE nic_speed_is_100mbps;
WORD nic_rx_bytes_remaining;
BYTE nic_tx_next_byte_is_odd_byte;
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








