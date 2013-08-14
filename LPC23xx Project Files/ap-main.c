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
//LPC23xx SAMPLE PROJECT C CODE FILE




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
	BYTE b_temp;
	WORD w_temp;
	CONSTANT BYTE *p_string_source;
	BYTE *p_string_dest;


	//------------------------------------------------------
	//----- ENABLE THE MEMORY ACCELERATOR MODULE (MAM) -----
	//------------------------------------------------------
	//if the application is run from on-chip Flash. It provides accelerated execution at higher frequencies and also helps in reducing power consumption
	//The MAM is only available in devices with on-chip Flash.
	//- Done in Philips_LPC230X_Startup.s

	//------------------------------------------------------
	//----- SET THE SYSTEM CLOCK AND PERIPHERAL CLOCKS -----
	//------------------------------------------------------
	//System clock CCLK done in Philips_LPC230X_Startup.s

	//0x0 PCLK_xyz = CCLK/4 [reset default]
	//0x1 PCLK_xyz = CCLK
	//0x2 PCLK_xyz = CCLK/2
	//0x3 PCLK_xyz = CCLK/8 except for CAN1, CAN2, and CAN filtering where 0x03 selects PCLK_xyz = CCLK/6.

	//PCLKSEL0 |= (0x0 << PCLKSEL0_PCLK_WDT_BIT);
	//PCLKSEL0 |= (0x0 << PCLKSEL0_PCLK_TIMER0_BIT);
	//PCLKSEL0 |= (0x0 << PCLKSEL0_PCLK_TIMER1_BIT);
	PCLKSEL0 |= (0x1 << PCLKSEL0_PCLK_UART0_BIT);
	//PCLKSEL0 |= (0x1 << PCLKSEL0_PCLK_UART1_BIT);
	//PCLKSEL0 |= (0x0 << PCLKSEL0_PCLK_PWM0_BIT);
	//PCLKSEL0 |= (0x0 << PCLKSEL0_PCLK_PWM1_BIT);
	//PCLKSEL0 |= (0x0 << PCLKSEL0_PCLK_I2C0_BIT);
	//PCLKSEL0 |= (0x1 << PCLKSEL0_PCLK_SPI_BIT);
	//PCLKSEL0 |= (0x0 << PCLKSEL0_PCLK_RTC_BIT);
	//PCLKSEL0 |= (0x1 << PCLKSEL0_PCLK_SSP1_BIT);
	//PCLKSEL0 |= (0x0 << PCLKSEL0_PCLK_DAC_BIT);
	//PCLKSEL0 |= (0x0 << PCLKSEL0_PCLK_ADC_BIT);
	//PCLKSEL0 |= (0x0 << PCLKSEL0_PCLK_CAN1_BIT);
	//PCLKSEL0 |= (0x0 << PCLKSEL0_PCLK_CAN2_BIT);
	//PCLKSEL0 |= (0x0 << PCLKSEL0_PCLK_ACF_BIT);
	//PCLKSEL1 |= (0x0 << PCLKSEL1_PCLK_BAT_RAM_BIT);
	PCLKSEL1 |= (0x1 << PCLKSEL1_PCLK_GPIO_BIT);
	//PCLKSEL1 |= (0x0 << PCLKSEL1_PCLK_PCB_BIT);
	//PCLKSEL1 |= (0x0 << PCLKSEL1_PCLK_I2C1_BIT);
	//PCLKSEL1 |= (0x1 << PCLKSEL1_PCLK_SSP0_BIT);
	//PCLKSEL1 |= (0x0 << PCLKSEL1_PCLK_TIMER2_BIT);
	//PCLKSEL1 |= (0x0 << PCLKSEL1_PCLK_TIMER3_BIT);
	//PCLKSEL1 |= (0x0 << PCLKSEL1_PCLK_UART2_BIT);
	//PCLKSEL1 |= (0x1 << PCLKSEL1_PCLK_UART3_BIT);
	//PCLKSEL1 |= (0x0 << PCLKSEL1_PCLK_I2C2_BIT);
	//PCLKSEL1 |= (0x0 << PCLKSEL1_PCLK_I2S_BIT);
	//PCLKSEL1 |= (0x0 << PCLKSEL1_PCLK_MCI_BIT);
	//PCLKSEL1 |= (0x0 << PCLKSEL1_PCLK_SYSCON_BIT);


	//---------------------------------------------------
	//----- SET THE MEMORY MAPPING CONTROL REGISTER -----	
	//---------------------------------------------------
	//(MEMMAP at address 0xE01F C040) accordingly. The MEMMAP register gives the application the flexibility of executing interrupts from different memory
	//regions. For instance, if MEMAP is set to 0x2, the interrupt vectors would be mapped to 0x4000 0000 (bottom of on-chip SRAM).
	//- Done in Philips_LPC230X_Startup.s


	//-------------------------
	//----- CONFIGURE WDT -----
	//-------------------------
	WDCLKSEL = 0;							//Clk Source = Internal RC oscillator (default).  The nominal IRC frequency is 4 MHz.
	WDTC =	1000000;						//32bit the time-out value (/4 prescaller) (min 0x000000ff - thus the minimum time-out interval is TWDCLK × 256 × 4.
	WDMOD = (WDMOD_WDEN | WDMOD_WDRESET);	//Watchdog enabled and will cause reset.
    ClrWdt();								//Start the Watchdog timer



	//-------------------------------------------------
	//----- ENABLE / DISABLE POWER TO PERIPHERALS -----
	//-------------------------------------------------
	PCONP = (
			PCONP_PCTIM0 |			//Timer/Counter 0
			//PCONP_PCTIM1 |		//Timer/Counter 1
			PCONP_PCUART0 |			//UART0
			//PCONP_PCUART1 |		//UART1
			//PCONP_PCPWM0 |		//PWM0
			//PCONP_PCPWM1 |		//PWM1
			//PCONP_PCI2C0 |		//The I2C0 interface
			//PCONP_PCSPI |			//The SPI interface
			//PCONP_PCRTC |			//The RTC power/clock
			//PCONP_PCSSP1 |		//The SSP1 interface
			//PCONP_PCEMC |			//External Memory Controller 1
			//PCONP_PCAD |			//A/D converter (ADC) (Note: Clear the PDN bit in the AD0CR before clearing this bit, and set this bit before setting PDN) (reset value = 0)
			//PCONP_PCAN1 |			//CAN Controller 1
			//PCONP_PCAN2 |			//CAN Controller 2
			//PCONP_PCI2C1 |		//The I2C1 interface
			//PCONP_PCSSP0 |		//The SSP0 interface
			//PCONP_PCTIM2 |		//Timer 2
			//PCONP_PCTIM3 |		//Timer 3
			//PCONP_PCUART2 |		//UART 2
			//PCONP_PCUART3 |		//UART 3
			//PCONP_PCI2C2 |		//I2C interface 2
			//PCONP_PCI2CS |		//I2S interface
			//PCONP_PCSDC |			//SD card interface
			//PCONP_PCGPDMA |		//GP DMA function
			PCONP_PCENET |			//Ethernet block
			//PCONP_PUSB |			//USB interface
			0x00000000
			);

	
	//--------------------------
	//----- CONFIGURE GPIO -----
	//--------------------------
	//On the LPC2000, there are certain pins that should not be held low on reset.
	//For instance, by driving P0.14 low on reset would make the on-chip bootloader to take control of the part after reset (Please refer to the Flash
	//Memory System and Programming Chapter in the User Manual for detailed information). There could also be an additional pin in certain devices which
	//should not be held low on reset. If low on reset, then the device behavior is not guaranteed. The following are the pins in various devices:
	//  a. In LPC213x and LPC214x devices, P0.31 should not held low on reset
	//  b. In LPC2114/2124/2212/2214/2119/2129/2194/2290/2292/2294/2210 and 2220 devices, P0.26 should not be held low on reset.
	//(Device will powerup with all IO pins as inputs)

	SCS |= SCS_GPIOM_MASK;		//High speed GPIO mode for ports 0 and 1 (not slower legacy mode)

	FIO0SET = 0x00000004;		//1 = set pin high
	FIO0CLR = 0x00180000;		//1 = set pin low
	FIO0DIR = 0x00180004;		//0 = input, 1 = output.  (GPIO pins P0.29 and P0.30 are shared with the USB D+/? pins and must be configured to be the same direction (input or output)
	//FIO0MASK = 0x00000000;	//1 = pin is not affected by writes into the port's FIOSET, FIOCLR and FIOPIN registers.
	//FIO0PIN					//Read port state.  Write port state if all pins must be set at once to 1's & 0's


	FIO1SET = 0x00000000;		//1 = set pin high
	FIO1CLR = 0x00000000;		//1 = set pin low
	FIO1DIR = 0x00000000;		//0 = input, 1 = output
	//FIO1MASK = 0x00000000;	//1 = pin is not affected by writes into the port's FIOSET, FIOCLR and FIOPIN registers.
	//FIO1PIN					//Read port state.  Write port state if all pins must be set at once to 1's & 0's

	FIO2SET = 0x00000000;		//1 = set pin high
	FIO2CLR = 0x00000000;		//1 = set pin low
	FIO2DIR = 0x00000000;		//0 = input, 1 = output
	//FIO2MASK = 0x00000000;	//1 = pin is not affected by writes into the port's FIOSET, FIOCLR and FIOPIN registers.
	//FIO2PIN					//Read port state.  Write port state if all pins must be set at once to 1's & 0's

	FIO3SET = 0x06000000;		//1 = set pin high
	FIO3CLR = 0x00000000;		//1 = set pin low
	FIO3DIR = 0x06000000;		//0 = input, 1 = output
	//FIO3MASK = 0x00000000;	//1 = pin is not affected by writes into the port's FIOSET, FIOCLR and FIOPIN registers.
	//FIO3PIN					//Read port state.  Write port state if all pins must be set at once to 1's & 0's

	FIO4SET = 0x00000000;		//1 = set pin high
	FIO4CLR = 0x00000000;		//1 = set pin low
	FIO4DIR = 0x00000000;		//0 = input, 1 = output
	//FIO4MASK = 0x00000000;	//1 = pin is not affected by writes into the port's FIOSET, FIOCLR and FIOPIN registers.
	//FIO4PIN					//Read port state.  Write port state if all pins must be set at once to 1's & 0's



  	//------------------------------------------
	//----- CONFIGURE PIN PULL UPS / DOWNS -----
	//------------------------------------------
	//0x0 pull-up resistor enabled [reset default]
	//0x2 neither pull-up nor pull-down.
	//0x3 pull-down resistor enabled.

	//PORT 0
	//PINMODE0 |= (0x0 << PINSEL0_P0_0_BIT);
	//PINMODE0 |= (0x0 << PINSEL0_P0_1_BIT);
	//PINMODE0 |= (0x0 << PINSEL0_P0_2_BIT);
	//PINMODE0 |= (0x0 << PINSEL0_P0_3_BIT);
	//PINMODE0 |= (0x0 << PINSEL0_P0_4_BIT);
	//PINMODE0 |= (0x0 << PINSEL0_P0_5_BIT);
	//PINMODE0 |= (0x0 << PINSEL0_P0_6_BIT);
	//PINMODE0 |= (0x0 << PINSEL0_P0_7_BIT);
	//PINMODE0 |= (0x0 << PINSEL0_P0_8_BIT);
	//PINMODE0 |= (0x0 << PINSEL0_P0_9_BIT);
	//PINMODE0 |= (0x0 << PINSEL0_P0_10_BIT);
	//PINMODE0 |= (0x0 << PINSEL0_P0_11_BIT);
	//PINMODE0 |= (0x0 << PINSEL0_P0_12_BIT);
	//PINMODE0 |= (0x0 << PINSEL0_P0_13_BIT);
	//PINMODE0 |= (0x0 << PINSEL0_P0_14_BIT);
	//PINMODE0 |= (0x0 << PINSEL0_P0_15_BIT);
	//PINMODE1 |= (0x0 << PINSEL1_P0_16_BIT);
	//PINMODE1 |= (0x0 << PINSEL1_P0_17_BIT);
	//PINMODE1 |= (0x0 << PINSEL1_P0_18_BIT);
	//PINMODE1 |= (0x0 << PINSEL1_P0_19_BIT);
	//PINMODE1 |= (0x0 << PINSEL1_P0_20_BIT);
	//PINMODE1 |= (0x0 << PINSEL1_P0_21_BIT);
	//PINMODE1 |= (0x0 << PINSEL1_P0_22_BIT);
	//PINMODE1 |= (0x0 << PINSEL1_P0_23_BIT);
	//PINMODE1 |= (0x0 << PINSEL1_P0_24_BIT);
	//PINMODE1 |= (0x0 << PINSEL1_P0_25_BIT);
	//PINMODE1 |= (0x0 << PINSEL1_P0_26_BIT);
	//PINMODE1 |= (0x0 << PINSEL1_P0_27_BIT);
	//PINMODE1 |= (0x0 << PINSEL1_P0_28_BIT);
	//PINMODE1 |= (0x0 << PINSEL1_P0_29_BIT);
	//PINMODE1 |= (0x0 << PINSEL1_P0_30_BIT);
	//PINMODE1 |= (0x0 << PINSEL1_P0_31_BIT);

	//PORT 1
	//PINMODE2 |= (0x0 << PINSEL2_P1_0_BIT);
	//PINMODE2 |= (0x0 << PINSEL2_P1_1_BIT);
	//PINMODE2 |= (0x0 << PINSEL2_P1_2_BIT);
	//PINMODE2 |= (0x0 << PINSEL2_P1_3_BIT);
	//PINMODE2 |= (0x0 << PINSEL2_P1_4_BIT);
	//PINMODE2 |= (0x0 << PINSEL2_P1_5_BIT);
	//PINMODE2 |= (0x0 << PINSEL2_P1_6_BIT);
	//PINMODE2 |= (0x0 << PINSEL2_P1_7_BIT);
	//PINMODE2 |= (0x0 << PINSEL2_P1_8_BIT);
	//PINMODE2 |= (0x0 << PINSEL2_P1_9_BIT);
	//PINMODE2 |= (0x0 << PINSEL2_P1_10_BIT);
	//PINMODE2 |= (0x0 << PINSEL2_P1_11_BIT);
	//PINMODE2 |= (0x0 << PINSEL2_P1_12_BIT);
	//PINMODE2 |= (0x0 << PINSEL2_P1_13_BIT);
	//PINMODE2 |= (0x0 << PINSEL2_P1_14_BIT);
	//PINMODE2 |= (0x0 << PINSEL2_P1_15_BIT);
	//PINMODE3 |= (0x0 << PINSEL3_P1_16_BIT);
	//PINMODE3 |= (0x0 << PINSEL3_P1_17_BIT);
	//PINMODE3 |= (0x0 << PINSEL3_P1_18_BIT);
	//PINMODE3 |= (0x0 << PINSEL3_P1_19_BIT);
	//PINMODE3 |= (0x0 << PINSEL3_P1_20_BIT);
	//PINMODE3 |= (0x0 << PINSEL3_P1_21_BIT);
	//PINMODE3 |= (0x0 << PINSEL3_P1_22_BIT);
	//PINMODE3 |= (0x0 << PINSEL3_P1_23_BIT);
	//PINMODE3 |= (0x0 << PINSEL3_P1_24_BIT);
	//PINMODE3 |= (0x0 << PINSEL3_P1_25_BIT);
	//PINMODE3 |= (0x0 << PINSEL3_P1_26_BIT);
	//PINMODE3 |= (0x0 << PINSEL3_P1_27_BIT);
	//PINMODE3 |= (0x0 << PINSEL3_P1_28_BIT);
	//PINMODE3 |= (0x0 << PINSEL3_P1_29_BIT);
	//PINMODE3 |= (0x0 << PINSEL3_P1_30_BIT);
	//PINMODE3 |= (0x0 << PINSEL3_P1_31_BIT);

	//PORT 2
	//PINMODE4 |= (0x0 << PINSEL4_P2_0_BIT);
	//PINMODE4 |= (0x0 << PINSEL4_P2_1_BIT);
	//PINMODE4 |= (0x0 << PINSEL4_P2_2_BIT);
	//PINMODE4 |= (0x0 << PINSEL4_P2_3_BIT);
	//PINMODE4 |= (0x0 << PINSEL4_P2_4_BIT);
	//PINMODE4 |= (0x0 << PINSEL4_P2_5_BIT);
	//PINMODE4 |= (0x0 << PINSEL4_P2_6_BIT);
	//PINMODE4 |= (0x0 << PINSEL4_P2_7_BIT);
	//PINMODE4 |= (0x0 << PINSEL4_P2_8_BIT);
	//PINMODE4 |= (0x0 << PINSEL4_P2_9_BIT);
	//PINMODE4 |= (0x0 << PINSEL4_P2_10_BIT);
	//PINMODE4 |= (0x0 << PINSEL4_P2_11_BIT);
	//PINMODE4 |= (0x0 << PINSEL4_P2_12_BIT);
	//PINMODE4 |= (0x0 << PINSEL4_P2_13_BIT);
	//PINMODE4 |= (0x0 << PINSEL4_P2_14_BIT);
	//PINMODE4 |= (0x0 << PINSEL4_P2_15_BIT);
	//PINMODE5 |= (0x0 << PINSEL5_P2_16_BIT);
	//PINMODE5 |= (0x0 << PINSEL5_P2_17_BIT);
	//PINMODE5 |= (0x0 << PINSEL5_P2_18_BIT);
	//PINMODE5 |= (0x0 << PINSEL5_P2_19_BIT);
	//PINMODE5 |= (0x0 << PINSEL5_P2_20_BIT);
	//PINMODE5 |= (0x0 << PINSEL5_P2_21_BIT);
	//PINMODE5 |= (0x0 << PINSEL5_P2_22_BIT);
	//PINMODE5 |= (0x0 << PINSEL5_P2_23_BIT);
	//PINMODE5 |= (0x0 << PINSEL5_P2_24_BIT);
	//PINMODE5 |= (0x0 << PINSEL5_P2_25_BIT);
	//PINMODE5 |= (0x0 << PINSEL5_P2_26_BIT);
	//PINMODE5 |= (0x0 << PINSEL5_P2_27_BIT);
	//PINMODE5 |= (0x0 << PINSEL5_P2_28_BIT);
	//PINMODE5 |= (0x0 << PINSEL5_P2_29_BIT);
	//PINMODE5 |= (0x0 << PINSEL5_P2_30_BIT);
	//PINMODE5 |= (0x0 << PINSEL5_P2_31_BIT);

	//PORT 3
	//PINMODE6 |= (0x0 << PINSEL6_P3_0_BIT);
	//PINMODE6 |= (0x0 << PINSEL6_P3_1_BIT);
	//PINMODE6 |= (0x0 << PINSEL6_P3_2_BIT);
	//PINMODE6 |= (0x0 << PINSEL6_P3_3_BIT);
	//PINMODE6 |= (0x0 << PINSEL6_P3_4_BIT);
	//PINMODE6 |= (0x0 << PINSEL6_P3_5_BIT);
	//PINMODE6 |= (0x0 << PINSEL6_P3_6_BIT);
	//PINMODE6 |= (0x0 << PINSEL6_P3_7_BIT);
	//PINMODE6 |= (0x0 << PINSEL6_P3_8_BIT);
	//PINMODE6 |= (0x0 << PINSEL6_P3_9_BIT);
	//PINMODE6 |= (0x0 << PINSEL6_P3_10_BIT);
	//PINMODE6 |= (0x0 << PINSEL6_P3_11_BIT);
	//PINMODE6 |= (0x0 << PINSEL6_P3_12_BIT);
	//PINMODE6 |= (0x0 << PINSEL6_P3_13_BIT);
	//PINMODE6 |= (0x0 << PINSEL6_P3_14_BIT);
	//PINMODE6 |= (0x0 << PINSEL6_P3_15_BIT);
	//PINMODE7 |= (0x0 << PINSEL7_P3_16_BIT);
	//PINMODE7 |= (0x0 << PINSEL7_P3_17_BIT);
	//PINMODE7 |= (0x0 << PINSEL7_P3_18_BIT);
	//PINMODE7 |= (0x0 << PINSEL7_P3_19_BIT);
	//PINMODE7 |= (0x0 << PINSEL7_P3_20_BIT);
	//PINMODE7 |= (0x0 << PINSEL7_P3_21_BIT);
	//PINMODE7 |= (0x0 << PINSEL7_P3_22_BIT);
	//PINMODE7 |= (0x0 << PINSEL7_P3_23_BIT);
	//PINMODE7 |= (0x0 << PINSEL7_P3_24_BIT);
	//PINMODE7 |= (0x0 << PINSEL7_P3_25_BIT);
	//PINMODE7 |= (0x0 << PINSEL7_P3_26_BIT);
	//PINMODE7 |= (0x0 << PINSEL7_P3_27_BIT);
	//PINMODE7 |= (0x0 << PINSEL7_P3_28_BIT);
	//PINMODE7 |= (0x0 << PINSEL7_P3_29_BIT);
	//PINMODE7 |= (0x0 << PINSEL7_P3_30_BIT);
	//PINMODE7 |= (0x0 << PINSEL7_P3_31_BIT);

	//PORT 4
	//PINMODE8 |= (0x0 << PINSEL8_P4_0_BIT);
	//PINMODE8 |= (0x0 << PINSEL8_P4_1_BIT);
	//PINMODE8 |= (0x0 << PINSEL8_P4_2_BIT);
	//PINMODE8 |= (0x0 << PINSEL8_P4_3_BIT);
	//PINMODE8 |= (0x0 << PINSEL8_P4_4_BIT);
	//PINMODE8 |= (0x0 << PINSEL8_P4_5_BIT);
	//PINMODE8 |= (0x0 << PINSEL8_P4_6_BIT);
	//PINMODE8 |= (0x0 << PINSEL8_P4_7_BIT);
	//PINMODE8 |= (0x0 << PINSEL8_P4_8_BIT);
	//PINMODE8 |= (0x0 << PINSEL8_P4_9_BIT);
	//PINMODE8 |= (0x0 << PINSEL8_P4_10_BIT);
	//PINMODE8 |= (0x0 << PINSEL8_P4_11_BIT);
	//PINMODE8 |= (0x0 << PINSEL8_P4_12_BIT);
	//PINMODE8 |= (0x0 << PINSEL8_P4_13_BIT);
	//PINMODE8 |= (0x0 << PINSEL8_P4_14_BIT);
	//PINMODE8 |= (0x0 << PINSEL8_P4_15_BIT);
	//PINMODE9 |= (0x0 << PINSEL9_P4_16_BIT);
	//PINMODE9 |= (0x0 << PINSEL9_P4_17_BIT);
	//PINMODE9 |= (0x0 << PINSEL9_P4_18_BIT);
	//PINMODE9 |= (0x0 << PINSEL9_P4_19_BIT);
	//PINMODE9 |= (0x0 << PINSEL9_P4_20_BIT);
	//PINMODE9 |= (0x0 << PINSEL9_P4_21_BIT);
	//PINMODE9 |= (0x0 << PINSEL9_P4_22_BIT);
	//PINMODE9 |= (0x0 << PINSEL9_P4_23_BIT);
	//PINMODE9 |= (0x0 << PINSEL9_P4_24_BIT);
	//PINMODE9 |= (0x0 << PINSEL9_P4_25_BIT);
	//PINMODE9 |= (0x0 << PINSEL9_P4_26_BIT);
	//PINMODE9 |= (0x0 << PINSEL9_P4_27_BIT);
	//PINMODE9 |= (0x0 << PINSEL9_P4_28_BIT);
	//PINMODE9 |= (0x0 << PINSEL9_P4_29_BIT);
	//PINMODE9 |= (0x0 << PINSEL9_P4_30_BIT);
	//PINMODE9 |= (0x0 << PINSEL9_P4_31_BIT);


  	//-------------------------------------------
	//----- CONFIGURE PIN SPECIAL FUNCTIONS -----
	//-------------------------------------------
    //0x0 Primary function, typically GPIO port [reset default]
	//0x1 First alternate function
	//0x2 Second alternate function
	//0x3 Third alternate function

	//PORT 0
	//PINSEL0 |= (0x0 << PINSEL0_P0_0_BIT);
	//PINSEL0 |= (0x0 << PINSEL0_P0_1_BIT);
	PINSEL0 |= (0x1 << PINSEL0_P0_2_BIT);			//UART0
	PINSEL0 |= (0x1 << PINSEL0_P0_3_BIT);			//UART0
	//PINSEL0 |= (0x0 << PINSEL0_P0_4_BIT);
	//PINSEL0 |= (0x0 << PINSEL0_P0_5_BIT);
	//PINSEL0 |= (0x0 << PINSEL0_P0_6_BIT);
	//PINSEL0 |= (0x0 << PINSEL0_P0_7_BIT);
	//PINSEL0 |= (0x0 << PINSEL0_P0_8_BIT);
	//PINSEL0 |= (0x0 << PINSEL0_P0_9_BIT);
	//PINSEL0 |= (0x0 << PINSEL0_P0_10_BIT);
	//PINSEL0 |= (0x0 << PINSEL0_P0_11_BIT);
	//PINSEL0 |= (0x0 << PINSEL0_P0_12_BIT);
	//PINSEL0 |= (0x0 << PINSEL0_P0_13_BIT);
	//PINSEL0 |= (0x0 << PINSEL0_P0_14_BIT);
	//PINSEL0 |= (0x0 << PINSEL0_P0_15_BIT);
	//PINSEL1 |= (0x0 << PINSEL1_P0_16_BIT);
	//PINSEL1 |= (0x0 << PINSEL1_P0_17_BIT);
	//PINSEL1 |= (0x0 << PINSEL1_P0_18_BIT);
	//PINSEL1 |= (0x0 << PINSEL1_P0_19_BIT);
	//PINSEL1 |= (0x0 << PINSEL1_P0_20_BIT);
	//PINSEL1 |= (0x0 << PINSEL1_P0_21_BIT);
	//PINSEL1 |= (0x0 << PINSEL1_P0_22_BIT);
	//PINSEL1 |= (0x0 << PINSEL1_P0_23_BIT);
	//PINSEL1 |= (0x0 << PINSEL1_P0_24_BIT);
	//PINSEL1 |= (0x0 << PINSEL1_P0_25_BIT);
	//PINSEL1 |= (0x0 << PINSEL1_P0_26_BIT);
	//PINSEL1 |= (0x0 << PINSEL1_P0_27_BIT);
	//PINSEL1 |= (0x0 << PINSEL1_P0_28_BIT);
	//PINSEL1 |= (0x0 << PINSEL1_P0_29_BIT);
	//PINSEL1 |= (0x0 << PINSEL1_P0_30_BIT);
	//PINSEL1 |= (0x0 << PINSEL1_P0_31_BIT);

	//PORT 1
	PINSEL2 |= (0x1 << PINSEL2_P1_0_BIT);		//Ethernet
	PINSEL2 |= (0x1 << PINSEL2_P1_1_BIT);		//Ethernet
	//PINSEL2 |= (0x0 << PINSEL2_P1_2_BIT);
	//PINSEL2 |= (0x0 << PINSEL2_P1_3_BIT);
	PINSEL2 |= (0x1 << PINSEL2_P1_4_BIT);		//Ethernet
	//PINSEL2 |= (0x0 << PINSEL2_P1_5_BIT);
	//PINSEL2 |= (0x0 << PINSEL2_P1_6_BIT);
	//PINSEL2 |= (0x0 << PINSEL2_P1_7_BIT);
	PINSEL2 |= (0x1 << PINSEL2_P1_8_BIT);		//Ethernet
	PINSEL2 |= (0x1 << PINSEL2_P1_9_BIT);		//Ethernet
	PINSEL2 |= (0x1 << PINSEL2_P1_10_BIT);		//Ethernet
	//PINSEL2 |= (0x0 << PINSEL2_P1_11_BIT);
	//PINSEL2 |= (0x0 << PINSEL2_P1_12_BIT);
	//PINSEL2 |= (0x0 << PINSEL2_P1_13_BIT);
	PINSEL2 |= (0x1 << PINSEL2_P1_14_BIT);		//Ethernet
	PINSEL2 |= (0x1 << PINSEL2_P1_15_BIT);		//Ethernet
	PINSEL3 |= (0x1 << PINSEL3_P1_16_BIT);		//Ethernet
	PINSEL3 |= (0x1 << PINSEL3_P1_17_BIT);		//Ethernet
	//PINSEL3 |= (0x0 << PINSEL3_P1_18_BIT);
	//PINSEL3 |= (0x0 << PINSEL3_P1_19_BIT);
	//PINSEL3 |= (0x0 << PINSEL3_P1_20_BIT);
	//PINSEL3 |= (0x0 << PINSEL3_P1_21_BIT);
	//PINSEL3 |= (0x0 << PINSEL3_P1_22_BIT);
	//PINSEL3 |= (0x0 << PINSEL3_P1_23_BIT);
	//PINSEL3 |= (0x0 << PINSEL3_P1_24_BIT);
	//PINSEL3 |= (0x0 << PINSEL3_P1_25_BIT);
	//PINSEL3 |= (0x0 << PINSEL3_P1_26_BIT);
	//PINSEL3 |= (0x0 << PINSEL3_P1_27_BIT);
	//PINSEL3 |= (0x0 << PINSEL3_P1_28_BIT);
	//PINSEL3 |= (0x0 << PINSEL3_P1_29_BIT);
	//PINSEL3 |= (0x0 << PINSEL3_P1_30_BIT);
	//PINSEL3 |= (0x0 << PINSEL3_P1_31_BIT);

	//PORT 2
	//PINSEL4 |= (0x0 << PINSEL4_P2_0_BIT);
	//PINSEL4 |= (0x0 << PINSEL4_P2_1_BIT);
	//PINSEL4 |= (0x0 << PINSEL4_P2_2_BIT);
	//PINSEL4 |= (0x0 << PINSEL4_P2_3_BIT);
	//PINSEL4 |= (0x0 << PINSEL4_P2_4_BIT);
	//PINSEL4 |= (0x0 << PINSEL4_P2_5_BIT);
	//PINSEL4 |= (0x0 << PINSEL4_P2_6_BIT);
	//PINSEL4 |= (0x0 << PINSEL4_P2_7_BIT);
	//PINSEL4 |= (0x0 << PINSEL4_P2_8_BIT);
	//PINSEL4 |= (0x0 << PINSEL4_P2_9_BIT);
	//PINSEL4 |= (0x0 << PINSEL4_P2_10_BIT);
	//PINSEL4 |= (0x0 << PINSEL4_P2_11_BIT);
	//PINSEL4 |= (0x0 << PINSEL4_P2_12_BIT);
	//PINSEL4 |= (0x0 << PINSEL4_P2_13_BIT);
	//PINSEL4 |= (0x0 << PINSEL4_P2_14_BIT);
	//PINSEL4 |= (0x0 << PINSEL4_P2_15_BIT);
	//PINSEL5 |= (0x0 << PINSEL5_P2_16_BIT);
	//PINSEL5 |= (0x0 << PINSEL5_P2_17_BIT);
	//PINSEL5 |= (0x0 << PINSEL5_P2_18_BIT);
	//PINSEL5 |= (0x0 << PINSEL5_P2_19_BIT);
	//PINSEL5 |= (0x0 << PINSEL5_P2_20_BIT);
	//PINSEL5 |= (0x0 << PINSEL5_P2_21_BIT);
	//PINSEL5 |= (0x0 << PINSEL5_P2_22_BIT);
	//PINSEL5 |= (0x0 << PINSEL5_P2_23_BIT);
	//PINSEL5 |= (0x0 << PINSEL5_P2_24_BIT);
	//PINSEL5 |= (0x0 << PINSEL5_P2_25_BIT);
	//PINSEL5 |= (0x0 << PINSEL5_P2_26_BIT);
	//PINSEL5 |= (0x0 << PINSEL5_P2_27_BIT);
	//PINSEL5 |= (0x0 << PINSEL5_P2_28_BIT);
	//PINSEL5 |= (0x0 << PINSEL5_P2_29_BIT);
	//PINSEL5 |= (0x0 << PINSEL5_P2_30_BIT);
	//PINSEL5 |= (0x0 << PINSEL5_P2_31_BIT);

	//PORT 3
	//PINSEL6 |= (0x0 << PINSEL6_P3_0_BIT);
	//PINSEL6 |= (0x0 << PINSEL6_P3_1_BIT);
	//PINSEL6 |= (0x0 << PINSEL6_P3_2_BIT);
	//PINSEL6 |= (0x0 << PINSEL6_P3_3_BIT);
	//PINSEL6 |= (0x0 << PINSEL6_P3_4_BIT);
	//PINSEL6 |= (0x0 << PINSEL6_P3_5_BIT);
	//PINSEL6 |= (0x0 << PINSEL6_P3_6_BIT);
	//PINSEL6 |= (0x0 << PINSEL6_P3_7_BIT);
	//PINSEL6 |= (0x0 << PINSEL6_P3_8_BIT);
	//PINSEL6 |= (0x0 << PINSEL6_P3_9_BIT);
	//PINSEL6 |= (0x0 << PINSEL6_P3_10_BIT);
	//PINSEL6 |= (0x0 << PINSEL6_P3_11_BIT);
	//PINSEL6 |= (0x0 << PINSEL6_P3_12_BIT);
	//PINSEL6 |= (0x0 << PINSEL6_P3_13_BIT);
	//PINSEL6 |= (0x0 << PINSEL6_P3_14_BIT);
	//PINSEL6 |= (0x0 << PINSEL6_P3_15_BIT);
	//PINSEL7 |= (0x0 << PINSEL7_P3_16_BIT);
	//PINSEL7 |= (0x0 << PINSEL7_P3_17_BIT);
	//PINSEL7 |= (0x0 << PINSEL7_P3_18_BIT);
	//PINSEL7 |= (0x0 << PINSEL7_P3_19_BIT);
	//PINSEL7 |= (0x0 << PINSEL7_P3_20_BIT);
	//PINSEL7 |= (0x0 << PINSEL7_P3_21_BIT);
	//PINSEL7 |= (0x0 << PINSEL7_P3_22_BIT);
	//PINSEL7 |= (0x0 << PINSEL7_P3_23_BIT);
	//PINSEL7 |= (0x0 << PINSEL7_P3_24_BIT);
	//PINSEL7 |= (0x0 << PINSEL7_P3_25_BIT);
	//PINSEL7 |= (0x0 << PINSEL7_P3_26_BIT);
	//PINSEL7 |= (0x0 << PINSEL7_P3_27_BIT);
	//PINSEL7 |= (0x0 << PINSEL7_P3_28_BIT);
	//PINSEL7 |= (0x0 << PINSEL7_P3_29_BIT);
	//PINSEL7 |= (0x0 << PINSEL7_P3_30_BIT);
	//PINSEL7 |= (0x0 << PINSEL7_P3_31_BIT);

	//PORT 4
	//PINSEL8 |= (0x0 << PINSEL8_P4_0_BIT);
	//PINSEL8 |= (0x0 << PINSEL8_P4_1_BIT);
	//PINSEL8 |= (0x0 << PINSEL8_P4_2_BIT);
	//PINSEL8 |= (0x0 << PINSEL8_P4_3_BIT);
	//PINSEL8 |= (0x0 << PINSEL8_P4_4_BIT);
	//PINSEL8 |= (0x0 << PINSEL8_P4_5_BIT);
	//PINSEL8 |= (0x0 << PINSEL8_P4_6_BIT);
	//PINSEL8 |= (0x0 << PINSEL8_P4_7_BIT);
	//PINSEL8 |= (0x0 << PINSEL8_P4_8_BIT);
	//PINSEL8 |= (0x0 << PINSEL8_P4_9_BIT);
	//PINSEL8 |= (0x0 << PINSEL8_P4_10_BIT);
	//PINSEL8 |= (0x0 << PINSEL8_P4_11_BIT);
	//PINSEL8 |= (0x0 << PINSEL8_P4_12_BIT);
	//PINSEL8 |= (0x0 << PINSEL8_P4_13_BIT);
	//PINSEL8 |= (0x0 << PINSEL8_P4_14_BIT);
	//PINSEL8 |= (0x0 << PINSEL8_P4_15_BIT);
	//PINSEL9 |= (0x0 << PINSEL9_P4_16_BIT);
	//PINSEL9 |= (0x0 << PINSEL9_P4_17_BIT);
	//PINSEL9 |= (0x0 << PINSEL9_P4_18_BIT);
	//PINSEL9 |= (0x0 << PINSEL9_P4_19_BIT);
	//PINSEL9 |= (0x0 << PINSEL9_P4_20_BIT);
	//PINSEL9 |= (0x0 << PINSEL9_P4_21_BIT);
	//PINSEL9 |= (0x0 << PINSEL9_P4_22_BIT);
	//PINSEL9 |= (0x0 << PINSEL9_P4_23_BIT);
	//PINSEL9 |= (0x0 << PINSEL9_P4_24_BIT);
	//PINSEL9 |= (0x0 << PINSEL9_P4_25_BIT);
	//PINSEL9 |= (0x0 << PINSEL9_P4_26_BIT);
	//PINSEL9 |= (0x0 << PINSEL9_P4_27_BIT);
	//PINSEL9 |= (0x0 << PINSEL9_P4_28_BIT);
	//PINSEL9 |= (0x0 << PINSEL9_P4_29_BIT);
	//PINSEL9 |= (0x0 << PINSEL9_P4_30_BIT);
	//PINSEL9 |= (0x0 << PINSEL9_P4_31_BIT);


	//--------------------------------
	//----- SETUP AtoD CONVERTER -----
	//--------------------------------
	/*
	AD0CR = (
			(0x10 << AD0CR_SEL_BIT) |			//AD bit to sample
			(3 << AD0CR_CLKDIV_BIT) |			//PCLK is divided by (this value + 1) to produce the clock for the A/D converter, which should be less than or equal to 4.5 MHz.
			(AD0CR_BURST) |						//Continuous burst sample (we are only using 1 input)
			(0 << AD0CR_CLKS_BIT) |				//Defines number of clocks used for each conversion in Burst mode (0 = 10 bit max resolution)
			(AD0CR_PDN));						//AtoD enabled
	*/



	//------------------------
	//----- SETUP TIMERS -----
	//------------------------

	//----- SETUP TIMER 0 (32bit) -----
	//Used for: Heartbeat
	T0CTCR = (											//Count Control Register
			(0x0 << T0CTCR_Counter_Timer_Mode_BIT) |	//0x0 = Timer Mode: every rising PCLK edge, 0x1 = Counter Mode: TC incremented on rising edges on the CAP input, 0x2 = Counter Mode: TC incremented on falling edges on the CAP input, 0x3 = Counter Mode: TC is incremented on both edges on the CAP input
			(0x0 << T0CTCR_Count_Input_Select_BIT));	//When bits 1:0 in this register are not 00, these bits select which CAP pin is sampled for clocking
	T0PR = 0;								//Prescale register.  Prescale Counter increments on every PCLK. When it reaches the TxPR value the Timer incremented and the Prescale Counter is reset on the next PCLK. Timer will increment on every PCLK when PR = 0, every 2 PCLKs when PR = 1, etc.
	T0MCR = (T0MCR_MR0I | T0MCR_MR0R);		//Interrupt on MR0, Reset on MR0
    T0MR0 =	15000;							//Match register
    //T0CCR = 0;							//Capture Control Register
    //T0EMR = 0;							//External Match Register
	T0TCR =	T0TCR_Counter_Enable;			//Timer Control Register (T0TCR_Counter_Enable, T0TCR_Counter_Reset)
    //IRQ enabled in interrupts section below


	//----- SETUP TIMER 1 (32bit) -----
	//Used for: Available
	/*
	T1CTCR = (											//Count Control Register
			(0x0 << T1CTCR_Counter_Timer_Mode_BIT) |	//0x0 = Timer Mode: every rising PCLK edge, 0x1 = Counter Mode: TC incremented on rising edges on the CAP input, 0x2 = Counter Mode: TC incremented on falling edges on the CAP input, 0x3 = Counter Mode: TC is incremented on both edges on the CAP input
			(0x0 << T1CTCR_Count_Input_Select_BIT));	//When bits 1:0 in this register are not 00, these bits select which CAP pin is sampled for clocking
	T1PR = 0;								//Prescale register.  Prescale Counter increments on every PCLK. When it reaches the TxPR value the Timer incremented and the Prescale Counter is reset on the next PCLK. Timer will increment on every PCLK when PR = 0, every 2 PCLKs when PR = 1, etc.
	T1MCR = (T1MCR_MR0I | T1MCR_MR0R);		//Interrupt on MR0, Reset on MR0
    T1MR0 =									//Match register
    //T1CCR = 0;							//Capture Control Register
    //T1EMR = 0;							//External Match Register
	T1TCR =	T1TCR_Counter_Enable;			//Timer Control Register (T0TCR_Counter_Enable, T0TCR_Counter_Reset)
	*/

	//----- SETUP TIMER 2 (32bit) -----
	//Used for: Available
	/*
	T2CTCR = (											//Count Control Register
			(0x0 << T2CTCR_Counter_Timer_Mode_BIT) |	//0x0 = Timer Mode: every rising PCLK edge, 0x1 = Counter Mode: TC incremented on rising edges on the CAP input, 0x2 = Counter Mode: TC incremented on falling edges on the CAP input, 0x3 = Counter Mode: TC is incremented on both edges on the CAP input
			(0x0 << T2CTCR_Count_Input_Select_BIT));	//When bits 1:0 in this register are not 00, these bits select which CAP pin is sampled for clocking
	T2PR = 0;								//Prescale register.  Prescale Counter increments on every PCLK. When it reaches the TxPR value the Timer incremented and the Prescale Counter is reset on the next PCLK. Timer will increment on every PCLK when PR = 0, every 2 PCLKs when PR = 1, etc.
	T2MCR = (T2MCR_MR0I | T2MCR_MR0R);		//Interrupt on MR0, Reset on MR0
    T2MR0 =									//Match register
    //T2CCR = 0;							//Capture Control Register
    //T2EMR = 0;							//External Match Register
	T2TCR =	T2TCR_Counter_Enable;			//Timer Control Register (T0TCR_Counter_Enable, T0TCR_Counter_Reset)
	*/


	//----- SETUP TIMER 3 (32bit) -----
	//Used for: Available
	/*
	T3CTCR = (											//Count Control Register
			(0x0 << T3CTCR_Counter_Timer_Mode_BIT) |	//0x0 = Timer Mode: every rising PCLK edge, 0x1 = Counter Mode: TC incremented on rising edges on the CAP input, 0x2 = Counter Mode: TC incremented on falling edges on the CAP input, 0x3 = Counter Mode: TC is incremented on both edges on the CAP input
			(0x0 << T3CTCR_Count_Input_Select_BIT));	//When bits 1:0 in this register are not 00, these bits select which CAP pin is sampled for clocking
	T3PR = 0;								//Prescale register.  Prescale Counter increments on every PCLK. When it reaches the TxPR value the Timer incremented and the Prescale Counter is reset on the next PCLK. Timer will increment on every PCLK when PR = 0, every 2 PCLKs when PR = 1, etc.
	T3MCR = (T3MCR_MR0I | T3MCR_MR0R);		//Interrupt on MR0, Reset on MR0
    T3MR0 =									//Match register
    //T3CCR = 0;							//Capture Control Register
    //T3EMR = 0;							//External Match Register
	T3TCR =	T3TCR_Counter_Enable;			//Timer Control Register (T0TCR_Counter_Enable, T0TCR_Counter_Reset)
	*/




	//---------------------
	//----- SETUP SSP -----
	//---------------------

	//----- SETUP SPI -----
	//Used for: Available
	/*
	//USING SPI PORT (Slower than SSP0 and no FIFO so not using)
	S0SPCCR = 150;							//8 bit SPI Clock Counter Register. This register controls the frequency of a master's SCK.
											//The number of SPI peripheral clock cycles that make up an SPI clock (both edges). In Master mode, this register must be an even number greater than or equal to 8.
											//The SPI0 SCK rate may be calculated as: PCLK_SPI / SPCCR0 value. The SPI peripheral clock is determined by the PCLKSEL0 register contents for PCLK_SPI.
											//SPI PCLK = 60MHz.  400KHz = 2.5uS = 150, 25MHz = 40nS = 8(min allowed) = 7.5MHz actual speed
	S0SPCR = 0x0820;						//SPI Control Register. This register controls the operation of the SPI.
											//Clock low in idle bus state
											//Input data is valid on the rising edge of the clock (Our transmit transistion occurs on transition from active (high) to Idle (low) clock state)
	*/

	/*
	//USING SSP0 PORT
	SSP0CPSR = 150;							//CPSDVSR.  Even value between 2 and 254 by which SSPPCLK is divided before being used by SCR
	SSP0CR0 = (((1 - 1) << SSP0CR0_SCR_BIT) | 0x07);	//8 bit, idle low, input data valid on the rising edge of the clock (Our transmit transistion occurs on transition from active (high) to Idle (low) clock state)
											//SCR (Serial clock rate) is required value - 1
    SSP0CR1 = SSP0CR1_SSE;					//Enable SSP port (after setting up other registers)
											//Input data is valid on the rising edge of the clock (Our transmit transistion occurs on transition from active (high) to Idle (low) clock state)

	//RX has an 8 frame fifo so ensure its empty
    while (SSP0SR & SSP0SR_RNE)
		b_temp = SSP0DR;
	*/


	//----- SETUP SSP 1 -----
	//Used for: Available
	/*
	SSP1CPSR = 2;							//CPSDVSR.  Even value between 2 and 254 by which SSPPCLK is divided before being used by SCR
	SSP1CR0 = (((18 - 1) << SSP1CR0_SCR_BIT) | 0x07);	//8 bit, idle low, input data valid on the rising edge of the clock (Our transmit transistion occurs on transition from active (high) to Idle (low) clock state)
											//SCR (Serial clock rate) is required value - 1
    SSP1CR1 = SSP1CR1_SSE;					//Enable SSP port (after setting up other registers)
											//Input data is valid on the rising edge of the clock (Our transmit transistion occurs on transition from active (high) to Idle (low) clock state)
	*/





	//-----------------------
	//----- SETUP USART -----
	//-----------------------
	//On reset UART0 is enabled, other UART's are disabled
	//Ensure UART receive pins do no have pull down resistors enabled

	//----- SETUP UART 0 -----
	//Used for: Available
	U0LCR = 0x3b;								//Line control register (8 bit, "0" stick parity)
	U0FCR = 0;									//FIFO control register (8 bit)
	
	//Set BAUD rate to 56818 @ PCLK = CCLK/0
    U0LCR |= U0LCR_Divisor_Latch_Access_Bit;
	U0DLL = 66;									//Divisor latch (baud rate) LSB (8 bit) (DLAB = 1)
	U0DLM = 0;									//Divisor latch (baud rate) MSB (8 bit) (DLAB = 1)
	//U0FDR = ((0x01 << U0FDR_MULVAL_BIT) | (0x00 << U0FDR_DIVADDVAL_BIT));	//Fractional divider register (8 bit)

	U0TER = 0x80;								//Transmit enable register (8 bit)

	//Setup interrupts
	U0LCR &= ~U0LCR_Divisor_Latch_Access_Bit;
	U0IER = U0IER_RBR_Interrupt_Enable;			//Interrupt enable register (8 bit) (DLAB = 0)
	//IRQ enabled in interrupts section below


	//----- SETUP UART 1 -----
	//Used for: Available
	/*
	//(N.B. UART1 has additional modem control function, but this can be ignored unless enabled)
	U1LCR = 0x03;								//Line control register (8 bit) (8 bit, no parity)
	U1FCR = 0;									//FIFO control register (8 bit)
	
	//Set BAUD rate to 9600 @ PCLK = CCLK/0
    U1LCR |= U1LCR_Divisor_Latch_Access_Bit;
	U1DLL = 135;								//Divisor latch (baud rate) LSB (8 bit) (DLAB = 1)
	U1DLM = 1;									//Divisor latch (baud rate) MSB (8 bit) (DLAB = 1)
	//U1FDR = ((0x01 << U1FDR_MULVAL_BIT) | (0x00 << U1FDR_DIVADDVAL_BIT));	//Fractional divider register (8 bit)

	U1TER = 0x80;								//Transmit enable register (8 bit)

	//Setup interrupts
	U1LCR &= ~U1LCR_Divisor_Latch_Access_Bit;
	U1IER = U1IER_RBR_Interrupt_Enable;			//Interrupt enable register (8 bit) (DLAB = 0)
	//IRQ enabled in interrupts section below
	*/
    


	//----- SETUP UART 3 -----
	//Used for: Available
	/*
	U3LCR = 0x03;								//Line control register (8 bit) (8 bit, no parity)
	U3FCR = 0;									//FIFO control register (8 bit)
	
	//Set BAUD rate to 17857.1 @ PCLK = CCLK/0
    U3LCR |= U3LCR_Divisor_Latch_Access_Bit;
	U3DLL = 210;								//Divisor latch (baud rate) LSB (8 bit) (DLAB = 1)
	U3DLM = 0;									//Divisor latch (baud rate) MSB (8 bit) (DLAB = 1)
	//U3FDR = ((0x01 << U3FDR_MULVAL_BIT) | (0x00 << U3FDR_DIVADDVAL_BIT));	//Fractional divider register (8 bit)

	U3TER = 0x80;								//Transmit enable register (8 bit)

	//Setup interrupts
	U3LCR &= ~U3LCR_Divisor_Latch_Access_Bit;
	U3IER = U3IER_RBR_Interrupt_Enable;			//Interrupt enable register (8 bit) (DLAB = 0)
	//IRQ enabled in interrupts section below
	*/




	//---------------------
	//----- SETUP DMA -----
	//---------------------
	//DMA0 used for: Available
	//DMA1 used for: Available
	//DMACConfiguration = DMACConfiguration_E;	//Enable DMA, little endian mode (include DMACConfiguration_M for big endian)

	//Defining ram to be used for DMA example:
	//BYTE dma_my_tx_buffer[512] __attribute__ ((section(".usb_ram")));		//Must use DMA accessible ram area

	//---------------------------------------
	//----- SETUP RTC (REAL TIME CLOCK) -----
	//---------------------------------------
	//CCR = CCR_CLKSRC;							//Take clock from 32.768kHz oscillator
	//CCR |= CCR_CLKEN;							//Enable clock



	//----------------------------
	//----- SETUP INTERRUPTS -----
	//----------------------------
	//It is recommended that only one interrupt source should be classified as an FIQ.
	//It is always safe to program the Default Vector Address Register (VICDefVectAddr) with a dummy ISR address wherein the VIC would /be updated (by performing a
	//write operation on the VIC Vector Address register (VICVectAddr) to avoid any spurious interrupts.


	//----- TIMER 0 -----
    VICVectAddr4 = (unsigned int)&timer0_irq_handler;		//C function to call on interrupt
    VICVectPriority4 = 10;									//Priority [Highest]0 to 15[lowest]
	VICIntEnable |= VICIntEnable_TIMER0;

	//----- UART 0 -----
	//Used for: Available
    //VICVectAddr6 = (unsigned int)&uart0_irq_handler;		//C function to call on interrupt
    //VICVectPriority6 = 8;									//Priority [Highest]0 to 15[lowest]
	//VICIntEnable |= VICIntEnable_UART0;

	//----- ENABLE INTERRUPTS -----
	ENABLE_INT;


	//----------------------------------------------
	//----- GENERAL APPLICATION INITIALISATION -----
	//----------------------------------------------


	

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
	//static BYTE done_udp_tx = 0;
	//static BYTE our_udp_socket = UDP_INVALID_SOCKET;



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
		ClrWdt();

		//----- READ SWITCHES -----
		read_switches();

		//----- PROCESS MODE -----
		process_user_mode();

		//----- PROCESS ETHERNET STACK -----
		tcp_ip_process_stack();



		
	}
	return(0);
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
	if (FIO0PIN & 0x00080000)
		switches_read |= 0x01;
	if (FIO0PIN & 0x00100000)
		switches_read |= 0x02;

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

//*******************************************
//*******************************************
//********** TIMER 0 HEARTBEAT IRQ **********
//*******************************************
//*******************************************
void timer0_irq_handler (void)
{
	static BYTE hb_10ms_timer = 0;
	static BYTE hb_100ms_timer = 0;
	static WORD hb_1sec_timer = 0;


	T0IR = 0x3f;							//Reset irq

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





	} //if (hb_100ms_timer == 100)

	hb_1sec_timer++;
	if (hb_1sec_timer == 1000)
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


	} //if (hb_1sec_timer == 1000)


}	





