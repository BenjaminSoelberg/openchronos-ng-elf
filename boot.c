/*
    boot.c: Openchronos-ng on boot wireless updater

    Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>

	          http://www.openchronos-ng.sourceforge.net

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
   Developer note: I thought about using SYSRIVECT, to override the
	interrupt handler of PORT2 (buttons). This would allow temporarily
	replacing the interrupt vector in RAM at boot, providing button
	interrupts needed to wake up the system from low power.
	Unfortunately this would require to modify the linker script to
	reserve 128 bytes at the TOP of the RAM for the new interrupt vector.
	Since we are not handling watchdog timer interrupts in the main program,
	I thought we could use it as a source of external events to wakeup the
	CPU from low power mode and check buttons state.
*/


#include "openchronos.h"
#include "drivers/pmm.h"

/* Entry point of of the Flash Updater in BSL memory */
#define CALL_RFSBL()   ((void (*)())0x1000)()

#define ALL_BUTTONS 0x1F
#define BTN_BL_PIN BIT3

inline void initialize_aclk()
{
	/* Select XIN, XOUT on P5.0 and P5.1 */
	P5SEL |= 0x03;

	/* XT1 On, Highest drive strength */
	UCSCTL6 &= ~XT1OFF;

	/* Internal load cap */
	UCSCTL6 |= XCAP_3;

	/* Select XT1 as FLL reference */
	UCSCTL3 = SELA__XT1CLK;

	/* Enable the FLL control loop */
	UCSCTL4 = SELA__XT1CLK | SELS__DCOCLKDIV | SELM__DCOCLKDIV;
}

inline void initialize_cpu_12mhz()
{
	/* Disable the FLL control loop */
	_BIS_SR(SCG0);

	/* Set lowest possible DCOx, MODx */
	UCSCTL0 = 0x0000;

	/* Select suitable range */
	UCSCTL1 = DCORSEL_5;

	/* Set DCO Multiplier */
	UCSCTL2 = FLLD_1 + 0x16E;
	_BIC_SR(SCG0);

	/* Worst-case settling time for the DCO when the DCO range bits have been
	changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
	UG for optimization.
	32 x 32 x 8 MHz / 32,768 Hz = 250000 = MCLK cycles for DCO to settle */
	__delay_cycles(250000);

	/* Loop until XT1 & DCO stabilizes, use do-while to insure that
	body is executed at least once */
	do {
		UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);

		/* Clear fault flags */
		SFRIFG1 &= ~OFIFG;
	} while ((SFRIFG1 & OFIFG));
}

inline void initialize_buttons()
{
	/* Set button ports to input */
	P2DIR &= ~ALL_BUTTONS;

	/* Enable internal pull-downs */
	P2OUT &= ~ALL_BUTTONS;
	P2REN |= ALL_BUTTONS;
}

inline void initialize_lcd()
{
	/* clear entire display memory */
	LCDBMEMCTL |= LCDCLRBM + LCDCLRM;

	/* flickers in the sun */
	/* LCD_FREQ = ACLK/12/8 = 341.3Hz */

	/* still flickers in the sun, when watch is moving */
	/* LCD_FREQ = ACLK/10/8 = 409.6Hz */

	/* no flickering */
	/* LCD_FREQ = ACLK/8/8 = 512Hz */
	/* Frame frequency = 512Hz/2/4 = 64Hz, LCD mux 4, LCD on */
	LCDBCTL0 = (LCDDIV0 + LCDDIV1 + LCDDIV2)
	         | (LCDPRE0 + LCDPRE1) | LCD4MUX | LCDON;

	/* LCB_BLK_FREQ = ACLK/8/2048 = 2Hz */
	LCDBBLKCTL = LCDBLKPRE1 | LCDBLKDIV0 | LCDBLKDIV1
	           | LCDBLKDIV2 | LCDBLKMOD0;

	/* I/O to COM outputs */
	P5SEL |= (BIT5 | BIT6 | BIT7);
	P5DIR |= (BIT5 | BIT6 | BIT7);

	/* Activate LCD output */
	/* Select LCD segments S0-S15 */
	LCDBPCTL0 = 0xFFFF;
	/* Select LCD segments int16_t-S22 */
	LCDBPCTL1 = 0x00FF;

#ifdef USE_LCD_CHARGE_PUMP
	/* Charge pump voltage generated internally,
	   internal bias (V2-V4) generation */
	LCDBVCTL = LCDCPEN | VLCD_2_72;
#endif
}

inline void jump_to_rfbsl()
{
	/* clear display memory (useful to know if rfbsl failed) */
	LCDBMEMCTL |= LCDCLRBM + LCDCLRM;

	/* finally jump to the BSL memory address */
	CALL_RFSBL();
}


/* put bootmenu in the init8 section which is executed before main */
__attribute__((naked, section(".crt_0042"), used))
static void init8(void)
{
	/* Stop watchdog timer */
	WDTCTL = WDTPW + WDTHOLD;

	/* Configure PMM */
	SetVCore(3);

	/* Set global high power request enable */
	{
		PMMCTL0_H  = 0xA5;
		PMMCTL0_L |= PMMHPMRE;
		PMMCTL0_H  = 0x00;
	}

	/* Enable 32kHz ACLK */
	initialize_aclk();

	/* Configure CPU clock for 12MHz */
	initialize_cpu_12mhz();	

	/* Configure buttons for input */
	initialize_buttons();

	/* Initialize LCD */
	initialize_lcd();

	/* Write 'boot' to the screen without using display functions */
	LCDM2 = 199; /* 'b' */
	LCDM3 = 198; /* 'o' */
	LCDM4 = 198; /* 'o' */
	LCDM6 = 135; /* 't' */

	/* configure watchdog interrupt timer, used for polling buttons */
	{
		/* ACLK timer source, 250ms timer mode, resume watchdog */
		WDTCTL = WDT_ADLY_250;

		/* Enable watchdog timer interrupts */
		SFRIE1 |= WDTIE;
	}

	/* Enable global interrupts */
	__enable_interrupt();

	/* loop if no button is pressed, enter RFBSL if backlight is pressed */
	do {
		_BIS_SR(LPM3_bits | GIE);

		if ((P2IN & ALL_BUTTONS) == BTN_BL_PIN)
			jump_to_rfbsl();

	} while ((P2IN & ALL_BUTTONS) == 0);

	/* Disable them again, they will be re-enabled later on in main() */
	__disable_interrupt();

}

__attribute__((interrupt(WDT_VECTOR)))
void WDT_ISR(void)
{
	/* exit from LPM3 after interrupt */
	_BIC_SR_IRQ(LPM3_bits);
}

