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
//PIC32 SAMPLE PROJECT C CODE FILE





//----- INCLUDE FILES FOR THIS SOURCE CODE FILE -----
#include "main.h"					//Global data type definitions (see https://github.com/ibexuk/C_Generic_Header_File )
#define	MAIN_C						//(Define used for following header file to flag that it is the header file for this source file)
#include "ap-main.h"				//(Include header file for this source file)
#undef MAIN_C

//----- OTHER PROJECT FILES REQUIRED BY THIS SOURCE CODE FILE -----
#include "eth-main.h"
#include "eth-nic.h"
#include "eth-dhcp.h"
#include "eth-arp.h"
#include "eth-udp.h"
#include "eth-tcp.h"
#include "eth-pop3.h"
#include "eth-smtp.h"
#include "eth-netbios.h"
#include "eth-sntp.h"

//----- COMPILER LIBRARY FILES REQUIRED BY THIS SOURCE CODE FILE -----






//******************************************
//******************************************
//********** DEVICE CONFIGURATION **********
//******************************************
//******************************************
//These configuration defines do not need to be included, but having them means that the configuration bits will be automatically set and will
//be included in the .hex file created for the project, so that they do not need to be manually set when programming devices at manufacture.
//(The config names are given for each device in MPLAB help, under 'Topics... -> PIC32MX Configuration Settings')

#ifdef __DEBUG				//Debug mode selected in MPLAB
//----- WE'RE IN DEVELOPMENT MODE -----
	//--- PLL Input Divider bits --
	//DIV_1 = Divide by 1, DIV_2 = Divide by 2, DIV_3 = Divide by 3, DIV_4 = Divide by 4, DIV_5 = Divide by 5, DIV_6 = Divide by 6, DIV_10 = Divide by 10, DIV_12 = Divide by 12  
	#pragma config FPLLIDIV = DIV_2


	//--- PLL Output Divider Value ---
	//DIV_1 = Divide by 1, DIV_2 = Divide by 2, DIV_4 = Divide by 4, DIV_8 = Divide by 8, DIV_16 = Divide by 16, DIV_32 = Divide by 32 , DIV_64 = Divide by 64, DIV_256 = Divide by 256  
	#pragma config FPLLODIV = DIV_1

	//--- PLL Multiplier bits ---
	//MUL_15 = Multiply by 15, MUL_16 = Multiply by 16, MUL_17 = Multiply by 17, MUL_18 = Multiply by 18, MUL_19 = Multiply by 19, MUL_20 = Multiply by 20, MUL_21 = Multiply by 21, MUL_24 = Multiply by 24  
	#pragma config FPLLMUL = MUL_20

	//--- Watchdog Timer Enable bit ---
	//OFF = Disabled, ON = Enabled
	#pragma config FWDTEN = ON

	//--- Watchdog Timer Postscale Select bits ---
	//PS1 = 1:1, PS2 = 1:2, PS4 = 1:4, PS8 = 1, PS16 = 1:16, PS32 = 1:32, PS64 = 1:64,, PS128 = 1:128, PS256 = 1:256, PS512 = 1:512, PS1024 = 1:1024, PS2048 = 1:2048,
	//PS4096 = 1:4096, PS8192 = 1:8,192, PS16384 = 1:16,384, PS32768 = 1:32,768, PS65536 = 1:65,536, PS131072 = 1:131,072, PS262144 = 1:262,144, PS524288 = 1:524,288, PS1048576 = 1:1,048,576  
	#pragma config WDTPS = PS32768

	//--- Clock Switching and Monitor Selection bits ---
	//CSECME = Clock Switching Enabled, Clock Monitoring Enabled, CSECMD = Clock Switching Enabled, Clock Monitoring Disabled, CSDCMD = Clock Switching Disabled, Clock Monitoring Disabled
	#pragma config FCKSM = CSDCMD

	//--- Bootup PBCLK divider ---
	//DIV_1 = Divide by 1, DIV_2 = Divide by 2, DIV_4 = Divide by 4, DIV_8 = Divide by 8
	#pragma config FPBDIV = DIV_2

	//--- CLKO Enable bit ---
	//OFF  = Disabled, ON = Enabled
	#pragma config OSCIOFNC = OFF

	//--- Primary Oscillator bits ---
	//EC = EC oscillator, XT = XT oscillator, HS = HS oscillator, OFF = Disabled  
	#pragma config POSCMOD = HS

	//--- Internal External Switch Over bit ---
	//OFF = Disabled, ON = Enabled  
	#pragma config IESO = OFF

	//--- Secondary oscillator Enable bit ---
	//OFF = Disabled, ON = Enabled
	#pragma config FSOSCEN = OFF

	//--- Oscillator Selection bits ---
	//FRC = Fast RC oscillator, FRCPLL = Fast RC oscillator w/ PLL, PRI = Primary oscillator (XT, HS, EC), PRIPLL = Primary oscillator (XT, HS, EC) w/ PLL, 
	//SOSC = Secondary oscillator, LPRC = Low power RC oscillator, FRCDIV16 = Fast RC oscillator with divide by 16, FRCDIV = Fast RC oscillator with divide
	#pragma config FNOSC = PRIPLL

	//--- Code Protect Enable bit ---
	//ON = Enabled, OFF = Disabled  
	#pragma config CP = OFF

	//--- Boot Flash Write Protect bit ---
	//ON = Enabled, OFF = Disabled
	#pragma config BWP = OFF

	//--- Program Flash Write Protect bit ---
	//OFF = Disabled  
	#pragma config PWP = OFF

	//--- ICE/ICD Comm Channel Select ---
	//ICS_PGx1 = ICE pins are shared with PGC1, PGD1, ICS_PGx2 = ICE pins are shared with PGC2, PGD2, 
	#pragma config ICESEL = ICS_PGx2

	//Background Debugger Enable bit:-
	//ON = Enabled , OFF = Disabled  
	#pragma config DEBUG = ON
	
#else
//----- WE'RE NOT IN DEVELOPMENT MODE -----
	//--- PLL Input Divider bits --
	//DIV_1 = Divide by 1, DIV_2 = Divide by 2, DIV_3 = Divide by 3, DIV_4 = Divide by 4, DIV_5 = Divide by 5, DIV_6 = Divide by 6, DIV_10 = Divide by 10, DIV_12 = Divide by 12  
	#pragma config FPLLIDIV = DIV_2


	//--- PLL Output Divider Value ---
	//DIV_1 = Divide by 1, DIV_2 = Divide by 2, DIV_4 = Divide by 4, DIV_8 = Divide by 8, DIV_16 = Divide by 16, DIV_32 = Divide by 32 , DIV_64 = Divide by 64, DIV_256 = Divide by 256  
	#pragma config FPLLODIV = DIV_1

	//--- PLL Multiplier bits ---
	//MUL_15 = Multiply by 15, MUL_16 = Multiply by 16, MUL_17 = Multiply by 17, MUL_18 = Multiply by 18, MUL_19 = Multiply by 19, MUL_20 = Multiply by 20, MUL_21 = Multiply by 21, MUL_24 = Multiply by 24  
	#pragma config FPLLMUL MUL_20

	//--- Watchdog Timer Enable bit ---
	//OFF = Disabled, ON = Enabled
	#pragma config FWDTEN = ON

	//--- Watchdog Timer Postscale Select bits ---
	//PS1 = 1:1, PS2 = 1:2, PS4 = 1:4, PS8 = 1, PS16 = 1:16, PS32 = 1:32, PS64 = 1:64,, PS128 = 1:128, PS256 = 1:256, PS512 = 1:512, PS1024 = 1:1024, PS2048 = 1:2048,
	//PS4096 = 1:4096, PS8192 = 1:8,192, PS16384 = 1:16,384, PS32768 = 1:32,768, PS65536 = 1:65,536, PS131072 = 1:131,072, PS262144 = 1:262,144, PS524288 = 1:524,288, PS1048576 = 1:1,048,576  
	#pragma config WDTPS = PS32768

	//--- Clock Switching and Monitor Selection bits ---
	//CSECME = Clock Switching Enabled, Clock Monitoring Enabled, CSECMD = Clock Switching Enabled, Clock Monitoring Disabled, CSDCMD = Clock Switching Disabled, Clock Monitoring Disabled
	#pragma config FCKSM = CSDCMD

	//--- Bootup PBCLK divider ---
	//DIV_1 = Divide by 1, DIV_2 = Divide by 2, DIV_4 = Divide by 4, DIV_8 = Divide by 8
	#pragma config FPBDIV = DIV_2

	//--- CLKO Enable bit ---
	//OFF  = Disabled, ON = Enabled
	#pragma config OSCIOFNC = OFF

	//--- Primary Oscillator bits ---
	//EC = EC oscillator, XT = XT oscillator, HS = HS oscillator, OFF = Disabled  
	#pragma config POSCMOD = HS

	//--- Internal External Switch Over bit ---
	//OFF = Disabled, ON = Enabled  
	#pragma config IESO = OFF

	//--- Secondary oscillator Enable bit ---
	//OFF = Disabled, ON = Enabled
	#pragma config FSOSCEN OFF

	//--- Oscillator Selection bits ---
	//FRC = Fast RC oscillator, FRCPLL = Fast RC oscillator w/ PLL, PRI = Primary oscillator (XT, HS, EC), PRIPLL = Primary oscillator (XT, HS, EC) w/ PLL, 
	//SOSC = Secondary oscillator, LPRC = Low power RC oscillator, FRCDIV16 = Fast RC oscillator with divide by 16, FRCDIV = Fast RC oscillator with divide
	#pragma config FNOSC = PRIPLL

	//--- Code Protect Enable bit ---
	//ON = Enabled, OFF = Disabled  
	#pragma config CP = OFF

	//--- Boot Flash Write Protect bit ---
	//ON = Enabled, OFF = Disabled
	#pragma config BWP = OFF

	//--- Program Flash Write Protect bit ---
	//OFF = Disabled  
	#pragma config PWP = OFF

	//--- ICE/ICD Comm Channel Select ---
	//ICS_PGx1 = ICE pins are shared with PGC1, PGD1, ICS_PGx2 = ICE pins are shared with PGC2, PGD2, 
	#pragma config ICESEL = ICS_PGx2

	//Background Debugger Enable bit:-
	//ON = Enabled , OFF = Disabled  
	#pragma config DEBUG = OFF
#endif










//*************************************************************************************************************************************
//*************************************************************************************************************************************
//*************************************************************************************************************************************
//*************************************************************************************************************************************
//*************************************************************************************************************************************
//*************************************************************************************************************************************
//*************************************************************************************************************************************

//********************************
//********************************
//********** INITIALISE **********
//********************************
//********************************
void initialise (void)
{
	CONSTANT BYTE *p_string_source;
	BYTE *p_string_dest;
	WORD w_temp;

	//##### GENERAL NOTE ABOUT PIC32'S #####
	//Try and use the peripheral libraries instead of special function registers for everything (literally everything!) to avoid
	//bugs that can be caused by the pipeline and interrupts.
	

	//---------------------------------
	//----- CONFIGURE PERFORMANCE -----
	//---------------------------------
	
	//----- SETUP EVERYTHING FOR OPTIMUM PERFORMANCE -----
	SYSTEMConfigPerformance (80000000);		//Note this sets peripheral bus to '1' max speed (regardless of configuration bit setting)
											//Use PBCLK divider of 1:1 to calculate UART baud, timer tick etc

	//----- SET PERIPHERAL BUS DIVISOR -----
	//To minimize dynamic power the PB divisor should be chosen to run the peripherals at the lowest frequency that provides acceptable system performance
	//mOSCSetPBDIV(OSC_PB_DIV_1);			//OSC_PB_DIV_1, OSC_PB_DIV_2, OSC_PB_DIV_4, OSC_PB_DIV_8, 



	//-------------------------
	//----- SETUP IO PINS -----
	//-------------------------
	//(Device will powerup with all IO pins as inputs)

	//----- TURN OFF THE JTAG PORT -----
	//(JTAG is on by default)
	mJTAGPortEnable(0);

	#define	PORTA_IO	0x0080				//Setup the IO pin type (0 = output, 1 = input)
	mPORTAWrite(0x0000);					//Set initial ouput pin states
	mPORTASetPinsDigitalIn(PORTA_IO);		//(Sets high bits as input)
	mPORTASetPinsDigitalOut(~PORTA_IO);		//(Sets high bits as output)

	#define	PORTB_IO	0x00C0				//Setup the IO pin type (0 = output, 1 = input)
	mPORTBWrite(0x3000);					//Set initial ouput pin states - RB13:12 high for Explorer 16 signals select
	mPORTBSetPinsDigitalIn(PORTB_IO);		//(Sets high bits as input)
	mPORTBSetPinsDigitalOut(~PORTB_IO);		//(Sets high bits as output)

	#define	PORTC_IO	0x0000				//Setup the IO pin type (0 = output, 1 = input)
	mPORTCWrite(0x0000);					//Set initial ouput pin states
	mPORTCSetPinsDigitalIn(PORTC_IO);		//(Sets high bits as input)
	mPORTCSetPinsDigitalOut(~PORTC_IO);		//(Sets high bits as output)

	#define	PORTD_IO	0x2080				//Setup the IO pin type (0 = output, 1 = input)
	mPORTDWrite(0x4000);					//Set initial ouput pin states
	mPORTDSetPinsDigitalIn(PORTD_IO);		//(Sets high bits as input)
	mPORTDSetPinsDigitalOut(~PORTD_IO);		//(Sets high bits as output)

	#define	PORTE_IO	0x0200				//Setup the IO pin type (0 = output, 1 = input)
	mPORTEWrite(0x0000);					//Set initial ouput pin states
	mPORTESetPinsDigitalIn(PORTE_IO);		//(Sets high bits as input)
	mPORTESetPinsDigitalOut(~PORTE_IO);		//(Sets high bits as output)

	#define	PORTF_IO	0x0080				//Setup the IO pin type (0 = output, 1 = input)
	mPORTFWrite(0x0000);					//Set initial ouput pin states
	mPORTFSetPinsDigitalIn(PORTF_IO);		//(Sets high bits as input)
	mPORTFSetPinsDigitalOut(~PORTF_IO);		//(Sets high bits as output)

	#define	PORTG_IO	0x0000				//Setup the IO pin type (0 = output, 1 = input)
	mPORTGWrite(0x0000);					//Set initial ouput pin states
	mPORTGSetPinsDigitalIn(PORTG_IO);		//(Sets high bits as input)
	mPORTGSetPinsDigitalOut(~PORTG_IO);		//(Sets high bits as output)


	//Read pins using:
	// mPORTAReadBits(BIT_0);
	//Write pins using:
	// mPORTAClearBits(BIT_0);
	// mPORTASetBits(BIT_0);
	// mPORTAToggleBits(BIT_0);

	//----- INPUT CHANGE NOTIFICATION CONFIGURATION -----
	//EnableCN0();
	//ConfigCNPullups(CN10_PULLUP_ENABLE | CN11_PULLUP_ENABLE);


	//----- SETUP THE A TO D PINS -----
	//mPORTBSetPinsAnalogIn(BIT_0 | BIT_1);





	//------------------------
	//----- SETUP TIMERS -----
	//------------------------
	//(INCLUDE THE USAGE OF ALL TIMERS HERE EVEN IF NOT SETUP HERE SO THIS IS THE ONE POINT OF
	//REFERENCE TO KNOW WHICH TIMERS ARE IN USE AND FOR WHAT).

	//----- SETUP TIMER 1 -----
	//Used for: Available
	//OpenTimer1((T1_ON | T1_IDLE_CON | T1_GATE_OFF | T1_PS_1_4 | T1_SOURCE_INT), 20000);

	//----- SETUP TIMER 2 -----
	//Used for: Available
	//OpenTimer2((T2_ON | T2_IDLE_CON | T2_GATE_OFF | T2_PS_1_4 | T2_SOURCE_INT), 20000);

	//----- SETUP TIMER 3 -----
	//Used for: Available
	//OpenTimer3((T3_ON | T3_IDLE_CON | T3_GATE_OFF | T3_PS_1_4 | T3_SOURCE_INT), 20000);

	//----- SETUP TIMER 4 -----
	//Used for: Available
	//OpenTimer4((T4_ON | T4_IDLE_CON | T4_GATE_OFF | T4_PS_1_4 | T4_SOURCE_INT), 20000);

	//----- SETUP TIMER 5 -----
	//Used for: Heartbeat
	OpenTimer5((T5_ON | T5_IDLE_CON | T5_GATE_OFF | T5_PS_1_4 | T5_SOURCE_INT), 20000);
	ConfigIntTimer5(T5_INT_ON | T5_INT_PRIOR_7);


	//------------------------------------------
	//----- SETUP SYNCHRONOUS SERIAL PORTS -----
	//------------------------------------------

	//----- SETUP SPI 1 -----
	//Used for: ENC28J60
	SpiChnOpen(1,						//SPI Channel
				(SPICON_ON | SPICON_MSTEN | SPICON_CKE | SPICON_SMP | SPICON_FRZ),	 //SPICON_CKP = idle high, SPICON_CKE = output changes on active to idle clock, SPICON_SMP = Input data sampled at end of data output time
				20);			//Fpb divisor to extract the baud rate: BR = Fpb / fpbDiv.  A value between 2 and 1024
								//Baud = 4 = 20MHz @ 80MHz bus speed (Max frequency ENC28J60 will accept = 20MHz)

	//----- SETUP SPI 2 -----
	//Used for: Disabled


	//----- SETUP I2C 1 -----
	//Used for: Disabled


	//----- SETUP I2C 2 -----
	//Used for: Disabled



	//-----------------------
	//----- SETUP USART -----
	//-----------------------

	//----- SETUP UART 1 -----
	//Used for: Disabled


	//----- SETUP UART 2 -----
	//Used for: Disabled





	//-----------------------------
	//----- ENABLE INTERRUPTS -----
	//-----------------------------
	//INTEnableSystemSingleVectoredInt();
	INTEnableSystemMultiVectoredInt();






	//------------------------------
	//----- CONFIGURE ETHERNET -----
	//------------------------------
	//----- TO USE A MANUALLY CONFIGURED IP SETTINGS -----
	/*
	eth_dhcp_using_manual_settings = 1;
	our_ip_address.v[0] = 192;			//MSB
	our_ip_address.v[1] = 168;
	our_ip_address.v[2] = 0;
	our_ip_address.v[3] = 51;			//LSB
	our_subnet_mask.v[0] = 255;			//MSB
	our_subnet_mask.v[1] = 255;
	our_subnet_mask.v[2] = 255;
	our_subnet_mask.v[3] = 0;			//LSB
	our_gateway_ip_address.v[0] = 192;
	our_gateway_ip_address.v[1] = 168;
	our_gateway_ip_address.v[2] = 0;
	our_gateway_ip_address.v[3] = 1;
	*/

	//----- TO USE DHCP CONFIGURED IP SETTINGS -----
	eth_dhcp_using_manual_settings = 0;

	//----- SET OUR ETHENET UNIQUE MAC ADDRESS -----
	our_mac_address.v[0] = 0x00;		//MSB	(This is a generic address - replace with your own globally unique MAC address for release products)
	our_mac_address.v[1] = 0x50;
	our_mac_address.v[2] = 0xC2;
	our_mac_address.v[3] = 0x50;
	our_mac_address.v[4] = 0x10;
	our_mac_address.v[5] = 0x32;		//LSB

	//----- INITIALISE ETHERNET -----
	tcp_ip_initialise();

	//----- SET OUR NETBIOS NAME -----
	netbios_our_network_name[0] = 'e';		//(16 byte null terminated array (put a 0x00 after the last character))
	netbios_our_network_name[1] = 'm';		//Not case sensitive
	netbios_our_network_name[2] = 'b';
	netbios_our_network_name[3] = 'e';
	netbios_our_network_name[4] = 'd';
	netbios_our_network_name[5] = 'd';
	netbios_our_network_name[6] = 'e';
	netbios_our_network_name[7] = 'd';
	netbios_our_network_name[8] = '-';
	netbios_our_network_name[9] = 'd';
	netbios_our_network_name[10] = 'e';
	netbios_our_network_name[11] = 'v';
	netbios_our_network_name[12] = 'i';
	netbios_our_network_name[13] = 'c';
	netbios_our_network_name[14] = 'e';
	netbios_our_network_name[15] = 0x00;

	//----- SETUP DEMO APPLICATION DEFAULT STRINGS -----
	//Default POP3 Server
	p_string_source = default_pop3_server;
	p_string_dest = our_pop3_server;
	do 
		*p_string_dest++ = *p_string_source;
	while (*p_string_source++ != 0x00);

	//Default POP3 Account Username
	p_string_source = default_pop3_username;
	p_string_dest = our_pop3_username;
	do 
		*p_string_dest++ = *p_string_source;
	while (*p_string_source++ != 0x00);

	//Default POP3 Account Password
	p_string_source = default_pop3_password;
	p_string_dest = our_pop3_password;
	do 
		*p_string_dest++ = *p_string_source;
	while (*p_string_source++ != 0x00);

	//Default SMTP Server
	p_string_source = default_smtp_server;
	p_string_dest = our_smtp_server;
	do 
		*p_string_dest++ = *p_string_source;
	while (*p_string_source++ != 0x00);

	//Default SMTP Account Username
	p_string_source = default_smtp_username;
	p_string_dest = our_smtp_username;
	do 
		*p_string_dest++ = *p_string_source;
	while (*p_string_source++ != 0x00);

	//Default SMTP Account Password
	p_string_source = default_smtp_password;
	p_string_dest = our_smtp_password;
	do 
		*p_string_dest++ = *p_string_source;
	while (*p_string_source++ != 0x00);

	//Default SMTP Email To Address
	p_string_source = default_smtp_to;
	p_string_dest = our_smtp_to;
	do 
		*p_string_dest++ = *p_string_source;
	while (*p_string_source++ != 0x00);

	//Default SMTP Email From address
	p_string_source = default_smtp_from;
	p_string_dest = our_smtp_from;
	do 
		*p_string_dest++ = *p_string_source;
	while (*p_string_source++ != 0x00);

	//Default SMTP Email Subject
	p_string_source = default_smtp_subject;
	p_string_dest = our_smtp_subject;
	do 
		*p_string_dest++ = *p_string_source;
	while (*p_string_source++ != 0x00);


}






//**********************************************************************************************************************************************
//**********************************************************************************************************************************************
//**********************************************************************************************************************************************
//**********************************************************************************************************************************************
//**********************************************************************************************************************************************
//**********************************************************************************************************************************************
//**********************************************************************************************************************************************

//***********************************
//***********************************
//********** MAIN FUNCTION **********
//***********************************
//***********************************
int main (void)
{


	//**********************
	//**********************
	//***** INITIALISE *****
	//**********************
	//**********************
	initialise();





	//*********************
	//*********************
	//***** MAIN LOOP *****
	//*********************
	//*********************
	while(1)						//(Do forever)
	{
		//----- RESET THE WATCHDOG TIMEOUT TIMER -----
		ClearWDT();

		//----- READ SWITCHES -----
		read_switches();

		//----- PROCESS MODE -----
		process_user_mode();

		//----- PROCESS ETHERNET STACK -----
		tcp_ip_process_stack();





		
	}
}







//***********************************
//***********************************
//********** READ SWITCHES **********
//***********************************
//***********************************
void read_switches (void)
{
	BYTE switches_read;
	static BYTE switches_last = 0;
	static BYTE switches_debounced_last = 0;


	//RESET THE NEW SWITCH PRESS REGISTER
	switches_new = 0;

	//ONLY DO EVERY 10MS FOR SWITCH CONTACT DEBOUNCE
	if (read_switches_flag == 0)
		return;

	read_switches_flag = 0;

	//GET THE SWITCH INPUTS
	switches_read = 0;
	if (!mPORTDReadBits(BIT_13))
		switches_read |= 0x01;
	if (!mPORTAReadBits(BIT_7))
		switches_read |= 0x02;
	if (!mPORTDReadBits(BIT_7))
		switches_read |= 0x04;

	//DEBOUNCE
	switches_debounced = switches_last & switches_read;

	//FLAG NEW BUTTON PRESSES
	switches_new = switches_debounced ^ switches_debounced_last;
	switches_new &= switches_debounced;

	//STORE LAST REGISTERS FOR NEXT TIME
	switches_debounced_last = switches_debounced;
	switches_last = switches_read;
}




//***************************************
//***************************************
//********** PROCESS USER MODE **********
//***************************************
//***************************************
//Provide a few simple functions to demonstrate the driver functions
void process_user_mode (void)
{

	//--------------------
	//----- DO LED'S -----
	//--------------------
	
	//----- TURN ON LED A IF WE ARE CONNECTED -----
	if (nic_is_linked)
		LED_A_ON(1);
	else
		LED_A_ON(0);

	//----- TURN ON LED B IF WE HAVE A VALID IP ADDRESS -----
	if (nic_linked_and_ip_address_valid)
		LED_B_ON(1);
	else
		LED_B_ON(0);


	//-----------------------------
	//----- DO USER PROCESSES -----
	//-----------------------------

	//----- PROCESS DEMO UDP SERVER SOCKET -----
	process_demo_udp_server();

	//----- PROCES DEMO UDP CLIENT SOCKET -----
	if (SWITCH_A_NEW_PRESS)
		process_demo_udp_client(1);			//Switch has just been pressed so start comms to remote UDP server
	else
		process_demo_udp_client(0);			//Just process as normal

	//----- PROCESS DEMO TCP SERVER SOCKET -----
	process_demo_tcp_server();

	//----- PROCESS DEMO TCP CLIENT SOCKET ------
	if (SWITCH_B_NEW_PRESS)
		process_demo_tcp_client(1);			//Switch has just been pressed so start comms to remote TCP server
	else
		process_demo_tcp_client(0);			//Just process as normal

	//----- TRIGGER SNTP GET TIME PROCESS -----
	if (SWITCH_A_NEW_PRESS)					//Also triggered by switch 1.  sntp_send_receive_handler below will be called by the SNTP driver.
		sntp_get_time();

}



//*********************************************
//*********************************************
//********** PROCESS DEMO UDP SERVER **********
//*********************************************
//*********************************************
void process_demo_udp_server (void)
{
	static BYTE our_udp_socket = UDP_INVALID_SOCKET;
	static BYTE our_udp_server_state = SM_OPEN_SOCKET;
	BYTE data;
	BYTE array_buffer[4];


	if (!nic_linked_and_ip_address_valid)
	{
		//----- WE ARE NOT CONNECTED OR DO NOT YET HAVE AN IP ADDRESS -----
		our_udp_server_state = SM_OPEN_SOCKET;
		
		//Ensure our socket is closed if we have just lost the Ethernet connection
		udp_close_socket(&our_udp_socket);
		
		return;										//Exit as we can't do anything without a connection
	}


	switch (our_udp_server_state)
	{
	case SM_OPEN_SOCKET:	
		//----- OPEN SOCKET -----
		our_udp_socket = udp_open_socket(0x00, 6451, 1);		//Leave device_info as null to setup to receive from anyone, remote_port can be anything for rx
		if (our_udp_socket != UDP_INVALID_SOCKET)
		{
			our_udp_server_state = SM_PROCESS_SOCKET;
			break;
		}
		//Could not open a socket - none currently available - keep trying

		break;


	case SM_PROCESS_SOCKET:
		//----- PROCESS SOCKET -----
		if (udp_check_socket_for_rx(our_udp_socket))
		{
			//SOCKET HAS RECEIVED A PACKET - PROCESS IT

			//READ THE PACKET AS REQURIED
			if (!udp_read_next_rx_byte(&data))
			{
				//Error - no more bytes in rx packet
			}
			//OR USE
			if (!udp_read_rx_array (array_buffer, sizeof(array_buffer)))
			{
				//Error - no more bytes in rx packet
			}

			//DUMP THE PACKET
			udp_dump_rx_packet();
		
			//SEND RESPONSE
			our_udp_server_state = SM_TX_RESPONSE;
		}
		break;
			
	case SM_TX_RESPONSE:
		//----- TX RESPONSE -----
		//SETUP TX

		//To respond to the sender leave our sockets remote device info as this already contains the remote device settings
		//Or to broadcast on our subnet do this:
		udp_socket[our_udp_socket].remote_device_info.ip_address.Val = our_ip_address.Val | ~our_subnet_mask.Val;
		udp_socket[our_udp_socket].remote_device_info.mac_address.v[0] = 0xff;
		udp_socket[our_udp_socket].remote_device_info.mac_address.v[1] = 0xff;
		udp_socket[our_udp_socket].remote_device_info.mac_address.v[2] = 0xff;
		udp_socket[our_udp_socket].remote_device_info.mac_address.v[3] = 0xff;
		udp_socket[our_udp_socket].remote_device_info.mac_address.v[4] = 0xff;
		udp_socket[our_udp_socket].remote_device_info.mac_address.v[5] = 0xff;
		udp_socket[our_udp_socket].remote_port = 6450;							//Set the port numbers as desired
		udp_socket[our_udp_socket].local_port = 6451;
		
		if (!udp_setup_tx(our_udp_socket))
		{
			//RETURN THE SOCKET BACK TO BROADCAST READY TO RECEIVE FROM ANYONE AGAIN
			//udp_socket[our_udp_socket].remote_device_info.ip_address.Val = 0xffffffff;		//Only enable this if you are broadcasting responses and don't want to miss incomming packets to this socket from other devices
			
			//Can't tx right now - try again next time
			break;
		}

		//WRITE THE UDP DATA
		udp_write_next_byte('H');
		udp_write_next_byte('e');
		udp_write_next_byte('l');
		udp_write_next_byte('l');
		udp_write_next_byte('o');
		udp_write_next_byte(' ');
		udp_write_next_byte('W');
		udp_write_next_byte('o');
		udp_write_next_byte('r');
		udp_write_next_byte('l');
		udp_write_next_byte('d');
		udp_write_next_byte(0x00);
		//You can also use udp_write_array()

		//SEND THE PACKET
		udp_tx_packet();

		//RETURN THE SOCKET BACK TO BROADCAST READY TO RECEIVE FROM ANYONE AGAIN
		udp_socket[our_udp_socket].remote_device_info.ip_address.Val = 0xffffffff;
			
		our_udp_server_state = SM_PROCESS_SOCKET;
		break;

	} //switch (our_udp_server_state)
}



//*********************************************
//*********************************************
//********** PROCESS DEMO UDP CLIENT **********
//*********************************************
//*********************************************
//start_comms
//	1 = Initate this socket sending a packet and waiting for a response
//	0 = Continue in idle state or completing previously started comms
void process_demo_udp_client(BYTE start_comms)
{
	static BYTE our_udp_socket = UDP_INVALID_SOCKET;
	static BYTE our_udp_client_state = SM_OPEN_SOCKET;
	static DEVICE_INFO remote_device_info;
	BYTE data;
	BYTE array_buffer[4];


	if (start_comms)
	{
		//----- TRIGGER COMMS -----
		our_udp_client_state = SM_OPEN_SOCKET;
	}

	if (!nic_linked_and_ip_address_valid)
	{
		//----- WE ARE NOT CONNECTED OR DO NOT YET HAVE AN IP ADDRESS -----
		our_udp_client_state = SM_IDLE;
		
		//Ensure our socket is closed if we have just lost the Ethernet connection
		udp_close_socket(&our_udp_socket);

		return;										//Exit as we can't do anything without a connection
	}

	switch (our_udp_client_state)
	{
	case SM_IDLE:
		//----- DO NOTHING -----
		break;


	case SM_OPEN_SOCKET:	
		//----- OPEN SOCKET -----

		//Set to broadcast on our subnet (alternatively set the IP and MAC address to a remote devices address - use ARP first if the MAC address is unknown)
		remote_device_info.ip_address.Val = our_ip_address.Val | ~our_subnet_mask.Val;
		remote_device_info.mac_address.v[0] = 0xff;
		remote_device_info.mac_address.v[1] = 0xff;
		remote_device_info.mac_address.v[2] = 0xff;
		remote_device_info.mac_address.v[3] = 0xff;
		remote_device_info.mac_address.v[4] = 0xff;
		remote_device_info.mac_address.v[5] = 0xff;

		our_udp_socket = udp_open_socket(&remote_device_info, (WORD)6453, (WORD)6452);		//Set the port numbers as desired
		if (our_udp_socket != UDP_INVALID_SOCKET)
		{
			our_udp_client_state = SM_TX_PACKET;
			break;
		}
		//Could not open a socket - none currently available - keep trying
		break;


	case SM_TX_PACKET:
		//----- TX PACKET TO REMOTE DEVICE -----
		//SETUP TX
		if (udp_setup_tx (our_udp_socket) == 0)
		{
			//Can't tx right now - try again next time
			break;
		}
		
		//WRITE THE TCP DATA
		udp_write_next_byte('H');
		udp_write_next_byte('e');
		udp_write_next_byte('l');
		udp_write_next_byte('l');
		udp_write_next_byte('o');
		udp_write_next_byte(' ');
		udp_write_next_byte('W');
		udp_write_next_byte('o');
		udp_write_next_byte('r');
		udp_write_next_byte('l');
		udp_write_next_byte('d');
		udp_write_next_byte(0x00);
		//You can also use udp_write_array()

		//SEND THE PACKET
		udp_tx_packet();
		
		udp_client_socket_timeout_timer = 10;
		our_udp_client_state = SM_WAIT_FOR_RESPONSE;
		break;


	case SM_WAIT_FOR_RESPONSE:
		//----- WAIT FOR RESPONSE -----
		if (udp_check_socket_for_rx(our_udp_socket))
		{
			//SOCKET HAS RECEIVED A PACKET - PROCESS IT

			//READ THE PACKET AS REQURIED
			if (!udp_read_next_rx_byte(&data))
			{
				//Error - no more bytes in rx packet
			}
			//OR USE
			if (!udp_read_rx_array (array_buffer, sizeof(array_buffer)))
			{
				//Error - no more bytes in rx packet
			}

			//DUMP THE PACKET
			udp_dump_rx_packet();
			
			//EXIT
			our_udp_client_state = SM_CLOSE_SOCKET;
		}
		
		if (udp_client_socket_timeout_timer == 0)
		{
			//TIMED OUT - NO RESPONSE FROM REMOTE DEVICE
			our_udp_client_state = SM_CLOSE_SOCKET;
		}
		break;

	case SM_CLOSE_SOCKET:
		//----- CLOSE THE SOCKET -----
		udp_close_socket(&our_udp_socket);
		
		our_udp_client_state = SM_IDLE;
		break;
	
	} //switch (our_udp_client_state)
}	



//********************************************
//********************************************
//********** PROCESS DEMO TCP SERVER *********
//********************************************
//********************************************
void process_demo_tcp_server (void)
{
	static BYTE our_tcp_server_socket = TCP_INVALID_SOCKET;
	static BYTE our_tcp_server_state = SM_OPEN_SOCKET;
	BYTE data;
	BYTE array_buffer[4];


	if (!nic_linked_and_ip_address_valid)
	{
		//----- WE ARE NOT CONNECTED OR DO NOT YET HAVE AN IP ADDRESS -----
		our_tcp_server_state = SM_OPEN_SOCKET;
		
		//Ensure our socket is closed if we have just lost the Ethernet connection
		tcp_close_socket_from_listen(our_tcp_server_socket);
		
		return;										//Exit as we can't do anything without a connection
	}


	switch (our_tcp_server_state)
	{
	case SM_OPEN_SOCKET:	
		//----- OPEN SOCKET -----
		our_tcp_server_socket = tcp_open_socket_to_listen(4101);		//We will listen on port 4101 (change as required)
		if (our_tcp_server_socket != TCP_INVALID_SOCKET)
		{
			our_tcp_server_state = SM_WAIT_FOR_CONNECTION;
			break;
		}
		//Could not open a socket - none currently available - keep trying
		
		break;
	
	case SM_WAIT_FOR_CONNECTION:
		//----- WAIT FOR A CLIENT TO CONNECT -----
		if(tcp_is_socket_connected(our_tcp_server_socket))
		{
			//A CLIENT HAS CONNECTED TO OUR SOCKET
			our_tcp_server_state = SM_PROCESS_CONNECTION;
			tcp_server_socket_timeout_timer = 10;				//Set our client has been lost timeout (to avoid client disappearing and causing this socket to never be closed)
		}
		break;

	case SM_PROCESS_CONNECTION:
		//----- PROCESS CLIENT CONNECTION -----

		if (tcp_server_socket_timeout_timer == 0)
		{
			//THERE HAS BEEN NO COMMUNICATIONS FROM CLIENT TIMEOUT - RESET SOCKET AS WE ASSUME CLIENT HAS BEEN LOST
			tcp_close_socket(our_tcp_server_socket);		//As this socket is a server the existing connection will be closed but the socket will be reset to wait for a new connection (use tcp_close_socket_from_listen if you want to fully close it)
			our_tcp_server_state = SM_WAIT_FOR_CONNECTION;
		}

		if (tcp_check_socket_for_rx(our_tcp_server_socket))
		{
			//SOCKET HAS RECEIVED A PACKET - PROCESS IT
			tcp_server_socket_timeout_timer = 10;				//Reset our timeout timer

			//READ THE PACKET AS REQURIED
			if (tcp_read_next_rx_byte(&data) == 0)
			{
				//Error - no more bytes in rx packet
			}
			//OR USE
			if (tcp_read_rx_array (array_buffer, sizeof(array_buffer)) == 0)
			{
				//Error - no more bytes in rx packet
			}

			//DUMP THE PACKET
			tcp_dump_rx_packet();
		
			//SEND RESPONSE
			our_tcp_server_state = SM_TX_RESPONSE;
		}

		if (tcp_does_socket_require_resend_of_last_packet(our_tcp_server_socket))
		{
			//RE-SEND LAST PACKET TRANSMITTED
			//(TCP requires resending of packets if they are not acknowledged and to
			//avoid requiring a large RAM buffer the application needs to remember
			//the last packet sent on a socket so it can be resent if requried).
			our_tcp_server_state = SM_TX_RESPONSE;
		}

		if(!tcp_is_socket_connected(our_tcp_server_socket))
		{
			//THE CLIENT HAS DISCONNECTED
			our_tcp_server_state = SM_WAIT_FOR_CONNECTION;
		}

		break;


	case SM_TX_RESPONSE:
		//----- TX RESPONSE -----
		if (!tcp_setup_socket_tx(our_tcp_server_socket))
		{
			//Can't tx right now - try again next time
			break;
		}

		//WRITE THE TCP DATA
		tcp_write_next_byte('H');
		tcp_write_next_byte('e');
		tcp_write_next_byte('l');
		tcp_write_next_byte('l');
		tcp_write_next_byte('o');
		tcp_write_next_byte(' ');
		tcp_write_next_byte('W');
		tcp_write_next_byte('o');
		tcp_write_next_byte('r');
		tcp_write_next_byte('l');
		tcp_write_next_byte('d');
		tcp_write_next_byte(0x00);
		//You can also use tcp_write_array()

		//SEND THE PACKET
		tcp_socket_tx_packet(our_tcp_server_socket);
		
		our_tcp_server_state = SM_PROCESS_CONNECTION;
		break;
	}
}



//********************************************
//********************************************
//********** PROCESS DEMO TCP CLIENT *********
//********************************************
//********************************************
//start_comms
//	1 = Initate this socket attempting to connect to a remote TCP server socket, send a packet and wait for a response
//	0 = Continue in idle state or completing previously started comms
void process_demo_tcp_client (BYTE start_comms)
{
	static BYTE our_tcp_client_socket = TCP_INVALID_SOCKET;
	static BYTE our_tcp_client_state = SM_IDLE;
	static DEVICE_INFO remote_device_info;
	BYTE data;
	BYTE array_buffer[4];


	if (start_comms)
	{
		//----- START COMMS PROCESS -----
		our_tcp_client_state = SM_OPEN_SOCKET;
	}
	
	
	if (!nic_linked_and_ip_address_valid)
	{
		//----- WE ARE NOT CONNECTED OR DO NOT YET HAVE AN IP ADDRESS -----
		our_tcp_client_state = SM_IDLE;
		
		//Ensure our socket is closed if we have just lost the Ethernet connection
		tcp_close_socket(our_tcp_client_socket);
		
		return;								//Exit as we can't do anything without a connection
	}

	switch (our_tcp_client_state)
	{
	case SM_IDLE:
		//----- DO NOTHING -----
		break;


	case SM_OPEN_SOCKET:	
		//----- OPEN SOCKET -----
		remote_device_info.ip_address.v[0] = 192;		//The IP address of the remote device we want to connect to (change as required)
		remote_device_info.ip_address.v[1] = 168;
		remote_device_info.ip_address.v[2] = 0;
		remote_device_info.ip_address.v[3] = 20;
		remote_device_info.mac_address.v[0] = 0;			//Set to zero so TCP driver will automatically use ARP to find MAC address
		remote_device_info.mac_address.v[1] = 0;
		remote_device_info.mac_address.v[2] = 0;
		remote_device_info.mac_address.v[3] = 0;
		remote_device_info.mac_address.v[4] = 0;
		remote_device_info.mac_address.v[5] = 0;

		our_tcp_client_socket = tcp_connect_socket(&remote_device_info, 4102);		//Connect to remote device port 4102 (the port it is listening on - change as required)

		if (our_tcp_client_socket != TCP_INVALID_SOCKET)
		{
			tcp_client_socket_timeout_timer = 10;			//Set our wait for connection timeout
			our_tcp_client_state = SM_WAIT_FOR_CONNECTION;
			break;
		}
		//Could not open a socket - none currently available - keep trying
		
		break;


	case SM_WAIT_FOR_CONNECTION:
		//----- WAIT FOR SOCKET TO CONNECT -----
		if (tcp_is_socket_connected(our_tcp_client_socket));
			our_tcp_client_state = SM_TX_PACKET;

		if (tcp_client_socket_timeout_timer == 0)
		{
			//CONNECTION REQUEST FAILED
			our_tcp_client_state = SM_COMMS_FAILED;
		}
		break;


	case SM_TX_PACKET:
		//----- TX PACKET TO REMOTE DEVICE -----
		if (!tcp_setup_socket_tx(our_tcp_client_socket))
		{
			//Can't tx right now - try again next time
			break;
		}

		//WRITE THE TCP DATA
		tcp_write_next_byte('H');
		tcp_write_next_byte('e');
		tcp_write_next_byte('l');
		tcp_write_next_byte('l');
		tcp_write_next_byte('o');
		tcp_write_next_byte(' ');
		tcp_write_next_byte('W');
		tcp_write_next_byte('o');
		tcp_write_next_byte('r');
		tcp_write_next_byte('l');
		tcp_write_next_byte('d');
		tcp_write_next_byte(0x00);
		//You can also use tcp_write_array()

		//SEND THE PACKET
		tcp_socket_tx_packet(our_tcp_client_socket);

		tcp_client_socket_timeout_timer = 10;			//Set our wait for response timeout
		our_tcp_client_state = SM_WAIT_FOR_RESPONSE;
		break;


	case SM_WAIT_FOR_RESPONSE:
		//----- WAIT FOR RESPONSE -----

		if (tcp_client_socket_timeout_timer == 0)
		{
			//WAIT FOR RESPOSNE TIMEOUT
			our_tcp_client_state = SM_REQUEST_DISCONNECT;
		}

		if (tcp_check_socket_for_rx(our_tcp_client_socket))
		{
			//SOCKET HAS RECEIVED A PACKET - PROCESS IT

			//READ THE PACKET AS REQURIED
			if (tcp_read_next_rx_byte(&data) == 0)
			{
				//Error - no more bytes in rx packet
			}
			//OR USE
			if (tcp_read_rx_array (array_buffer, sizeof(array_buffer)) == 0)
			{
				//Error - no more bytes in rx packet
			}

			//DUMP THE PACKET
			tcp_dump_rx_packet();

			our_tcp_client_state = SM_REQUEST_DISCONNECT;
		}

		if (tcp_does_socket_require_resend_of_last_packet(our_tcp_client_socket))
		{
			//RE-SEND LAST PACKET TRANSMITTED
			//(TCP requires resending of packets if they are not acknowledged and to
			//avoid requiring a large RAM buffer the application needs to remember
			//the last packet sent on a socket so it can be resent if requried).
			our_tcp_client_state = SM_TX_PACKET;
		}

		if(!tcp_is_socket_connected(our_tcp_client_socket))
		{
			//THE CLIENT HAS DISCONNECTED
			our_tcp_client_state = SM_COMMS_FAILED;
		}

		break;


	case SM_REQUEST_DISCONNECT:
		//----- REQUEST TO DISCONNECT FROM REMOTE SERVER -----
		tcp_request_disconnect_socket (our_tcp_client_socket);

		tcp_client_socket_timeout_timer = 10;			//Set our wait for disconnect timeout
		our_tcp_client_state = SM_WAIT_FOR_DISCONNECT;
		break;


	case SM_WAIT_FOR_DISCONNECT:
		//----- WAIT FOR SOCKET TO BE DISCONNECTED -----

		if (tcp_is_socket_closed(our_tcp_client_socket))
		{
			our_tcp_client_state = SM_COMMS_COMPLETE;
		}

		if (tcp_client_socket_timeout_timer == 0)
		{
			//WAIT FOR DISCONNECT TIMEOUT
			tcp_close_socket(our_tcp_client_socket);	//Force the socket closed at our end
			our_tcp_client_state = SM_COMMS_FAILED;
		}
		break;


	case SM_COMMS_COMPLETE:
		//----- COMMUNICATIONS COMPLETE -----
	
		break;


	case SM_COMMS_FAILED:
		//----- COMMUNICATIONS FAILED -----
	
		break;

	}

}



//****************************************
//****************************************
//********** SNTP EVENT HANDLER **********
//****************************************
//****************************************
//We have set this function to be called in eth-sntp.h when we trigger the SNTP client.  Its called once when the SNTP request
//gets sent (after DNS and ARP lookup has occurred, in case we want to start a tight timeout timer) and once when the response
//is received.
void sntp_send_receive_handler (BYTE event, DWORD sntp_seconds_value)
{

	if (event == 1)
	{
		//----- JUST SENT SNTP REQUEST -----
		//< Start some timeout timer here if you want to only accept quickly received responses
	}
	else if (event == 2)
	{
		//----- JUST RECEIVED SNTP RESPONSE -----
		//< Deal with the response.  sntp_seconds_value is SNTP server time in seconds

	}
	else if (event == 3)
	{
		//----- SNTP FAILED -----
		//< There was no response at one of the steps to get a SNTP response

	}

}



//*********************************************************
//*********************************************************
//********** PROCESS EACH LINE OF RECEIVED EMAIL **********
//*********************************************************
//*********************************************************
BYTE process_pop3_received_email_line (BYTE status, BYTE *string, BYTE *sender_email_address)
{

	if (status == 0)
	{
		//----- START OF A NEW EMAIL - THIS IS THE SUBJECT -----

		return(0);
	}
	else if (status == 1)
	{
		//----- NEXT LINE OF THIS EMAIL -----

		return(0);
	}
	else
	{
		//----- END OF EMAIL -----
		return(0x00);		//0x01 = delete email, 0x00 = don't delete this email
	}
}



BYTE send_email_body_text[] = "Hello World!\r\nThis is a test email\r\n";
BYTE send_email_file_attachment[] = "This is a demo file attachemnt.\r\n0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";	//This is just a simple demo file contents - attached file can be any binary file

//*********************************************************
//*********************************************************
//********** PROVIDE NEXT BYTE OF OUTGOING EMAIL **********
//*********************************************************
//*********************************************************
BYTE provide_smtp_next_data_byte (BYTE sending_email_body, BYTE start_of_new_tcp_packet, WORD resend_move_back_bytes, BYTE* next_byte)
{

	if (sending_email_body)
	{
		//-------------------------------------------
		//----- PROVIDE NEXT BYTE OF EMAIL BODY -----
		//-------------------------------------------
		if (resend_move_back_bytes)
		{
			//WE NEED TO RESEND THE LAST PACKET
			if (resend_move_back_bytes >= send_email_body_next_byte)		//(Should always be true but check in case of error)
				send_email_body_next_byte -= resend_move_back_bytes;
		}
		
		if (send_email_body_text[send_email_body_next_byte] != 0x00)
		{
			//NEXT BYTE TO SEND
			*next_byte = send_email_body_text[send_email_body_next_byte++];
			return(1);
		}
		else
		{
			//ALL BYTES SENT
			return(0);
		}
	}
	else
	{
		//------------------------------------------------------
		//----- PROVIDE NEXT BYTE OF EMAIL FILE ATTACHMENT -----
		//------------------------------------------------------
		if (resend_move_back_bytes)
		{
			//WE NEED TO RESEND THE LAST PACKET
			if (resend_move_back_bytes >= send_email_file_next_byte)		//(Should always be true but check in case of error)
				send_email_file_next_byte -= resend_move_back_bytes;
		}
		
		if (send_email_file_attachment[send_email_file_next_byte] != 0x00)
		{
			//NEXT BYTE TO SEND
			*next_byte = send_email_file_attachment[send_email_file_next_byte++];
			return(1);
		}
		else
		{
			//ALL BYTES SENT
			return(0);
		}
	}
}



//******************************************************************
//******************************************************************
//********** PROCESS HTTP SERVER AUTHORISE CLIENT REQUEST **********
//******************************************************************
//******************************************************************
//This optional function is called every time a HEAD, GET or POST request is received from an HTTP client.  It is useful for
//applications where you want to only provide HTTP to certain clients based on their MAC or IP address, or where you want the
//to option to effectively disconnect clients after some form of initial sign in page.
//
//requested_filename
//	Pointer to a null terminated string containing the filename that will be returned to the client after the driver has finished
//	reading all of the request. Your application may alter this if desired (max length = HTTP_MAX_FILENAME_LENGTH).  Can be ignored if you wish.
//requested_filename_extension
//	Pointer to 3 byte string containing the filename extension.  Your application may alter this if desired or it can be ignored.
//tcp_socket_number
//	Allows your application to identify a user by their mac or IP address (e.g. tcp_socket[tcp_socket_number].remote_device_info.ip_address).
//	Can be ignored if you wish.
//Return
//	1 to authorise the request (http server will process and then respond)
//	0 to reject the request (http server will send a 400 Bad Request response)

BYTE process_http_authorise_request (BYTE *requested_filename, BYTE *requested_filename_extension, BYTE tcp_socket_number)
{

	//return(0);			//Deny the request
	return(1);			//Authorise the request
}



//***********************************************************************
//***********************************************************************
//********** PROCESS HTTP SERVER OUTPUT DYNAMIC DATA VARIABLES **********
//***********************************************************************
//***********************************************************************
//This optional function is called each time a special tilde ~my_varaible_name- dynamic data marker is found as an HTML page is being
//send to a client following a request.
//
//variable_name
//	Pointer to a null terminated string containing the varaible name (in the source HTML) between the tilde '~' and '-' characters.
//tcp_socket_number
//	Included in case it is helpful for your application to identify a user (e.g. by their mac or IP address,  e.g. tcp_socket[tcp_socket_number].remote_device_info.ip_address).
//	Can be ignored if you wish.
//Return
//	A pointer to the start of a null termianted string which contains the string to be transmitted (max length 100 characters).

BYTE *process_http_dynamic_data (BYTE *variable_name, BYTE tcp_socket_number)
{
	static BYTE output_string;
	

	//FIND FOR THE DYNAMIC DATA NAME
	if (find_string_in_string_no_case (variable_name, html_name_pop3_server))
	{
		return(&our_pop3_server[0]);
	}
	else if (find_string_in_string_no_case (variable_name, html_name_pop3_username))
	{
		return(&our_pop3_username[0]);
	}
	else if (find_string_in_string_no_case (variable_name, html_name_pop3_password))
	{
		return(&our_pop3_password[0]);
	}
	else if (find_string_in_string_no_case (variable_name, html_name_smtp_server))
	{
		return(&our_smtp_server[0]);
	}
	else if (find_string_in_string_no_case (variable_name, html_name_smtp_username))
	{
		return(&our_smtp_username[0]);
	}
	else if (find_string_in_string_no_case (variable_name, html_name_smtp_password))
	{
		return(&our_smtp_password[0]);
	}
	else if (find_string_in_string_no_case (variable_name, html_name_smtp_to))
	{
		return(&our_smtp_to[0]);
	}
	else if (find_string_in_string_no_case (variable_name, html_name_smtp_from))
	{
		return(&our_smtp_from[0]);
	}
	else if (find_string_in_string_no_case (variable_name, html_name_smtp_subject))
	{
		return(&our_smtp_subject[0]);
	}

	//Unknown variable name - return a null response so nothing is sent
	output_string = 0x00;
	return(&output_string);
}



//***********************************************************
//***********************************************************
//********** PROCESS HTTP SERVER INPUTS FROM FORMS **********
//***********************************************************
//***********************************************************
//This optional function is called each time an input value is received with a GET or POST request.  
//
//input_name
//	Pointer to a null terminated string containing the input name sent by the client (i.e. the name of the form item in your HTML page)
//input_value
//	Pointer to a null terminated string containing the value returned for this item
//requested_filename
//	Pointer to a null terminated string containing the filename that will be returned to the client after the driver has finished reading all of the input
//	data.  Your application may alter this if desired (max length = HTTP_MAX_FILENAME_LENGTH).  Can be ignored if you wish.
//requested_filename_extension
//	Pointer to 3 byte string containing the filename extension.  Your application may alter this if desired or it can be ignored.
//tcp_socket_number
//	Included in case it is helpful for your application to identify a user (e.g. by their mac or IP address, tcp_socket[tcp_socket_number].remote_device_info.ip_address)
//	Can be ignored if you wish.
void process_http_inputs (BYTE *input_name, BYTE *input_value, BYTE *requested_filename, BYTE *requested_filename_extension, BYTE tcp_socket_number)
{
	BYTE *destination_string;
	BYTE copy_string = 0;


	//---------------------------------------
	//----- PROCESS BASED ON INPUT NAME -----
	//---------------------------------------
	if (find_string_in_string_no_case (input_name, html_name_pop3_server))
	{
		destination_string = &our_pop3_server[0];
		copy_string = 1;
	}
	else if (find_string_in_string_no_case (input_name, html_name_pop3_username))
	{
		destination_string = &our_pop3_username[0];
		copy_string = 1;
	}
	else if (find_string_in_string_no_case (input_name, html_name_pop3_password))
	{
		destination_string = &our_pop3_password[0];
		copy_string = 1;
	}
	else if (find_string_in_string_no_case (input_name, html_name_pop3_check_mail))
	{
		if (*input_value == '1')
		{
			//RECEIVE EMAIL
			email_start_receive();		//Trigger receiving email
		}
	}
	else if (find_string_in_string_no_case (input_name, html_name_smtp_server))
	{
		destination_string = &our_smtp_server[0];
		copy_string = 1;
	}
	else if (find_string_in_string_no_case (input_name, html_name_smtp_username))
	{
		destination_string = &our_smtp_username[0];
		copy_string = 1;
	}
	else if (find_string_in_string_no_case (input_name, html_name_smtp_password))
	{
		destination_string = &our_smtp_password[0];
		copy_string = 1;
	}
	else if (find_string_in_string_no_case (input_name, html_name_smtp_to))
	{
		destination_string = &our_smtp_to[0];
		copy_string = 1;
	}
	else if (find_string_in_string_no_case (input_name, html_name_smtp_from))
	{
		destination_string = &our_smtp_from[0];
		copy_string = 1;
	}
	else if (find_string_in_string_no_case (input_name, html_name_smtp_subject))
	{
		destination_string = &our_smtp_subject[0];
		copy_string = 1;
	}
	else if (find_string_in_string_no_case (input_name, html_name_smtp_send_mail))
	{
		if (*input_value == '1')
		{
			//SEND EMAIL
			send_email_body_next_byte = 0;
			send_email_file_next_byte = 0;
			email_start_send(1, 1);		//Trigger send email.  Use authenticated login, include a file attachment
		}
	}

	//----------------------------------------------------
	//----- COPY THE NEW STRING VALUE FROM THE INPUT -----
	//----------------------------------------------------
	if (copy_string)
	{
		while (*input_value != 0x00)		//This is safe as all our destination strings have a length of HTTP_MAX_INPUT_VALUE_LENGTH.  If using shorter strings then check for max string length
		{
			*destination_string++ = *input_value++;
		}
		*destination_string++ = 0x00;
	}

	//-----------------------------------
	//----- ALTER RETURNED HTML FILE ----
	//-----------------------------------
	//An example of how to change the page returned to 'sucess.htm':-
	//*requested_filename++ = 's';
	//*requested_filename++ = 'u';
	//*requested_filename++ = 'c';
	//*requested_filename++ = 'c';
	//*requested_filename++ = 'e';
	//*requested_filename++ = 's';
	//*requested_filename++ = 's';
	//*requested_filename++ = 0x00;
	//*requested_filename_extension++ = 'h';
	//*requested_filename_extension++ = 't';
	//*requested_filename_extension++ = 'm';
}



//********************************************************
//********************************************************
//********** PROCESS INPUT FROM MULTIPART FORMS **********
//********************************************************
//********************************************************

//*****************************************************
//***** PROCESS MULTIPART HEADER FOR NEXT SECTION *****
//*****************************************************
//This function is called for each header found for a new multipart section, of a multipart form post
//It will be called 1 or more times, and this signifies that when process_http_multipart_form_data is next called it will
//be with the data for this new section of the multipart message (i.e. any call to this function means your about to receive
//data for a new multipart section, so reset whatever your application needs to reset to start dealing with the new data).
//The following are the possible values that this function can be called with (always lower case):-
//	content-disposition		Value will be 'form-data' or 'file'
//	name					Name of the corresponding form control
//	filename				Name of the file when content-disposition = file (note that the client is not requried to
//							provide this, but usually will).  May or may not include full path depending on browser.
//							If its important to you to read filename with potentially long paths ensure you set
//							HTTP_MAX_POST_LINE_LENGTH to a value that not cause the end to be cropped off long paths.
//	content-type			Value dependant on the content.  e.g. text/plain, image/gif, etc.
//							If not present then you must assume content-type = text/plain; charset=us-ascii
//
//input_name
//	Pointer to the null termianted header name string
//input_value
//	Pointer to the null termianted value string (converted to lowercase)
//requested_filename
//	Pointer to the null termianted string containing the filename, in case its useful
//requested_filename_extension
//	Pointer to the 3 byte string containing the filename extension, in case its useful
//tcp_socket_number
//	Included in case it is helpful for your application to identify a user (e.g. by their mac or IP address, tcp_socket[tcp_socket_number].remote_device_info.ip_address)

void process_http_multipart_form_header (const BYTE *input_name, BYTE *input_value, BYTE *requested_filename, BYTE *requested_filename_extension, BYTE tcp_socket_number)
{


}


//*******************************************************
//***** RECEIVE EACH BYTE OF THIS MULTIPART SECTION *****
//*******************************************************
//This function is called with each decoded byte in turn of a multipart section.  The data you get
//here is the same as the data submitted by the user.
//process_http_multipart_form_header() is called one of more times before this function starts getting
//called for each section.
void process_http_multipart_form_data (BYTE data)
{


}


//********************************************
//***** END OF MULTIPART SECTION REACHED *****
//********************************************
//Do any processing of last data block that may be required.
//This function is called after the last byte has been received for a multipart section to allow you to carry out any
//operations with the data just recevied before the next multipart section is started or the end of the form post.
void process_http_multipart_form_last_section_done (void)
{


}








//*************************************************************************************************************************************
//*************************************************************************************************************************************
//*************************************************************************************************************************************
//*************************************************************************************************************************************
//*************************************************************************************************************************************
//*************************************************************************************************************************************
//*************************************************************************************************************************************

//************************************************
//************************************************
//********** INTERRUPT VECTOR LOCATIONS **********
//************************************************
//************************************************


//*********************************************
//*********************************************
//********** HEARTBEAT IRQ (Timer 5) **********
//*********************************************
//*********************************************
void __ISR(_TIMER_5_VECTOR, ipl7) Timer5IntHandler(void) 	//(ipl# must match the priority level assigned to the irq where its enabed)
{
	static BYTE hb_10ms_timer = 0;
	static BYTE hb_100ms_timer = 0;
	static BYTE hb_1sec_timer = 0;
	
	INTClearFlag(INT_T5);						//Reset the timer irq flag

		//-----------------------------
		//-----------------------------
		//----- HERE EVERY 1 mSec -----
		//-----------------------------
		//-----------------------------


		//----- NIC DHCP TIMER -----
		if (eth_dhcp_1ms_timer)
			eth_dhcp_1ms_timer--;


		hb_10ms_timer++;
		if (hb_10ms_timer == 10)
		{
			//------------------------------
			//------------------------------
			//----- HERE EVERY 10 mSec -----
			//------------------------------
			//------------------------------
			hb_10ms_timer = 0;


			//----- GENERAL USE 10mS TIMER -----
			if (general_use_10ms_timer)
				general_use_10ms_timer--;


			//----- READ SWITCHES FLAG -----
			read_switches_flag = 1;

			//----- USER MODE 10mS TIMER -----
			if (user_mode_10ms_timer)
				user_mode_10ms_timer--;


			//----- ETHERNET GENERAL TIMER -----
			ethernet_10ms_clock_timer_working++;


		} //if (hb_10ms_timer == 10)

		hb_100ms_timer++;
		if (hb_100ms_timer == 100)
		{
			//-------------------------------
			//-------------------------------
			//----- HERE EVERY 100 mSec -----
			//-------------------------------
			//-------------------------------
			hb_100ms_timer = 0;

			//----- GENERAL USE 100mS TIMER -----
			if (general_use_100ms_timer)
				general_use_100ms_timer--;






			hb_1sec_timer++;
			if (hb_1sec_timer == 10)
			{
				//----------------------------
				//----------------------------
				//----- HERE EVERY 1 Sec -----
				//----------------------------
				//----------------------------
				hb_1sec_timer = 0;

				//----- NIC DHCP TIMERS -----
				if (eth_dhcp_1sec_renewal_timer)
					eth_dhcp_1sec_renewal_timer--;
				if (eth_dhcp_1sec_lease_timer)
					eth_dhcp_1sec_lease_timer--;

				//----- TEST APPLICATION TIMEOUT TIMERS -----
				if (tcp_server_socket_timeout_timer)
					tcp_server_socket_timeout_timer--;
				if (tcp_client_socket_timeout_timer)
					tcp_client_socket_timeout_timer--;
				if (udp_client_socket_timeout_timer)
					udp_client_socket_timeout_timer--;


			} //if (hb_1sec_timer == 10)
		} //if (hb_100ms_timer == 100)

}



//***************************************************
//***************************************************
//********** UNHANDLED EXCEPTION INTERRUPT **********
//***************************************************
//***************************************************
//Useful interrupt to include as it will be called if any exception occurs that is not handled
//There are CPU registers that will tell you where the exception occurred etc.
//View them in the watch window for a more useful description to be shown.
void __attribute__((nomips16)) _general_exception_handler(void)
{
	Nop();



}













