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
//Microchip PIC32 (BUILT IN NETWORK INTERFACE CONTROLLER) C CODE FILE


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



	//FOLLOW MICROCHIP RECCOMENDED ETHERNET INITIALISATION SEQUENCE

	//----- ENSURE ETHERNET IS TURNED OFF -----
	INTEnable(INT_ETHERNET, INT_DISABLED);	// disable ethernet interrupts
	IEC1bits.ETHIE = 0;
	ETHCON1bits.ON = 0;
	ETHCON1bits.RXEN = 0;
	ETHCON1bits.TXRTS = 0;
	
	while (ETHSTATbits.ETHBUSY)		//Wait for Etherent logic to become idle
		;

	ETHCON1bits.ON = 1;
	while(ETHSTATbits.BUFCNT)
		ETHCON1bits.BUFCDEC = 1;

	IFS1bits.ETHIF = 0;
	ETHIRQCLR=0xffffffff;
	ETHTXSTCLR = 0xffffffff;
	ETHRXSTCLR = 0xffffffff;
	
	//------------------------------
	//----- MAC INITIALISATION -----
	//------------------------------
	EMACxCFG1bits.SOFTRESET = 1;
	EMACxCFG1=0;					//Clear the reset
	

	//----- SETUP ETHERNET INTERFACE PINS -----
	//(Some pins (e.g. analog) are not auto setup when Ethernet peripheral is turned on)
	__DEVCFG3bits_t bcfg3;		//Detect which pins are in use from the PIC32 config bits setting
	bcfg3 = DEVCFG3bits;
	if(bcfg3.FETHIO)
	{
		//Default Ethenet pins
		PORTSetPinsDigitalOut(_ETH_MDC_PORT, _ETH_MDC_BIT);	
		PORTSetPinsDigitalIn(_ETH_MDIO_PORT, _ETH_MDIO_BIT);
		PORTSetPinsDigitalOut(_ETH_TXEN_PORT, _ETH_TXEN_BIT);	
		PORTSetPinsDigitalOut(_ETH_TXD0_PORT, _ETH_TXD0_BIT);	
		PORTSetPinsDigitalOut(_ETH_TXD1_PORT, _ETH_TXD1_BIT);	
		PORTSetPinsDigitalIn(_ETH_RXCLK_PORT, _ETH_RXCLK_BIT);
		PORTSetPinsDigitalIn(_ETH_RXDV_PORT, _ETH_RXDV_BIT);
		PORTSetPinsDigitalIn(_ETH_RXD0_PORT, _ETH_RXD0_BIT);
		PORTSetPinsDigitalIn(_ETH_RXD1_PORT, _ETH_RXD1_BIT);
		PORTSetPinsDigitalIn(_ETH_RXERR_PORT, _ETH_RXERR_BIT);
	}
	else
	{
		//Alternative Ethenet pins
		PORTSetPinsDigitalOut(_ETH_ALT_MDC_PORT, _ETH_ALT_MDC_BIT);	
		PORTSetPinsDigitalIn(_ETH_ALT_MDIO_PORT, _ETH_ALT_MDIO_BIT);
		PORTSetPinsDigitalOut(_ETH_ALT_TXEN_PORT, _ETH_ALT_TXEN_BIT);	
		PORTSetPinsDigitalOut(_ETH_ALT_TXD0_PORT, _ETH_ALT_TXD0_BIT);	
		PORTSetPinsDigitalOut(_ETH_ALT_TXD1_PORT, _ETH_ALT_TXD1_BIT);	
		PORTSetPinsDigitalIn(_ETH_ALT_RXCLK_PORT, _ETH_ALT_RXCLK_BIT);
		PORTSetPinsDigitalIn(_ETH_ALT_RXDV_PORT, _ETH_ALT_RXDV_BIT);
		PORTSetPinsDigitalIn(_ETH_ALT_RXD0_PORT, _ETH_ALT_RXD0_BIT);
		PORTSetPinsDigitalIn(_ETH_ALT_RXD1_PORT, _ETH_ALT_RXD1_BIT);
		PORTSetPinsDigitalIn(_ETH_ALT_RXERR_PORT, _ETH_ALT_RXERR_BIT);
	}




	//----- RESET THE PHY IC -----
	NIC_PHY_RST(0);
	NIC_PHY_PD(1);
	for (time_out = 100; time_out; time_out--)		//Short delay
		;
	NIC_PHY_RST(1);


	//-----------------------------------------------
	//----- KSZ8001 ETHERNET PHY INITIALISATION -----
	//-----------------------------------------------
	//RMII (Reduced MII) Data Interface
	EthMIIMConfig(INSTRUCTION_CLOCK_FREQUENCY, 2500000);

	//----- RESET PHY IC -----
	nic_write_phy_register(NIC_PHY_REG_BASIC_CTRL_REG, 0x8000);
	for (time_out = 0; time_out < 0x100000; time_out++)				//Wait for reset to complete
    {
		reg_value = nic_read_phy_register(NIC_PHY_REG_BASIC_CTRL_REG);
		if (!(reg_value & 0x8000))
			break;													//Reset complete
	}

	//----- CHECK IF PHY IS KSZ8001 -----
	id1 = nic_read_phy_register(NIC_PHY_IDENTIFIER_1);		//0x0022
	id2 = nic_read_phy_register(NIC_PHY_IDENTIFIER_2);		//0x161#
	//Check values if desired - if they don't match you have a problem

	
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


	//----- CONFIGURE PHY INTERRUPT PIN -----
	nic_write_phy_register(NIC_PHY_IRQ_CTRL_STATUS_REG, NIC_PHY_IRQ_CTRL_STATUS_VALUE);		//Enable Link Down Interrupt, Enable Link up Interrupt


	//----- CONFIGURE PHY LED PINS -----
	nic_write_phy_register(NIC_PHY_CTRL_REG, (NIC_LEDS_FUNCTION << 14));


	//-----------------------------
	//----- MAC CONFIGURATION -----
	//-----------------------------
	EMACxCFG1bits.RXENABLE = 1;
	EMACxCFG1bits.TXPAUSE = 1;
	EMACxCFG1bits.RXPAUSE = 1;


	EMACxCFG2bits.PADENABLE = 1;
	EMACxCFG2bits.CRCENABLE = 1;			//The MAC will append a CRC to every frame whether padding was required or not
	EMACxCFG2bits.HUGEFRM = 0;
	EMACxCFG2bits.FULLDPLX = 0;				//Full duplex is enabled if a full duplex link is detected

	EMACxIPGT = 0x12;						//Back-to-Back Interpacket Gap bits.  Default to Half-Duplex mode (Full duplex is enabled is a full duplex link is detected)

	EMACxIPGRbits.NB2BIPKTGP1 = 0x0c;		//Non-Back-to-Back Interpacket Gap Part 1 bits
	EMACxIPGRbits.NB2BIPKTGP2 = 0x12;		//Non-Back-to-Back Interpacket Gap Part 2 bits

	EMACxCLRTbits.CWINDOW = 0x37;			//Collision Window bits
	EMACxCLRTbits.RETX = 0x0f;				//Retransmission Maximum bits

	EMACxMAXF = 0x5ee;						//Maximum Frame Length bits

	//----- SET MAC ADDRESS -----
	//(Comment out if pre programming the MAC address as part of factory programming)
	EMACxSA0bits.STNADDR6 = our_mac_address.v[5];
	EMACxSA0bits.STNADDR5 = our_mac_address.v[4];
	EMACxSA1bits.STNADDR4 = our_mac_address.v[3];
	EMACxSA1bits.STNADDR3 = our_mac_address.v[2];
	EMACxSA2bits.STNADDR2 = our_mac_address.v[1];
	EMACxSA2bits.STNADDR1 = our_mac_address.v[0];

	//----------------------------------------------
	//----- ETHERNET CONTROLLER INITIALISATION -----
	//----------------------------------------------

	//Enable auto flow control
	ETHRXWMbits.RXFWM = 2;			//Receive Full Watermark bits
	ETHRXWMbits.RXEWM = 1;			//Receive Empty Watermark bits
	ETHCON1bits.AUTOFC = 1;


	//----- INITIALISE THE RX ACCEPTANCE / REJECTION FILTERS -----
	//ETHHT0 = 
	//ETHHT1 = 
	//ETHPMM0 = 
	//ETHPMM1 = 
	//ETHPMCS = 
	ETHRXFCCLR = ETH_FILT_ALL_FILTERS;
	ETHRXFCSET = (ETH_FILT_CRC_ERR_REJECT | ETH_FILT_RUNT_REJECT | ETH_FILT_ME_UCAST_ACCEPT | ETH_FILT_BCAST_ACCEPT);

	//SET RX BUFFERS SIZE
	ETHCON2 = (NIC_FRAGMENT_BUFFERS_SIZE / 16) << 4;		//0x60 = 1536

	//The TX and RX DMA engines are responsible for transferring data from system memory to the MAC for transmit packets and for transferring data from the
	//MAC to system memory for receive packets. The DMA engines each have access to the system memory by acting as two different bus masters, one bus master
	//for transmit and one for receive. The DMA engines use separate Ethernet Descriptor Tables (EDTs) for TX and RX operations to determine where the TX/RX
	//packet buffer resides in the system memory. Both transmit and receive descriptors, called Ethernet Descriptors (EDs), used by the DMA engines have a
	//similar format, with only the status word formats being different. 
	

	//----- INITIALISE TX DMA DESCRIPTORS -----
	for (count = 0; count < NIC_NUM_OF_TX_FRAGMENT_BUFFERS; count++)
	{
		nic_tx_descriptor[count].header.Val = 0;
		nic_tx_descriptor[count].status.Val = 0;
		
		nic_tx_descriptor[count].header.npv = 1;
		nic_tx_descriptor[count].header.eown = 0;			//0 = software owned, 1 = hardware owned
		nic_tx_descriptor[count].p_data_buffer_address = (BYTE*)((DWORD)&nic_tx_data_buffer[count][0] & 0x1fffffff);		//& 0x1FFFFFFF to convert virtual address to physical address
		
		if (count < (NIC_NUM_OF_TX_FRAGMENT_BUFFERS - 1))
			nic_tx_descriptor[count].next_ethernet_descriptor = (DWORD)&nic_tx_descriptor[(count + 1)] & 0x1fffffff;		//& 0x1FFFFFFF to convert virtual address to physical address
		else
			nic_tx_descriptor[count].next_ethernet_descriptor = (DWORD)&nic_tx_descriptor[0] & 0x1fffffff;		//& 0x1FFFFFFF to convert virtual address to physical address
	}
	ETHTXST = (DWORD)&nic_tx_descriptor[0] & 0x1fffffff;		//& 0x1FFFFFFF to convert virtual address to physical address
	nic_tx_current_buffer = 0;



	//----- INITIALISE RX DMA DESCRIPTORS -----
	for (count = 0; count < NIC_NUM_OF_RX_FRAGMENT_BUFFERS; count++)
	{
		nic_rx_descriptor[count].header.Val = 0;
		nic_rx_descriptor[count].status.Val = 0;

		nic_rx_descriptor[count].header.byte_count = NIC_FRAGMENT_BUFFERS_SIZE;		
		nic_rx_descriptor[count].header.npv = 1;
		nic_rx_descriptor[count].p_data_buffer_address = (BYTE*)((DWORD)&nic_rx_data_buffer[count][0] & 0x1fffffff);		//& 0x1FFFFFFF to convert virtual address to physical address

		if (count < (NIC_NUM_OF_RX_FRAGMENT_BUFFERS - 1))
			nic_rx_descriptor[count].next_ethernet_descriptor = (DWORD)&nic_rx_descriptor[(count + 1)] & 0x1fffffff;		//& 0x1FFFFFFF to convert virtual address to physical address
		else
			nic_rx_descriptor[count].next_ethernet_descriptor = (DWORD)&nic_rx_descriptor[0] & 0x1fffffff;		//& 0x1FFFFFFF to convert virtual address to physical address

		nic_rx_descriptor[count].header.eown = 1;			//0 = software owned, 1 = hardware owned
	}
	ETHRXST = (DWORD)&nic_rx_descriptor[0] & 0x1fffffff;		//& 0x1FFFFFFF to convert virtual address to physical address
	nic_rx_current_buffer = 0;

	//----- ENABLE THE ETHERNET CONTROLLER -----
	ETHCON1bits.ON = 1;
	ETHCON1bits.RXEN = 1;

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
	WORD data;
	BYTE b_data;
	DWORD time_out;


	if (ETHIRQbits.TXBUSE)
	{
		//----------------------------------------
		//----- TX DMA ENGINE TRANSFER ERROR -----
		//----------------------------------------
		nic_initialise(NIC_INIT_SPEED);

		//Exit
		return(0);
	}

	if (ETHIRQbits.TXABORT)
	{
		//--------------------
		//----- TX ERROR -----
		//--------------------

		//We will just dump the packet (we could try re-sending if we wanted to)
		ETHIRQCLR = ETH_EV_TXABORT;

		//Exit
		return(0);
	}
	
	if (ETHIRQbits.TXDONE)
	{
		//-------------------------------------------
		//----- PACKET TX COMPLETE IRQ (TX INT) -----
		//-------------------------------------------

		ETHIRQCLR = ETH_EV_TXDONE;
		
		//No need to process - memory is automatically released for use

	}

	//----- EXIT IF WE'RE WAITING FOR THE LAST PACKET TO BE DUMPED -----
	if (nic_rx_packet_waiting_to_be_dumped)
		return;

	if (ETHIRQbits.RXOVFLW)
	{
		//--------------------------------
		//----- RECEIVE OVERFLOW IRQ -----
		//--------------------------------
        nic_initialise(NIC_INIT_SPEED);

		//Exit
		return(0);
	}

	if (ETHIRQbits.RXBUSE)
	{
		//----------------------------------------
		//----- RX DMA ENGINE TRANSFER ERROR -----
		//----------------------------------------
		nic_initialise(NIC_INIT_SPEED);

		//Exit
		return(0);
	}
	
	if (nic_rx_descriptor[nic_rx_current_buffer].header.eown == 0)			//0 = software owned, 1 = hardware owned
	{
		//---------------------------------------------------------
		//----- NEXT RECEIVE BUFFER HAS BECOME SOFTWARE OWNED -----
		//---------------------------------------------------------

		//Ethernet interface notes:-
		//- nic_rx_current_buffer is the next buffer that will be used by the receive engine.
		//- When header.eown == 0 the receive engine has passed the buffer to software indicating that is contains either a valid
		//packet or some soret of error.  Because this driver sets the data buffer size as 1536 bytes we dont' need to worry about packets
		//spanning multiple buffers - a buffer will always contain an entire packet.
		
		//Use SOP and EOP to extract the message, use BYTE_COUNT, RXF_RSV, RSV and PKT_CHECKSUM to get the message characteristics.
		
		ETHCON1bits.BUFCDEC = 1;			//Decrement the packet buffer copunts bits (needs clearing for flow control)

		if (
		(nic_rx_descriptor[nic_rx_current_buffer].header.eop) &&
		(nic_rx_descriptor[nic_rx_current_buffer].header.sop) &&
		(!nic_rx_descriptor[nic_rx_current_buffer].status.crc_error) &&
		(!nic_rx_descriptor[nic_rx_current_buffer].status.length_check_error) &&
		(nic_rx_descriptor[nic_rx_current_buffer].status.rx_ok) &&
		((nic_rx_descriptor[nic_rx_current_buffer].status.unicast_match) || (nic_rx_descriptor[nic_rx_current_buffer].status.broadcast_match))
		)
		{
			//-------------------------------
			//----- A FRAGMENT RECEIVED -----
			//-------------------------------

			//----- HAS A WHOLE FRAME BEEN RECEVIED? -----
			//Yes - we use a fragment size that will hold an entire frame
			
	        nic_rx_bytes_remaining = (WORD)nic_rx_descriptor[nic_rx_current_buffer].status.rx_bytes_count;
	
			//SETUP THE DATA POINTER REGISTER TO READ THE RX PACKET
			nic_setup_read_data();
	
			nic_rx_packet_total_ethernet_bytes = nic_rx_bytes_remaining;
	
			//The ethernet stack processing routine will continue receiving the rest of the packet
			nic_rx_packet_waiting_to_be_dumped = PROCESS_NIC_CALLS_BEFORE_DUMP_RX;
			return(nic_rx_bytes_remaining);
		}
		else
		{
			//-------------------------
			//----- RECEIVE ERROR -----
			//-------------------------
	
			//REJECT PACKET
	        nic_rx_packet_waiting_to_be_dumped = 1;				//Force packet to be dumped
			nic_rx_dump_packet();
			return(0);
		}
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
	//nic_rx_current_buffer = ;	//Already set
	nic_rx_buffer_next_byte = (BYTE*)&nic_rx_data_buffer[nic_rx_current_buffer][0];

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
	nic_rx_buffer_next_byte = (BYTE*)&nic_rx_data_buffer[nic_rx_current_buffer][0];
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

	if (nic_rx_packet_waiting_to_be_dumped)
	{
		//Give the descriptor back to the receive hardware
		nic_rx_descriptor[nic_rx_current_buffer].p_data_buffer_address = (BYTE*)((DWORD)&nic_rx_data_buffer[nic_rx_current_buffer][0] & 0x1fffffff);		//& 0x1FFFFFFF to convert virtual address to physical address
		nic_rx_descriptor[nic_rx_current_buffer].status.Val = 0;
		nic_rx_descriptor[nic_rx_current_buffer].header.eown = 1;			//0 = software owned, 1 = hardware owned

		//We move up to the next descriptor
		if (++nic_rx_current_buffer >= NIC_NUM_OF_RX_FRAGMENT_BUFFERS)
			nic_rx_current_buffer = 0;

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


	if (nic_tx_descriptor[nic_tx_current_buffer].header.eown)			//0 = software owned, 1 = hardware owned
	{
		//----- NO TRANSMISSION BUFFER CURRENTLY AVAILABLE -----
		return(0);
	}


	//--------------------
	//----- SETUP TX -----
	//--------------------

	//Get packet buffer start position
    nic_tx_buffer_next_byte = &nic_tx_data_buffer[nic_tx_current_buffer][0];

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
	if(nic_tx_len >= NIC_FRAGMENT_BUFFERS_SIZE)
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

	tx_buffer = &nic_tx_data_buffer[nic_tx_current_buffer][0];		//Set to start of tx buffer

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
	nic_tx_descriptor[nic_tx_current_buffer].p_data_buffer_address = (BYTE*)((DWORD)&nic_tx_data_buffer[nic_tx_current_buffer][0] & 0x1fffffff);		//& 0x1FFFFFFF to convert virtual address to physical address
	
	nic_tx_descriptor[nic_tx_current_buffer].status.Val = 0;
	
	nic_tx_descriptor[nic_tx_current_buffer].header.byte_count = nic_tx_len;
	nic_tx_descriptor[nic_tx_current_buffer].header.sop = 1;
	nic_tx_descriptor[nic_tx_current_buffer].header.eop = 1;
	nic_tx_descriptor[nic_tx_current_buffer].header.eown = 1;			//0 = software owned, 1 = hardware owned
	
	//Enable the transmission of the message
	ETHCON1bits.TXRTS = 1;
	
	if (++nic_tx_current_buffer >= NIC_NUM_OF_TX_FRAGMENT_BUFFERS)
		nic_tx_current_buffer = 0;
		
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
		EMACxCFG2bits.FULLDPLX = 1;
		EMACxIPGT = 0x15;						//Back-to-Back Interpacket Gap bits.  In Full-Duplex the recommended setting is 0x15 which represents the minimum IPG of 0.96 탎 (in 100 Mbps) or 9.6 탎 (in 10 Mbps).
	}
	else
	{
		//Half duplex mode
		EMACxCFG2bits.FULLDPLX = 0;
		EMACxIPGT = 0x12;						//Back-to-Back Interpacket Gap bits.  In Half-Duplex mode the recommended setting is 0x12 which also represents the minimum IPG of 0.96 탎 (in 100 Mbps) or 9.6 탎 (in 10 Mbps).	
	}

	//Configure 100MBit/10MBit mode
	if (return_status & 0x01)
	{
		//100MBit mode
		EMACxSUPPbits.SPEEDRMII = 1;
	}
	else
	{
		//10MBit mode
		EMACxSUPPbits.SPEEDRMII = 0;
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

	//Wait for any previous operation to complete
	while (EthMIIMBusy())
		;

	EthMIIMWriteStart(address, 1, data);
	while (EthMIIMBusy())
		;
}



//***************************************
//***************************************
//********** READ PHY REGISTER **********
//***************************************
//***************************************
//(Via the MII serial interface)
WORD nic_read_phy_register (BYTE address)
{
	
	//Wait for any previous operation to complete
	while (EthMIIMBusy())
		;
		
	EthMIIMReadStart(address, 1);
	while(EthMIIMBusy())
		;
	return((WORD)EthMIIMReadResult());
}


