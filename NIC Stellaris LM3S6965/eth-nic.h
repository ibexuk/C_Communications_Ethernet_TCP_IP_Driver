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



///////////////////////////////////////////////////////////////////////////////
/*
  Stellaris ethernet controller nic

*/
///////////////////////////////////////////////////////////////////////////////

#ifndef eth_nic
#define eth_nic
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// global define

// embedded stack define
#define NIC_INIT_SPEED        1 // 1 100Mbps

// nic hw define
#define ETH_BASE   0x40048000

#define ETH_MIICLK 2500000
#define ETH_PHYCLK 25000000
#define ETH_MIIDIV ((ETH_PHYCLK >> 1) / ETH_MIICLK)

#define NIC_MAX_FRAME_LENGTH        2048      //Max Ethernet Frame Size

///////////////////////////////////////////////////////////////////////////////
// Peripheral enable

#define GPIO_RCGC2              0x400FE108

#define GPIO_RCGC2_PORTF_BIT    0x00000020
#define GPIO_RCGC2_EPHY0_BIT    0x10000000
#define GPIO_RCGC2_EMAC0_BIT    0x40000000

///////////////////////////////////////////////////////////////////////////////
// led hw

#define GPIO_PORTF_BASE         0x40025000
#define GPIO_OFFS_DIR           0x00000400  // Data direction register.
#define GPIO_OFFS_AFSEL         0x00000420  // Mode control select register.
#define GPIO_OFFS_DR4R          0x00000504  // 4ma drive select register.
#define GPIO_OFFS_DEN           0x0000051C  // Digital input enable register.

#define ETH_LED_PORT_BASE GPIO_PORTF_BASE
#define ETH_LED_GREEN     0x04
#define ETH_LED_YELLOW    0x08

///////////////////////////////////////////////////////////////////////////////
// PHY define

// regs
#define ETH_PHY_MR0                 0x00000000  // PHY cfg
#define ETH_PHY_MR1                 0x00000001  // PHY status
#define ETH_PHY_MR2                 0x00000002  // PHY ID1
#define ETH_PHY_MR3                 0x00000003  // PHY ID2
#define ETH_PHY_MR4                 0x00000004  // PHY auto negotiation
#define ETH_PHY_MR5                 0x00000005  // PHY auto negotiation
#define ETH_PHY_MR6                 0x00000006  // PHY auto negotiation
#define ETH_PHY_MR16                0x00000010  // PHY vendor specific
#define ETH_PHY_MR17                0x00000011  // PHY control register
#define ETH_PHY_MR18                0x00000012  // PHY diagnostic
#define ETH_PHY_MR19                0x00000013  // PHY transceiver ctrl
#define ETH_PHY_MR23                0x00000017  // PHY led config
#define ETH_PHY_MR24                0x00000018  // PHY MDI/MDIX ctrl

// bits
/////////////////////////////////////////////////////////////////////////////////// ETH_PHY0 bits
#define ETH_PHY_MR0_COLT            0x00000080  // 1 Collision Test
#define ETH_PHY_MR0_DUPLEX          0x00000100  // 1 Full Duplex Mode   0 Half Duplex
#define ETH_PHY_MR0_RANEG           0x00000200  // 1 Restart Auto-Negotiation clr by hw
#define ETH_PHY_MR0_ISO             0x00000400  // Isolate.
#define ETH_PHY_MR0_PWRDN           0x00000800  // 1 Power Down.
#define ETH_PHY_MR0_ANEGEN          0x00001000  // 1 Enable Auto-Negotiation
#define ETH_PHY_MR0_SPEEDSL         0x00002000  // 1 Enable 100Mb/s   0 10Mb/s
#define ETH_PHY_MR0_LOOPBK          0x00004000  // 1 Loopback Mode enable
#define ETH_PHY_MR0_RESET           0x00008000  // 1 Reset Registers.

/////////////////////////////////////////////////////////////////////////////////// ETH_PHY1 bits
#define ETH_PHY_MR1_EXTD            0x00000001  // 1 Extended Capabilities enabled
#define ETH_PHY_MR1_JAB             0x00000002  // 1 Jabber Condition detect
#define ETH_PHY_MR1_LINK            0x00000004  // 1 Link OK
#define ETH_PHY_MR1_ANEGA           0x00000008  // 1 Auto-Negotiation
#define ETH_PHY_MR1_RFAULT          0x00000010  // 1 Remote Fault condition
#define ETH_PHY_MR1_ANEGC           0x00000020  // 1 Auto-Negotiation Complete
#define ETH_PHY_MR1_MFPS            0x00000040  // 1 Management Frames with Preamble suppressed
#define ETH_PHY_MR1_10T_H           0x00000800  // 1 Support 10BASE-T Half-Duplex Mode.
#define ETH_PHY_MR1_10T_F           0x00001000  // 1 Support 10BASE-T Full-Duplex Mode.
#define ETH_PHY_MR1_100X_H          0x00002000  // 1 Support 100BASE-TX Half-Duplex Mode.
#define ETH_PHY_MR1_100X_F          0x00004000  // 1 Support 100BASE-TX Full-Duplex Mode.

/////////////////////////////////////////////////////////////////////////////////// ETH_PHY2 bits
#define ETH_PHY_MR2_OUI_MASK        0x0000FFFF  // OUI [21..6]

/////////////////////////////////////////////////////////////////////////////////// ETH_PHY3 bits
#define ETH_PHY_MR3_REVN_MASK       0x0000000F  // Revision Number
#define ETH_PHY_MR3_MODELN_MASK     0x000003F0  // Model Number
#define ETH_PHY_MR3_OUI_M           0x0000FC00  // OUI [5..0]

/////////////////////////////////////////////////////////////////////////////////// ETH_PHY4 bits
#define ETH_PHY_MR4_SEL_MASK        0x0000001F  // Selector field mask  Hard coded 0x01 802.3
#define ETH_PHY_MR4_A0              0x00000020  // 1 Technology Ability  10Base-T half duplex
#define ETH_PHY_MR4_A1              0x00000040  // 1 Technology Ability  10Base-T full duplex
#define ETH_PHY_MR4_A2              0x00000080  // 1 Technology Ability 100Base-T half duplex
#define ETH_PHY_MR4_A3              0x00000100  // 1 Technology Ability 100Base-T full duplex
#define ETH_PHY_MR4_RF              0x00002000  // 1 Remote Fault.
#define ETH_PHY_MR4_NP              0x00008000  // 1 Next Page info

/////////////////////////////////////////////////////////////////////////////////// ETH_PHY5 bits
#define ETH_PHY_MR5_SEL_MASK        0x0000001F  // Selector Field.
#define ETH_PHY_MR5_AF_MASk         0x00001FE0  // Technology Ability Field.
#define ETH_PHY_MR5_RF              0x00002000  // 1 Remote Fault
#define ETH_PHY_MR5_ACK             0x00004000  // 1 ACK negotiation
#define ETH_PHY_MR5_NP              0x00008000  // 1 Next Page info


//ETH_PHY_MR5_SEL field values
#define ETH_PHY_MR5_SEL_8023        0x00000001  // IEEE Std 802.3
#define ETH_PHY_MR5_SEL_8029        0x00000002  // IEEE Std 802.9 ISLAN-16T
#define ETH_PHY_MR5_SEL_8025        0x00000003  // IEEE Std 802.5
#define ETH_PHY_MR5_SEL_1394        0x00000004  // IEEE Std 1394

/////////////////////////////////////////////////////////////////////////////////// ETH_PHY6 bits
#define ETH_PHY_MR6_LPANEGA         0x00000001  // Link Partner is Auto-Negotiation Able
#define ETH_PHY_MR6_PRX             0x00000002  // New Page Received
#define ETH_PHY_MR6_LPNPA           0x00000008  // Link Partner is Next Page Able
#define ETH_PHY_MR6_PDF             0x00000010  // Parallel Detection Fault

/////////////////////////////////////////////////////////////////////////////////// ETH_PHY16 bits
#define ETH_PHY_MR16_RXCC           0x00000001  // Receive Clock Control power saving
#define ETH_PHY_MR16_PCSBP          0x00000002  // PCS Bypass
#define ETH_PHY_MR16_RVSPOL         0x00000010  // Receive Data Polarity
#define ETH_PHY_MR16_APOL           0x00000020  // Auto-Polarity Disable
#define ETH_PHY_MR16_NL10           0x00000400  // Natural Loopback Mode
#define ETH_PHY_MR16_SQEI           0x00000800  // SQE Inhibit Testing
#define ETH_PHY_MR16_TXHIM          0x00001000  // Transmit High Impedance Mode
#define ETH_PHY_MR16_INPOL          0x00004000  // Interrupt Polarity  !! MUST BE 0
#define ETH_PHY_MR16_RPTR           0x00008000  // Repeater Mode

/////////////////////////////////////////////////////////////////////////////////// ETH_PHY17 bits
#define ETH_PHY_MR17_INT_ANEGCOMP   0x00000001  // Auto-Negotiation Complete Interrupt
#define ETH_PHY_MR17_INT_RFAULT     0x00000002  // Remote Fault Interrupt
#define ETH_PHY_MR17_INT_LSCHG      0x00000004  // Link Status Change Interrupt
#define ETH_PHY_MR17_INT_LPACK      0x00000008  // LP Acknowledge Interrupt
#define ETH_PHY_MR17_INT_PDF        0x00000010  // Parallel Detection Fault Interrupt
#define ETH_PHY_MR17_INT_PRX        0x00000020  // Page Receive Interrupt
#define ETH_PHY_MR17_INT_RXER       0x00000040  // Receive Error Interrupt
#define ETH_PHY_MR17_INT_JABBER     0x00000080  // Jabber Event Interrupt
#define ETH_PHY_MR17_IE_ANEGCOMP    0x00000100  // Auto-Negotiation Complete Interrupt Enable
#define ETH_PHY_MR17_IE_RFAULT      0x00000200  // Remote Fault Interrupt Enable
#define ETH_PHY_MR17_IE_LSCHG       0x00000400  // Link Status Change Interrupt Enable
#define ETH_PHY_MR17_IE_LPACK       0x00000800  // LP Acknowledge Interrupt Enable
#define ETH_PHY_MR17_IE_PDF         0x00001000  // Parallel Detection Fault Interrupt Enable
#define ETH_PHY_MR17_IE_PRX         0x00002000  // Page Received Interrupt Enable
#define ETH_PHY_MR17_IE_RXER        0x00004000  // Receive Error Interrupt Enable
#define ETH_PHY_MR17_IE_JABBER      0x00008000  // Jabber Interrupt Enable

/////////////////////////////////////////////////////////////////////////////////// ETH_PHY18 bits
#define ETH_PHY_MR18_RX_LOCK        0x00000100  // Receive PLL Lock
#define ETH_PHY_MR18_RXSD           0x00000200  // Receive Detection
#define ETH_PHY_MR18_RATE           0x00000400  // Rate
#define ETH_PHY_MR18_DPLX           0x00000800  // Duplex Mode
#define ETH_PHY_MR18_ANEGF          0x00001000  // Auto-Negotiation Failure

/////////////////////////////////////////////////////////////////////////////////// ETH_PHY19 bits
#define ETH_PHY_MR19_TXO_MASK       0x0000C000  // Transmit Amplitude Selection.

// PHY_MR19_TXO values
#define ETH_PHY_MR19_TXO_00DB       0x00000000  // Gain set for 0.0dB of insertion loss
#define ETH_PHY_MR19_TXO_04DB       0x00004000  // Gain set for 0.4dB of insertion loss
#define ETH_PHY_MR19_TXO_08DB       0x00008000  // Gain set for 0.8dB of insertion loss
#define ETH_PHY_MR19_TXO_12DB       0x0000C000  // Gain set for 1.2dB of insertion loss

/////////////////////////////////////////////////////////////////////////////////// ETH_PHY23 bits
#define ETH_PHY_MR23_LED0_MASK      0x0000000F  // LED0 Source.
#define ETH_PHY_MR23_LED1_MASK      0x000000F0  // LED1 Source.

// LED0
#define ETH_PHY_MR23_LED0_LINK      0x00000000  // Link OK (Default LED0)
#define ETH_PHY_MR23_LED0_RXTX      0x00000001  // RX or TX Activity
#define ETH_PHY_MR23_LED0_TX        0x00000002  // TX Activity
#define ETH_PHY_MR23_LED0_RX        0x00000003  // RX Activity
#define ETH_PHY_MR23_LED0_COL       0x00000004  // Collision
#define ETH_PHY_MR23_LED0_100       0x00000005  // 100BASE-TX mode
#define ETH_PHY_MR23_LED0_10        0x00000006  // 10BASE-T mode
#define ETH_PHY_MR23_LED0_DUPLEX    0x00000007  // Full-Duplex
#define ETH_PHY_MR23_LED0_LINKACT   0x00000008  // Link OK & Blink=RX or TX Activity

// LED1
#define ETH_PHY_MR23_LED1_LINK      0x00000000  // Link OK
#define ETH_PHY_MR23_LED1_RXTX      0x00000010  // RX or TX Activity (Default LED1)
#define ETH_PHY_MR23_LED1_TX        0x00000020  // TX Activity
#define ETH_PHY_MR23_LED1_RX        0x00000030  // RX Activity
#define ETH_PHY_MR23_LED1_COL       0x00000040  // Collision
#define ETH_PHY_MR23_LED1_100       0x00000050  // 100BASE-TX mode
#define ETH_PHY_MR23_LED1_10        0x00000060  // 10BASE-T mode
#define ETH_PHY_MR23_LED1_DUPLEX    0x00000070  // Full-Duplex
#define ETH_PHY_MR23_LED1_LINKACT   0x00000080  // Link OK & Blink=RX or TX  Activity

/////////////////////////////////////////////////////////////////////////////////// ETH_PHY24 bits
#define ETH_PHY_MR24_MDIX_SD_MASK   0x0000000F  // Auto-Switching Seed.
#define ETH_PHY_MR24_MDIX_CM        0x00000010  // Auto-Switching Complete.
#define ETH_PHY_MR24_MDIX           0x00000020  // Auto-Switching Configuration.
#define ETH_PHY_MR24_AUTO_SW        0x00000040  // Auto-Switching Enable.
#define ETH_PHY_MR24_PD_MODE        0x00000080  // Parallel Detection Mode.

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// MAC define

//regs
#define ETH_MAC_RIS                 0x00000000  // RO Ethernet MAC Raw Interrupt Status
#define ETH_MAC_IACK                0x00000000  // WO Interrupt Acknowledge Register
#define ETH_MAC_IM                  0x00000004  // Interrupt Mask Register
#define ETH_MAC_RCTL                0x00000008  // Receive Control Register
#define ETH_MAC_TCTL                0x0000000C  // Transmit Control Register
#define ETH_MAC_DATA                0x00000010  // Data Register
#define ETH_MAC_IA0                 0x00000014  // Individual Address Register 0
#define ETH_MAC_IA1                 0x00000018  // Individual Address Register 1
#define ETH_MAC_THR                 0x0000001C  // Threshold Register
#define ETH_MAC_MCTL                0x00000020  // Management Control Register
#define ETH_MAC_MDV                 0x00000024  // Management Divider Register
#define ETH_MAC_MTXD                0x0000002C  // Management Transmit Data Reg
#define ETH_MAC_MRXD                0x00000030  // Management Receive Data Reg
#define ETH_MAC_NP                  0x00000034  // Number of Packets Register
#define ETH_MAC_TR                  0x00000038  // Transmission Request Register
#define ETH_MAC_TS                  0x0000003C  // Timer Support Register

// bits
/////////////////////////////////////////////////////////////////////////////////// ETH_MACRIS
#define ETH_MAC_RIS_RXINT           0x00000001  // Packet Received
#define ETH_MAC_RIS_TXERR           0x00000002  // Transmit Error
#define ETH_MAC_RIS_TXEMPTY         0x00000004  // Transmit FIFO Empty
#define ETH_MAC_RIS_FOVRN           0x00000008  // FIFO Overrrun
#define ETH_MAC_RIS_RXERR           0x00000010  // Receive Error
#define ETH_MAC_RIS_MDINT           0x00000020  // MII Transaction Complete
#define ETH_MAC_RIS_PHYINT          0x00000040  // PHY Interrupt

/////////////////////////////////////////////////////////////////////////////////// ETH_MACIACK
#define ETH_MAC_IACK_RXINT          0x00000001  // Packet Received ack
#define ETH_MAC_IACK_TXERR          0x00000002  // Transmit Error ack
#define ETH_MAC_IACK_TXEMPTY        0x00000004  // Transmit FIFO Empty ack
#define ETH_MAC_IACK_FOVRN          0x00000008  // FIFO Overrrun ack
#define ETH_MAC_IACK_RXERR          0x00000010  // Receive Error ack
#define ETH_MAC_IACK_MDINT          0x00000020  // MII Transaction Complete ack
#define ETH_MAC_IACK_PHYINT         0x00000040  // PHY Interrupt ack

/////////////////////////////////////////////////////////////////////////////////// ETH_MACIM
#define ETH_MAC_IM_RXINT            0x00000001  // Packet Received enable
#define ETH_MAC_IM_TXERR            0x00000002  // Transmit Error enable
#define ETH_MAC_IM_TXEMPTY          0x00000004  // Transmit FIFO Empty enable
#define ETH_MAC_IM_FOVRN            0x00000008  // FIFO Overrrun enable
#define ETH_MAC_IM_RXERR            0x00000010  // Receive Error enable
#define ETH_MAC_IM_MDINT            0x00000020  // MII Transaction Complete enable
#define ETH_MAC_IM_PHYINT           0x00000040  // PHY Interrupt enable

/////////////////////////////////////////////////////////////////////////////////// ETH_MACRCTL
#define ETH_MAC_RCTL_RXEN           0x00000001  // Enable Ethernet Receiver
#define ETH_MAC_RCTL_AMUL           0x00000002  // Enable Multicast Packets
#define ETH_MAC_RCTL_PRMS           0x00000004  // Enable Promiscuous Mode
#define ETH_MAC_RCTL_BADCRC         0x00000008  // Reject Packets With Bad CRC
#define ETH_MAC_RCTL_RSTFIFO        0x00000010  // Clear the Receive FIFO

/////////////////////////////////////////////////////////////////////////////////// ETH_MACTCTL
#define ETH_MAC_TCTL_TXEN           0x00000001  // Enable Ethernet Transmitter
#define ETH_MAC_TCTL_PADEN          0x00000002  // Enable Automatic Padding
#define ETH_MAC_TCTL_CRC            0x00000004  // Enable CRC Generation
#define ETH_MAC_TCTL_DUPLEX         0x00000010  // Enable Duplex mode

/////////////////////////////////////////////////////////////////////////////////// ETH_MACTHR
#define ETH_MAC_THR_THRMASK         0x0000003F  // Transmit Threshold Value

/////////////////////////////////////////////////////////////////////////////////// ETH_MACMCTL
#define ETH_MAC_MCTL_START          0x00000001  // MII regs transaction enable
#define ETH_MAC_MCTL_WRITE          0x00000002  // MII regs transaction type
#define ETH_MAC_MCTL_REGADR_MASK    0x000000F8  // Address for Next MII Transaction

/////////////////////////////////////////////////////////////////////////////////// ETH_MACMDV
// Fmdc = Fipclk / (2 * (ETH_MAC_MDV_DIV + 1 ))     max 2.5MHz
#define ETH_MAC_MDV_DIVMASK         0x000000FF  // Clock Divider for MDC for TX

/////////////////////////////////////////////////////////////////////////////////// ETH_MACMTXD
#define ETH_MAC_MTXD_MASK           0x0000FFFF

/////////////////////////////////////////////////////////////////////////////////// ETH_MACMRXD
#define ETH_MAC_MRXD_MASK           0x0000FFFF

/////////////////////////////////////////////////////////////////////////////////// ETH_MACTR
#define ETH_MAC_TR_NEWTX            0x00000001
///////////////////////////////////////////////////////////////////////////////////

//----- DATA TYPE DEFINITIONS -----

typedef struct _ETHERNET_HEADER
{
  MAC_ADDR    destination_mac_address;
  MAC_ADDR    source_mac_address;
  WORD_VAL    type;
} ETHERNET_HEADER;
#define ETHERNET_HEADER_LENGTH    14    //Can't use sizeof as some compilers will add pad bytes within the struct


#ifdef NIC_C
///////////////////////////////////////////////////////////////////////////////////
//----- INTERNAL FUNCTIONS -----
void nic_initialise      (BYTE init_config);
WORD nic_check_for_rx    (void);
void nic_setup_read_data (void);
BYTE nic_read_next_byte  (BYTE *data);
BYTE nic_read_array      (BYTE *array_buffer, WORD array_length);
void nic_move_pointer    (WORD move_pointer_to_ethernet_byte);
void nic_rx_dump_packet  (void);
BYTE nic_setup_tx        (void);
void write_eth_header_to_nic (MAC_ADDR *remote_mac_address, WORD ethernet_packet_type);
void nic_write_next_byte (BYTE data);
void nic_write_array     (BYTE *array_buffer, WORD array_length);
void nic_write_tx_word_at_location (WORD byte_address, WORD data);
void nix_tx_packet       (void);
BYTE nic_ok_to_do_tx     (void);
#else
///////////////////////////////////////////////////////////////////////////////////
//----- EXTERNAL FUNCTIONS -----
extern void nic_initialise      (BYTE init_config);
extern WORD nic_check_for_rx    (void);
extern void nic_setup_read_data (void);
extern BYTE nic_read_next_byte  (BYTE *data);
extern BYTE nic_read_array      (BYTE *array_buffer, WORD array_length);
extern void nic_move_pointer    (WORD move_pointer_to_ethernet_byte);
extern void nic_rx_dump_packet  (void);
extern BYTE nic_setup_tx        (void);
extern void write_eth_header_to_nic (MAC_ADDR *remote_mac_address, WORD ethernet_packet_type);
extern void nic_write_next_byte (BYTE data);
extern void nic_write_array     (BYTE *array_buffer, WORD array_length);
extern void nic_write_tx_word_at_location (WORD byte_address, WORD data);
extern void nix_tx_packet       (void);
extern BYTE nic_ok_to_do_tx     (void);

#endif


///////////////////////////////////////////////////////////////////////////////////
//********** MEMORY **********
#ifdef NIC_C
//--------------------------------------------
//----- INTERNAL ONLY MEMORY DEFINITIONS -----
//--------------------------------------------
DWORD nic_tx_start_buffer;
BYTE *nic_tx_buffer_next_byte;
DWORD nic_rx_start_buffer;
BYTE *nic_rx_buffer_next_byte;
BYTE nic_read_next_byte_get_word;
WORD nic_tx_next_byte_next_data_word;
WORD nic_rx_packet_total_ethernet_bytes;
BYTE auto_negotation_disabled;

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






#endif
