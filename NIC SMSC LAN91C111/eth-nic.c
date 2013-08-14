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



#include "main.h"					//Global data type definitions (see https://github.com/ibexuk/C_Generic_Header_File )
#include "eth-main.h"		//Include before our header file
#define	NIC_C
#include "eth-nic.h"

#ifdef STACK_USE_UDP
#include "eth-udp.h"
#endif

#ifdef STACK_USE_TCP
#include "eth-tcp.h"
#endif




//***************************
//***************************
//********** DELAY **********
//***************************
//***************************
//(The accuracy of this function is not important as long as calling with a value of 1 means the delay will
//be at least 1mS - it is only used for the initialise function)
void nic_delay_ms (BYTE delay_ms)
{
	DWORD count;

	while (delay_ms)
	{
		count = (INSTRUCTION_CLOCK_FREQUENCY / 1000) / 3;	//Divide by 3 as this loop will take a minimum of 3 steps
															//(may be more depending on compiler but we don't care)
		while (count)
		{
			count--;
		}
		delay_ms--;
	}
}




//***********************************
//***********************************
//********** RESET THE NIC **********
//***********************************
//***********************************
void nic_reset (void)
{
	//Reset high for >100nS
	NIC_RESET_PIN(1);

	NIC_DELAY_100NS();

	//Low to run
	NIC_RESET_PIN(0);
}



//************************************
//************************************
//********** INITIALISE NIC **********
//************************************
//************************************
//Call with:
//0 = allow speed 10 / 100 Mbps
//1 = force speed to 10 Mbps
void nic_initialise (BYTE init_config)
{
	WORD data;

	//-----------------------------
	//----- DO HARDWARE RESET -----
	//-----------------------------
	//POWER UP - WAIT 50MS
	nic_delay_ms(50);

	//RESET THE NIC
	nic_reset();

	//WAIT 50MS
	nic_delay_ms(50);

	//-------------------------------------------------
	//----- WRITE THE NIC CONFIGURATION REGISTERS ----- 
	//-------------------------------------------------
	//(The eeprom is not present so the nic defaults to base address 0x0300)

	//----- SEND MMU RESET COMMAND -----
	//Set nic bank 2
	nic_write(NIC_REG_BANK, 0x3302);

	//MMU Command
	//(Reset MMU to initial state)
	nic_write(NIC_REG_MMU_COMMAND, 0x0040);
	
	//Wait for the busy flag to clear
	while (nic_read(NIC_REG_MMU_COMMAND) & 0x0001)
		;

	//----- DO BANK 0 REGISTERS -----
	//Set bank 0
	nic_write(NIC_REG_BANK, 0x3300);

	//Transmit Control (done at end)
	//nic_write(NIC_REG_TCR, 0x0000);

	//EPH Status
	//nic_write(NIC_REG_EPH_STATUS, 0x0000);

	//Receive Control (done at end)
	//nic_write(NIC_REG_RCR, 0x0000);

	//Counter
	//nic_write(NIC_REG_COUNTER, 0x0000);

	//Memory Information
	//nic_write(NIC_REG_MIR, 0x0000);

	//Receive / Phy Control Register
	//(Speed and duplex auto negotiation on, LEDA function, LED B function)
	nic_write(NIC_REG_RPCR, NIC_CONST_PPCR_REGISTER);

	//Reserved
	//nic_write(NIC_REG_RESERVED, 0x0000);

	//----- DO BANK 1 REGISTERS -----
	//Set Bank 1
	nic_write(NIC_REG_BANK, 0x3301);

	//Configuration
	//(No wait states on ARDY except for data reg if not ready, external MII disabled)
	nic_write(NIC_REG_CONFIG, 0xb0b1);

	//Base Address
	//nic_write(NIC_REG_BASE, 0x0000);

	//Individual Address (MAC) 0-1 (Bit 0 is first bit of address on the cable)
	nic_write(NIC_REG_IA0_1, (((WORD)our_mac_address.v[1] << 8) + (WORD)our_mac_address.v[0]));		//(Litle endian)

	//Individual Address (MAC) 2-3
	nic_write(NIC_REG_IA2_3, (((WORD)our_mac_address.v[3] << 8) + (WORD)our_mac_address.v[2]));		//(Litle endian)

	//Individual Address (MAC) 4-5
	nic_write(NIC_REG_IA4_5, (((WORD)our_mac_address.v[5] << 8) + (WORD)our_mac_address.v[4]));		//(Litle endian)

	//General Purpose
	//nic_write(NIC_REG_GEN_PURPOSE, 0x0000);

	//Control
	//(Bad CRC packets are dumped, Auto release TX memory after good TX, Link Irq on, counter roll over irq off, tx error irq off)
	nic_write(NIC_REG_CONTROL, 0x1a90);

	//----- DO BANK 2 REGISTERS -----
	//Set bank 2
	nic_write(NIC_REG_BANK, 0x3302);

	//MMU Command
	//(Reset MMU to initial state - already done above)
	//nic_write(NIC_REG_MMU_COMMAND, 0x0040);
	//Wait for the busy flag to clear
	//while (nic_read(NIC_REG_MMU_COMMAND) & 0x0001)
	//	;

	//Packet Number
	//nic_write(NIC_REG_PNR, 0x0000);		//(Litle endian)

	//Fifo ports
	//nic_write(NIC_REG_FIFO_PORTS, 0x0000);

	//Pointer
	//nic_write(NIC_REG_POINTER, 0x0000);

	//Data
	//nic_write(NIC_REG_DATA, 0x0000);

	//Data High
	//nic_write(NIC_REG_DATA_H, 0x0000);

	//Interrupt Status
	//nic_write(NIC_REG_INTERRUPT, 0x001f);		//(Litle endian)

	//----- DO BANK 3 REGISTERS -----
	//Select bank 3
	nic_write(NIC_REG_BANK, 0x3303);

	//Multicast 0-1
	nic_write(NIC_REG_MT0_1, 0x0000);

	//Multicast 2-3
	nic_write(NIC_REG_MT2_3, 0x0000);

	//Multicast 4-5
	nic_write(NIC_REG_MT4_5, 0x0000);

	//Multicast 6-7
	nic_write(NIC_REG_MT6_7, 0x0000);

	//Management
	//nic_write(NIC_REG_MGMT, 0x0000);

	//Revision
	//nic_write(NIC_REG_REVISION, 0x0000);

	//Early Receive
	//(Receive IRQ Threshold = max)
	nic_write(NIC_REG_ERCV, 0x001f);

	//----------------------------------------------
	//----- WRITE TO THE PHY CONTROL REGISTERS -----
	//----------------------------------------------

	//----- SET 'AUTO NEGOTIATE AVAILABLE SPEEDS AND DUPLEX MODES' -----

	//WE USE HALF DUPLEX MODE
	if (init_config)
	{
		//Only 10Mbps available
		data = 0x0021;
	}
	else
	{
		//10/100Mbps available
		data = 0x00a1;
	}

	nic_write_phy_register(NIC_PHY_AUTO_NEG_ADVERTISEMENT, data);

	//----- TURN OFF ISOLATION MODE SO AUTO NEGOTIATION STARTS -----
	nic_write_phy_register(NIC_PHY_CONTROL, 0x1000);

	//----------------------------
	//----- ENABLE TX AND RX -----
	//----------------------------
	//Set bank 0
	nic_write(NIC_REG_BANK, 0x3300);

	//Transmit Control
	nic_write(NIC_REG_TCR, NIC_CONST_TX_CTRL_REGISTER);

	//Receive Control
	nic_write(NIC_REG_RCR, NIC_CONST_RX_CTRL_REGISTER);

	//Select nic bank 2
	nic_write(NIC_REG_BANK, 0x3302);

	//Interrupt Status
	//(Clear the TX INT bit if set)
	nic_write(NIC_REG_INTERRUPT, (NIC_CONST_IRQ_REGISTER | 0x0002));		//(little endian)
	
	nic_is_linked = 0;
	nic_speed_is_100mbps = 0;
	nic_rx_packet_waiting_to_be_dumped = 0;
}



//********************************************
//********************************************
//********** CHECK FOR NIC ACTIVITY **********
//********************************************
//********************************************
//Returns 0 if no rx waiting (other nic activities may have been processed) or the number of bytes in the packet if rx is waiting
WORD nic_check_for_rx (void)
{
	WORD data;
	BYTE b_data;

	
	//IF NIC ISN'T FLAGGING AN INTERRUPT THEN EXIT
	if (!NIC_INTR0_IRQ)
		return(0);

	//Set nic bank 2
	nic_write(NIC_REG_BANK, 0x3302);

	//Read the interrupt register
	data = nic_read(NIC_REG_INTERRUPT);

	if (data & 0x0002)								//(little endian)
	{
		//-------------------------------------------
		//----- PACKET TX COMPLETE IRQ (TX INT) -----
		//-------------------------------------------
		//We have Auto Release set so there is a packet that couldn't be sent due to an error and TXEN has been disabled
		//We will just dump the packet (we could try re-sending if we wanted to)

		//Read the TX FIFO packet number from the FIFO ports register
		data = nic_read(NIC_REG_FIFO_PORTS);
		//if (data & 0x0080)				//Check FIFO isn't empty
		//		//error!
	
		//Write the packet number into the packet number register
		nic_write(NIC_REG_PNR, (data &= 0x00ff));
	
		//The status word can now be read if its wanted
		//nic_write(NIC_REG_POINTER, 0xa000);			//RCV & READ bits set (no auto increment - special hardware problems fix mode)
		//data = nic_read(NIC_REG_DATA);

		//Ensure MMU isn't busy
		while (nic_read(NIC_REG_MMU_COMMAND) & 0x0001)
			;

		//Send the 'Release Specific Packet' MMU Command
		nic_write(NIC_REG_MMU_COMMAND, 0x00a0);
		
		//Wait for the busy flag to clear
		while (nic_read(NIC_REG_MMU_COMMAND) & 0x0001)
			;

		//Remove the packet number from the completion FIFO by writing the TX INT Acknowledge Register
		//(Clear the TX INT bit if set)
		nic_write(NIC_REG_INTERRUPT, (NIC_CONST_IRQ_REGISTER | 0x0002));			//(little endian)

		//Enable tx (TX ENA will have been cleared due to the error)
		//Select nic bank 0
		nic_write(NIC_REG_BANK, 0x3300);
		//Transmit Control
		nic_write(NIC_REG_TCR, NIC_CONST_TX_CTRL_REGISTER);

		//Exit
		return(0);
	}
	else if (data & 0x0010)									//(little endian)
	{
		//------------------------------------------
		//----- RECEIVE OVERFLOW IRQ (RX_OVRN) -----
		//------------------------------------------
		nic_rx_overflow_clear();
		return(0);
	}
	else if (data & 0x0001)								//(little endian)
	{
		//-------------------------------------------
		//----- A PACKET RECEIVED IRQ (RCV INT) -----
		//-------------------------------------------

		//----- READ THE RX FIFO -----
		//Set nic bank 2
		nic_write(NIC_REG_BANK, 0x3302);
		
		//Get the Fifo Register
		data = nic_read(NIC_REG_FIFO_PORTS);
		
		//Exit if there is not an rx packet waiting?
		//(REMPTY bit 15 = 1)
		if (data & 0x8000)
			return(0);

		//----- THERE IS A RX PACKET WAITING -----
		//Setup the data pointer register to read the rx packet
		nic_setup_read_data();
		nic_rx_bytes_remaining = 4;			//(Required to avoid nic_read_next_byte returning 0)		

		//----- GET THE RX STATUS AND BYTE COUNT -----
		//Read the data register to get the packet
		nic_read_next_byte(&b_data);
		data = (WORD)(b_data & 0x7f);
		nic_read_next_byte(&b_data);
		data += (((WORD)(b_data & 0xfc)) << 8);
		
		//CHECK RX STATUS WORD ERROR
		if (
			(data & 0x8000)					//Check for frame alignment error
			//|| (data & 0x4000)			//Check for broadcast (allowed)
			|| (data & 0x2000)				//Check for bad CRC
			//|| (data & 0x1000)			//Odd number of bytes? (allowed)
			|| (data & 0x0800)				//Check for frame too long (>802.3 max size)
			|| (data & 0x0400)				//Check for frame too short (<802.3 min size)
			//|| (data & 0x0001)			//Check for multicast packet (allowed)
			)
		{
			//REJECT PACKET
			nic_rx_dump_packet();
			return(0);
		}
	
		//Get the low bit of the receive byte count from the status word
		if (data & 0x1000)
			data = 1;
		else
			data = 0;
	
		//Get the byte count
		nic_read_next_byte(&b_data);
		data += (WORD)b_data;
		nic_read_next_byte(&b_data);
		data += ((WORD)(b_data & 0x07) << 8);
		
		//Sutract the status word, byte count word and control word bytes (CRC_STRIP is on so CRC is not included)
		nic_rx_bytes_remaining = (data - 6);
		nic_rx_packet_total_ethernet_bytes = nic_rx_bytes_remaining;

		//The ethernet stack processing routine will continue receiving the rest of the packet
		nic_rx_packet_waiting_to_be_dumped = PROCESS_NIC_CALLS_BEFORE_DUMP_RX;
		return(nic_rx_bytes_remaining);
	}
	else if (data & 0x0020)								//(little endian)
	{
		//---------------------------------
		//----- EPH IRQ (LINK CHANGE) -----
		//---------------------------------
	
		//----- CLEAR LE ENABLE BIT TO ACKNOWLEDGE THE IRQ -----
		//Set bank 1
		nic_write(NIC_REG_BANK, 0x3301);
		//Read control reg
		data = nic_read(NIC_REG_CONTROL);
		//Write control reg
		nic_write(NIC_REG_CONTROL, (data & ~0x0080));
		//Write control reg
		nic_write(NIC_REG_CONTROL, (data | 0x0080));
	
		//----- GET THE LINK PIN STATUS -----
		//Set bank 0
		nic_write(NIC_REG_BANK, 0x3300);
		//Get the EPH Status register
		data = nic_read(NIC_REG_EPH_STATUS);

		if (data & 0x4000)
		{
			//----- NIC IS LINKED -----
			if (nic_is_linked == 0)
			{
				//----- NIC HAS JUST LINKED -----
				//Read the link status
				if (nic_read_link_setup() & 0x01)
					nic_speed_is_100mbps = 1;
				else
					nic_speed_is_100mbps = 0;

				nic_is_linked = 1;
			}
		}
		else
		{
			//----- NIC IS NOT LINKED -----
			if (nic_is_linked)
			{
				//----- NIC HAS JUST LOST LINK -----
				//Reset the nic ready for the next link
				nic_initialise(NIC_INIT_SPEED);
			}
		}
		return(0);
	}
	//<<<<<<< ADD OTHER IRQ PIN HANDLERS HERE
	else
	{
		return(0);
	}
}



//**********************************************************
//**********************************************************
//********** CHECK IF OK TO START A NEW TX PACKET **********
//**********************************************************
//**********************************************************
BYTE nic_ok_to_do_tx (void)
{

	//EXIT IF NIC IS NOT CURRENTLY CONNECTED
	if (nic_is_linked == 0)
		return(0);

	//EXIT IF THE NIC HAS A RECEIVED PACKET THAT IS WAITING PROCESSING AND TO BE DUMPED
	if (nic_rx_packet_waiting_to_be_dumped)
		return(0);

	//WE ARE OK TO TX A NEW PACKET
	return(1);
}



//************************************************
//************************************************
//********** NIC SETUP READ DATA BUFFER **********
//************************************************
//************************************************
void nic_setup_read_data (void)
{
	//Set nic bank 2
	nic_write(NIC_REG_BANK, 0x3302);

	//Data pointer register
	while(nic_read(NIC_REG_POINTER) & 0x0800)		//Check the data write FIFO is empty before writing to the pointer register
		;

	nic_write(NIC_REG_POINTER, 0xe000);				//RCV, AUTO INC & READ bits set

	nic_read_next_byte_get_word = 1;

	//Set the address
	nic_write_address(NIC_REG_DATA);

	//Next operation can read the nic and data will be transfered from the data buffer
}



//****************************************
//****************************************
//********** NIC READ NEXT BYTE **********
//****************************************
//****************************************
//(nic_setup_read_data must have already been called)
//The nic stores the ethernet rx in little endian words.  This routine deals with this and allows us to work in bytes.
//Returns 1 if read successful, 0 if there are no more bytes in the rx buffer
BYTE nic_read_next_byte (BYTE *data)
{
	static WORD last_read_data;


	if (nic_rx_bytes_remaining == 0)
	{
		*data = 0;
		return(0);
	}
	
	nic_rx_bytes_remaining--;

	if (nic_read_next_byte_get_word)
	{
		//----- READ A NEW WORD AND RETURN THE LOW BYTE -----
		nic_read_next_byte_get_word = 0;

		#ifdef NIC_BUS_WIDTH_IS_8_BIT
			//USING 8 BIT BUS - READ NEXT 2 BYTES
			//Get the data
			NIC_DATA_BUS_TRIS(0xff);				//Set port to inputs

			//Get the low byte
			NIC_NBE0_LOW_NBE1_HIGH();				//Enable the low byte output
			NIC_RD(0);
			NIC_DELAY_READ_WRITE();
			last_read_data = (WORD)NIC_DATA_BUS_IP;
			NIC_RD(1);

			//Get the high byte
			NIC_NBE0_HIGH_NBE1_LOW();				//Enable the high byte output
			NIC_RD(0);
			NIC_DELAY_READ_WRITE();
			last_read_data |= ((WORD)NIC_DATA_BUS_IP << 8);
			NIC_RD(1);

			//Return data port to output
			NIC_DATA_BUS_TRIS(0x00);				//Set port to outputs
		#endif

		#ifdef NIC_BUS_WIDTH_IS_16_BIT
			//USING 16 BIT BUS - READ WORD
			//Get the data
			NIC_DATA_BUS_TRIS(0xffff);				//Set port to inputs

			NIC_RD(0);
			NIC_DELAY_READ_WRITE();
			last_read_data = NIC_DATA_BUS_IP;
			NIC_RD(1);

			//Return data port to output
			NIC_DATA_BUS_TRIS(0x0000);				//Set port to outputs
		#endif

		*data = (BYTE)(last_read_data & 0x00ff);

		return(1);
	}
	else
	{
		//----- RETURN THE HIGH BYTE FROM THE LAST READ -----
		nic_read_next_byte_get_word = 1;

		*data = (BYTE)(last_read_data >> 8);

		return(1);
	}
}



//************************************
//************************************
//********** NIC READ ARRAY **********
//************************************
//************************************
//(nic_setup_read_data must have already been called)
BYTE nic_read_array (BYTE *array_buffer, WORD array_length)
{

	for( ;array_length > 0; array_length--)
	{
		if (nic_read_next_byte(array_buffer++) == 0)
			return(0);						//Error - no more bytes available in rx buffer
	}

    return(1);
}



//**************************************
//**************************************
//********** NIC MOVE POINTER **********
//**************************************
//**************************************
//Moves the pointer to a specified byte ready to be read next, with a value of 0 = the first byte of the Ethernet header
void nic_move_pointer (WORD move_pointer_to_ethernet_byte)
{
	WORD new_pointer_value;
	BYTE dummy_data;

	//Set nic bank 2
	nic_write(NIC_REG_BANK, 0x3302);

	//Data pointer register
	while(nic_read(NIC_REG_POINTER) & 0x0800)		//Check the data write FIFO is empty before writing to the pointer register
		;

	//----- SET THE NEW ADDRESS -----
	new_pointer_value = ((move_pointer_to_ethernet_byte + 4) & 0x07fe);	//Needs to be word aligned - low bit dealt with later, +4 to move past the nic status word and byte count
	new_pointer_value |= 0xe000;										//RCV, AUTO INC & READ bits set
	nic_write(NIC_REG_POINTER, new_pointer_value);

	nic_read_next_byte_get_word = 1;

	//Set the address ready for data read
	nic_write_address(NIC_REG_DATA);

	//If new pointer value is odd then read the first byte of the new word
	if (move_pointer_to_ethernet_byte & 0x0001)
		nic_read_next_byte(&dummy_data);

	//ADDRESSED BYTE IS NOW READY TO BE READ

	//----- ADJUST THE NIC DATA BYTES REMAINING COUNT -----
	nic_rx_bytes_remaining = nic_rx_packet_total_ethernet_bytes - move_pointer_to_ethernet_byte;
}



//****************************************
//****************************************
//********** NIC DUMP RX PACKET **********
//****************************************
//****************************************
//Discard any remaining bytes in the current RX packet and free up the nic for the next rx packet
void nic_rx_dump_packet (void)
{
	//Set nic bank 2
	nic_write(NIC_REG_BANK, 0x3302);

	//Ensure MMU isn't busy
	while (nic_read(NIC_REG_MMU_COMMAND) & 0x0001)
		;

	//Send 'Remove and Release From Top Of RX' MMU Command
	nic_write(NIC_REG_MMU_COMMAND, 0x0080);

	nic_rx_packet_waiting_to_be_dumped = 0;
}



//**********************************
//**********************************
//********** NIC SETUP TX **********
//**********************************
//**********************************
//Checks the nic to see if it is ready to accept a new tx packet.  If so it sets up the nic ready for the first byte of the data area to be sent.
//Returns 1 if nic ready, 0 if not.
BYTE nic_setup_tx (void)
{
	WORD data;


	//----- CHECK THE NIC ISN'T OVERFLOWED -----
	//(can happen if nic processing has been stopped for a length of time by other application processes)

	//Set nic bank 2
	nic_write(NIC_REG_BANK, 0x3302);

	//Read the irq register
	//(If rx overrun bit is set then we need to clear it as the nic locks until we do)
	if (nic_read(NIC_REG_INTERRUPT) & 0x0010)
	{
		//Overflow has occured
		nic_rx_overflow_clear();
	}

	//Send the allocate memory for tx command to the nic
	//Set nic bank 2
	nic_write(NIC_REG_BANK, 0x3302);

	//Ensure MMU isn't busy
	while (nic_read(NIC_REG_MMU_COMMAND) & 0x0001)
		;

	//MMU Command
	//(Allocate memory for TX)
	nic_write(NIC_REG_MMU_COMMAND, 0x0020);

	//Wait for MMU busy flag to clear
	while (nic_read(NIC_REG_MMU_COMMAND) & 0x0001)
		;

	//Wait for the ALLOC INT bit to be set
	while (1)
	{
		//Set nic bank 2
		//nic_write(NIC_REG_BANK, 0x3302);

		data = nic_read(NIC_REG_INTERRUPT);
		//Check for overflow - If rx overrun bit is set then we need to clear it as the nic locks until we do
		if (data & 0x0010)
			return(0);					//Clear will happen next time

		//ALLOC INT bit set?
		if (data & 0x0008)
			break;
	}

	//Read the allocation result register
	data = nic_read(NIC_REG_PNR);
	if (data & 0x8000)					//(Little endian)
	{
		//----- ALLOCATION FAILED - NO TX SPACE CURRENTLY AVAILABLE -----
		return(0);
	}
	data &= 0xff00;
	
	//Copy the allocation packet number
	data |= (data  >> 8);

	//--------------------------------------------
	//----- ALLOCATION SUCCESSFUL - SETUP TX -----
	//--------------------------------------------
	//Setup the packet number register with the obtained allocation packet number
	nic_write(NIC_REG_PNR, data);

	//Setup the pointer register for a data write to the tx memory buffer
	while(nic_read(NIC_REG_POINTER) & 0x0800)		//Check the data write FIFO is empty before writing to the pointer register
		;

	nic_write(NIC_REG_POINTER, 0x4000);				//AUTO INC bit set, RCV & READ bits not set

	//Set the address
	nic_write_address(NIC_REG_DATA);

	nic_tx_len = 0;
	nic_tx_next_byte_is_odd_byte = 1;

	//Write the status word to the data register (word 0)
	nic_write_next_byte(0x00);
	nic_write_next_byte(0x00);

	//Send number of bytes to tx (zero for now - we re-write this at the end)
	nic_write_next_byte(0x00);
	nic_write_next_byte(0x00);

	//Next byte is the 1st byte of the Ethernet Frame

	nic_tx_len = 0;

	return(1);
}



//********************************************
//********************************************
//********** NIC TX WRITE NEXT BYTE **********
//********************************************
//********************************************
//(nic_setup_tx must have already been called)
//The nic stores the ethernet tx in words.  This routine deals with this and allows us to work in bytes.
void nic_write_next_byte (BYTE data)
{

	//----- UPDATE THE TX LEN COUNT AND EXIT IF TOO MANY BYTES HAVE BEEN WRITTEN FOR THE NIC -----
	if(nic_tx_len >= (2048 - 6))
		return;

	nic_tx_len++;

	if (nic_tx_next_byte_is_odd_byte)
	{
		//----- WRITE THE ODD BYTE -----
		nic_tx_next_byte_next_data_word = (WORD)data;

		nic_tx_next_byte_is_odd_byte = 0;
	}
	else
	{
		//----- WRITE THE EVEN BYTE -----

		#ifdef NIC_BUS_WIDTH_IS_8_BIT
			//USING 8 BIT BUS - WRITE NEXT 2 BYTES
			NIC_DATA_BUS_TRIS(0x00);				//Set port to outputs

			//Write the low byte
			NIC_NBE0_LOW_NBE1_HIGH();				//Enable the low byte output

			NIC_DATA_BUS_OP((BYTE)nic_tx_next_byte_next_data_word);
			NIC_WR(0);
			NIC_DELAY_READ_WRITE();
			NIC_WR(1);

			//WriteGet the high byte
			NIC_NBE0_HIGH_NBE1_LOW();				//Enable the high byte output

			NIC_DATA_BUS_OP(data);
			NIC_WR(0);
			NIC_DELAY_READ_WRITE();
			NIC_WR(1);
		#endif

		#ifdef NIC_BUS_WIDTH_IS_16_BIT
			//USING 16 BIT BUS - WRITE WORD
			nic_tx_next_byte_next_data_word |= ((WORD)data << 8);

			NIC_DATA_BUS_TRIS(0x0000);				//Set port to outputs

			NIC_DATA_BUS_OP(nic_tx_next_byte_next_data_word);
			NIC_WR(0);
			NIC_DELAY_READ_WRITE();
			NIC_WR(1);
		#endif

		nic_tx_next_byte_is_odd_byte = 1;
	}
}



//*************************************
//*************************************
//********** NIC WRITE ARRAY **********
//*************************************
//*************************************
//(nic_setup_tx must have already been called)
void nic_write_array (BYTE *array_buffer, WORD array_length)
{

	for( ;array_length > 0; array_length--)
	{
		nic_write_next_byte(*array_buffer);
		array_buffer++;
	}
}



//*********************************************************
//*********************************************************
//********** NIC WRITE WORD AT SPECIFIC LOCATION **********
//*********************************************************
//*********************************************************
//byte_address must be word aligned
void nic_write_tx_word_at_location (WORD byte_address, WORD data)
{
	WORD last_pointer_value;
	WORD pointer_value;

	//GET THE CURRENT POINTER LOCATION
	last_pointer_value = 0x0800;
	while (last_pointer_value & 0x0800)						//NOT EMPTY bit must be low
		last_pointer_value = nic_read(NIC_REG_POINTER);

	//MOVE THE POINTER TO THE REQUESTED WORD ADDRESS
	pointer_value = ((last_pointer_value & 0xf800) + byte_address + 4);		//+4 to get past Status and Byte count nic words before the data
	nic_write(NIC_REG_POINTER, pointer_value);
	nic_write(NIC_REG_DATA, data);

	//RESTORE POINTER TO PREVIOUS LOCATION
	nic_write(NIC_REG_POINTER, last_pointer_value);
	nic_write_address(NIC_REG_DATA);
}



//**************************************************
//**************************************************
//********** WRITE ETHERNET HEADER TO NIC **********
//**************************************************
//**************************************************
//nic_setup_tx() must have been called first
void write_eth_header_to_nic (MAC_ADDR *remote_mac_address, WORD ethernet_packet_type)
{
	//Send remote MAC [6]
	nic_write_next_byte(remote_mac_address->v[0]);
	nic_write_next_byte(remote_mac_address->v[1]);
	nic_write_next_byte(remote_mac_address->v[2]);
	nic_write_next_byte(remote_mac_address->v[3]);
	nic_write_next_byte(remote_mac_address->v[4]);
	nic_write_next_byte(remote_mac_address->v[5]);

	//Send our MAC [6]
	nic_write_next_byte(our_mac_address.v[0]);
	nic_write_next_byte(our_mac_address.v[1]);
	nic_write_next_byte(our_mac_address.v[2]);
	nic_write_next_byte(our_mac_address.v[3]);
	nic_write_next_byte(our_mac_address.v[4]);
	nic_write_next_byte(our_mac_address.v[5]);

	//Send type [2]
	nic_write_next_byte((BYTE)(ethernet_packet_type >> 8));
	nic_write_next_byte((BYTE)(ethernet_packet_type & 0x00ff));
}



//**************************************************************
//**************************************************************
//********** TRANSMIT THE PACKET IN THE NIC TX BUFFER **********
//**************************************************************
//**************************************************************
void nix_tx_packet (void)
{
	
	//-----------------------------------------------------------
	//----- IF PACKET IS BELOW MINIMUM LENGTH ADD PAD BYTES -----
	//-----------------------------------------------------------
	while (nic_tx_len < 60)
	{
		nic_write_next_byte(0x00);
	}

	//--------------------------------
	//----- ADD THE ETHERNET CRC -----
	//--------------------------------
	//No need - the nic does this for us

	//------------------------------------------------------------------
	//----- ADD THE CONTROL BYTE AND THE FINAL ODD BYTE IF PRESENT -----
	//------------------------------------------------------------------
	if (nic_tx_next_byte_is_odd_byte)
	{
		nic_write(NIC_REG_DATA, 0x0000);
	}
	else
	{
		nic_write(NIC_REG_DATA, ((nic_tx_next_byte_next_data_word & 0xff) | 0x2000));
		nic_tx_len--;									//(Decrement as the final odd byte is included below)
	}

	//-------------------------------------------------
	//----- WRITE THE TOTAL NUMBER OF BYTES TO TX -----
	//-------------------------------------------------
	//Setup the pointer register for a data write to the tx memory buffer

	while(nic_read(NIC_REG_POINTER) & 0x0800)			//Check the data write FIFO is empty before writing to the pointer register
		;

	nic_write(NIC_REG_POINTER, (0x4000 + 2));			//+2 to offset to the BYTE COUNT register

	//Write the tx byte count
	nic_write(NIC_REG_DATA, (nic_tx_len + 6));			//(+6 nic bytes: status[2], byte count [2], control byte[1], last data byte[1])

	//---------------------------
	//----- SEND THE PACKET -----
	//---------------------------
	//Ensure MMU isn't busy
	while (nic_read(NIC_REG_MMU_COMMAND) & 0x0001)
		;

	//Send 'ENQUEUE PACKET NUMBER TO TX FIFO' MMU Command
	nic_write(NIC_REG_MMU_COMMAND, 0x00c0);

	return;
}



//*****************************************
//*****************************************
//********** READ NIC LINK SETUP **********
//*****************************************
//*****************************************
//Returns:
//	Bit 0 = high for 100Base-T speed, low for 10Base-T speed
//	Bit 1 = high for full duplex, low for half duplex
BYTE nic_read_link_setup (void)
{
	WORD data;
	BYTE return_status = 0;

	//----- READ THE LINK STATUS REGISTER FROM THE PHY -----
	//Dummy read (it doesn't load the correct value the first time)
	nic_read_phy_register (NIC_PHY_STATUS_OUTPUT);

	//Read the status output register 
	data = nic_read_phy_register (NIC_PHY_STATUS_OUTPUT);

	if (data & 0x0080)
	{
		//HIGH SPEED
		return_status |= 0x01;
	}

	if (data & 0x0040)
	{
		//FULL DUPLEX
		return_status |= 0x02;
	}

	//----- SETUP THE TX REGISTER TO MATCH THE LINK STATUS -----
	//Set bank 0
	nic_write(NIC_REG_BANK, 0x3300);

	//Get the Transmit Control Register
	data = nic_read(NIC_REG_TCR);

	//If DPLXDET (full duplex) was set then the TX reg SWFDUP bit (15) must be set
	data &= 0x7fff;
	if (return_status & 0x02)
		data |= 0x8000;

	//Re-write the transmit Control register
	nic_write(NIC_REG_TCR, data);

	return (return_status);
}



//*************************************************************
//*************************************************************
//********** NIC RECEIVE OVERFLOW - CLEAR RX BUFFERS **********
//*************************************************************
//*************************************************************
void nic_rx_overflow_clear (void)
{
	
	nic_rx_packet_waiting_to_be_dumped = 0;
	
	//----- READ THE RX FIFO -----
	//Set nic bank 2
	nic_write(NIC_REG_BANK, 0x3302);

	//Get the Fifo Register
	while ((nic_read(NIC_REG_FIFO_PORTS) & 0x8000) == 0)		//Is there a packet waiting (REMPTY bit 15 = 0)?
	{
		//----- THERE IS A PACKET WAITING -----
		//Setup the data pointer register to read the rx packet
		while(nic_read(NIC_REG_POINTER) & 0x0800)		//Check the data write FIFO is empty before writing to the pointer register
			;

		nic_write(NIC_REG_POINTER, 0xe000);				//RCV, AUTO INC & READ bits set

		//Get the rx status and byte count
		//(Read the data register to get the packet)
		nic_read(NIC_REG_DATA);

		//Dump the packet
		nic_rx_dump_packet();

		//Set nic bank 2 (ready for next read of Fifo)
		nic_write(NIC_REG_BANK, 0x3302);
	}

	//----- ALL DONE - CLEAR THE OVERFLOW IRQ FLAG -----
	//Set nic bank 2 (ready for next read of Fifo)
	//nic_write(NIC_REG_BANK, 0x3302);

	//Clear the RX_OVR bit if set
	nic_write(NIC_REG_INTERRUPT, (NIC_CONST_IRQ_REGISTER | 0x0010));		//(little endian)
	
}



//***************************************
//***************************************
//********** WRITE NIC ADDRESS **********
//***************************************
//***************************************
static void nic_write_address (WORD address)
{

	//(We only use 3 pins for addressing)
	if (address & 0x0002)
		NIC_ADDR_1(1);
	else
		NIC_ADDR_1(0);

	if (address & 0x0004)
		NIC_ADDR_2(1);
	else
		NIC_ADDR_2(0);

	if (address & 0x0008)
		NIC_ADDR_3(1);
	else
		NIC_ADDR_3(0);
}



//*******************************
//*******************************
//********** NIC WRITE **********
//*******************************
//*******************************
static void nic_write (WORD address, WORD data)
{
	//Set the address
	nic_write_address(address);

	//Write the data

	#ifdef NIC_BUS_WIDTH_IS_8_BIT
		//USING 8 BIT BUS - WRITE 2 BYTES
		NIC_DATA_BUS_TRIS(0x00);				//Set port to outputs

		//Write the low byte
		NIC_NBE0_LOW_NBE1_HIGH();				//Enable the low byte output

		NIC_DATA_BUS_OP((BYTE)(data & 0x00ff));
		NIC_WR(0);
		NIC_DELAY_READ_WRITE();
		NIC_WR(1);

		//Write the high byte
		NIC_NBE0_HIGH_NBE1_LOW();				//Enable the high byte output

		NIC_DATA_BUS_OP((BYTE)(data >> 8));
		NIC_WR(0);
		NIC_DELAY_READ_WRITE();
		NIC_WR(1);
	#endif

	#ifdef NIC_BUS_WIDTH_IS_16_BIT
		//USING 16 BIT BUS - WRITE WORD
		NIC_DATA_BUS_TRIS(0x0000);				//Set port to outputs

		NIC_DATA_BUS_OP(data);
		NIC_WR(0);
		NIC_DELAY_READ_WRITE();
		NIC_WR(1);
	#endif
}



//******************************
//******************************
//********** NIC READ **********
//******************************
//******************************
static WORD nic_read (WORD address)
{
	WORD data;

	//Set the address
	nic_write_address(address);

	//Get the data
	#ifdef NIC_BUS_WIDTH_IS_8_BIT
		//USING 8 BIT BUS - READ 2 BYTES
		//Get the data
		NIC_DATA_BUS_TRIS(0xff);				//Set port to inputs

		//Get the low byte
		NIC_NBE0_LOW_NBE1_HIGH();				//Enable the low byte output
		NIC_RD(0);
		NIC_DELAY_READ_WRITE();
		data = (WORD)NIC_DATA_BUS_IP;
		NIC_RD(1);

		//Get the high byte
		NIC_NBE0_HIGH_NBE1_LOW();				//Enable the high byte output
		NIC_RD(0);
		NIC_DELAY_READ_WRITE();
		data |= ((WORD)NIC_DATA_BUS_IP << 8);
		NIC_RD(1);

		//Return data port to output
		NIC_DATA_BUS_TRIS(0x00);				//Set port to outputs
	#endif

	#ifdef NIC_BUS_WIDTH_IS_16_BIT
		//USING 16 BIT BUS - READ WORD
		//Get the data
		NIC_DATA_BUS_TRIS(0xffff);				//Set port to inputs

		NIC_RD(0);
		NIC_DELAY_READ_WRITE();
		data = NIC_DATA_BUS_IP;
		NIC_RD(1);

		//Return data port to output
		NIC_DATA_BUS_TRIS(0x0000);				//Set port to outputs
	#endif


	return (data);
}



//****************************************
//****************************************
//********** WRITE PHY REGISTER **********
//****************************************
//****************************************
//(Do via the MII serial interface)
void nic_write_phy_register (BYTE address, WORD data)
{
	BYTE b_count;
	WORD w_count;

	//Set nic bank 3
	nic_write(NIC_REG_BANK, 0x3303);

	//Write 32 1's to synchronise the interface	
	for (b_count = 0; b_count < 32; b_count++)
	{
		nic_write(NIC_REG_MGMT, 0x3339);
		nic_write(NIC_REG_MGMT, 0x333d);
	}

	//Write start bits <01>
	nic_write_phy_0();
	nic_write_phy_1();

	//Command bits <Write = 01>
	nic_write_phy_0();
	nic_write_phy_1();

	//PHY Address, which is 00000 for LAN91C111's internal PHY
	nic_write_phy_0();
	nic_write_phy_0();
	nic_write_phy_0();
	nic_write_phy_0();
	nic_write_phy_0();

	//PHY Reg to write - 5 bits - MSB first <Control = 00000>
	for (b_count = 0x10; b_count > 0; b_count >>= 1)
	{
		if (address & b_count)
			nic_write_phy_1();
		else
			nic_write_phy_0();
	}

	//Turnaround bit <10>
	nic_write_phy_1();
	nic_write_phy_0();

	//Data bits (MSB first)
	for (w_count = 0x8000; w_count > 0; w_count >>= 1)
	{
		if (data & w_count)
			nic_write_phy_1();
		else
			nic_write_phy_0();
	}

	//Exit
	nic_write(NIC_REG_MGMT, 0x3330);
}



//***************************************
//***************************************
//********** READ PHY REGISTER **********
//***************************************
//***************************************
//(Via the MII serial interface)
WORD nic_read_phy_register (BYTE address)
{
	BYTE b_count;
	WORD w_count;
	WORD data;

	//Set nic bank 3
	nic_write(NIC_REG_BANK, 0x3303);

	//Write 32 1's to synchronise the interface	
	for (b_count = 0; b_count < 32; b_count++)
	{
		nic_write(NIC_REG_MGMT, 0x3339);
		nic_write(NIC_REG_MGMT, 0x333d);
	}

	//Write start bits <01>
	nic_write_phy_0();
	nic_write_phy_1();

	//Read Command bits <Write = 10>
	nic_write_phy_1();
	nic_write_phy_0();

	//PHY Address, which is 00000 for LAN91C111's internal PHY
	nic_write_phy_0();
	nic_write_phy_0();
	nic_write_phy_0();
	nic_write_phy_0();
	nic_write_phy_0();

	//PHY Reg to read - 5 bits - MSB first <Control = 00000>
	for (b_count = 0x10; b_count > 0; b_count >>= 1)
	{
		if (address & b_count)
			nic_write_phy_1();
		else
			nic_write_phy_0();
	}

	//Turnaround bit <Z>
	nic_write_phy_z();

	//Get data bits (MSB first)
	data = 0;
	//Data bits (MSB first)
	for (w_count = 0x8000; w_count > 0; w_count >>= 1)
	{
		nic_write(NIC_REG_MGMT, 0x3330);

		nic_write(NIC_REG_MGMT, 0x3334);
		if (nic_read(NIC_REG_MGMT) & 0x02)
			data |= w_count;

		nic_write(NIC_REG_MGMT, 0x3330);
	}

	//Turnaround bit <Z>
	nic_write_phy_z();

	return(data);
}



//************************************
//************************************
//********** WRITE 0 TO PHY **********
//************************************
//************************************
static void nic_write_phy_0 (void)
{
	nic_write(NIC_REG_MGMT, 0x3338);
	nic_write(NIC_REG_MGMT, 0x333c);
	nic_write(NIC_REG_MGMT, 0x3338);
}



//************************************
//************************************
//********** WRITE 1 TO PHY **********
//************************************
//************************************
static void nic_write_phy_1 (void)
{
	nic_write(NIC_REG_MGMT, 0x3339);
	nic_write(NIC_REG_MGMT, 0x333d);
	nic_write(NIC_REG_MGMT, 0x3339);
}



//************************************
//************************************
//********** WRITE Z TO PHY **********
//************************************
//************************************
static void nic_write_phy_z (void)
{
	nic_write(NIC_REG_MGMT, 0x3330);
	nic_write(NIC_REG_MGMT, 0x3334);
	nic_write(NIC_REG_MGMT, 0x3330);
}








