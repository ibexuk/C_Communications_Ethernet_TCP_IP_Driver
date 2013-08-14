/*****************************************************************************
 * Copyright (c) 2001, 2002 Rowley Associates Limited.                       *
 *                                                                           *
 * This file may be distributed under the terms of the License Agreement     *
 * provided with this software.                                              *
 *                                                                           *
 * THIS FILE IS PROVIDED AS IS WITH NO WARRANTY OF ANY KIND, INCLUDING THE   *
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. *
 *****************************************************************************/

//NOTE THAT IF YOU EDIT THIS FILE YOU MUST COMPILE IT - BUILD DOESN'T DO THIS (SELECT THIS FILE AND SELECT COMPILE)

//Errata: Rev ?A? devices: Code execution from internal flash is restricted to a maximum of 60MHz.
//FCCO = (2 × M × FIN) / N
//Note that M and N values are (PLLCFG_MSEL_BIT + 1) and (PLLCFG_NSEL_BIT + 1)
//Set to 480 MHz (must be in the range of 275 MHz to 550 MHz)
#define	PLLCFG_VAL ((19 << PLLCFG_MSEL_BIT) | (0 << PLLCFG_NSEL_BIT))

//Device specific - Comment out for debug mode, needed for release mode
//#define  STARTUP_FROM_RESET

//Don't use
//#define VECTORED_IRQ_INTERRUPTS     

//Needed as our secondary bootloader uses the flash vector locations
#define	SRAM_EXCEPTIONS

//60 MHz for the CPU clock.  Divide PLL by 8 (CCLKCFG_VAL + 1).  CCLKCFG_VAL must be odd
#define	CCLKCFG_VAL 7


/*****************************************************************************
 *                           Preprocessor Definitions
 *                           ------------------------
 *
 * VECTORED_IRQ_INTERRUPTS
 *
 *   Enable vectored IRQ interrupts. If defined, the PC register will be loaded
 *   with the contents of the VICVectAddr register on an IRQ exception.
 *
 * STARTUP_FROM_RESET
 *
 *   If defined, the program will startup from power-on/reset. If not defined
 *   the program will just loop endlessly from power-on/reset.
 *
 *   This definition is not defined by default on this target because the
 *   debugger is unable to reset this target and maintain control of it over the
 *   JTAG interface. The advantage of doing this is that it allows the debugger
 *   to reset the CPU and run programs from a known reset CPU state on each run.
 *   It also acts as a safety net if you accidently download a program in FLASH
 *   that crashes and prevents the debugger from taking control over JTAG
 *   rendering the target unusable over JTAG. The obvious disadvantage of doing
 *   this is that your application will not startup without the debugger.
 *
 *   We advise that on this target you keep STARTUP_FROM_RESET undefined whilst
 *   you are developing and only define STARTUP_FROM_RESET when development is
 *   complete.
 *
 * PLLCFG_VAL
 *
 *   Override the default PLL configuration by defining PLLCFG_VAL.
 *
 * CCLKCFG_VAL
 *
 *   Override the default CPU clock divider configuration by defining 
 *   CCLKCFG_VAL.
 *
 * NO_PLL_ENABLE
 *
 *   If defined, the PLL will not be enabled.
 *
 * USBCLKCFG_VAL
 *
 *   Override the default USB clock divider configuration by defining 
 *   USBCLKCFG_VAL.
 *
 * MAMCR_VAL & MAMTIM_VAL
 * 
 *   Override the default MAM configuration (fully enabled, 3 fetch cycles)
 *   by defining MAMCR_VAL and MAMTIM_VAL.
 *
 * SRAM_EXCEPTIONS
 *
 *   If defined, enable copying and re-mapping of interrupt vectors from User 
 *   FLASH to SRAM. If undefined, interrupt vectors will be mapped in User 
 *   FLASH.
 *
 *****************************************************************************/



#include <targets/LPC2000.h>

#if OSCILLATOR_CLOCK_FREQUENCY==12000000

/* Fosc = 12Mhz, Fcco = 288Mhz, cclk = 72Mhz, usbclk = 48Mhz */
#ifndef PLLCFG_VAL
#define PLLCFG_VAL ((11 << PLLCFG_MSEL_BIT) | (0 << PLLCFG_NSEL_BIT))
#endif
#ifndef CCLKCFG_VAL
#define CCLKCFG_VAL 3
#endif
#ifndef USBCLKCFG_VAL
#define USBCLKCFG_VAL 5
#endif

#endif

#ifndef MAMCR_VAL
#define MAMCR_VAL 2
#endif

#ifndef MAMTIM_VAL
#define MAMTIM_VAL 3
#endif

  .section .vectors, "ax"
  .code 32
  .align 0
  .global _vectors
  .global reset_handler

/*****************************************************************************
 * Exception Vectors                                                         *
 *****************************************************************************/
_vectors:
  ldr pc, [pc, #reset_handler_address - . - 8]  /* reset */
  ldr pc, [pc, #undef_handler_address - . - 8]  /* undefined instruction */
  ldr pc, [pc, #swi_handler_address - . - 8]    /* swi handler */
  ldr pc, [pc, #pabort_handler_address - . - 8] /* abort prefetch */
  ldr pc, [pc, #dabort_handler_address - . - 8] /* abort data */
#ifdef VECTORED_IRQ_INTERRUPTS
  .word 0xB9206E58                              /* boot loader checksum */
  ldr pc, [pc, #-0x120]                         /* irq handler */
#else
  .word 0xB8A06F60                              /* boot loader checksum */
  ldr pc, [pc, #irq_handler_address - . - 8]    /* irq handler */
#endif
  ldr pc, [pc, #fiq_handler_address - . - 8]    /* fiq handler */

reset_handler_address:
#ifdef STARTUP_FROM_RESET
  .word reset_handler
#else
  .word reset_wait
#endif
undef_handler_address:
  .word undef_handler
swi_handler_address:
  .word swi_handler
pabort_handler_address:
  .word pabort_handler
dabort_handler_address:
  .word dabort_handler
#ifndef VECTORED_IRQ_INTERRUPTS
irq_handler_address:
  .word irq_handler
#endif
fiq_handler_address:
  .word fiq_handler

  .section .init, "ax"
  .code 32
  .align 0

/******************************************************************************
 *                                                                            *
 * Default exception handlers                                                 *
 *                                                                            *
 ******************************************************************************/
reset_handler:
  ldr r0, =SCB_BASE

#if defined(PLLCFG_VAL) && !defined(NO_PLL_ENABLE)
  /* Configure PLL Multiplier/Divider */
  ldr r1, [r0, #PLLSTAT_OFFSET]
  tst r1, #PLLSTAT_PLLC
  beq 1f

  /* Disconnect PLL */
  ldr r1, =PLLCON_PLLE
  str r1, [r0, #PLLCON_OFFSET]
  mov r1, #0xAA
  str r1, [r0, #PLLFEED_OFFSET]
  mov r1, #0x55
  str r1, [r0, #PLLFEED_OFFSET]
1:
  /* Disable PLL */
  ldr r1, =0
  str r1, [r0, #PLLCON_OFFSET]
  mov r1, #0xAA
  str r1, [r0, #PLLFEED_OFFSET]
  mov r1, #0x55
  str r1, [r0, #PLLFEED_OFFSET]

  /* Enable main oscillator */
  ldr r1, [r0, #SCS_OFFSET]
  orr r1, r1, #SCS_OSCEN
  str r1, [r0, #SCS_OFFSET]
1:
  ldr r1, [r0, #SCS_OFFSET]
  tst r1, #SCS_OSCSTAT
  beq 1b

  /* Select main oscillator as the PLL clock source */
  ldr r1, =1
  str r1, [r0, #CLKSRCSEL_OFFSET]

  /* Set PLLCFG */
  ldr r1, =PLLCFG_VAL
  str r1, [r0, #PLLCFG_OFFSET]
  mov r1, #0xAA
  str r1, [r0, #PLLFEED_OFFSET]
  mov r1, #0x55
  str r1, [r0, #PLLFEED_OFFSET]

  /* Enable PLL */
  ldr r1, =PLLCON_PLLE
  str r1, [r0, #PLLCON_OFFSET]
  mov r1, #0xAA
  str r1, [r0, #PLLFEED_OFFSET]
  mov r1, #0x55
  str r1, [r0, #PLLFEED_OFFSET]

#ifdef CCLKCFG_VAL
  /* Set the CPU clock divider */
  ldr r1, =CCLKCFG_VAL
  str r1, [r0, #CCLKCFG_OFFSET]
#endif

#ifdef USBCLKCFG_VAL
  /* Set the CPU clock divider */
  ldr r1, =USBCLKCFG_VAL
  str r1, [r0, #USBCLKCFG_OFFSET]
#endif
  
  /* Wait for PLL to lock */
1:
  ldr r1, [r0, #PLLSTAT_OFFSET]
  tst r1, #PLLSTAT_PLOCK
  beq 1b
  /* PLL Locked, connect PLL as clock source */
  mov r1, #(PLLCON_PLLE | PLLCON_PLLC)
  str r1, [r0, #PLLCON_OFFSET]
  mov r1, #0xAA
  str r1, [r0, #PLLFEED_OFFSET]
  mov r1, #0x55
  str r1, [r0, #PLLFEED_OFFSET]
  /* Wait for PLL to connect */
1:
  ldr r1, [r0, #PLLSTAT_OFFSET]
  tst r1, #PLLSTAT_PLLC
  beq 1b
#endif

  /* Initialise memory accelerator module */
  mov r1, #0
  str r1, [r0, #MAMCR_OFFSET]
  ldr r1, =MAMTIM_VAL
  str r1, [r0, #MAMTIM_OFFSET]
  ldr r1, =MAMCR_VAL
  str r1, [r0, #MAMCR_OFFSET]
 
#if defined(SRAM_EXCEPTIONS)
  /* Copy exception vectors into SRAM */
  mov r8, #0x40000000
  ldr r9, =_vectors
  ldmia r9!, {r0-r7}
  stmia r8!, {r0-r7}
  ldmia r9!, {r0-r6}
  stmia r8!, {r0-r6}

  /* Re-map interrupt vectors from SRAM */
  ldr r0, =SCB_BASE
  mov r1, #2 /* User RAM Mode. Interrupt vectors are re-mapped from SRAM */
  str r1, [r0, #MEMMAP_OFFSET]
#endif /* SRAM_EXCEPTIONS */
  
  b _start

#ifndef STARTUP_FROM_RESET
reset_wait:

#endif

/******************************************************************************
 *                                                                            *
 * Default exception handlers                                                 *
 * These are declared weak symbols so they can be redefined in user code.     * 
 *                                                                            *
 ******************************************************************************/
undef_handler:
  b undef_handler
  
swi_handler:
  b swi_handler
  
pabort_handler:
  b pabort_handler
  
dabort_handler:
  b dabort_handler
  
fiq_handler:
  b fiq_handler

irq_handler:
  b irq_handler

  .weak undef_handler, swi_handler, pabort_handler, dabort_handler, fiq_handler, irq_handler
                                                    

                  
