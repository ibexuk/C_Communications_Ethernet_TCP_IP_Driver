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
//be at least 1mS - it is only used for the initialisation)
void nic_delay_ms (BYTE delay_ms)
{
	DWORD count;

	count = (INSTRUCTION_CLOCK_FREQUENCY / 1000) / 3;	//Divide by 3 as this loop will take a minimum of 3 steps
														//(may be more depending on compiler but we don't care)
	while(count)
	{
		count--;
	}
}




//***********************************
//***********************************
//********** RESET THE NIC **********
//***********************************
//***********************************
void nic_reset (void)
{
	NIC_WR(1);
	NIC_RD(1);
	NIC_DATA_BUS_TRIS(0xff);			//Set port to inputs
	nic_write_address(0);

	//Reset high for >800nS
	NIC_RESET_PIN(1);

	NIC_DELAY_800NS();

	//Low to run
	NIC_RESET_PIN(0);
}



//************************************
//************************************
//********** INITIALISE NIC **********
//************************************
//************************************
//Call with:
//init_config = don't care (speed configuration options not available with this IC)
void nic_initialise (BYTE init_config)
{

	//-----------------------------
	//----- DO HARDWARE RESET -----
	//-----------------------------
	nic_delay_ms(10);

	//RESET THE NIC
	nic_reset();

	nic_delay_ms(50);

	//Wait for reset bit to be set
	while((nic_read(NIC_REG_ISR) & 0x80) == 0);

	//-------------------------------------------------
	//----- WRITE THE NIC CONFIGURATION REGISTERS ----- 
	//-------------------------------------------------
	//As the nic doesn't have its own eeprom we need to setup the bank3 config registers
	//Note that the DO pin of where the eeprom would be connected must be pulled low.
	//Config 1 and 2 are not altered by the eeprom change as in jumper mode they use the default state of defined pins
	//Config 3 is affected by the eeprom and contains several bits that are read only (i.e. can only be set by the eeprom).  The important pin
	//is the FUDUP pin which must be low, or the nic looses transmissions by assuming that it can tx while rx is active.  This means that the
	//LED control pins also have to be low.  Ideally the LED pins would be high as it gives the more useful 'Link' and 'Active' led outptus,
	//but this is the compromise.

	//Stop DMA and Set page 3
	nic_write(NIC_REG_CR, 0xe1);

	//Enable writing to the config registers
	nic_write(NIC_REG_9346CR, 0xc0);

	//Config 0 is read only
	//nic_write(NIC_REG_CONFIG0, 0x);

	//Disable IRQ's (rest is set by jumpers)
	nic_write(NIC_REG_CONFIG1, 0x00);

	//Set network medium type
	nic_write(NIC_REG_CONFIG2, 0x00);

	//Set not in sleep mode, not in powerdown mode
	nic_write(NIC_REG_CONFIG3, 0x30);

	//Disable writing to the config registers
	nic_write(NIC_REG_9346CR, 0x00);

	nic_delay_ms(50);

	//Set page 0
	nic_write(NIC_REG_CR, 0x21);

	//Setup the the Data Configuration Register (DCR)
	nic_write(NIC_REG_DCR, 0x58);

	//Clear the remote byte count registers
	nic_write(NIC_REG_RBCR0, 0x00);
	nic_write(NIC_REG_RBCR1, 0x00);

	//Initialise the receive configuration register (monitor mode)
	nic_write(NIC_REG_RCR, 0x20);

	//Turn on loopback mode
	nic_write(NIC_REG_TCR, 0x02);

	//Initialise the receive buffer ring
	//RTL8019AS has 16K bytes of ram
	//Tx buffer starts at 0x4000
	//Rx ring starts / tx buffer ends at 0x0x4600 - this gives 1536 bytes to tx, max tx size allowed on ethernet is 1500 bytes.
	//Rx ring ends at 0x6000 - this gives 6656 bytes to rx (used in 256 byte pages by nic)
	nic_write(NIC_REG_BNDRY, NIC_RX_START_LOC_IN_RING);
	nic_write(NIC_REG_PSTART, NIC_RX_START_LOC_IN_RING);
	nic_write(NIC_REG_PSTOP, NIC_RX_END_LOC_IN_RING);

	//Clear the Interrupt Status Register
	nic_write(NIC_REG_ISR, 0xff);

	//Initialise the interrupt mask register (all irq's disabled)
	nic_write(NIC_REG_IMR, 0x00);

	//Stop DMA and Set page 1
	nic_write(NIC_REG_CR, 0x61);

	//Set the MAC address in the nic registers
	nic_write(NIC_REG_PAR0, our_mac_address.v[0]);
	nic_write(NIC_REG_PAR1, our_mac_address.v[1]);
	nic_write(NIC_REG_PAR2, our_mac_address.v[2]);
	nic_write(NIC_REG_PAR3, our_mac_address.v[3]);
	nic_write(NIC_REG_PAR4, our_mac_address.v[4]);
	nic_write(NIC_REG_PAR5, our_mac_address.v[5]);

	//Initialise the multicast address registers
	//Set all MARx to 0 = reject all multicast
	nic_write(NIC_REG_MAR0, 0x00);
	nic_write(NIC_REG_MAR1, 0x00);
	nic_write(NIC_REG_MAR2, 0x00);
	nic_write(NIC_REG_MAR3, 0x00);
	nic_write(NIC_REG_MAR4, 0x00);
	nic_write(NIC_REG_MAR5, 0x00);
	nic_write(NIC_REG_MAR6, 0x00);
	nic_write(NIC_REG_MAR7, 0x00);

	//Initialise the current pointer (to the same addr as the rx buffer start)
	nic_write(NIC_REG_CURR, NIC_RX_START_LOC_IN_RING);

	//Initialize the interface

	//Set page 0
	nic_write(NIC_REG_CR, 0x21);

	//Wait 1.6mS for any tx/rx to complete
	nic_delay_ms(2);

	//Normal operation, initialize remote dma, fifo threshhold 8 bytes
	nic_write(NIC_REG_DCR, 0xd8);

	//Remote dma byte count = 0000h
	nic_write(NIC_REG_RBCR0, 0x00);
	nic_write(NIC_REG_RBCR1, 0x00);

	//Remote dma start address = 4000h
	nic_write(NIC_REG_RSAR0, 0x00);
	nic_write(NIC_REG_RSAR1, NIC_TX_START_LOC_IN_RING);

	//Monitor mode
	nic_write(NIC_REG_RCR, 0x20);

	//Place NIC in loopback
	nic_write(NIC_REG_TCR, 0x02);

	//Clear all interrupt flags
	nic_write(NIC_REG_ISR, 0xff);

	//Unmask all interrupts
	nic_write(NIC_REG_IMR, 0xff);

	//4000h < tx buffer
	nic_write(NIC_REG_TPSR, NIC_TX_START_LOC_IN_RING);

	//Stop nic, change to register page 1
	nic_write(NIC_REG_CR, 0x61);

	//Next place to rx a packet
	nic_write(NIC_REG_CURR, NIC_RX_START_LOC_IN_RING);

	//Start nic, abort remote dma (page 0)
	nic_write(NIC_REG_CR, 0x22);

	//Change from loopback mode to normal op
	nic_write(NIC_REG_TCR, 0x00);

	//Accept broadcast packets
	nic_write(NIC_REG_RCR, 0x04);

	//Clear any pending interrupts
	nic_write(NIC_REG_ISR, 0xff);

	//Put in start mode
	//Start nic and set page 1
	nic_write(NIC_REG_CR, 0x22);

	//Initialise the Transmit Configuration register
	nic_write(NIC_REG_TCR, 0x00);					//Normal tx, CRC appended by transmitter, remote tx disable command disabled (also set in the receive overlflow routine)

	//Accept broadcast packets
	nic_write(NIC_REG_RCR, 0x04);					//Accept broadcast but not multicast, packets with receive errors are rejected

	//----- DO FINAL FLAGS SETUP -----
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
	BYTE current_register;
	BYTE received_packet_status;


	//--------------------------------------------------------
	//----- IF NIC ISN'T FLAGGING AN INTERRUPT THEN EXIT -----
	//--------------------------------------------------------
	//if (NIC_INT0_IRQ == 0)
	//	return(0);

	//--------------------------------------
	//----- CHECK FOR RECEIVE OVERFLOW -----
	//--------------------------------------
	//Set nic page 0
	nic_write(NIC_REG_CR, 0x20);

	//Read the interrupt status register
	if (nic_read(NIC_REG_ISR) & 0x10)
	{
		//-----------------------------------
		//----- RECEIVE BUFFER OVERFLOW -----
		//-----------------------------------
		nic_rx_overflow_clear();
		return(0);
	}

	//--------------------------------
	//----- CHECK FOR RX WAITING -----
	//--------------------------------
	//Read the 'current' register (front of ring / write pointer)
	nic_write(NIC_REG_CR, 0x62);			//Set page 1 and abort DMA
	current_register = nic_read(NIC_REG_CURR);

	//Read the 'boundary' register (back of the receive ring / read pointer)
	nic_write(NIC_REG_CR, 0x22);			//Set page 0 and abort DMA
	boundary_register = nic_read(NIC_REG_BNDRY);

	//Check difference between current and boundary - if different then there is at least 1 packet to process
	if (current_register != boundary_register)
	{
		//--------------------------------------
		//----- A PACKET HAS BEEN RECEIVED -----
		//--------------------------------------

		//Setup the data pointer register to read the rx packet
		nic_setup_read_data();
		nic_rx_bytes_remaining = 4;			//(Required to avoid nic_read_next_byte returning 0)	

		//----- GET THE PRE PACKET HEADER -----
		//GET THE STATUS BYTE [byte 0]
		nic_read_next_byte(&received_packet_status);

		//GET THE NEXT PACKET POINTER VALUE [byte 1]
		//Store ready to be used when this packet is dumped
		nic_read_next_byte(&nic_rx_next_packet_pointer_value);

		//GET THE PACKET LENGTH [bytes 2 & 3]
		nic_read_next_byte(&b_data);
		data = (WORD)b_data;
		nic_read_next_byte(&b_data);
		data += (((WORD)b_data) << 8);

		nic_rx_bytes_remaining = data - 4;
		nic_rx_packet_total_ethernet_bytes = nic_rx_bytes_remaining;

		//----- CHECK THE STATUS BYTE -----
		//Check the status byte 'Received Packet Intact' flag is set to indicate packet was received without errors
		if ((received_packet_status & 0x01) == 0)
		{
			nic_rx_dump_packet();
			return(0);
		}

		//The ethernet stack processing routine will continue receiving the rest of the packet
		nic_rx_packet_waiting_to_be_dumped = PROCESS_NIC_CALLS_BEFORE_DUMP_RX;
		return(nic_rx_bytes_remaining);
	}

	//----------------------------------
	//----- CHECK IF NIC IS LINKED -----
	//----------------------------------
	//Set Page 3
	nic_write(NIC_REG_CR, 0xe0);

	//Read CONFIG0.
	//'BNC' bit = 0 if nic is linked
	if ((nic_read(NIC_REG_CONFIG0) & 0x04) == 0)
	{
		//----- NIC IS LINKED -----
		nic_is_linked = 1;
	}
	else
	{
		//----- NIC IS NOT LINKED -----
		if (nic_is_linked)
		{
			//----- NIC HAS JUST LOST LINK -----
			//Reset the nic ready for the next link
			nic_initialise(0);
		}
		nic_is_linked = 0;
	}

	//Set back to page 0.
	nic_write(NIC_REG_CR, 0x20);


	//<<<<<<< ADD OTHER IRQ PIN HANDLERS HERE IF IRQ'S USED


	return(0);
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
	
	//Start dma on next packet using the 'send packet' auto mode
	nic_write(NIC_REG_CR, 0x1a);

	//Set address for dma reads
	nic_write_address(NIC_REG_REM_DMA);

	//Next operation can read the nic and data will be transfered from the data buffer
}



//****************************************
//****************************************
//********** NIC READ NEXT BYTE **********
//****************************************
//****************************************
//(nic_setup_read_data must have already been called)
//Returns 1 if read successful, 0 if there are no more bytes in the rx buffer
BYTE nic_read_next_byte (BYTE *data)
{

	//Check for all of packet has been read
	if (nic_rx_bytes_remaining == 0)
	{
		*data = 0;
		return(0);
	}

	nic_rx_bytes_remaining--;

	//READ NEXT BYTE
	NIC_DATA_BUS_TRIS(0xff);			//Set port to inputs

	NIC_RD(0);
	NIC_DELAY_READ_WRITE();
	*data = (WORD)NIC_DATA_BUS_IP;
	NIC_RD(1);

	//RETURN DATA PORT TO OUTPUT
	NIC_DATA_BUS_TRIS(0x00);			//Set port to outputs
	
	return(1);
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
	WORD_VAL new_pointer_value;

	//Set page 0 and abort any active DMA
	nic_write(NIC_REG_CR, 0x21);

	//Wait for DMA to complete
	//while ((nic_read(NIC_REG_ISR) & 0x40 == 0));

	//----- SET THE NEW ADDRESS -----
	new_pointer_value.Val = ((WORD)boundary_register << 8) + move_pointer_to_ethernet_byte + 4;	//+4 to move past the nic status and byte count

	nic_write(NIC_REG_RSAR0, new_pointer_value.v[0]);
	nic_write(NIC_REG_RSAR1, new_pointer_value.v[1]);

	//Write a dummy value to the number of bytes to be read
	nic_write(NIC_REG_RBCR0, (BYTE)(1536 & 0x00ff));
	nic_write(NIC_REG_RBCR1, (BYTE)(1536 >> 8));

	//Set for remote read
	nic_write(NIC_REG_CR, 0x0a);

	//Set address for dma reads
	nic_write_address(NIC_REG_REM_DMA);

	//Next operation can read the nic and data will be transfered from the data buffer

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

	//Set page 0 and abort DMA
	nic_write(NIC_REG_CR, 0x22);

	//Wait for DMA to complete
	//while ((nic_read(NIC_REG_ISR) & 0x40 == 0));

	//Change boundary reg to start of next packet
	nic_write(NIC_REG_BNDRY, nic_rx_next_packet_pointer_value);

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

	//----- CHECK THE NIC ISN'T OVERFLOWED -----
	//(can happen if nic processing has been stopped for a length of time by other application processes)

	//Set nic page 0
	nic_write(NIC_REG_CR, 0x20);

	//Read the interrupt status register
	if (nic_read(NIC_REG_ISR) & 0x10)
	{
		//Overflow has occured
		nic_rx_overflow_clear();
	}

	//----- EXIT IF NIC IS STILL DOING LAST TX -----
	//Stop dma and set page 0
	nic_write(NIC_REG_CR, 0x22);

	//Is nic tx still busy from last tx?
	if (nic_read(NIC_REG_CR) & 0x04)
		return(0);

	//----- ABORT ANY DMA CURRENTLY ACTIVE -----
	nic_write(NIC_REG_CR, 0x20);

	//Wait for DMA to complete
	//while ((nic_read(NIC_REG_ISR) & 0x40 == 0));

	//Set no of bytes to transmit to nic via dma (dummy value)
	nic_write(NIC_REG_RBCR0, (BYTE)(1536 & 0x00ff));
	nic_write(NIC_REG_RBCR1, (BYTE)(1536 >> 8));

	//Set remote DMA address to write to
	nic_write(NIC_REG_CRDA0, 0x00);
	nic_write(NIC_REG_CRDA1, NIC_TX_START_LOC_IN_RING);

	//Set remote write
	nic_write(NIC_REG_CR, 0x12);

	//Set the address to DMA
	nic_write_address(NIC_REG_REM_DMA);

	nic_tx_len = 0;

	//Next byte is the 1st byte of the Ethernet Frame

	return(1);
}



//********************************************
//********************************************
//********** NIC TX WRITE NEXT BYTE **********
//********************************************
//********************************************
//(nic_setup_tx must have already been called)
void nic_write_next_byte (BYTE data)
{

	//UPDATE THE TX LEN COUNT AND EXIT IF TOO MANY BYTES HAVE BEEN WRITTEN FOR THE NIC
	if(nic_tx_len >= 1536)
		return;

	nic_tx_len++;

	//WRITE THE BYTE
	NIC_DATA_BUS_TRIS(0x00);			//Set port to outputs

	NIC_DATA_BUS_OP(data);
	NIC_WR(0);
	NIC_DELAY_READ_WRITE();
	NIC_WR(1);
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
	WORD_VAL last_pointer_value;
	WORD_VAL pointer_value;

	//Set page 0 and abort any active DMA
	nic_write(NIC_REG_CR, 0x21);

	//Wait for DMA to complete
	//while ((nic_read(NIC_REG_ISR) & 0x40 == 0));

	//GET THE CURRENT POINTER LOCATION
	last_pointer_value.v[0] = nic_read(NIC_REG_CRDA0);
	last_pointer_value.v[1] = nic_read(NIC_REG_CRDA1);

	//MOVE THE POINTER TO THE REQUESTED WORD ADDRESS
	pointer_value.Val = (((WORD)NIC_TX_START_LOC_IN_RING << 8) + (WORD)byte_address);
	nic_write(NIC_REG_RSAR0, pointer_value.v[0]);
	nic_write(NIC_REG_RSAR1, pointer_value.v[1]);

	//Set number of bytes to write
	nic_write(NIC_REG_RBCR0, 2);
	nic_write(NIC_REG_RBCR1, 0);

	//Set remote write
	nic_write(NIC_REG_CR, 0x12);

	//WRITE THE WORD
	nic_write(NIC_REG_REM_DMA, (BYTE)(data & 0x00ff));
	nic_write(NIC_REG_REM_DMA, (BYTE)(data >> 8));

	//Write a dummy value to the number of bytes to be written
	nic_write(NIC_REG_RBCR0, (BYTE)(1536 & 0x00ff));
	nic_write(NIC_REG_RBCR1, (BYTE)(1536 >> 8));

	//RESTORE POINTER TO PREVIOUS LOCATION
	nic_write(NIC_REG_RSAR0, last_pointer_value.v[0]);
	nic_write(NIC_REG_RSAR1, last_pointer_value.v[1]);

	//Set remote write
	nic_write(NIC_REG_CR, 0x12);

	//Set the address to DMA
	nic_write_address(NIC_REG_REM_DMA);
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

	//----------------------------
	//----- SETUP TX POINTER -----
	//----------------------------
	//Set page 0 and abort any active DMA
	nic_write(NIC_REG_CR, 0x20);

	//Wait for DMA to complete
	//while ((nic_read(NIC_REG_ISR) & 0x40 == 0));

	//Set tx pointer to beginning of buffer
	nic_write(NIC_REG_ISR, 0x40);
	nic_write(NIC_REG_TPSR, NIC_TX_START_LOC_IN_RING);

	//-------------------------------------------------
	//----- WRITE THE TOTAL NUMBER OF BYTES TO TX -----
	//-------------------------------------------------
	//Set no of bytes to transmit
	nic_write(NIC_REG_TBCR0, (BYTE)(nic_tx_len & 0x00ff));
	nic_write(NIC_REG_TBCR1, (BYTE)(nic_tx_len >> 8));

	//---------------------------
	//----- SEND THE PACKET -----
	//---------------------------
	nic_write(NIC_REG_CR, 0x26);
}



//*************************************************************
//*************************************************************
//********** NIC RECEIVE OVERFLOW - CLEAR RX BUFFERS **********
//*************************************************************
//*************************************************************
//Do special process to get all received packets from the rx ring and then reset the nic		
void nic_rx_overflow_clear (void)
{
	BYTE b_temp;
	BYTE resend_flag;
	BYTE current_register;
	BYTE boundary_register;
	BYTE received_packet_status;

	
	nic_rx_packet_waiting_to_be_dumped = 0;

	//Read and store the value of the TXP bit in the command register (CR)
	b_temp = nic_read(NIC_REG_CR);					//TXP is bit 2

	//Issue the stop command
	nic_write(NIC_REG_CR, 0x21);

	//Wait 1.6mS for any tx or rx to complete
	nic_delay_ms(2);

	//Clear the remote byte registers
	nic_write(NIC_REG_RBCR0, 0x00);
	nic_write(NIC_REG_RBCR1, 0x00);

	//Read the value of the TXP bit
	if ((b_temp & 0x04) == 0)
	{
		//TXP is 0 - set the 'resend' variable to 0
		resend_flag = 0;
	}
	else
	{
		//TXP is 1 - read the Interrupt Status Register (ISR)
		b_temp = nic_read(NIC_REG_ISR);
		if ((b_temp & 0x02) || (b_temp & 0x08))		//PTX = 1 or TXE = 1?
		{
			//One of the flags is 1 - set the 'resend' variable to 0
			resend_flag = 0;
		}
		else
		{
			//Neither is 1, set the 'resend' variable to 1
			resend_flag = 1;
		}
	}

	//Set mode 1 loopback
	nic_write(NIC_REG_TCR, 0x02);

	//Issue start command to allow remote DMA
	nic_write(NIC_REG_CR, 0x22);

	//----- REMOVE PENDING PACKETS FROM THE RECEIVE BUFFER RING -----
	//Read the 'current' register (front of ring / write pointer)
	nic_write(NIC_REG_CR, 0x62);			//Set page 1 and abort DMA
	current_register = nic_read(NIC_REG_CURR);

	//Read the 'boundary' register (back of the receive ring / read pointer)
	nic_write(NIC_REG_CR, 0x22);			//Set page 0 and abort DMA
	boundary_register = nic_read(NIC_REG_BNDRY);

	//Check difference between current and boundary - if different then there is at least 1 packet to process
	while (current_register != boundary_register)
	{
		//Setup the data pointer register to read the rx packet
		nic_setup_read_data();
		nic_rx_bytes_remaining = 4;			//(Required to avoid nic_read_next_byte returning 0)	

		//----- GET THE PRE PACKET HEADER -----
		//GET THE STATUS BYTE [byte 0]
		nic_read_next_byte(&received_packet_status);

		//GET THE NEXT PACKET POINTER VALUE [byte 1]
		//Store ready to be used when this packet is dumped
		nic_read_next_byte(&nic_rx_next_packet_pointer_value);

		//GET THE PACKET LENGTH [bytes 2 & 3]
		nic_read_next_byte(&b_temp);
		nic_read_next_byte(&b_temp);

		nic_rx_dump_packet();

		//Read the 'current' register (front of ring / write pointer)
		nic_write(NIC_REG_CR, 0x62);			//Set page 1 and abort DMA
		current_register = nic_read(NIC_REG_CURR);

		//Read the 'boundary' register (back of the receive ring / read pointer)
		nic_write(NIC_REG_CR, 0x22);			//Set page 0 and abort DMA
		boundary_register = nic_read(NIC_REG_BNDRY);
	}

	//----- CLEAR THE OVERFLOW WARNING BIT IN THE INTERRUPT STATUS REGISTER -----
	nic_write(NIC_REG_ISR, 0xff);

	//----- CANCEL LOOPBACK MODE AND SET TRANSMIT BACK TO NORMAL -----
	nic_write(NIC_REG_TCR, 0x00);				//(TX CRC enabled)

	//----- IF THE 'RESEND' VARIABLE IS SET TO 1 THEN RE-ISSUE THE TRANSMIT COMMAND -----
	if (resend_flag)
	{
		nic_write(NIC_REG_CR, 0x26);
	}

}



//***************************************
//***************************************
//********** WRITE NIC ADDRESS **********
//***************************************
//***************************************
static void nic_write_address (WORD address)
{
	if (address & 0x01)
		NIC_ADDR_0(1);
	else
		NIC_ADDR_0(0);

	if (address & 0x02)
		NIC_ADDR_1(1);
	else
		NIC_ADDR_1(0);

	if (address & 0x04)
		NIC_ADDR_2(1);
	else
		NIC_ADDR_2(0);

	if (address & 0x08)
		NIC_ADDR_3(1);
	else
		NIC_ADDR_3(0);

	if (address & 0x10)
		NIC_ADDR_4(1);
	else
		NIC_ADDR_4(0);
}



//*******************************
//*******************************
//********** NIC WRITE **********
//*******************************
//*******************************
static void nic_write (WORD address, BYTE data)
{

	//Set the address
	nic_write_address(address);

	//Write the data
	NIC_DATA_BUS_TRIS(0x00);			//Set port to outputs
	NIC_DATA_BUS_OP(data);
	NIC_WR(0);
	NIC_DELAY_READ_WRITE();
	NIC_WR(1);

}



//******************************
//******************************
//********** NIC READ **********
//******************************
//******************************
static BYTE nic_read (WORD address)
{
	BYTE data;

	//Set the address
	nic_write_address(address);

	//Get the data
	NIC_DATA_BUS_TRIS(0xff);			//Set port to inputs

	NIC_RD(0);
	NIC_DELAY_READ_WRITE();
	data = (WORD)NIC_DATA_BUS_IP;
	NIC_RD(1);

	//Return data port to output
	NIC_DATA_BUS_TRIS(0x00);			//Set port to outputs

	return (data);
}









