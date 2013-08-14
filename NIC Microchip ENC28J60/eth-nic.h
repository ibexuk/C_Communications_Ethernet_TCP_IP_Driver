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
//MICROCHIP ENC28J60 (NETWORK INTERFACE CONTROLLER) C CODE HEADER FILE




//##################################
//##################################
//########## USING DRIVER ##########
//##################################
//##################################

//Check this header file for defines to setup and any usage notes
//Configure the IO pins as requried in your applications initialisation.

//#########################
//##### SETUP SPI BUS #####
//#########################
//CLK needs to idle in low state
//The ENC28J60 clocks in data on the rising edge, and outputs data on the falling edge
//SPI bus clock frequency up to 20MHz (rising edge to next rising edge of clk)
/*
  For C18 compiler add this to the main application initialise:-
	//SETUP SPI BUS FOR COMMS TO ENC28J60
	SSP1STATbits.SMP = 1;
	SSP1STATbits.CKE = 1;		//Data is valid on the rising edge of the clock (Transmit occurs on transition from active to Idle clock state)
	SSP1CON1bits.CKP = 0;		//Clock low in idle bus state
	SSP1CON1bits.SSPM3 = 0;		//SPI in master mode and set bus frequency to highest possible <= 20MHz
	SSP1CON1bits.SSPM2 = 0;
	SSP1CON1bits.SSPM1 = 0;
	SSP1CON1bits.SSPM0 = 0;
	SSP1CON1bits.SSPEN = 1;		//Enable SSP Port


  For C30 compiler add this to the main application initialise:-
	//SETUP SPI BUS FOR COMMS TO ENC28J60
	w_temp = SPI1BUF;
	SPI1STAT = 0;
	SPI1CON1 = 0b0000001100111011;	//SPI in master mode (SPI1STATbits.SPIEN must be 0 to write to this register)
									//Data is valid on the rising edge of the clock (Transmit occurs on transition from active to Idle clock state)
									//Clock low in idle bus state
									//Prescallers 1:1 2:1 = 8MHz (Max frequency ENC28J60 will accept = 20MHz, but PIC24FJ128GA010 silicon bug limits max usable to 8MHz)
	SPI1STATbits.SPIEN = 1;			//Enable the port


  For C32 compiler add this to the main application initialise:-
	SpiChnOpen(1,						//SPI Channel
				(SPICON_ON | SPICON_MSTEN | SPICON_CKE | SPICON_SMP | SPICON_FRZ),	 //SPICON_CKP = idle high, SPICON_CKE = output changes on active to idle clock, SPICON_SMP = Input data sampled at end of data output time
				20);			//Fpb divisor to extract the baud rate: BR = Fpb / fpbDiv.  A value between 2 and 1024
								//Baud = 4 = 20MHz @ 80MHz bus speed (Max frequency ENC28J60 will accept = 20MHz)
*/


//#########################
//##### PROVIDE DELAY #####
//#########################
//Ensure the nic_delay_1ms() will provide a 1mS delay

//###########################
//##### OPERATION NOTES #####
//###########################
//Since the ENC28J60 doesn't support auto-negotiation, full-duplex mode is not compatible with most switches/routers.  If a dedicated network is used 
//where the duplex of the remote node can be manually configured, you may change this configuration.  Otherwise, half duplex should always be used.


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
//#define	NIC_USING_MICROCHIP_C18_COMPILER
//#define	NIC_USING_MICROCHIP_C30_COMPILER
#define	NIC_USING_MICROCHIP_C32_COMPILER
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
//SPI BUS CONTROL REGISTERS:-
//#define	NIC_SPI_BUF_FULL			SSP1STATbits.BF			//>0 when the SPI receive buffer contains a received byte, also signifying that transmit is complete
//#define	NIC_SPI_TX_BYTE(data)		SSP1BUF = data			//Macro to write a byte and start transmission over the SPI bus
//#define	NIC_SPI_RX_BYTE_BUFFER		SSP1BUF					//Register to read last received byte from
//##### Due to silicon bugs on some PIC18's the following alternative defines are needed as workarounds #####
#define	NIC_SPI_BUF_FULL			PIR1bits.SSPIF
#define	NIC_SPI_TX_BYTE(data)		PIR1bits.SSPIF = 0; SSP1BUF = data
#define	NIC_SPI_RX_BYTE_BUFFER		SSP1BUF

//CONTROL PINS:-
#define	NIC_CS(state)			LATDbits.LATD3 = state		//0 = select IC
#define	NIC_RESET_PIN(state)	LATDbits.LATD2 = state		//0 = reset IC

//INPUT PINS:-
#define	NIC_INT_PIN				PORTBbits.RB0				//This pin is used as a signal input rather than an interrupt, so does not need to be connected to
															//an interrupt input on the microcontroller / processor if processing of the stack will simply be
															//carried out constantly from the user application main loop

//---------------------------------------
//----- INITIALISATION DELAY DEFINE -----
//---------------------------------------
//Resetting the nic requires minimum setup time.  This define should cause a delay of at least 400nS
#define	NIC_DELAY_400NS			Nop(); Nop(); Nop(); Nop();		//('Nop();' is a single cycle null instruction for the C18 compiler, include multiple times if required)

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
//SPI BUS CONTROL REGISTERS:-
#define	NIC_SPI_BUF_FULL				(IFS0bits.SPI1IF && !_RF6) 		//>0 when the SPI receive buffer contains a received byte, also signifying that transmit is complete
																		//Checks clock pin for silicon bug workaround
#define	NIC_SPI_TX_BYTE(data)			IFS0bits.SPI1IF = 0; SPI1BUF = data			//Macro to write a byte and start transmission over the SPI bus
#define	NIC_SPI_RX_BYTE_BUFFER			SPI1BUF					//Register to read last received byte from

//CONTROL PINS:-
#define	NIC_CS(state)			_LATD14 = state		//0 = select IC
#define	NIC_RESET_PIN(state)	_LATD15 = state		//0 = reset IC

//INPUT PINS:-
#define	NIC_INT_PIN				_RE9						//This pin is used as a signal input rather than an interrupt, so does not need to be connected to
															//an interrupt input on the microcontroller / processor if processing of the stack will simply be
															//carried out constantly from the user application main loop

//---------------------------------------
//----- INITIALISATION DELAY DEFINE -----
//---------------------------------------
//Resetting the nic requires minimum setup time.  This define should cause a delay of at least 400nS
#define	NIC_DELAY_400NS			Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop();		//('Nop();' is a single cycle null instruction for the C30 compiler, include multiple times if required)


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
//SPI BUS CONTROL REGISTERS:-
#define	NIC_SPI_BUF_FULL				SpiChnGetRxIntFlag(1)		//>0 when the SPI receive buffer contains a received byte, also signifying that transmit is complete
#define	NIC_SPI_TX_BYTE(data)			SpiChnClrRxIntFlag(1); SpiChnPutC(1, data)			//Macro to write a byte and start transmission over the SPI bus
#define	NIC_SPI_RX_BYTE_BUFFER			SpiChnGetC(1)				//Macro to read last received byte from

//CONTROL PINS:-
#define	NIC_CS(state)			(state ? mPORTDSetBits(BIT_14) : mPORTDClearBits(BIT_14))		//0 = select IC
#define	NIC_RESET_PIN(state)	(state ? mPORTDSetBits(BIT_15) : mPORTDClearBits(BIT_15))		//0 = reset IC

//INPUT PINS:-
#define	NIC_INT_PIN				mPORTEReadBits(BIT_9)		//This pin is used as a signal input rather than an interrupt, so does not need to be connected to
															//an interrupt input on the microcontroller / processor if processing of the stack will simply be
															//carried out constantly from the user application main loop

//---------------------------------------
//----- INITIALISATION DELAY DEFINE -----
//---------------------------------------
//Resetting the nic requires minimum setup time.  This define should cause a delay of at least 400nS
#define	NIC_DELAY_400NS			Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop();
									//('Nop();' is a single cycle null instruction for the C32 compiler, include multiple times if required)

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

//NIC RAM DEFINITIONS
#define NIC_RAMSIZE				8192
#define NIC_TXSTART 			(NIC_RAMSIZE - (1 + 1514 + 7))
#define NIC_RXSTART				0									//Should be an even memory address; must be 0 for silicon errata workaround
#define	NIC_RXSTOP				((NIC_TXSTART - 2) | 0x0001)		//Odd for errata workaround
#define NIC_RXSIZE				(NIC_RXSTOP - NIC_RXSTART + 1)

#define NIC_BASE_TX_ADDR		(NIC_TXSTART + 1)


//----- NIC REGISTER DEFINITIONS -----
//- BANK 0 -
#define NIC_ERDPTL				0x0000
#define NIC_ERDPTH				0x0001
#define NIC_EWRPTL				0x0002
#define NIC_EWRPTH				0x0003
#define NIC_ETXSTL				0x0004
#define NIC_ETXSTH				0x0005
#define NIC_ETXNDL				0x0006
#define NIC_ETXNDH				0x0007
#define NIC_ERXSTL				0x0008
#define NIC_ERXSTH				0x0009
#define NIC_ERXNDL				0x000a
#define NIC_ERXNDH				0x000b
#define NIC_ERXRDPTL			0x000c
#define NIC_ERXRDPTH			0x000d
#define NIC_ERXWRPTL			0x000e
#define NIC_ERXWRPTH			0x000f
#define NIC_EDMASTL				0x0010
#define NIC_EDMASTH				0x0011
#define NIC_EDMANDL				0x0012
#define NIC_EDMANDH				0x0013
#define NIC_EDMADSTL			0x0014
#define NIC_EDMADSTH			0x0015
#define NIC_EDMACSL				0x0016
#define NIC_EDMACSH				0x0017
#define NIC_EIE					0x001b
#define NIC_EIR					0x001c
#define NIC_ESTAT				0x001d
#define NIC_ECON2				0x001e
#define NIC_ECON1				0x001f

//- BANK 1 -
#define NIC_EHT0				0x0100
#define NIC_EHT1				0x0101
#define NIC_EHT2				0x0102
#define NIC_EHT3				0x0103
#define NIC_EHT4				0x0104
#define NIC_EHT5				0x0105
#define NIC_EHT6				0x0106
#define NIC_EHT7				0x0107
#define NIC_EPMM0				0x0108
#define NIC_EPMM1				0x0109
#define NIC_EPMM2				0x010a
#define NIC_EPMM3				0x010b
#define NIC_EPMM4				0x010c
#define NIC_EPMM5				0x010d
#define NIC_EPMM6				0x010e
#define NIC_EPMM7				0x010f
#define NIC_EPMCSL				0x0110
#define NIC_EPMCSH				0x0111
#define NIC_EPMOL				0x0114
#define NIC_EPMOH				0x0115
#define NIC_ERXFCON				0x0118
#define NIC_EPKTCNT				0x0119
//#define NIC_EIE				0x011b
//#define NIC_EIR				0x011c
//#define NIC_ESTAT				0x011d
//#define NIC_ECON2				0x011e
//#define NIC_ECON1				0x011f

//- BANK 2 -
#define NIC_MACON1				0x0200
#define NIC_MACON3				0x0202
#define NIC_MACON4				0x0203
#define NIC_MABBIPG				0x0204
#define NIC_MAIPGL				0x0206
#define NIC_MAIPGH				0x0207
#define NIC_MACLCON1			0x0208
#define NIC_MACLCON2			0x0209
#define NIC_MAMXFLL				0x020a
#define NIC_MAMXFLH				0x020b
#define NIC_MICMD				0x0212
#define NIC_MIREGADR			0x0214
#define NIC_MIWRL				0x0216
#define NIC_MIWRH				0x0217
#define NIC_MIRDL				0x0218
#define NIC_MIRDH				0x0219
//#define NIC_EIE				0x021b
//#define NIC_EIR				0x021c
//#define NIC_ESTAT				0x021d
//#define NIC_ECON2				0x021e
//#define NIC_ECON1				0x021f

//- BANK 3 -
#define NIC_MAADR5				0x0300
#define NIC_MAADR6				0x0301
#define NIC_MAADR3				0x0302
#define NIC_MAADR4				0x0303
#define NIC_MAADR1				0x0304
#define NIC_MAADR2				0x0305
#define NIC_EBSTSD				0x0306
#define NIC_EBSTCON				0x0307
#define NIC_EBSTCSL				0x0308
#define NIC_EBSTCSH				0x0309
#define NIC_MISTAT				0x030a
#define NIC_EREVID				0x0312
#define NIC_ECOCON				0x0315
#define NIC_EFLOCON				0x0317
#define NIC_EPAUSL				0x0318
#define NIC_EPAUSH				0x0319
//#define NIC_EIE				0x031b
//#define NIC_EIR				0x031c
//#define NIC_ESTAT				0x031d
//#define NIC_ECON2				0x031e
//#define NIC_ECON1				0x031f


//----- OPCODES -----
//(ORed with a 5 bit address)
#define	NIC_WCR (0x2<<5)			//Write Control Register
#define NIC_BFS (0x4<<5)			//Bit Field Set
#define	NIC_BFC (0x5<<5)			//Bit Field Clear
#define	NIC_RCR (0x0<<5)			//Read Control Register
#define NIC_RBM ((0x1<<5) | 0x1a)	//Read Buffer Memory
#define	NIC_WBM ((0x3<<5) | 0x1a) 	//Write Buffer Memory
#define	NIC_SR  ((0x7<<5) | 0x1f)	//System Reset command does not use an address.  


//----- REGISTER STRUCTURES TYPEDEF -----
typedef union _NIC_REG 
{
	BYTE val;

	// EIE bits ----------
	struct {
		BYTE RXERIE:1;
		BYTE TXERIE:1;
		BYTE :1;
		BYTE TXIE:1;
		BYTE LINKIE:1;
		BYTE DMAIE:1;
		BYTE PKTIE:1;
		BYTE INTIE:1;
	} EIEbits;

	// EIR bits ----------
	struct {
		BYTE RXERIF:1;
		BYTE TXERIF:1;
		BYTE :1;
		BYTE TXIF:1;
		BYTE LINKIF:1;
		BYTE DMAIF:1;
		BYTE PKTIF:1;
		BYTE :1;
	} EIRbits;

	// ESTAT bits ---------
	struct {
		BYTE CLKRDY:1;
		BYTE TXABRT:1;
		BYTE RXBUSY:1;
		BYTE :1;
		BYTE LATECOL:1;
		BYTE :1;
		BYTE BUFER:1;
		BYTE INT:1;
	} ESTATbits;

	// ECON2 bits --------
	struct {
		BYTE :3;
		BYTE VRPS:1;
		BYTE :1;
		BYTE PWRSV:1;
		BYTE PKTDEC:1;
		BYTE AUTOINC:1;
	} ECON2bits;
		
	// ECON1 bits --------
	struct {
		BYTE BSEL0:1;
		BYTE BSEL1:1;
		BYTE RXEN:1;
		BYTE TXRTS:1;
		BYTE CSUMEN:1;
		BYTE DMAST:1;
		BYTE RXRST:1;
		BYTE TXRST:1;
	} ECON1bits;
		
	// ERXFCON bits ------
	struct {
		BYTE BCEN:1;
		BYTE MCEN:1;
		BYTE HTEN:1;
		BYTE MPEN:1;
		BYTE PMEN:1;
		BYTE CRCEN:1;
		BYTE ANDOR:1;
		BYTE UCEN:1;
	} ERXFCONbits;
		
	// MACON1 bits --------
	struct {
		BYTE MARXEN:1;
		BYTE PASSALL:1;
		BYTE RXPAUS:1;
		BYTE TXPAUS:1;
		BYTE :4;
	} MACON1bits;
		

	// MACON3 bits --------
	struct {
		BYTE FULDPX:1;
		BYTE FRMLNEN:1;
		BYTE HFRMEN:1;
		BYTE PHDREN:1;
		BYTE TXCRCEN:1;
		BYTE PADCFG0:1;
		BYTE PADCFG1:1;
		BYTE PADCFG2:1;
	} MACON3bits;
	struct {
		BYTE FULDPX:1;
		BYTE FRMLNEN:1;
		BYTE HFRMEN:1;
		BYTE PHDREN:1;
		BYTE TXCRCEN:1;
		BYTE PADCFG:3;
	} MACON3bits2;
		
	// MACON4 bits --------
	struct {
		BYTE :4;
		BYTE NOBKOFF:1;
		BYTE BPEN:1;
		BYTE DEFER:1;
		BYTE :1;
	} MACON4bits;
		
	// MICMD bits ---------
	struct {
		BYTE MIIRD:1;
		BYTE MIISCAN:1;
		BYTE :6;
	} MICMDbits;

	// EBSTCON bits -----
	struct {
		BYTE BISTST:1;
		BYTE TME:1;
		BYTE TMSEL0:1;
		BYTE TMSEL1:1;
		BYTE PSEL:1;
		BYTE PSV0:1;
		BYTE PSV1:1;
		BYTE PSV2:1;
	} EBSTCONbits;
	struct {
		BYTE BISTST:1;
		BYTE TME:1;
		BYTE TMSEL:2;
		BYTE PSEL:1;
		BYTE PSV:3;
	} EBSTCONbits2;
		
	// MISTAT bits --------
	struct {
		BYTE BUSY:1;
		BYTE SCAN:1;
		BYTE NVALID:1;
		BYTE :5;
	} MISTATbits;
		
	// ECOCON bits -------
	struct {
		BYTE COCON0:1;
		BYTE COCON1:1;
		BYTE COCON2:1;
		BYTE :5;
	} ECOCONbits;
	struct {
		BYTE COCON:3;
		BYTE :5;
	} ECOCONbits2;
		
	// EFLOCON bits -----
	struct {
		BYTE FCEN0:1;
		BYTE FCEN1:1;
		BYTE FULDPXS:1;
		BYTE :5;
	} EFLOCONbits;
	struct {
		BYTE FCEN:2;
		BYTE FULDPXS:1;
		BYTE :5;
	} EFLOCONbits2;
} NIC_REG;



//----- PH Register Locations -----
#define NIC_PHCON1	0x00
#define NIC_PHSTAT1	0x01
#define NIC_PHID1	0x02
#define NIC_PHID2	0x03
#define NIC_PHCON2	0x10
#define NIC_PHSTAT2	0x11
#define NIC_PHIE	0x12
#define NIC_PHIR	0x13
#define NIC_PHLCON	0x14


typedef union {
	WORD val;
	WORD_VAL VAL;

	//PHCON1 bits
	struct {
		WORD :8;
		WORD PDPXMD:1;
		WORD :2;
		WORD PPWRSV:1;
		WORD :2;
		WORD PLOOPBK:1;
		WORD PRST:1;
	} PHCON1bits;

	//PHSTAT1 bits
	struct {
		WORD :1;
		WORD JBSTAT:1;
		WORD LLSTAT:1;
		WORD :5;
		WORD :3;
		WORD PHDPX:1;
		WORD PFDPX:1;
		WORD :3;
	} PHSTAT1bits;

	//PHID2 bits
	struct {
		WORD PREV0:1;
		WORD PREV1:1;
		WORD PREV2:1;
		WORD PREV3:1;
		WORD PPN0:1;
		WORD PPN1:1;
		WORD PPN2:1;
		WORD PPN3:1;
		WORD PPN4:1;
		WORD PPN5:1;
		WORD PID19:1;
		WORD PID20:1;
		WORD PID21:1;
		WORD PID22:1;
		WORD PID23:1;
		WORD PID24:1;
	} PHID2bits;
	struct {
		WORD PREV:4;
		WORD PPNL:4;
		WORD PPNH:2;
		WORD PID:6;
	} PHID2bits2;

	//PHCON2 bits
	struct {
		WORD :8;
		WORD HDLDIS:1;
		WORD :1;
		WORD JABBER:1;
		WORD :2;
		WORD TXDIS:1;
		WORD FRCLNK:1;
		WORD :1;
	} PHCON2bits;

	//PHSTAT2 bits
	struct {
		WORD :5;
		WORD PLRITY:1;
		WORD :2;
		WORD :1;
		WORD DPXSTAT:1;
		WORD LSTAT:1;
		WORD COLSTAT:1;
		WORD RXSTAT:1;
		WORD TXSTAT:1;
		WORD :2;
	} PHSTAT2bits;

	//PHIE bits
	struct {
		WORD :1;
		WORD PGEIE:1;
		WORD :2;
		WORD PLNKIE:1;
		WORD :3;
		WORD :8;
	} PHIEbits;

	//PHIR bits
	struct {
		WORD :2;
		WORD PGIF:1;
		WORD :1;
		WORD PLNKIF:1;
		WORD :3;
		WORD :8;
	} PHIRbits;

	//PHLCON bits
	struct {
		WORD :1;
		WORD STRCH:1;
		WORD LFRQ0:1;
		WORD LFRQ1:1;
		WORD LBCFG0:1;
		WORD LBCFG1:1;
		WORD LBCFG2:1;
		WORD LBCFG3:1;
		WORD LACFG0:1;
		WORD LACFG1:1;
		WORD LACFG2:1;
		WORD LACFG3:1;
		WORD :4;
	} PHLCONbits;
	struct {
		WORD :1;
		WORD STRCH:1;
		WORD LFRQ:2;
		WORD LBCFG:4;
		WORD LACFG:4;
		WORD :4;
	} PHLCONbits2;
} PHYREG;


//----- Individual Register Bits -----
// ETH/MAC/MII bits

//EIE bits
#define	EIE_INTIE		(1<<7)
#define	EIE_PKTIE		(1<<6)
#define	EIE_DMAIE		(1<<5)
#define	EIE_LINKIE		(1<<4)
#define	EIE_TXIE		(1<<3)
#define	EIE_TXERIE		(1<<1)
#define	EIE_RXERIE		(1)

//EIR bits
#define	EIR_PKTIF		(1<<6)
#define	EIR_DMAIF		(1<<5)
#define	EIR_LINKIF		(1<<4)
#define	EIR_TXIF		(1<<3)
#define	EIR_TXERIF		(1<<1)
#define	EIR_RXERIF		(1)
	
//ESTAT bits
#define	ESTAT_INT		(1<<7)
#define ESTAT_BUFER		(1<<6)
#define	ESTAT_LATECOL	(1<<4)
#define	ESTAT_RXBUSY	(1<<2)
#define	ESTAT_TXABRT	(1<<1)
#define	ESTAT_CLKRDY	(1)
	
//ECON2 bits
#define	ECON2_AUTOINC	(1<<7)
#define	ECON2_PKTDEC	(1<<6)
#define	ECON2_PWRSV		(1<<5)
#define	ECON2_VRPS		(1<<3)
	
//ECON1 bits
#define	ECON1_TXRST		(1<<7)
#define	ECON1_RXRST		(1<<6)
#define	ECON1_DMAST		(1<<5)
#define	ECON1_CSUMEN	(1<<4)
#define	ECON1_TXRTS		(1<<3)
#define	ECON1_RXEN		(1<<2)
#define	ECON1_BSEL1		(1<<1)
#define	ECON1_BSEL0		(1)
	
//ERXFCON bits
#define	ERXFCON_UCEN	(1<<7)
#define	ERXFCON_ANDOR	(1<<6)
#define	ERXFCON_CRCEN	(1<<5)
#define	ERXFCON_PMEN	(1<<4)
#define	ERXFCON_MPEN	(1<<3)
#define	ERXFCON_HTEN	(1<<2)
#define	ERXFCON_MCEN	(1<<1)
#define	ERXFCON_BCEN	(1)
	
//MACON1 bits
#define	MACON1_TXPAUS	(1<<3)
#define	MACON1_RXPAUS	(1<<2)
#define	MACON1_PASSALL	(1<<1)
#define	MACON1_MARXEN	(1)
	
//MACON3 bits
#define	MACON3_PADCFG2	(1<<7)
#define	MACON3_PADCFG1	(1<<6)
#define	MACON3_PADCFG0	(1<<5)
#define	MACON3_TXCRCEN	(1<<4)
#define	MACON3_PHDREN	(1<<3)
#define	MACON3_HFRMEN	(1<<2)
#define	MACON3_FRMLNEN	(1<<1)
#define	MACON3_FULDPX	(1)
	
//MACON4 bits
#define	MACON4_DEFER	(1<<6)
#define	MACON4_BPEN		(1<<5)
#define	MACON4_NOBKOFF	(1<<4)
	
//MICMD bits
#define	MICMD_MIISCAN	(1<<1)
#define	MICMD_MIIRD		(1)

//EBSTCON bits
#define	EBSTCON_PSV2	(1<<7)
#define	EBSTCON_PSV1	(1<<6)
#define	EBSTCON_PSV0	(1<<5)
#define	EBSTCON_PSEL	(1<<4)
#define	EBSTCON_TMSEL1	(1<<3)
#define	EBSTCON_TMSEL0	(1<<2)
#define	EBSTCON_TME		(1<<1)
#define	EBSTCON_BISTST	(1)

//MISTAT bits
#define	MISTAT_NVALID	(1<<2)
#define	MISTAT_SCAN		(1<<1)
#define	MISTAT_BUSY		(1)
	
//ECOCON bits
#define	ECOCON_COCON2	(1<<2)
#define	ECOCON_COCON1	(1<<1)
#define	ECOCON_COCON0	(1)
	
//EFLOCON bits
#define	EFLOCON_FULDPXS	(1<<2)
#define	EFLOCON_FCEN1	(1<<1)
#define	EFLOCON_FCEN0	(1)


//----- PHY bits -----
//PHCON1 bits
#define	PHCON1_PRST		((WORD)1<<15)
#define	PHCON1_PLOOPBK	((WORD)1<<14)
#define	PHCON1_PPWRSV	((WORD)1<<11)
#define	PHCON1_PDPXMD	((WORD)1<<8)

//PHSTAT1 bits
#define	PHSTAT1_PFDPX	((WORD)1<<12)
#define	PHSTAT1_PHDPX	((WORD)1<<11)
#define	PHSTAT1_LLSTAT	((WORD)1<<2)
#define	PHSTAT1_JBSTAT	((WORD)1<<1)

//PHID2 bits
#define	PHID2_PID24		((WORD)1<<15)
#define	PHID2_PID23		((WORD)1<<14)
#define	PHID2_PID22		((WORD)1<<13)
#define	PHID2_PID21		((WORD)1<<12)
#define	PHID2_PID20		((WORD)1<<11)
#define	PHID2_PID19		((WORD)1<<10)
#define	PHID2_PPN5		((WORD)1<<9)
#define	PHID2_PPN4		((WORD)1<<8)
#define	PHID2_PPN3		((WORD)1<<7)
#define	PHID2_PPN2		((WORD)1<<6)
#define	PHID2_PPN1		((WORD)1<<5)
#define	PHID2_PPN0		((WORD)1<<4)
#define	PHID2_PREV3		((WORD)1<<3)
#define	PHID2_PREV2		((WORD)1<<2)
#define	PHID2_PREV1		((WORD)1<<1)
#define	PHID2_PREV0		((WORD)1)

//PHCON2 bits
#define	PHCON2_FRCLNK	((WORD)1<<14)
#define	PHCON2_TXDIS	((WORD)1<<13)
#define	PHCON2_JABBER	((WORD)1<<10)
#define	PHCON2_HDLDIS	((WORD)1<<8)

//PHSTAT2 bits
#define	PHSTAT2_TXSTAT	((WORD)1<<13)
#define	PHSTAT2_RXSTAT	((WORD)1<<12)
#define	PHSTAT2_COLSTAT	((WORD)1<<11)
#define	PHSTAT2_LSTAT	((WORD)1<<10)
#define	PHSTAT2_DPXSTAT	((WORD)1<<9)
#define	PHSTAT2_PLRITY	((WORD)1<<5)

//PHIE bits
#define	PHIE_PLNKIE		((WORD)1<<4)
#define	PHIE_PGEIE		((WORD)1<<1)

//PHIR bits
#define	PHIR_PLNKIF		((WORD)1<<4)
#define	PHIR_PGIF		((WORD)1<<2)

//PHLCON bits
#define	PHLCON_LACFG3	((WORD)1<<11)
#define	PHLCON_LACFG2	((WORD)1<<10)
#define	PHLCON_LACFG1	((WORD)1<<9)
#define	PHLCON_LACFG0	((WORD)1<<8)
#define	PHLCON_LBCFG3	((WORD)1<<7)
#define	PHLCON_LBCFG2	((WORD)1<<6)
#define	PHLCON_LBCFG1	((WORD)1<<5)
#define	PHLCON_LBCFG0	((WORD)1<<4)
#define	PHLCON_LFRQ1	((WORD)1<<3)
#define	PHLCON_LFRQ0	((WORD)1<<2)
#define	PHLCON_STRCH	((WORD)1<<1)


typedef union {
	BYTE v[7];
	struct {
		WORD	 		ByteCount;
		BYTE	CollisionCount:4;
		BYTE	CRCError:1;
		BYTE	LengthCheckError:1;
		BYTE	LengthOutOfRange:1;
		BYTE	Done:1;
		BYTE	Multicast:1;
		BYTE	Broadcast:1;
		BYTE	PacketDefer:1;
		BYTE	ExcessiveDefer:1;
		BYTE	MaximumCollisions:1;
		BYTE	LateCollision:1;
		BYTE	Giant:1;
		BYTE	Underrun:1;
		WORD 	 		BytesTransmittedOnWire;
		BYTE	ControlFrame:1;
		BYTE	PAUSEControlFrame:1;
		BYTE	BackpressureApplied:1;
		BYTE	VLANTaggedFrame:1;
		BYTE	Zeros:4;
	} bits;
} TXSTATUS;

typedef union {
	BYTE v[4];
	struct {
		WORD	 		ByteCount;
		BYTE	PreviouslyIgnored:1;
		BYTE	RXDCPreviouslySeen:1;
		BYTE	CarrierPreviouslySeen:1;
		BYTE	CodeViolation:1;
		BYTE	CRCError:1;
		BYTE	LengthCheckError:1;
		BYTE	LengthOutOfRange:1;
		BYTE	ReceiveOk:1;
		BYTE	Multicast:1;
		BYTE	Broadcast:1;
		BYTE	DribbleNibble:1;
		BYTE	ControlFrame:1;
		BYTE	PauseControlFrame:1;
		BYTE	UnsupportedOpcode:1;
		BYTE	VLANType:1;
		BYTE	Zero:1;
	} bits;
} RXSTATUS;


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


typedef struct _ENC_PREAMBLE
{
    WORD			next_packet_pointer;
    RXSTATUS		status_vector;
} ENC_PREAMBLE;


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
void nic_delay_1ms (void);
void nic_reset (void);
static void nic_select_bank (WORD register);
static NIC_REG nic_read (BYTE address);
static void nic_write (BYTE address, BYTE data);
void nic_write_buffer(BYTE data);
static NIC_REG nic_read_mac_mii_register (BYTE address);
static void nic_bit_field_clear_register (BYTE address, BYTE data);
static void nic_bit_field_set_register (BYTE address, BYTE data);
PHYREG nic_read_phy_register(BYTE reg);
void nic_write_phy_register (BYTE register, WORD data);


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
WORD_VAL nic_next_packet_location;
WORD_VAL nic_current_packet_location;
BYTE enc28j60_revision_id;
WORD nic_rx_packet_total_ethernet_bytes;


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








