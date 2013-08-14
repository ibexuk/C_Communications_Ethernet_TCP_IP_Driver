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
//Customer submitted file


//*****************************************************************************
//
//! \addtogroup ethernet_api
//! @{
//
//*****************************************************************************
#define NIC_C

#include <string.h>

#include "libtype.h"

#include "eth-main.h"
#include "eth-nic.h"

#define uC_HW_REG(x) (*((volatile unsigned long *)(x)))


///////////////////////////////////////////////////////////////////////////////
BYTE nic_rxpacket[NIC_MAX_FRAME_LENGTH] ;
WORD nic_rxpacket_ndx ;
WORD nic_rx_packet_total_ethernet_bytes ;

BYTE nic_txpacket[NIC_MAX_FRAME_LENGTH] ;
WORD nic_txframe_ndx ;

///////////////////////////////////////////////////////////////////////////////
/*
   Write PHY regs
   ucPhyReg  = physical reg
   ulData    = data value

   return None
*/
///////////////////////////////////////////////////////////////////////////////
void nic_write_phy_register (BYTE ucRegAddr, WORD ulData)
{
  // Wait for any pending transaction to complete
  while(uC_HW_REG(ETH_BASE + ETH_MAC_MCTL) & ETH_MAC_MCTL_START) ;
  {
  }

  // Program the DATA to be written
  uC_HW_REG(ETH_BASE + ETH_MAC_MTXD) = ulData & ETH_MAC_MTXD_MASK ;

  // Program the PHY register address and initiate the transaction
  uC_HW_REG(ETH_BASE + ETH_MAC_MCTL) = (((ucRegAddr << 3) & ETH_MAC_MCTL_REGADR_MASK) | ETH_MAC_MCTL_WRITE | ETH_MAC_MCTL_START);

  // Wait for the write transaction to complete
  while(uC_HW_REG(ETH_BASE + ETH_MAC_MCTL) & ETH_MAC_MCTL_START) ;
  {
  }
}

///////////////////////////////////////////////////////////////////////////////
/*
   Read PHY regs
   ucPhyReg  = physical reg
   ulData    = data value

   return phy reg data read
*/
///////////////////////////////////////////////////////////////////////////////
WORD nic_read_phy_register (BYTE ucRegAddr)
{
  // Wait for any pending transaction to complete
  while(uC_HW_REG(ETH_BASE + ETH_MAC_MCTL) & ETH_MAC_MCTL_START) ;

  // Program the DATA to be written
  uC_HW_REG(ETH_BASE + ETH_MAC_MTXD) = ucRegAddr & ETH_MAC_MTXD_MASK ;

  // Program the PHY register address and initiate the transaction
  uC_HW_REG(ETH_BASE + ETH_MAC_MCTL) = (((ucRegAddr << 3) & ETH_MAC_MCTL_REGADR_MASK) | ETH_MAC_MCTL_START);

  // Wait for the write transaction to complete
  while(uC_HW_REG(ETH_BASE + ETH_MAC_MCTL) & ETH_MAC_MCTL_START) ;

  // Return the PHY data that was read.
  return(uC_HW_REG(ETH_BASE + ETH_MAC_MRXD) & ETH_MAC_MRXD_MASK);
}


///////////////////////////////////////////////////////////////////////////////
/*
   Enable nic RX/TX enable
*/
///////////////////////////////////////////////////////////////////////////////
void nic_enable()
{
  // Enable the Ethernet receiver.
  uC_HW_REG(ETH_BASE + ETH_MAC_RCTL) &= ~ETH_MAC_RCTL_RXEN;

  // Reset the receive FIFO.
  uC_HW_REG(ETH_BASE + ETH_MAC_RCTL) |= ETH_MAC_RCTL_RSTFIFO;

  // Enable the Ethernet receiver.
  uC_HW_REG(ETH_BASE + ETH_MAC_RCTL) |= ETH_MAC_RCTL_RXEN;

  // Enable Ethernet transmitter.
  uC_HW_REG(ETH_BASE + ETH_MAC_TCTL) |= ETH_MAC_TCTL_TXEN;
}

///////////////////////////////////////////////////////////////////////////////
/*
   Disable nic
*/
///////////////////////////////////////////////////////////////////////////////
void nic_disable()
{
  // Disable the Ethernet transmitter.
  uC_HW_REG(ETH_BASE + ETH_MAC_TCTL) &= ~ETH_MAC_TCTL_TXEN;

  // Disable the Ethernet receiver.
  uC_HW_REG(ETH_BASE + ETH_MAC_RCTL) &= ~ETH_MAC_RCTL_RXEN;

  // Reset the receive FIFO.
  uC_HW_REG(ETH_BASE + ETH_MAC_RCTL) |=  ETH_MAC_RCTL_RSTFIFO;
}


///////////////////////////////////////////////////////////////////////////////
/*
   Check for nic errors

   return 1 if error  0  ok
*/
///////////////////////////////////////////////////////////////////////////////
BYTE nic_error_check()
{
  DWORD Errormask ;

  Errormask = uC_HW_REG(ETH_BASE + ETH_MAC_RIS)  ;

  // RX overrun or TX error
  if (Errormask & (ETH_MAC_RIS_FOVRN | ETH_MAC_RIS_TXERR))
  {
    nic_initialise (NIC_INIT_SPEED) ;
    return (1) ;
  }

  // RX packet error discard packet
  if (Errormask & ETH_MAC_RIS_RXERR)
  {
    uC_HW_REG(ETH_BASE + ETH_MAC_IACK) = (ETH_MAC_IACK_RXERR) ;  // ACK error

//    nic_rx_packet_waiting_to_be_dumped = 1;       //Force packet to be dumped
//    nic_rx_dump_packet();
  }

  return 0 ;
}


///////////////////////////////////////////////////////////////////////////////
/*
   check for link ok
*/
///////////////////////////////////////////////////////////////////////////////
void nic_link_check()
{
  WORD uTemp ;

  uTemp = nic_read_phy_register (ETH_PHY_MR1)  ;

  nic_is_linked = (uTemp & ETH_PHY_MR1_LINK) ? 1 : 0;

  // manca info su link type 10/100
}

///////////////////////////////////////////////////////////////////////////////
/*
   Set MAC address

   pucMACAddr = MAC address
*/
///////////////////////////////////////////////////////////////////////////////
void EthernetMACAddrSet(MAC_ADDR *pMACAddr)
{
  volatile DWORD_VAL temp ;

  temp.val = 0 ;
  temp.v[0] = pMACAddr->v[0];
  temp.v[1] = pMACAddr->v[1];
  temp.v[2] = pMACAddr->v[2];
  temp.v[3] = pMACAddr->v[3];
  uC_HW_REG(ETH_BASE + ETH_MAC_IA0) = temp.val;

  temp.val = 0;
  temp.v[0] = pMACAddr->v[4];
  temp.v[1] = pMACAddr->v[5];
  uC_HW_REG(ETH_BASE + ETH_MAC_IA1) = temp.val;
}

///////////////////////////////////////////////////////////////////////////////
/*
   read MAC address

   pucMACAddr = MAC address
*/
///////////////////////////////////////////////////////////////////////////////
void EthernetMACAddrGet(MAC_ADDR *pMACAddr)
{
  volatile DWORD_VAL temp ;

  temp.val = uC_HW_REG(ETH_BASE + ETH_MAC_IA0);
  pMACAddr->v[0] = temp.v[0] ;
  pMACAddr->v[1] = temp.v[1] ;
  pMACAddr->v[2] = temp.v[2] ;
  pMACAddr->v[3] = temp.v[3] ;

  temp.val = uC_HW_REG(ETH_BASE + ETH_MAC_IA1);
  pMACAddr->v[4] = temp.v[0] ;
  pMACAddr->v[5] = temp.v[1] ;
}

///////////////////////////////////////////////////////////////////////////////
// Public functions

void nic_led_config()
{
  // init led lines
  uC_HW_REG(GPIO_RCGC2) = uC_HW_REG(GPIO_RCGC2) | GPIO_RCGC2_PORTF_BIT ; // enable PORTF peripheral
  uC_HW_REG(GPIO_RCGC2) = uC_HW_REG(GPIO_RCGC2) | GPIO_RCGC2_PORTF_BIT ; // enable PORTF peripheral
  uC_HW_REG(GPIO_RCGC2) = uC_HW_REG(GPIO_RCGC2) | GPIO_RCGC2_PORTF_BIT ; // enable PORTF peripheral
  uC_HW_REG(GPIO_RCGC2) = uC_HW_REG(GPIO_RCGC2) | GPIO_RCGC2_PORTF_BIT ; // enable PORTF peripheral

  // config led pins
  uC_HW_REG(ETH_LED_PORT_BASE + GPIO_OFFS_DIR)  = uC_HW_REG(ETH_LED_PORT_BASE + GPIO_OFFS_DIR)  | (ETH_LED_GREEN | ETH_LED_YELLOW) ; // DIrection
  uC_HW_REG(ETH_LED_PORT_BASE + GPIO_OFFS_AFSEL)= uC_HW_REG(ETH_LED_PORT_BASE + GPIO_OFFS_AFSEL)| (ETH_LED_GREEN | ETH_LED_YELLOW) ; // ALt. function
  uC_HW_REG(ETH_LED_PORT_BASE + GPIO_OFFS_DR4R) = uC_HW_REG(ETH_LED_PORT_BASE + GPIO_OFFS_DR4R) | (ETH_LED_GREEN | ETH_LED_YELLOW) ; // Drain 4mA
  uC_HW_REG(ETH_LED_PORT_BASE + GPIO_OFFS_DEN)  = uC_HW_REG(ETH_LED_PORT_BASE + GPIO_OFFS_DEN)  | (ETH_LED_GREEN | ETH_LED_YELLOW) ; // Enable
}

///////////////////////////////////////////////////////////////////////////////
/*
   return phy reg data read
*/
///////////////////////////////////////////////////////////////////////////////
void nic_initialise (BYTE init_config)
{
  // disable eth
  nic_disable() ;

  // First cfg clock prescaler
  uC_HW_REG(ETH_BASE + ETH_MAC_MDV) = (ETH_MIIDIV & ETH_MAC_MDV_DIVMASK) ;

  // Set eth MAC address
  EthernetMACAddrSet(&our_mac_address) ;

  // reset ALL pending isr
  uC_HW_REG(ETH_BASE + ETH_MAC_IACK) = (ETH_MAC_IACK_RXINT | ETH_MAC_IACK_TXERR | ETH_MAC_IACK_TXEMPTY |
                                        ETH_MAC_IACK_FOVRN | ETH_MAC_IACK_RXERR | ETH_MAC_IACK_MDINT   |
                                        ETH_MAC_IACK_PHYINT ) ;

  // Setup the Transmit Control Register
  uC_HW_REG(ETH_BASE + ETH_MAC_TCTL) = (ETH_MAC_TCTL_DUPLEX | ETH_MAC_TCTL_CRC | ETH_MAC_TCTL_PADEN ) ;

  // Setup the Receive Control Register.
  uC_HW_REG(ETH_BASE + ETH_MAC_RCTL) &= ~(ETH_MAC_RCTL_BADCRC | ETH_MAC_RCTL_PRMS | ETH_MAC_RCTL_AMUL); ;

   //----- DO FINAL FLAGS SETUP -----
  nic_is_linked = 0;
  nic_speed_is_100mbps = init_config;
  nic_rx_packet_waiting_to_be_dumped = 0;

  // configure led port
  nic_led_config() ;

  // enable eth
  nic_enable() ;
}

///////////////////////////////////////////////////////////////////////////////
WORD nic_check_for_rx (void)
{
  WORD j = 0 ;

  DWORD ulTemp ;

  WORD framelen ;

  if (nic_error_check()) return (0) ;

  nic_link_check() ;

  if(uC_HW_REG(ETH_BASE + ETH_MAC_NP))
  {
    nic_setup_read_data();

    // read first frame dword
    ulTemp = uC_HW_REG(ETH_BASE + ETH_MAC_DATA);
    framelen = (WORD)(ulTemp & 0x00000FFFF);

    // start fill data buffer
    nic_rxpacket[0] = (ulTemp >> 16) & 0x000000FF ;
    nic_rxpacket[1] = (ulTemp >> 24) & 0x000000FF ;

    // safety ctrl only
    if (framelen > NIC_MAX_FRAME_LENGTH) framelen = NIC_MAX_FRAME_LENGTH ;

    // read DWORD
    for (j = 4 ; j < framelen ; j+=4) // j start from second fifo dword of frame
    {
      *(DWORD *)&nic_rxpacket[j-2] = uC_HW_REG(ETH_BASE + ETH_MAC_DATA);
    }

    nic_rx_bytes_remaining = framelen - 2 - 4; // 2LEN+4FCS

    nic_rx_packet_total_ethernet_bytes = nic_rx_bytes_remaining;

    return nic_rx_bytes_remaining;
  }

  return 0 ;
}

///////////////////////////////////////////////////////////////////////////////
void nic_setup_read_data (void)
{
  memset (nic_rxpacket, 0, sizeof (nic_rxpacket)) ;
  nic_rxpacket_ndx =0  ;
}

///////////////////////////////////////////////////////////////////////////////
BYTE nic_read_next_byte (BYTE *data)
{
  *data = 0 ;

  if(nic_rx_bytes_remaining)
  {
   *data = nic_rxpacket[nic_rxpacket_ndx++] ;

    nic_rx_bytes_remaining--;

    return (1) ;
  }

  return 0 ;
}


///////////////////////////////////////////////////////////////////////////////
BYTE nic_read_array (BYTE *array_buffer, WORD array_length)
{
  WORD  len = array_length ;

  if (len > nic_rx_bytes_remaining) return 0 ;

  memcpy (array_buffer, &nic_rxpacket[nic_rxpacket_ndx], len) ;

  nic_rxpacket_ndx += len ;
  nic_rx_bytes_remaining -= len ;

  return(1);
}


///////////////////////////////////////////////////////////////////////////////
void nic_move_pointer (WORD move_pointer_to_ethernet_byte)
{
  if (move_pointer_to_ethernet_byte < NIC_MAX_FRAME_LENGTH)
  {
    nic_rxpacket_ndx = move_pointer_to_ethernet_byte ;
    nic_rx_bytes_remaining = nic_rx_packet_total_ethernet_bytes - move_pointer_to_ethernet_byte ;
  }
}

///////////////////////////////////////////////////////////////////////////////
//Discard any remaining bytes in the current RX packet and free up the nic for the next rx packet
void nic_rx_dump_packet (void)
{
//  if (nic_rx_packet_waiting_to_be_dumped)
  {
    nic_setup_read_data () ;

    nic_rx_packet_waiting_to_be_dumped = 0;
  }
}


///////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////
//Returns 1 if nic ready, 0 if not.
BYTE nic_setup_tx (void)
{
  // previous TX active ?
  if(uC_HW_REG(ETH_BASE + ETH_MAC_TR) & ETH_MAC_TR_NEWTX ) return(0);

  memset (nic_txpacket, 0, sizeof (nic_txpacket)) ;
  nic_tx_len = 0;

  return(1);
}

///////////////////////////////////////////////////////////////////////////////
//(nic_setup_tx must have already been called)
//The nic stores the ethernet tx in words.  This routine deals with this and allows us to work in bytes.
void nic_write_next_byte (BYTE data)
{
  //----- UPDATE THE TX LEN COUNT AND EXIT IF TOO MANY BYTES HAVE BEEN WRITTEN FOR THE NIC -----
  if(nic_tx_len >= (NIC_MAX_FRAME_LENGTH - 6))
    return;

  nic_txpacket[nic_tx_len++] = data;
}

///////////////////////////////////////////////////////////////////////////////
//(nic_setup_tx must have already been called)
void nic_write_array (BYTE *array_buffer, WORD array_length)
{
  WORD  len = array_length ;

  if (len > (NIC_MAX_FRAME_LENGTH - 6) - nic_tx_len) len = (NIC_MAX_FRAME_LENGTH - 6) - nic_tx_len;

  memcpy (&nic_txpacket[nic_tx_len], array_buffer, len) ;
  nic_tx_len += len ;

}

///////////////////////////////////////////////////////////////////////////////
//byte_address must be word aligned
void nic_write_tx_word_at_location (WORD byte_address, WORD data)
{
  if (byte_address < NIC_MAX_FRAME_LENGTH-1)
  {
    nic_txpacket[byte_address++] = data & 0x00FF ;
    nic_txpacket[byte_address  ] = data >> 8 ;
  }
}

///////////////////////////////////////////////////////////////////////////////
// pad bytes & crc are hw functions
void nix_tx_packet (void)
{
  DWORD_VAL temp ;

  WORD  j = 0;

  // write first word
  temp.val  = (DWORD)(nic_tx_len -  14); // len without header
  temp.v[2] = nic_txpacket[j++] ;
  temp.v[3] = nic_txpacket[j++] ;
  uC_HW_REG(ETH_BASE + ETH_MAC_DATA)= temp.val;

  // write word1 to wordN-1
  for (j=2; j <= (nic_tx_len - 4) ; j += 4)
  {
      uC_HW_REG(ETH_BASE + ETH_MAC_DATA) = *((DWORD *)&nic_txpacket[j]) ;
  }

  // write last wordN
  if(j != nic_tx_len)
  {
    temp.val = 0 ;

    if(j == (nic_tx_len - 1))
    {
      temp.v[0] = nic_txpacket[j++] ;
    }
    else if(j == (nic_tx_len - 2))
    {
      temp.v[0] = nic_txpacket[j++] ;
      temp.v[1] = nic_txpacket[j++] ;
    }
    else if(j == (nic_tx_len - 3))
    {
      temp.v[0] = nic_txpacket[j++] ;
      temp.v[1] = nic_txpacket[j++] ;
      temp.v[2] = nic_txpacket[j++] ;
    }

    uC_HW_REG(ETH_BASE + ETH_MAC_DATA) = temp.val;
  }

  // OK to tx
  uC_HW_REG(ETH_BASE + ETH_MAC_TR) = ETH_MAC_TR_NEWTX;
}

///////////////////////////////////////////////////////////////////////////////
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


///////////////////////////////////////////////////////////////////////////////
// eof



