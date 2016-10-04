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
//ARP (ADDRESS RESOLUTION PROTOCOL) C CODE FILE


#include "main.h"					//Global data type definitions (see https://github.com/ibexuk/C_Generic_Header_File )
#define ARP_C
#include "eth-arp.h"
#undef ARP_C

#include "eth-main.h"
#include "eth-nic.h"



//************************************
//************************************
//********** INITIALISE ARP **********
//************************************
//************************************
//Called from the tcp_ip_initialise function
void arp_initialise (void)
{

	sm_arp = SM_ARP_IDLE;

	arp_last_received_response.mac_address.v[0] = 0;
	arp_last_received_response.mac_address.v[1] = 0;
	arp_last_received_response.mac_address.v[2] = 0;
	arp_last_received_response.mac_address.v[3] = 0;
	arp_last_received_response.mac_address.v[4] = 0;
	arp_last_received_response.mac_address.v[5] = 0;
	arp_last_received_response.ip_address.Val = 0x00000000;
}



//********************************************
//********************************************
//********** ARP RESOLVE IP ADDRESS **********
//********************************************
//********************************************
//Returns 1 if request was sent, 0 if request can't be sent right now
BYTE arp_resolve_ip_address(IP_ADDR *ip_address_to_resolve)
{
	DEVICE_INFO remote_device_info;


	//Setup ready to tx a packet
	if (nic_setup_tx() == 0)
		return(0);		//nic is not currently ready for a new tx packet

	//Clear the last resolved arp ip address
	arp_last_received_response.ip_address.Val = 0x00000000;

	//Setup packet to tx
	remote_device_info.mac_address.v[0] = 0xff;
	remote_device_info.mac_address.v[1] = 0xff;
	remote_device_info.mac_address.v[2] = 0xff;
	remote_device_info.mac_address.v[3] = 0xff;
	remote_device_info.mac_address.v[4] = 0xff;
	remote_device_info.mac_address.v[5] = 0xff;
	
	remote_device_info.ip_address = *ip_address_to_resolve;

	//Send the packet
	arp_tx_packet(&remote_device_info, ARP_OPCODE_REQUEST);
	
	return(1);
}



//*********************************************
//*********************************************
//********** CHECK ARP RESOLVE STATE **********
//*********************************************
//*********************************************
//ip_address_being_resolved = The IP address that is waiting to be resolved
//resolved_mac_address = The mac address for the resolved IP address will be stored to here
//Returns 1 if resolved, 0 if not yet resolved
BYTE arp_is_resolve_complete (IP_ADDR *ip_address_being_resolved, MAC_ADDR *resolved_mac_address)
{

	if (
		(arp_last_received_response.ip_address.Val == ip_address_being_resolved->Val) ||											//If IP address matches last returned ARP response then arp is resolved
        ((our_gateway_ip_address.Val != 0x00000000) && (arp_last_received_response.ip_address.Val == our_gateway_ip_address.Val))	//or if the last returned IP address is the gateway IP address then arp
    )																																//is resolved (this would mean the target IP is on a different subnet so
	{
        resolved_mac_address->v[0] = arp_last_received_response.mac_address.v[0];
        resolved_mac_address->v[1] = arp_last_received_response.mac_address.v[1];
        resolved_mac_address->v[2] = arp_last_received_response.mac_address.v[2];
        resolved_mac_address->v[3] = arp_last_received_response.mac_address.v[3];
        resolved_mac_address->v[4] = arp_last_received_response.mac_address.v[4];
        resolved_mac_address->v[5] = arp_last_received_response.mac_address.v[5];
        return(1);
    }
    return(0);
}



//*******************************************
//*******************************************
//********** PROCESS ARP RX PACKET **********
//*******************************************
//*******************************************
//Called from the recevied packet ethernet stack state machine
//Returns 1 if ARP was processed, 0 if couldn't do it this time
BYTE arp_process_rx (void)
{
	static DEVICE_INFO remote_device_info;
	ARP_PACKET arp_packet;


	switch(sm_arp)
	{
	case SM_ARP_IDLE:
		//-----------------------------------------------------------------------------
		//-----------------------------------------------------------------------------
		//----- AN ARP PACKET IS WAITING TO BE READ OUT OF THE NIC RECEIVE BUFFER -----
		//-----------------------------------------------------------------------------
		//-----------------------------------------------------------------------------

		//READ THE PACKET INTO OUR BUFFER
		//We can't do this as the ARP_PACKET structure is not aligned for 32bit architectures so will often be padded:
		//if (!nic_read_array((BYTE*)&arp_packet, sizeof(arp_packet)))
		//	return(1);									//Error - packet was too small - dump it
		
		//Instead we do this to avoid padding problems:
		if (!nic_read_array((BYTE*)&arp_packet.hardware_type, 2))
			return(1);									//Error - packet was too small - dump it
		if (!nic_read_array((BYTE*)&arp_packet.protocol, 2))
			return(1);									//Error - packet was too small - dump it
		if (!nic_read_next_byte(&arp_packet.mac_addr_len))
			return(1);									//Error - packet was too small - dump it
		if (!nic_read_next_byte(&arp_packet.protocol_len))
			return(1);									//Error - packet was too small - dump it
		if (!nic_read_array((BYTE*)&arp_packet.op_code, 2))
			return(1);									//Error - packet was too small - dump it
		if (!nic_read_array((BYTE*)&arp_packet.sender_mac_addr, 6))
			return(1);									//Error - packet was too small - dump it
		if (!nic_read_array((BYTE*)&arp_packet.sender_ip_addr, 4))
			return(1);									//Error - packet was too small - dump it
		if (!nic_read_array((BYTE*)&arp_packet.target_mac_addr, 6))
			return(1);									//Error - packet was too small - dump it
		if (!nic_read_array((BYTE*)&arp_packet.target_ip_addr, 4))
			return(1);									//Error - packet was too small - dump it

		//Swap the bytes in hardware type, protocol & operation
		arp_packet.hardware_type = swap_word_bytes(arp_packet.hardware_type);
		arp_packet.protocol = swap_word_bytes(arp_packet.protocol);
		arp_packet.op_code = swap_word_bytes(arp_packet.op_code);

		//DUMP THE PACKET FROM THE NIC BUFFER
		nic_rx_dump_packet();

		//Check hardware type
		//(Don't care about the high byte, the low byte should be 0x01)
		if ((arp_packet.hardware_type & 0x00ff) != 0x0001)
			return(1);

		//Check protocol
		//(Should be 0x0800)
		if (arp_packet.protocol != 0x0800)
			return(1);

		//Check the mac address length
		if (arp_packet.mac_addr_len != MAC_ADDR_LENGTH)
			return(1);

		//Check the IP address length
		if (arp_packet.protocol_len != IP_ADDR_LENGTH)
			return(1);

		//CHECK THE TARGET IP ADDRESS MATCHES US
		if ((arp_packet.target_ip_addr.v[0] != our_ip_address.v[0]) ||
			(arp_packet.target_ip_addr.v[1] != our_ip_address.v[1]) ||
			(arp_packet.target_ip_addr.v[2] != our_ip_address.v[2]) ||
			(arp_packet.target_ip_addr.v[3] != our_ip_address.v[3]))
		{
			//----- PACKET IS NOT FOR US -----
			return(1);
		}

		//----- CHECK THE OP CODE -----
		//(Don't care about the high byte)
		if ((arp_packet.op_code & 0x00ff) == ARP_OPCODE_REQUEST)
		{
			//--------------------------------
			//----- ARP REQUEST RECEIVED -----
			//--------------------------------
			//Store the remote device details in case we can't tx our response now
			remote_device_info.mac_address.v[0] = arp_packet.sender_mac_addr.v[0];
			remote_device_info.mac_address.v[1] = arp_packet.sender_mac_addr.v[1];
			remote_device_info.mac_address.v[2] = arp_packet.sender_mac_addr.v[2];
			remote_device_info.mac_address.v[3] = arp_packet.sender_mac_addr.v[3];
			remote_device_info.mac_address.v[4] = arp_packet.sender_mac_addr.v[4];
			remote_device_info.mac_address.v[5] = arp_packet.sender_mac_addr.v[5];
			remote_device_info.ip_address.Val = arp_packet.sender_ip_addr.Val;
			
			sm_arp = SM_ARP_SEND_REPLY;
			//Don't break so that we attempt to reply now;
		}
		else if ((arp_packet.op_code & 0x00ff) == ARP_OPCODE_RESPONSE)
		{
			//---------------------------------
			//----- ARP RESPONSE RECEIVED -----
			//---------------------------------
			//Store the response ready for the requesting function to check when its next called
			arp_last_received_response.mac_address.v[0] = arp_packet.sender_mac_addr.v[0];
			arp_last_received_response.mac_address.v[1] = arp_packet.sender_mac_addr.v[1];
			arp_last_received_response.mac_address.v[2] = arp_packet.sender_mac_addr.v[2];
			arp_last_received_response.mac_address.v[3] = arp_packet.sender_mac_addr.v[3];
			arp_last_received_response.mac_address.v[4] = arp_packet.sender_mac_addr.v[4];
			arp_last_received_response.mac_address.v[5] = arp_packet.sender_mac_addr.v[5];
			arp_last_received_response.ip_address.Val = arp_packet.sender_ip_addr.Val;
			return(1);
		}
		else
		{
			//---------------------------
			//----- INVALID OP CODE -----
			//---------------------------
			return(1);
		}



	case SM_ARP_SEND_REPLY:		//case SM_ARP_IDLE will fall into this case if we want to try to reply to a received request
		//-----------------------------
		//-----------------------------
		//----- SEND ARP RESPONSE -----
		//-----------------------------
		//-----------------------------
		if (nic_setup_tx())
		{
			//--------------------------------------------
			//----- WE CAN TRANSMIT OUR RESPONSE NOW -----
			//--------------------------------------------
			arp_tx_packet(&remote_device_info, ARP_OPCODE_RESPONSE);
			sm_arp = SM_ARP_IDLE;
			return(1);
		}
		else
		{
			//-----------------------------------------------------------
			//----- CAN'T TX OUR RESPONSE NOW - TRY AGAIN NEXT TIME -----
			//-----------------------------------------------------------
			return(0);
		}

	}
	return(0);
}



//***********************************
//***********************************
//********** ARP TX PACKET **********
//***********************************
//***********************************
//nic_setup_tx() must have been called first
void arp_tx_packet (DEVICE_INFO *remote_device_info, WORD op_code)
{

	//----- WRITE THE ETHERNET HEADER -----
	write_eth_header_to_nic(&remote_device_info->mac_address, ETHERNET_TYPE_ARP);

	//----- WRITE THE ARP PACKET -----
	//SEND HARDWARE TYPE [2]
	nic_write_next_byte((BYTE)(ETHERNET_HARDWARE_TYPE >> 8));
	nic_write_next_byte((BYTE)(ETHERNET_HARDWARE_TYPE & 0x00ff));

	//SEND PROTOCOL TYPE [2]
	nic_write_next_byte((BYTE)(ETHERNET_PROTOCOL_ARP >> 8));
	nic_write_next_byte((BYTE)(ETHERNET_PROTOCOL_ARP & 0x00ff));

	//SEND MAC ADDRESS LENGTH [1]
	nic_write_next_byte((BYTE)MAC_ADDR_LENGTH);

	//SEND IP ADDRESS LENGTH [1]
	nic_write_next_byte((BYTE)IP_ADDR_LENGTH);

	//SEND OP CODE [2]
	nic_write_next_byte((BYTE)(op_code >> 8));
	nic_write_next_byte((BYTE)(op_code & 0x00ff));

	//SEND OUR MAC ADDRESS [6]
	nic_write_next_byte(our_mac_address.v[0]);
	nic_write_next_byte(our_mac_address.v[1]);
	nic_write_next_byte(our_mac_address.v[2]);
	nic_write_next_byte(our_mac_address.v[3]);
	nic_write_next_byte(our_mac_address.v[4]);
	nic_write_next_byte(our_mac_address.v[5]);

	//SEND OUR IP ADDRESS [4]
	nic_write_next_byte(our_ip_address.v[0]);
	nic_write_next_byte(our_ip_address.v[1]);
	nic_write_next_byte(our_ip_address.v[2]);
	nic_write_next_byte(our_ip_address.v[3]);

	//SEND DESTINATION MAC ADDRESS [6]
	if (op_code == ARP_OPCODE_RESPONSE)
	{
		//Sending a response - return to senders mac
		nic_write_next_byte(remote_device_info->mac_address.v[0]);
		nic_write_next_byte(remote_device_info->mac_address.v[1]);
		nic_write_next_byte(remote_device_info->mac_address.v[2]);
		nic_write_next_byte(remote_device_info->mac_address.v[3]);
		nic_write_next_byte(remote_device_info->mac_address.v[4]);
		nic_write_next_byte(remote_device_info->mac_address.v[5]);
	}
	else
	{
		//Sending a request - broadcast
		nic_write_next_byte(0xff);
		nic_write_next_byte(0xff);
		nic_write_next_byte(0xff);
		nic_write_next_byte(0xff);
		nic_write_next_byte(0xff);
		nic_write_next_byte(0xff);
	}

	//SEND DESTINATION IP ADDRESS [4]
	//If destination is not on the same subnet as us then send this to our gateway instead
	//Once the gateway returns its mac address the pending TCP/IP communication will automatically get sent to the
	//gateway which will do its job and forward packets on.
	if (((our_ip_address.v[0] ^ remote_device_info->ip_address.v[0]) & our_subnet_mask.v[0]) ||
		((our_ip_address.v[1] ^ remote_device_info->ip_address.v[1]) & our_subnet_mask.v[1]) ||
		((our_ip_address.v[2] ^ remote_device_info->ip_address.v[2]) & our_subnet_mask.v[2]) ||
		((our_ip_address.v[3] ^ remote_device_info->ip_address.v[3]) & our_subnet_mask.v[3]) )
	{
		//TARGET IS NOT ON OUR SUBNET - USE THE GATEWAY IP ADDRESS
		nic_write_next_byte(our_gateway_ip_address.v[0]);
		nic_write_next_byte(our_gateway_ip_address.v[1]);
		nic_write_next_byte(our_gateway_ip_address.v[2]);
		nic_write_next_byte(our_gateway_ip_address.v[3]);
	}
	else
	{
		//TARET IS ON OUR SUBNET - USE THE TARGET IP ADDRESS
		nic_write_next_byte(remote_device_info->ip_address.v[0]);
		nic_write_next_byte(remote_device_info->ip_address.v[1]);
		nic_write_next_byte(remote_device_info->ip_address.v[2]);
		nic_write_next_byte(remote_device_info->ip_address.v[3]);
	}



	//----- SEND THE PACKET -----
	nix_tx_packet();

	return;
}














