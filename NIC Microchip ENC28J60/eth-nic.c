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
//MICROCHIP ENC28J60 (NETWORK INTERFACE CONTROLLER) C CODE FILE


#include "main.h"					//Global data type definitions (see https://github.com/ibexuk/C_Generic_Header_File )
#define	NIC_C
#include "eth-nic.h"
#undef	NIC_C

#include "eth-main.h"



//###### IMPORTNAT NOTE #####
//ALL CODE IN THIS MODULE EXPECTS NIC MEMORY BANK 0 TO BE CURRENTLY SELECTED.
//IF CODE EVER CHANGES THE BANK, IT MUST RESTORE IT TO BANK 0 BEFORE RETURNING.



//*******************************
//*******************************
//********** 1mS DELAY **********
//*******************************
//*******************************
//(The accuracy of this function is not important as long as calling with a value of 1 means the delay will
//be at least 1mS - it is only used for the initialisation)
void nic_delay_1ms (void)
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

	NIC_CS(1);

	//Reset high for >2uS
	NIC_RESET_PIN(1);

	NIC_DELAY_400NS;
	NIC_DELAY_400NS;
	NIC_DELAY_400NS;
	NIC_DELAY_400NS;
	NIC_DELAY_400NS;
	
	//Low to reset for at least 400nS
	NIC_RESET_PIN(0);
	NIC_DELAY_400NS;

	//High to run
	NIC_RESET_PIN(1);

	//Pause 1mS
	nic_delay_1ms();
}



//************************************
//************************************
//********** INITIALISE NIC **********
//************************************
//************************************
//Call with:
//init_config = don't care (speed configuration options are not available with this IC)
void nic_initialise (BYTE init_config)
{
	BYTE b_temp;

	//-----------------------------
	//----- DO HARDWARE RESET -----
	//-----------------------------
	//RESET THE NIC
	nic_reset();



	//-------------------------------------------------
	//----- WRITE THE NIC CONFIGURATION REGISTERS ----- 
	//-------------------------------------------------

	//Ensure that we can communicate with the ENC28J60 before proceeding - Wait for CLKRDY to become set.
	//ESTAT bit 3 is unimplemented and should be low.  If it reads as high there is something wrong with the SPI connection.
	do
	{
		b_temp = nic_read(NIC_ESTAT).Val;
	} while((b_temp & 0x08) || (~b_temp & ESTAT_CLKRDY));


	//RESET the entire ENC28J60, clearing all registers
	NIC_CS(0);
	NIC_SPI_TX_BYTE(NIC_SR);
	while(!NIC_SPI_BUF_FULL)			//Wait for SPI to complete
		;
		
	b_temp = NIC_SPI_RX_BYTE_BUFFER;
	NIC_CS(1);

    nic_delay_1ms();

	//Select Bank 0 and configure receive buffer boundary pointers and buffer write protect pointer / receive buffer read pointer
	nic_next_packet_location.Val = NIC_RXSTART;

	nic_write(NIC_ERXSTL, (BYTE)(NIC_RXSTART & 0x00ff));
	nic_write(NIC_ERXSTH, (BYTE)(NIC_RXSTART >> 8));
	nic_write(NIC_ERXRDPTL, (BYTE)(NIC_RXSTOP & 0x00ff));	//Write low byte first
	nic_write(NIC_ERXRDPTH, (BYTE)(NIC_RXSTOP >> 8));
	nic_write(NIC_ERXNDL, (BYTE)(NIC_RXSTOP & 0x00ff));
	nic_write(NIC_ERXNDH, (BYTE)(NIC_RXSTOP >> 8));
	nic_write(NIC_ETXSTL, (BYTE)(NIC_TXSTART & 0x00ff));
	nic_write(NIC_ETXSTH, (BYTE)(NIC_TXSTART >> 8));

	//Write a permanant per packet control byte of 0x00
	nic_write(NIC_EWRPTL, (BYTE)(NIC_TXSTART & 0x00ff));
	nic_write(NIC_EWRPTH, (BYTE)(NIC_TXSTART >> 8));
	nic_write_buffer(0x00);


	//Enter Bank 1 and configure Receive Filters 
	//(No need to reconfigure - Unicast OR Broadcast with CRC checking is acceptable)
	//Write ERXFCON_CRCEN only to ERXFCON to enter promiscuous mode

	//Promiscious mode example:
	//nic_select_bank(NIC_ERXFCON);
	//nic_write((BYTE)NIC_ERXFCON, ERXFCON_CRCEN);

	//Enter Bank 2 and configure the MAC
	nic_select_bank(NIC_MACON1);

	//Enable the receive portion of the MAC
	nic_write((BYTE)NIC_MACON1, MACON1_TXPAUS | MACON1_RXPAUS | MACON1_MARXEN);
	
	//Pad packets to 60 bytes, add CRC, and check the Type / Length field.
	//#if defined(FULL_DUPLEX)
	//	nic_write((BYTE)NIC_MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN | MACON3_FULDPX);
	//	nic_write((BYTE)NIC_MABBIPG, 0x15);	
	//#else
		nic_write((BYTE)NIC_MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN);
		nic_write((BYTE)NIC_MABBIPG, 0x12);	
	//#endif

    //Allow infinite deferals if the link is busy 
    //(Do not time out a transmission if the link is saturated with other traffic
    nic_write((BYTE)NIC_MACON4, MACON4_DEFER);

	//Late collisions occur beyond 63+8 bytes (8 bytes for preamble/start of frame delimiter)
	//55 is all that is needed for IEEE 802.3, but ENC28J60 B5 errata for improper link pulse collisions will occur less often with a larger number.
    nic_write((BYTE)NIC_MACLCON2, 63);
	
	//Set non-back-to-back inter-packet gap to 9.6us.  The back-to-back inter-packet gap (MABBIPG) is set by MACSetDuplex() which is called later.
	nic_write((BYTE)NIC_MAIPGL, 0x12);
	nic_write((BYTE)NIC_MAIPGH, 0x0C);

	//Set the maximum packet size which the controller will accept
	nic_write((BYTE)NIC_MAMXFLL, (BYTE)((6 + 6 + 2 + 1500 + 4) & 0x00ff));		//1518 is IEEE 802.3 specified limit
	nic_write((BYTE)NIC_MAMXFLH, (BYTE)((6 + 6 + 2 + 1500 + 4) >> 8));
	
    //Select Bank 3 and initialize MAC address registers
	nic_select_bank(NIC_MAADR1);
    nic_write((BYTE)NIC_MAADR1, our_mac_address.v[0]);
    nic_write((BYTE)NIC_MAADR2, our_mac_address.v[1]);
    nic_write((BYTE)NIC_MAADR3, our_mac_address.v[2]);
    nic_write((BYTE)NIC_MAADR4, our_mac_address.v[3]);
    nic_write((BYTE)NIC_MAADR5, our_mac_address.v[4]);
    nic_write((BYTE)NIC_MAADR6, our_mac_address.v[5]);

	//Disable the CLKOUT output to reduce EMI emisssions
	nic_write((BYTE)NIC_ECOCON, 0x00);		//Output off (0V)
	//nic_write((BYTE)NIC_ECOCON, 0x01);	//25.000MHz
	//nic_write((BYTE)NIC_ECOCON, 0x03);	//8.3333MHz (*4 with PLL is 33.3333MHz)

	//Get the nic's revision ID value so that we can implement errata workarounds when needed
	enc28j60_revision_id = nic_read((BYTE)NIC_EREVID).Val;

	//Disable half duplex loopback in PHY (Bank will be changed to Bank 2)
	nic_write_phy_register(NIC_PHCON2, PHCON2_HDLDIS);

	//----- CONFIGURE LED'S -----
	//xxx0: Pulse stretching disabled
	//xxx2: Pulse stretch to 40ms (default)
	//xxx6: Pulse stretch to 73ms
	//xxxA: Pulse stretch to 139ms
	//							
	//xx1x: LEDB - TX
	//xx2x: LEDB - RX (default)
	//xx3x: LEDB - collisions
	//xx4x: LEDB - link
	//xx5x: LEDB - duplex
	//xx7x: LEDB - TX and RX
	//xx8x: LEDB - on
	//xx9x: LEDB - off
	//xxAx: LEDB - blink fast
	//xxBx: LEDB - blink slow
	//xxCx: LEDB - link and RX
	//xxDx: LEDB - link and TX and RX
	//xxEx: LEDB - duplex and collisions
	//
	//LEDB same but in next nibble up
	//x1xx: LEDA - TX
	//etc
	nic_write_phy_register(NIC_PHLCON, 0x3472);


	//----- SET THE MAC AND PHY INTO THE PROPER DUPLEX STATE -----
	//#if defined(FULL_DUPLEX)
	//	nic_write_phy_register(NIC_PHCON1, PHCON1_PDPXMD);
	//#elif defined(HALF_DUPLEX)
		nic_write_phy_register(NIC_PHCON1, 0x0000);
	//#else
	//	{
	//	//Use the external LEDB polarity to determine weather full or half duplex communication mode should be set.  
	//		NIC_REG Register;
	//		PHYREG PhyReg;
	//		
	//		// Read the PHY duplex mode
	//		PhyReg = nic_read_phy_register(PHCON1);
	//		DuplexState = PhyReg.PHCON1bits.PDPXMD;
	//	
	//		// Set the MAC to the proper duplex mode
	//		nic_select_bank(MACON3);
	//		Register = nic_read_mac_mii_register((BYTE)MACON3);
	//		Register.MACON3bits.FULDPX = PhyReg.PHCON1bits.PDPXMD;
	//		nic_write((BYTE)MACON3, Register.Val);
	//	
	//		// Set the back-to-back inter-packet gap time to IEEE specified requirements.  The meaning of the MABBIPG value changes with the duplex
	//		// state, so it must be updated in this function. In full duplex, 0x15 represents 9.6us; 0x12 is 9.6us in half duplex
	//		nic_write((BYTE)MABBIPG, PhyReg.PHCON1bits.PDPXMD ? 0x15 : 0x12);	
	//	}
	//#endif


	//Enable interrupts
	nic_write((BYTE)NIC_EIE, (EIE_INTIE | EIE_PKTIE | EIE_LINKIE));			//A packet received or a link status change will set the interrupt pin low

	//Enable the link change interrupt flag bit
	//(Has to be done for LINKIF to work in the normal interrupt register)
	nic_write_phy_register(NIC_PHIE, (WORD)(PHIE_PLNKIE | PHIE_PGEIE));

	//Return to default Bank 0
	nic_select_bank(NIC_ERDPTL);

	//Enable packet reception
	nic_bit_field_set_register(NIC_ECON1, ECON1_RXEN);


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
	BYTE b_data;
	ENC_PREAMBLE header;
	NIC_REG interrupt_flags;
	PHYREG phy_temp;


	//--------------------------------------------------------
	//----- IF NIC ISN'T FLAGGING AN INTERRUPT THEN EXIT -----
	//--------------------------------------------------------
	if (NIC_INT_PIN)
		return(0);

	interrupt_flags = nic_read(NIC_EIR);

	//----------------------------------
	//----- CHECK IF NIC IS LINKED -----
	//----------------------------------
	if (interrupt_flags.EIRbits.LINKIF)
	{
		phy_temp = nic_read_phy_register(NIC_PHIR);					//Read PHIR register to clear LINKIF flag
		
		if (nic_read_phy_register(NIC_PHSTAT1).PHSTAT1bits.LLSTAT)
		{
			//----- NIC IS LINKED -----
			nic_is_linked = 1;
		}
		else
		{
			//----- NIC IS NOT LINKED OR HAS LOST LINK SINCE LAST TEST -----
			if (nic_is_linked)
			{
				//----- NIC HAS JUST LOST LINK -----
				//Reset the nic ready for the next link
				nic_initialise(0);
				return(0);
			}
		}
	}

	//------------------------------------
	//----- CHECK FOR TRANSMIT ERROR -----
	//------------------------------------
	if (interrupt_flags.EIRbits.TXERIF)
	{
		//CLEAR THE INTERRUPT FLAG
		nic_bit_field_clear_register(NIC_EIR, EIR_TXERIF);
	}

	//--------------------------------
	//----- CHECK FOR RX WAITING -----
	//--------------------------------
	if (interrupt_flags.EIRbits.PKTIF)
	{
		//Test packet count register to see if at least one packet has been received and is waiting
		nic_select_bank(NIC_EPKTCNT);
		b_data = nic_read((BYTE)NIC_EPKTCNT).Val;
		nic_select_bank(NIC_ERDPTL);
		if(b_data)
		{
	     	//--------------------------------------
			//----- A PACKET HAS BEEN RECEIVED -----
			//--------------------------------------
	
			//Setup the data pointer register to read the rx packet
			nic_setup_read_data();
			nic_rx_bytes_remaining = 6;			//(Required to avoid nic_read_next_byte returning 0)	
	
			//----- GET THE PRE PACKET HEADER -----
			nic_read_array((BYTE*)&header, sizeof(header));
	
			//Validate the data returned from the ENC28J60.  Random data corruption, such as if a single SPI bit error occurs while communicating or a 
			//momentary power glitch could cause this to occur in rare circumstances.
			if (
			header.next_packet_pointer > NIC_RXSTOP ||
			((BYTE_VAL*)(&header.next_packet_pointer))->bits.b0 ||
			header.status_vector.bits.Zero ||
			header.status_vector.bits.CRCError ||
			header.status_vector.bits.ByteCount > 1518 ||
			!header.status_vector.bits.ReceiveOk
			)
			{
				nic_initialise(0);
				return(0);
			}

			//Save the location where the hardware will write the next packet to
			nic_next_packet_location.Val = header.next_packet_pointer;
	
			//Store the packet size
			nic_rx_bytes_remaining = header.status_vector.bits.ByteCount - 4;
			nic_rx_packet_total_ethernet_bytes = nic_rx_bytes_remaining;
	
			//----- CHECK THE STATUS BYTE -----
			//Check the receive OK flag
			if (header.status_vector.bits.ReceiveOk == 0)
			{
				nic_rx_dump_packet();
				return(0);
			}
	
			//The ethernet stack processing routine will continue receiving the rest of the packet
			nic_rx_packet_waiting_to_be_dumped = PROCESS_NIC_CALLS_BEFORE_DUMP_RX;
			return(nic_rx_bytes_remaining);
		}
	}

	//<<<<<<< ADD ANY OTHER IRQ PIN HANDLERS HERE

	return(0);
}



//**********************************************************
//**********************************************************
//********** CHECK IF OK TO START A NEW TX PACKET **********
//**********************************************************
//**********************************************************
//Returns:
//	0 = not ready
//	1 = ready
BYTE nic_ok_to_do_tx (void)
{

	//EXIT IF NIC IS NOT CURRENTLY CONNECTED
	if (nic_is_linked == 0)
		return(0);

	//EXIT IF THE NIC HAS A RECEIVED PACKET THAT IS WAITING PROCESSING AND TO BE DUMPED
	if (nic_rx_packet_waiting_to_be_dumped)
		return(0);

	//1 - if no Ethernet transmission is in progress
 	//0 - if a previous transmission was started, and it has not completed yet.  While 0, the data in the 
 	//transmit buffer and the TXST/TXND pointers must not be changed.
	if (nic_read(NIC_ECON1).ECON1bits.TXRTS)
		return(0);
	else
		return(1);
}



//************************************************
//************************************************
//********** NIC SETUP READ DATA BUFFER **********
//************************************************
//************************************************
void nic_setup_read_data (void)
{

	//SET THE SPI READ POINTER TO THE BEGINNING OF THE NEXT UNPROCESSED PACKET
	nic_current_packet_location.Val = nic_next_packet_location.Val;
	nic_write(NIC_ERDPTL, nic_current_packet_location.v[0]);
	nic_write(NIC_ERDPTH, nic_current_packet_location.v[1]);

	//NEXT OPERATION CAN READ THE NIC AND DATA WILL BE TRANSFERRED FROM THE DATA BUFFER

}



//****************************************
//****************************************
//********** NIC READ NEXT BYTE **********
//****************************************
//****************************************
//Read from nic buffer memory
//(nic_setup_read_data must have already been called)
//Returns 1 if read successful, 0 if there are no more bytes in the rx buffer
BYTE nic_read_next_byte (BYTE *data)
{
	BYTE b_temp;
	
	//Check for all of packet has been read
	if (nic_rx_bytes_remaining == 0)
	{
		*data = 0;
		return(0);
	}

	nic_rx_bytes_remaining--;

	//----- READ NEXT BYTE -----

	//SELECT NIC
	NIC_CS(0);
	
	//WRITE COMMAND
	NIC_SPI_TX_BYTE(NIC_RBM);
	while(!NIC_SPI_BUF_FULL)				//Wait for SPI to complete
		;
	
	b_temp = NIC_SPI_RX_BYTE_BUFFER;
	
	//READ DATA (INCREMENTS ERDPT AND THE INCREMENT WILL FOLLOW THE RECEIVE BUFFER WRAPPING BOUNDARY)
	NIC_SPI_TX_BYTE(0);
	while(!NIC_SPI_BUF_FULL)				//Wait for SPI to complete
		;
		
	*data  = NIC_SPI_RX_BYTE_BUFFER;
	
	//DE-SELECT NIC
	NIC_CS(1);

	return(1);
}



//************************************
//************************************
//********** NIC READ ARRAY **********
//************************************
//************************************
//Read from nic buffer memory
//(nic_setup_read_data must have already been called)
//Returns 1 if read successful, 0 if there are no more bytes in the rx buffer
BYTE nic_read_array (BYTE *array_buffer, WORD array_length)
{
	BYTE b_temp;

	//SETUP FOR SEQUENTIAL READ
	NIC_CS(0);
	NIC_SPI_TX_BYTE(NIC_RBM);
	while(!NIC_SPI_BUF_FULL)					//Wait for SPI to complete
		;
	
	b_temp = NIC_SPI_RX_BYTE_BUFFER;

	//GET THE DATA
	while(array_length)
	{
		//Check for all of packet has been read
		if (nic_rx_bytes_remaining == 0)
		{
			NIC_CS(1);
			while(array_length)
			{
				*array_buffer++ = 0;
				array_length--;
			}
			return(0);
		}
		nic_rx_bytes_remaining--;

		//GET NEXT DATA BYTE
		NIC_SPI_TX_BYTE(0);
		while(!NIC_SPI_BUF_FULL)				//Wait for SPI to complete
			;
		
		*array_buffer++ = NIC_SPI_RX_BYTE_BUFFER;

		array_length--;
	}

	//TERMINATE THE SEQUENTIAL READ
	NIC_CS(1);

    return(1);
}



//**************************************
//**************************************
//********** NIC MOVE POINTER **********
//**************************************
//**************************************
//Moves the pointer to a specified byte location ready to be read next (a value of 0 = the first byte of the Ethernet header)
void nic_move_pointer (WORD move_pointer_to_ethernet_byte)
{
	WORD_VAL new_pointer_value;

	//----- SET THE NEW ADDRESS -----
	new_pointer_value.Val = nic_current_packet_location.Val + move_pointer_to_ethernet_byte + 6;	//+6 to move past the pre packet pointer and status bytes
	
	// Since the receive buffer is circular, adjust if a wraparound is needed
	if(new_pointer_value.Val > NIC_RXSTOP)
		new_pointer_value.Val -= NIC_RXSIZE;
	
	//Set the SPI read pointer to the new calculated value
	nic_write(NIC_ERDPTL, new_pointer_value.v[0]);
	nic_write(NIC_ERDPTH, new_pointer_value.v[1]);

	//Next operation can read the nic and data will be transfered from the data buffer

	//----- ADDRESSED BYTE IS NOW READY TO BE READ -----

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
	WORD_VAL new_rx_read_location;

	//EXIT IF PACKET HAS ALREADY BEEN DISCARDED
	if(nic_rx_packet_waiting_to_be_dumped == 0)
		return;
	
	//DECREMENT THE NEXT PACKET POINTER BEFORE WRITING IT INTO THE ERXRDPT REGISTERS.
	//This is a silicon errata workaround.
	//RX buffer wrapping must be taken into account if the nic_next_packet_location is precisely RXSTART.
	new_rx_read_location.Val = nic_next_packet_location.Val - 1;
	if(new_rx_read_location.Val > NIC_RXSTOP)
	{
		new_rx_read_location.Val = NIC_RXSTOP;
	}

	//DECREMENT THE RX PACKET COUNTER REGISTER, EPKTCNT
	nic_bit_field_set_register(NIC_ECON2, ECON2_PKTDEC);

	//MOVE THE RECEIVE READ POINTER TO UNWRITE-PROTECT THE MEMORY USED BY THE LAST PACKET
	nic_write(NIC_ERXRDPTL, new_rx_read_location.v[0]);			//Write low byte first
	nic_write(NIC_ERXRDPTH, new_rx_read_location.v[1]);

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
	//(Don't need to with this nic)

	//----- EXIT IF NIC IS STILL DOING LAST TX -----
	//Is nic tx still busy from last tx?
	if (nic_read(NIC_ECON1).ECON1bits.TXRTS)
		return(0);

	//----- SETUP TO START WRITING TX DATA -----
	//Set no of bytes to transmit to nic via dma (dummy value)
	//(Not necessary for this nic)

	//Set the SPI write pointer to the start of the TX buffer
	nic_write(NIC_EWRPTL,((BYTE)(NIC_BASE_TX_ADDR & 0x00ff)));
	nic_write(NIC_EWRPTH,((BYTE)(NIC_BASE_TX_ADDR >> 8)));

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
	//----- UPDATE THE TX LEN COUNT AND EXIT IF TOO MANY BYTES HAVE BEEN WRITTEN FOR THE NIC -----
	if(nic_tx_len >= 1536)
		return;

	nic_tx_len++;

	//----- WRITE THE BYTE -----
	nic_write_buffer(data);
}



//*************************************
//*************************************
//********** NIC WRITE ARRAY **********
//*************************************
//*************************************
//(nic_setup_tx must have already been called)
void nic_write_array (BYTE *array_buffer, WORD array_length)
{
	BYTE b_temp;

	//----- SELECT NIC -----
	NIC_CS(0);
	
	//----- WRITE COMMAND -----
	NIC_SPI_TX_BYTE(NIC_WBM);
	while(!NIC_SPI_BUF_FULL)				//Wait for SPI to complete
		;
		
	b_temp = NIC_SPI_RX_BYTE_BUFFER;
	
	//----- WRITE THE DATA -----
	while(array_length)
	{
		//----- UPDATE THE TX LEN COUNT AND EXIT IF TOO MANY BYTES HAVE BEEN WRITTEN FOR THE NIC -----
		if(nic_tx_len >= 1536)
			break;

		NIC_SPI_TX_BYTE(*array_buffer);
		array_buffer++;
		array_length--;

		nic_tx_len++;

		while(!NIC_SPI_BUF_FULL)				//Wait for SPI to complete
			;
			
		b_temp = NIC_SPI_RX_BYTE_BUFFER;
	}

	//----- DE-SELECT THE NIC -----
	NIC_CS(1);
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

	//----- GET THE CURRENT POINTER LOCATION -----
	last_pointer_value.v[0] = nic_read(NIC_EWRPTL).Val;
	last_pointer_value.v[1] = nic_read(NIC_EWRPTH).Val;

	//----- MOVE THE POINTER TO THE REQUESTED ADDRESS -----
	nic_write(NIC_EWRPTL, ((BYTE)((NIC_BASE_TX_ADDR + byte_address) & 0x00ff)));
	nic_write(NIC_EWRPTH, ((BYTE)((NIC_BASE_TX_ADDR + byte_address) >> 8)));

	//----- WRITE THE WORD -----
	nic_write_buffer((BYTE)(data & 0x00ff));
	nic_write_buffer((BYTE)(data >> 8));

	//----- RESTORE POINTER TO PREVIOUS LOCATION -----
	nic_write(NIC_EWRPTL, (last_pointer_value.v[0]));
	nic_write(NIC_EWRPTH, (last_pointer_value.v[1]));
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
	WORD attempt_counter = 0x0000;
	WORD_VAL read_pointer_save;
	WORD_VAL tx_end;
	TXSTATUS tx_status;
	BYTE b_temp;

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

	//-------------------------------------
	//----- SILICON ERRATA WORKAROUND -----
	//-------------------------------------
	//Reset transmit logic if a TX Error has previously occured
	nic_bit_field_set_register(NIC_ECON1, ECON1_TXRST);
	nic_bit_field_clear_register(NIC_ECON1, ECON1_TXRST);
	nic_bit_field_clear_register(NIC_EIR, (EIR_TXERIF | EIR_TXIF));

	//----------------------------
	//----- SETUP TX POINTER -----
	//----------------------------
	//ETXST has already been set in the initialise function

	//----------------------------------------------
	//----- WRITE THE END OF TX PACKET POINTER -----
	//----------------------------------------------
	nic_write(NIC_ETXNDL, (BYTE)((nic_tx_len + NIC_BASE_TX_ADDR) & 0x00ff));
	nic_write(NIC_ETXNDH, (BYTE)((nic_tx_len + NIC_BASE_TX_ADDR) >> 8));

	//---------------------------
	//----- SEND THE PACKET -----
	//---------------------------
	nic_bit_field_set_register(NIC_ECON1, ECON1_TXRTS);

	//-------------------------------------
	//----- SILICON ERRATA WORKAROUND -----
	//-------------------------------------
	// Revision B5 and B7 silicon errata workaround
	if(enc28j60_revision_id == 0x05 || enc28j60_revision_id == 0x07)
	{
		while(!(nic_read(NIC_EIR).Val & (EIR_TXERIF | EIR_TXIF)) && (++attempt_counter < 1000))
			;
		
		if(nic_read(NIC_EIR).EIRbits.TXERIF || (attempt_counter >= 1000))
		{
			//Cancel the previous transmission if it has become stuck set
			nic_bit_field_clear_register(NIC_ECON1, ECON1_TXRTS);

			//Save the current read pointer
			read_pointer_save.v[0] = nic_read(NIC_ERDPTL).Val;
			read_pointer_save.v[1] = nic_read(NIC_ERDPTH).Val;

			//Get the location of the transmit status vector
			tx_end.v[0] = nic_read(NIC_ETXNDL).Val;
			tx_end.v[1] = nic_read(NIC_ETXNDH).Val;
			tx_end.Val++;
			
			//Read the transmit status vector
			nic_write(NIC_ERDPTL, tx_end.v[0]);
			nic_write(NIC_ERDPTH, tx_end.v[1]);
			nic_read_array((BYTE*)&tx_status, sizeof(tx_status));

			//Implement retransmission if a late collision occured (this can happen on B5 when certain link pulses arrive at the same time as the transmission)
			for(b_temp = 0; b_temp < 16; b_temp++)
			{
				if(nic_read(NIC_EIR).EIRbits.TXERIF && tx_status.bits.LateCollision)
				{
					//Reset the TX logic
					nic_bit_field_set_register(NIC_ECON1, ECON1_TXRST);
					nic_bit_field_clear_register(NIC_ECON1, ECON1_TXRST);
					nic_bit_field_clear_register(NIC_EIR, EIR_TXERIF | EIR_TXIF);

					//Transmit the packet again
					nic_bit_field_set_register(NIC_ECON1, ECON1_TXRTS);
					while(!(nic_read(NIC_EIR).Val & (EIR_TXERIF | EIR_TXIF)));

					//Cancel the previous transmission if it has become stuck set
					nic_bit_field_clear_register(NIC_ECON1, ECON1_TXRTS);

					//Read transmit status vector
					nic_write(NIC_ERDPTL, tx_end.v[0]);
					nic_write(NIC_ERDPTH, tx_end.v[1]);
					nic_read_array((BYTE*)&tx_status, sizeof(tx_status));
				}
				else
				{
					break;
				}
			}

			//Restore the current read pointer
			nic_write(NIC_ERDPTL, read_pointer_save.v[0]);
			nic_write(NIC_ERDPTH, read_pointer_save.v[1]);
		}
	}
}




//********************************************
//********************************************
//********** SELECT NIC MEMORY BANK **********
//********************************************
//********************************************
//Call with nic register address with the high byte containing the 2 bank select 2 bits.
static void nic_select_bank (WORD reg)
{
	nic_bit_field_clear_register(NIC_ECON1, (ECON1_BSEL1 | ECON1_BSEL0));
	nic_bit_field_set_register(NIC_ECON1, ((WORD_VAL*)&reg)->v[1]);
}


 
//******************************
//******************************
//********** NIC READ **********
//******************************
//******************************
//Call with:-
//	5 bit address of the ETH control register to read from (the top 3 bits must be 0)
//Returns:-
//	Byte read from the Ethernet controller's ETH register.
static NIC_REG nic_read (BYTE address)
{
	NIC_REG reg;

	//SELECT NIC
	NIC_CS(0);
	
	//WRITE ADDRESS
	NIC_SPI_TX_BYTE((NIC_RCR | address));
	while(!NIC_SPI_BUF_FULL)			//Wait for SPI to complete
		;
	
	reg.Val = NIC_SPI_RX_BYTE_BUFFER;

	//READ REGISTER
	NIC_SPI_TX_BYTE(0);					//Send a dummy byte to read
	while(!NIC_SPI_BUF_FULL)			//Wait for SPI to complete
		;
		
	reg.Val = NIC_SPI_RX_BYTE_BUFFER;

	//DE-SELECT NIC
	NIC_CS(1);

	return(reg);
}

 
//*******************************
//*******************************
//********** NIC WRITE **********
//*******************************
//*******************************
static void nic_write (BYTE address, BYTE data)
{		
	BYTE b_temp;

	//SELECT THE NIC
	NIC_CS(0);
	
	//WRITE ADDRESS
	NIC_SPI_TX_BYTE((NIC_WCR | address));
	while(!NIC_SPI_BUF_FULL)					//Wait for SPI to complete
		;
		
	b_temp = NIC_SPI_RX_BYTE_BUFFER;

	//WRITE THE DATA
	NIC_SPI_TX_BYTE(data);
	while(!NIC_SPI_BUF_FULL)					//Wait for SPI to complete
		;
		
	b_temp = NIC_SPI_RX_BYTE_BUFFER;

	//DESELECT THE NIC
	NIC_CS(1);
}



//*********************************************
//*********************************************
//********** NIC WRITE BUFFER MEMORY **********
//*********************************************
//*********************************************
void nic_write_buffer(BYTE data)
{
	BYTE b_temp;

	//SELECT NIC
	NIC_CS(0);
	
	//WRITE COMMAND
	NIC_SPI_TX_BYTE(NIC_WBM);
	while(!NIC_SPI_BUF_FULL)				//Wait for SPI to complete
		;
		
	b_temp = NIC_SPI_RX_BYTE_BUFFER;
	
	//WRITE DATA
	NIC_SPI_TX_BYTE(data);
	while(!NIC_SPI_BUF_FULL)				//Wait for SPI to complete
		;
	b_temp = NIC_SPI_RX_BYTE_BUFFER;

	//DE-SELECT NIC
	NIC_CS(1);
}



//***********************************************
//***********************************************
//********** NIC READ MAC/MII REGISTER **********
//***********************************************
//***********************************************
static NIC_REG nic_read_mac_mii_register (BYTE address)
{
	NIC_REG reg;

	//SELECT NIC
	NIC_CS(0);
	
	//SEND COMMAND
	NIC_SPI_TX_BYTE((NIC_RCR | address));
	while(!NIC_SPI_BUF_FULL)					//Wait for SPI to complete
		;
		
	reg.Val = NIC_SPI_RX_BYTE_BUFFER;
	
	//SENT DUMMY BYTE
	NIC_SPI_TX_BYTE(0x00);
	while(!NIC_SPI_BUF_FULL)					//Wait for SPI to complete
		;
		
	reg.Val = NIC_SPI_RX_BYTE_BUFFER;
	
	//READ THE REGISTER
	NIC_SPI_TX_BYTE(0x00);
	while(!NIC_SPI_BUF_FULL)					//Wait for SPI to complete
		;
		
	reg.Val = NIC_SPI_RX_BYTE_BUFFER;
	
	//DE-SELECT NIC
	NIC_CS(1);
	
	return(reg);
}



//**************************************************
//**************************************************
//********** NIC BIT FIELD CLEAR REGISTER **********
//**************************************************
//**************************************************
static void nic_bit_field_clear_register (BYTE address, BYTE data)
{
	BYTE b_temp;

	//SELECT NIC
	NIC_CS(0);

	//SEND COMMAND
	NIC_SPI_TX_BYTE((NIC_BFC | address));
	while(!NIC_SPI_BUF_FULL)						//Wait for SPI to complete
		;
		
	b_temp = NIC_SPI_RX_BYTE_BUFFER;
	
	//SEND DATA
	NIC_SPI_TX_BYTE(data);
	while(!NIC_SPI_BUF_FULL)						//Wait for SPI to complete
		;
		
	b_temp = NIC_SPI_RX_BYTE_BUFFER;
	
	
	//DE-SELECT NIC
	NIC_CS(1);
}



//************************************************
//************************************************
//********** NIC BIT FIELD SET REGISTER **********
//************************************************
//************************************************
static void nic_bit_field_set_register (BYTE address, BYTE data)
{
	BYTE b_temp;

	//SELECT NIC
	NIC_CS(0);
	
	//SEND COMMAND
	NIC_SPI_TX_BYTE((NIC_BFS | address));
	while(!NIC_SPI_BUF_FULL)						//Wait for SPI to complete
		;
		
	b_temp = NIC_SPI_RX_BYTE_BUFFER;
	
	//SEND DATA
	NIC_SPI_TX_BYTE(data);
	while(!NIC_SPI_BUF_FULL)						//Wait for SPI to complete
		;
		
	b_temp = NIC_SPI_RX_BYTE_BUFFER;
	
	//DE-SELECT NIC
	NIC_CS(1);
}



//*******************************************
//*******************************************
//********** NIC READ PHY REGISTER **********
//*******************************************
//*******************************************
//Performs an MII read operation.  While in progress, it simply polls the MII BUSY bit wasting time (10.24us).
PHYREG nic_read_phy_register(BYTE reg)
{
	PHYREG result;

	//Set the right address and start the register read operation
	nic_select_bank(NIC_MIREGADR);
	nic_write((BYTE)NIC_MIREGADR, reg);
	nic_write((BYTE)NIC_MICMD, MICMD_MIIRD);	

	//Loop to wait until the PHY register has been read through the MII
	//This requires 10.24us
	nic_select_bank(NIC_MISTAT);
	while(nic_read_mac_mii_register((BYTE)NIC_MISTAT).MISTATbits.BUSY);

	//Stop reading
	nic_select_bank(NIC_MIREGADR);
	nic_write((BYTE)NIC_MICMD, 0x00);	
	
	//Obtain results and return
	result.VAL.v[0] = nic_read_mac_mii_register((BYTE)NIC_MIRDL).Val;
	result.VAL.v[1] = nic_read_mac_mii_register((BYTE)NIC_MIRDH).Val;

	nic_select_bank(NIC_ERDPTL);	//Return to Bank 0
	return(result);
}



//********************************************
//********************************************
//********** NIC WRITE PHY REGISTER **********
//********************************************
//********************************************
//(Performs an MII write operation.  While in progress, it simply polls the MII BUSY bit wasting time.)
void nic_write_phy_register (BYTE reg, WORD data)
{
	//Write the register address
	nic_select_bank(NIC_MIREGADR);
	nic_write((BYTE)NIC_MIREGADR, reg);
	
	//Write the data
	nic_write((BYTE)NIC_MIWRL, ((WORD_VAL*)&data)->v[0]);			//Write low byte first
	nic_write((BYTE)NIC_MIWRH, ((WORD_VAL*)&data)->v[1]);

	//Wait until the PHY register has been written
	nic_select_bank(NIC_MISTAT);
	while(nic_read_mac_mii_register((BYTE)NIC_MISTAT).MISTATbits.BUSY);

	nic_select_bank(NIC_ERDPTL);	// Return to Bank 0
}





