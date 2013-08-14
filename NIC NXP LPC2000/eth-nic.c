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





//************************************
//************************************
//********** INITIALISE NIC **********
//************************************
//************************************
//Call with:
//0 = allow speed 10 / 100 Mbps
void nic_initialise (BYTE init_config)
{	
	DWORD reg_value;
	DWORD time_out;
	DWORD id1;
	DWORD id2;
	DWORD count;

	init_config = 0;			//Ensure 10 / 100 Mbps is selected


	//----- RESET INTERNAL MODULES -----
	MAC1 = (MAC1_RESET_TX | MAC1_RESET_MCS_TX | MAC1_RESET_RX | MAC1_RESET_MCS_RX | MAC1_SIMULATION_RESET | MAC1_SOFT_RESET);
	Command = (Command_RegReset | Command_TxReset | Command_RxReset);

	NIC_PHY_RST(0);
	NIC_PHY_PD(1);

	//----- SHORT DELAY -----
	for (time_out = 100; time_out; time_out--)
		;

	NIC_PHY_RST(1);

	//----- INITIALISE MAC CONTROL REGISTERS -----
	MAC1 = MAC1_PASS_ALL_RECEIVE_FRAMES;
	MAC2 = (MAC2_CRC_ENABLE | MAC2_PAD_CRC_ENABLE);
	MAXF = NIC_MAX_FRAME_LENGTH;
	CLRT = ((0x0f << CLRT_RETRANSMISSION_MAXIMUM_BIT) | (0x37 << CLRT_COLLISION_WINDOW_BIT));		//Default value
	IPGR = ((0x12 << IPGR_NON_BACK_TO_BACK_INTER_PACKET_GAP_PART2_BIT) | (0x00 << IPGR_NON_BACK_TO_BACK_INTER_PACKET_GAP_PART1_BIT));	//Recommended value

	//----- ENABLE REDUCED MII INTERFACE -----
	Command = (Command_RMII | Command_PassRuntFrame);


	//----- RESET MII MANAGEMENT HARDWARE -----
	//Set clock to host / 20, no suppress preamble, no scan increment 
	MCFG = ((0x06 << MCFG_CLOCK_SELECT_BIT) | MCFG_RESET_MII_MGMT);
	for (count = 0; count < 0x40; count++)
		;
	MCFG &= ~MCFG_RESET_MII_MGMT;									//Clear reset
	MCMD = 0;


	//----- RESET PHY IC -----
	nic_write_phy_register(NIC_PHY_REG_BASIC_CTRL_REG, 0x8000);
	for (time_out = 0; time_out < 0x100000; time_out++)				//Wait for reset to complete
    {
		reg_value = nic_read_phy_register(NIC_PHY_REG_BASIC_CTRL_REG);
		if (!(reg_value & 0x8000))
			break;													//Reset complete
	}

	//----- CHECK IF PHY IS KSZ8001 -----
	id1 = nic_read_phy_register(NIC_PHY_IDENTIFIER_1);
	id2 = nic_read_phy_register(NIC_PHY_IDENTIFIER_2);
	//Check values if desired

	
	//if (init_config)
	//{
	//	//Only 10Mbps available
	//	nic_write_phy_register(NIC_PHY_REG_BASIC_CTRL_REG, 0x0000);			//Configure for fixed 10Mbs, half duplex
	//	auto_negotation_disabled = 1;
	//}
	//else
	//{
		//10/100Mbps available
		nic_write_phy_register(NIC_PHY_REG_BASIC_CTRL_REG, 0x3000);			//Configure for auto negotiation (10/100Mbs, half/full duplex)
        auto_negotation_disabled = 0;
	//}


	//----- SET INDIVIDUAL ADDRESS (MAC) -----
	SA0 = (((WORD)our_mac_address.v[5] << SA0_STATION_ADDRESS_1st_octet_BIT) | ((WORD)our_mac_address.v[4] << SA0_STATION_ADDRESS_2nd_octet_BIT));
	SA1 = (((WORD)our_mac_address.v[3] << SA1_STATION_ADDRESS_3rd_octet_BIT) | ((WORD)our_mac_address.v[2] << SA1_STATION_ADDRESS_4th_octet_BIT));
	SA2 = (((WORD)our_mac_address.v[1] << SA2_STATION_ADDRESS_5th_octet_BIT) | ((WORD)our_mac_address.v[0] << SA2_STATION_ADDRESS_6th_octet_BIT));


	//----- CONFIGURE PHY INTERRUPT PIN -----
	nic_write_phy_register(NIC_PHY_IRQ_CTRL_STATUS_REG, NIC_PHY_IRQ_CTRL_STATUS_VALUE);		//Enable Link Down Interrupt, Enable Link up Interrupt


	//----- CONFIGURE PHY LED PINS -----
	nic_write_phy_register(NIC_PHY_CTRL_REG, (NIC_LEDS_FUNCTION << 14));


	//----- INITIALISE RX DMA DESCRIPTORS -----
	//Set The Receive Descriptor Registers
	RxDescriptor = NIC_RX_DESCRIPTOR_BASE_ADDRESS;					//Receive descriptor base address register
	RxStatus = NIC_RX_STATUS_BASE_ADDRESS;							//Receive status base address register
	RxDescriptorNumber = NIC_NUM_OF_RX_FRAGMENT_BUFFERS - 1;		//Number of descriptors in the descriptor array -1 for which RxDescriptor is the base address.

	//RX Descriptors Point to 0
	RxConsumeIndex  = 0;											//Receive consume index register
	
	for (count = 0; count < NIC_NUM_OF_RX_FRAGMENT_BUFFERS; count++)
	{
		NIC_RX_DESCRIPTOR_PACKET(count) = NIC_RX_BUFFER(count);
		NIC_RX_DESCRIPTOR_CONTROL(count) = (0x80000000 | (NIC_FRAGMENT_BUFFERS_SIZE - 1));			//0x80000000  = Trigger RxDone interrupt
		NIC_RX_STATUS_INFO(count) = 0;
		NIC_RX_STATUS_HASHCRC(count) = 0;
	}


	//----- INITIALISE TX DMA DESCRIPTORS -----
	//Set The Transmit Descriptor Registers
	TxDescriptor = NIC_TX_DESCRIPTOR_BASE_ADDRESS;
	TxStatus = NIC_TX_STATUS_BASE_ADDRESS;
	TxDescriptorNumber = NIC_NUM_OF_TX_FRAGMENT_BUFFERS - 1;

	//Tx Descriptors Point to 0
	TxProduceIndex = 0;
	
	for (count = 0; count < NIC_NUM_OF_TX_FRAGMENT_BUFFERS; count++)
	{
		NIC_TX_DESCRIPTOR_PACKET(count) = NIC_TX_BUFFER(count);
		NIC_TX_DESCRIPTOR_CONTROL(count) = 0;
		NIC_TX_STATUS_INFO(count) = 0;
	}


	//----- RECEIVE BROADCAST AND PERFECT MATCH PACKETS -----
	RxFilterCtrl = (RxFilterCtrl_AcceptBroadcastEn | RxFilterCtrl_AcceptPerfectEn);


	//----- ENABLE ETHERNET INTERRUPTS -----
	//IntEnable = (IntEnable_RxDoneIntEn | IntEnable_TxDoneIntEn);
    IntClear = 0xffff;

	//Enable receive and transmit mode of MAC Ethernet core
	Command |= (Command_RxEnable | Command_TxEnable);
	MAC1 |= MAC1_RECEIVE_ENABLE;


	//----- INITIALISE DRIVER SPECIFIC VARIABLES -----
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
	DWORD rx_status;
	WORD data;
	BYTE b_data;
	DWORD time_out;
	

	if ((IntStatus & IntStatus_TxDoneInt) || (IntStatus & IntStatus_TxFinishedInt))
	{
		//-------------------------------------------
		//----- PACKET TX COMPLETE IRQ (TX INT) -----
		//-------------------------------------------
        IntClear = 0x00c0;
		
		//No need to process - memory is automatically released for use

	}

	if (IntStatus & IntStatus_TxUnderrunInt)
	{
		//-----------------------
		//----- TX UNDERRUN -----
		//-----------------------
		//This is a fatal error and requires a soft reset of the transmit
        nic_initialise(NIC_INIT_SPEED);

		//Exit
		return(0);
	}

	if (IntStatus & IntStatus_TxErrorInt)
	{
		//--------------------
		//----- TX ERROR -----
		//--------------------
		//Caused by late collision, excessive collision, excessive defer, no descriptor, or underrun
		//We will just dump the packet (we could try re-sending if we wanted to)

		//After reporting a LateCollision, ExcessiveCollision, ExcessiveDefer or Underrun error, the transmission of the erroneous frame will be aborted,
		//remaining transmission data and frame fragments will be discarded and transmission will continue with the next frame in the descriptor array.
		//Therefore we do not ned to take action other than clearing the irq flag
		IntClear = 0x00000030;

		//Exit
		return(0);
	}
	
	//----- EXIT IF WE'RE WAITING FOR THE LAST PACKET TO BE DUMPED -----
	if (nic_rx_packet_waiting_to_be_dumped)
		return;

	if (IntStatus & IntStatus_RxOverrunInt)
	{
		//------------------------------------------
		//----- RECEIVE OVERFLOW IRQ (RX_OVRN) -----
		//------------------------------------------
        nic_initialise(NIC_INIT_SPEED);

		//Exit
		return(0);
	}
	
	
	if (
	(IntStatus & IntStatus_RxErrorInt) &&
    (!(IntStatus & IntStatus_RxDoneInt))			//If rx done is being flagged also then let the rx handler accept the packet as rx error can be flagged for packets that are OK
	)
	{
		//-------------------------
		//----- RECEIVE ERROR -----
		//-------------------------
        IntClear = 0x00000002;
        

		if (RxProduceIndex == RxConsumeIndex)				//Should not be equal, but check just in case
			return(0);
		
        rx_status = NIC_RX_STATUS_INFO(RxConsumeIndex);

		//REJECT PACKET
        nic_rx_packet_waiting_to_be_dumped = 1;				//Force packet to be dumped
		nic_rx_dump_packet();
		return(0);
	}

	if ((IntStatus & IntStatus_RxDoneInt) || (RxProduceIndex != RxConsumeIndex))
	{
		//---------------------------------------------
		//----- A FRAGMENT RECEIVED IRQ (RCV INT) -----
		//---------------------------------------------
        IntClear = 0x0000000a;				//Clear rx error irq also as rx errors can be flagged for packets that are actually OK

		//Ethernet interface notes:-
		//RxProduceIndex		//Index of the next descriptor that is going to be filled with the next frame recevied
		//RxConsumeIndex		//Index of the next descriptor that the software drivier is going to process
		//When RxProduceIndex = RxConsumeIndex the receive buffer is empty
		//Each receive descriptor takes two word locations (8 bytes) in memory, consisting of
		//	((control word [Control] << 32) | (Pointer to the data buffer for storing receive [Packet] << 0))
		//Each status field takes two words (8 bytes) in memory
		//	((StatusHashCRC << 32) | (StatusInfo << 0))

		if (RxProduceIndex == RxConsumeIndex)				//Should not be equal, but check just in case
			return(0);


		//----- HAS A WHOLE FRAME BEEN RECEVIED? -----
		//Yes - we use a fragment size that will hold an entire frame
		

		//GET THE STATUS WORD
		rx_status = NIC_RX_STATUS_INFO(RxConsumeIndex);

        nic_rx_bytes_remaining = (WORD)((rx_status & 0x000007ff) - 4) + 1 ;		//- 1 encoded.  -4 as we don't want the trailing CRC (its been checked by the hardware)

		//CHECK RX STATUS WORD ERROR
		if (
			(rx_status & 0x00040000)			//Check for control frame
			|| (rx_status & 0x00080000)			//Check for VLAN
			|| (rx_status & 0x00100000)			//Check for FailFilter
			//|| (rx_status & 0x00200000)		//Check for Multicast (allowed)
			//|| (rx_status & 0x00400000)		//Check for Broadcast (allowed)
			|| (rx_status & 0x00800000)			//Check for CRC error
			|| (rx_status & 0x01000000)			//Check for Symbol error
			|| (rx_status & 0x02000000)			//Check for Length error
			//|| (rx_status & 0x04000000)			//Check for Range error (not actually and error)
			|| (rx_status & 0x08000000)			//Check for alignment error
			|| (rx_status & 0x10000000)			//Check for Overrun
			|| (rx_status & 0x20000000)			//Check for No descriptor
			|| (!(rx_status & 0x40000000))		//Check for LastFlag (required)
			//|| (rx_status & 0x80000000)			//Check for Error (range error is not actually and error)
			)
		{
			//REJECT PACKET
            nic_rx_packet_waiting_to_be_dumped = 1;				//Force packet to be dumped
			nic_rx_dump_packet();
			return(0);
		}


		//SETUP THE DATA POINTER REGISTER TO READ THE RX PACKET
		nic_setup_read_data();

		nic_rx_packet_total_ethernet_bytes = nic_rx_bytes_remaining;

		//The ethernet stack processing routine will continue receiving the rest of the packet
		nic_rx_packet_waiting_to_be_dumped = PROCESS_NIC_CALLS_BEFORE_DUMP_RX;
		return(nic_rx_bytes_remaining);

	}

	if (!NIC_PHY_INT_PIN)
	{
		//---------------------------------
		//----- PHY IRQ (LINK CHANGE) -----
		//---------------------------------
	
		//----- CLEAR THE IRQ -----
		data = nic_read_phy_register (NIC_PHY_IRQ_CTRL_STATUS_REG);			//Read register to clear the interrupt

		//----- WAIT TO COMPLETE AUTO_NEGOTIATION -----
		for (time_out = 0; time_out < 0x100000; time_out++)
		{
			data = nic_read_phy_register (NIC_PHY_BASIC_STATUS_REG);
			if (auto_negotation_disabled)
				break;

			if (!(data & 0x0004))				//Link is down (not linked)
				break;

			if (data & 0x0020)
				break;							//Auto negotiation Complete
		}

		//----- GET THE LINK PIN STATUS -----
		data = nic_read_phy_register (NIC_PHY_BASIC_STATUS_REG);

		if (data & 0x0004)
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


	//<<<<<<< ADD OTHER HANDLERS HERE



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

	//Get packet buffer start position from the rx consume index to the rx fragment buffers
	nic_rx_start_buffer = RxConsumeIndex;
	nic_rx_buffer_next_byte = (BYTE*)NIC_RX_BUFFER(nic_rx_start_buffer);



	nic_read_next_byte_get_word = 1;

	//Next operation can read nic_rx_buffer_next_byte for the data
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

		//READ NEXT 2 BYTES
		last_read_data = (WORD)*nic_rx_buffer_next_byte++;
		last_read_data |= ((WORD)*nic_rx_buffer_next_byte++ << 8);

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



	//----- SET THE NEW ADDRESS -----
	nic_rx_buffer_next_byte = (BYTE*)(NIC_RX_BUFFER(nic_rx_start_buffer));
    nic_rx_buffer_next_byte += move_pointer_to_ethernet_byte;

	new_pointer_value = (move_pointer_to_ethernet_byte & 0x07fe);	//Needs to be word aligned - low bit dealt with later

	nic_read_next_byte_get_word = 1;

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
	DWORD ConsumeIndex;

	if (nic_rx_packet_waiting_to_be_dumped)
	{
		ConsumeIndex = RxConsumeIndex;

		//Increment the Consume Index pointer to release the fragment buffer
		if (++ConsumeIndex >= NIC_NUM_OF_RX_FRAGMENT_BUFFERS)
			ConsumeIndex = 0;   
		RxConsumeIndex = ConsumeIndex;

        nic_rx_packet_waiting_to_be_dumped = 0;
	}
	
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


	//Ethernet interface notes:-
	//TxProduceIndex		//Index of the next descriptor that is going to be filled by the software driver
	//TxConsumeIndex		//Index of the next descriptor that is going to be transmitted by the hardware
	//When TxProduceIndex = TxConsumeIndex the transmit buffer is empty
	//Each transmit descriptor takes two word locations (8 bytes) in memory, consisting of
	//	((control word [Control] << 32) | (Pointer to the data buffer containing transmit data [Packet] << 0))
	//Each status field takes one word (4 bytes) in memory


	if ((TxProduceIndex != TxConsumeIndex)  || (nic_tx_start_buffer != 0))
	{
		//----- LAST TRANSMISSION IS STILL ACTIVE -----
		return(0);
	}


	//------------------------------------
	//----- TRANSMIT IDLE - SETUP TX -----
	//------------------------------------
	nic_tx_len = 0;

	//Get packet buffer start position from the tx produce index to the tx fragment buffers
	nic_tx_start_buffer = TxProduceIndex;
    nic_tx_buffer_next_byte = (BYTE*)NIC_TX_BUFFER(nic_tx_start_buffer);

	NIC_TX_DESCRIPTOR_PACKET(nic_tx_start_buffer) = NIC_TX_BUFFER(nic_tx_start_buffer);
    //NIC_TX_DESCRIPTOR_CONTROL(nic_tx_start_buffer) = ;				//This will be written when we send


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

	*nic_tx_buffer_next_byte++ = data;
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
	BYTE *tx_buffer;

	tx_buffer = (BYTE*)NIC_TX_BUFFER(nic_tx_start_buffer);		//Set to start of tx buffer

    tx_buffer += byte_address;

	//Write the word
	*tx_buffer++ = (BYTE)(data & 0x00ff);
	*tx_buffer++ = (BYTE)(data >> 8);
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
	//No need - the hardware will do this for us



	//---------------------------
	//----- SEND THE PACKET -----
	//---------------------------
	NIC_TX_DESCRIPTOR_CONTROL(nic_tx_start_buffer) = (0xF0000000 | (nic_tx_len - 1));		//0xF0000000 = Do irq, Last fragment, Do Pad, Do CRC (It seems to be necessary
																							//for the irq flag (bit31) to be set otherwise the TxConsume register is not
																							//incremented by the hardware).

	if (++nic_tx_start_buffer >= NIC_NUM_OF_TX_FRAGMENT_BUFFERS)
		nic_tx_start_buffer = 0;
	TxProduceIndex = nic_tx_start_buffer;
    nic_tx_start_buffer = 0;

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

	//Read the status output register 
	data = nic_read_phy_register(NIC_PHY_BASIC_STATUS_REG);

	if (data & 0x4000)
	{
		//Capable of 100BASE-X full duplex
        return_status = 0x03;
	}
	else if (data & 0x2000)
	{
		//Capable of 100BASE-X half duplex
        return_status = 0x01;
	}
	else if (data & 0x1000)
	{
		//10Mbps with full duplex
        return_status = 0x02;
	}
	else if (data & 0x08000)
	{
		//1 = 10Mbps with half duplex
        return_status = 0x00;
	}


	//Configure Full/Half Duplex mode
	if (return_status & 0x02)
	{
		//Full duplex is enabled
		MAC2 |= MAC2_FULL_DUPLEX;
		Command |= Command_FullDuplex;
		IPGT = 0x00000015;						//Recommended value for Full Duplex
	}
	else
	{
		//Half duplex mode
		IPGT = 0x00000012;						//Recommended value for Half Duplex
	}

	//Configure 100MBit/10MBit mode
	if (return_status & 0x01)
	{
		//100MBit mode
		SUPP = 0x00000100;						//Reduced MII Logic Current Speed 
	}
	else
	{
		//10MBit mode
		SUPP = 0;
	}


	return (return_status);
}





//****************************************
//****************************************
//********** WRITE PHY REGISTER **********
//****************************************
//****************************************
//(Do via the MII serial interface)
void nic_write_phy_register (BYTE address, WORD data)
{
	DWORD time_out;

	MADR = ((0 << MADR_PHY_ADDRESS_BIT) | (address << MADR_REGISTER_ADDRESS_BIT));
    MWTD = data;				//Write the value

	//Wait for write to complete
	for (time_out = 0; time_out < NIC_MII_WRITE_TIMEOUT; time_out++)
	{
		if ((MIND & MIND_BUSY) == 0) 
			break;
	}
}



//***************************************
//***************************************
//********** READ PHY REGISTER **********
//***************************************
//***************************************
//(Via the MII serial interface)
WORD nic_read_phy_register (BYTE address)
{
	unsigned int time_out;

	MADR = ((0 << MADR_PHY_ADDRESS_BIT) | (address << MADR_REGISTER_ADDRESS_BIT));
	MCMD = MCMD_READ;

	//Wait for read to complete
	for (time_out = 0; time_out < NIC_MII_READ_TIMEOUT; time_out++)
	{
		if ((MIND & MIND_BUSY) == 0) 
			break;
	}
	MCMD = 0;
	return(MRDD);
}


















