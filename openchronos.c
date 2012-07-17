/*
    openchronos.c: openchronos-ng main loop & user interface
	 
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
// *************************************************************************************************
//
//	Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
//
//
//	  Redistribution and use in source and binary forms, with or without
//	  modification, are permitted provided that the following conditions
//	  are met:
//
//	    Redistributions of source code must retain the above copyright
//	    notice, this list of conditions and the following disclaimer.
//
//	    Redistributions in binary form must reproduce the above copyright
//	    notice, this list of conditions and the following disclaimer in the
//	    documentation and/or other materials provided with the
//	    distribution.
//
//	    Neither the name of Texas Instruments Incorporated nor the names of
//	    its contributors may be used to endorse or promote products derived
//	    from this software without specific prior written permission.
//
//	  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//	  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//	  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//	  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//	  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//	  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//	  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//	  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//	  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//	  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//	  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// *************************************************************************************************
// Initialization and control of application.
// *************************************************************************************************

// *************************************************************************************************
// Include section

#include <openchronos.h>

#include "modinit.h"

/* Driver */
#include <drivers/display.h>
#include <drivers/vti_as.h>
#include <drivers/vti_ps.h>
#include <drivers/radio.h>
#include <drivers/buzzer.h>
#include <drivers/ports.h>
#include <drivers/timer.h>
#include <drivers/pmm.h>
#include <drivers/rf1a.h>
#include <drivers/rtca.h>
#include <drivers/temperature.h>
#include <drivers/battery.h>

#define BIT_IS_SET(F, B)  ((F) | (B)) == (F)

/* Menu definitions and declarations */
struct menu {
	/* menu item name */
	char const * name;
	/* Pointer to up button handler */
	void (*up_btn_fn)(void);
	/* Pointer to down button handler */
	void (*down_btn_fn)(void);
	/* Pointer to function button (NUM) */
	void (*num_btn_fn)(void);
	/* Pointer to settings button (long STAR) */
	void (*lstar_btn_fn)(void);
	/* Pointer to function button (long NUM) */
	void (*lnum_btn_fn)(void);
	/* Pointer to simultaneous up&down press */
	void (*updown_btn_fn)(void);
	/* Pointer to activate function */
	void (*activate_fn)(void);
	/* Pointer to deactivate function */
	void (*deactivate_fn)(void);
	/* pointer to next menu item */
	struct menu *next;
	struct menu *prev;
};

/* The head of the linked list holding menu items */
static struct menu *menu_head;

/* Menu mode stuff */
static struct {
	uint8_t enabled:1;      /* is menu mode enabled? */
	struct menu *item;      /* the currently active menu item */
} menumode;

/* Menu edit mode stuff */
static struct {
	uint8_t enabled:1;          /* is edit mode enabled? */
	uint8_t pos:7;              /* the position for selected item */
	void (* complete_fn)(void); /* call this fn when editmode exits */
	struct menu_editmode_item *items;  /* vector of editmode items */
} menu_editmode;

/* the message bus */
static struct sys_messagebus *messagebus;

/***************************************************************************
 ************************* THE SYSTEM MESSAGE BUS **************************
 **************************************************************************/
void sys_messagebus_register(void (*callback)(enum sys_message),
                             enum sys_message listens)
{
	struct sys_messagebus **p = &messagebus;

	while (*p) {
		p = &(*p)->next;
	}

	*p = malloc(sizeof(struct sys_messagebus));
	(*p)->next = NULL;
	(*p)->fn = callback;
	(*p)->listens = listens;
}

void sys_messagebus_unregister(void (*callback)(enum sys_message))
{
	struct sys_messagebus *p = messagebus, *pp = NULL;

	while (p) {
		if (p->fn == callback) {
			if (!pp)
				messagebus = p->next;
			else
				pp->next = p->next;

			free(p);
		}

		pp = p;
		p = p->next;
	}
}

void check_events(void)
{
	enum sys_message msg = 0;

	/* drivers/rtca */
	if (rtca_last_event) {
		msg |= rtca_last_event;
		rtca_last_event = 0;
	}

	/* drivers/timer */
	if (timer0_last_event) {
		msg |= timer0_last_event << 7;
		timer0_last_event = 0;
	}

	/* drivers/accelerometer */
	if(as_last_interrupt){
		msg |= SYS_MSG_AS_INT;
		as_last_interrupt = 0;
	}

#ifdef CONFIG_BATTERY_MONITOR
	/* drivers/battery */
	if ((msg & SYS_MSG_RTC_MINUTE) == SYS_MSG_RTC_MINUTE) {
		msg |= SYS_MSG_BATT;
		battery_measurement();
	}
#endif

	{
		struct sys_messagebus *p = messagebus;

		while (p) {
			/* notify listener if he registered for any of these messages */
			if (msg & p->listens) {
				p->fn(msg);
			}

			/* move to next */
			p = p->next;
		}
	}


}

/***************************************************************************
 ************************ USER INPUT / MAIN MENU ***************************
 **************************************************************************/

static void editmode_handler(void)
{
	/* STAR button exits edit mode */
	if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_STAR)) {
		/* deselect item */
		menu_editmode.items[menu_editmode.pos].deselect();

		menu_editmode.complete_fn();
		menu_editmode.enabled = 0;

	} else if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_NUM)) {
		/* deselect current item */
		menu_editmode.items[menu_editmode.pos].deselect();

		/* select next item */
		menu_editmode.pos++;
		if (! menu_editmode.items[menu_editmode.pos].set)
			menu_editmode.pos = 0;
		menu_editmode.items[menu_editmode.pos].select();

	} else if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_UP)) {
		menu_editmode.items[menu_editmode.pos].set(1);

	} else if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_DOWN)) {
		menu_editmode.items[menu_editmode.pos].set(-1);
	}
}

static void menumode_handler(void)
{
	if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_STAR)) {
		/* exit mode mode */
		menumode.enabled = 0;
	
		/* clear both lines but keep symbols! */
		display_clear(0, 1);
		display_clear(0, 2);
		
		/* turn off up/down symbols */
		display_symbol(0, LCD_SYMB_ARROW_UP, SEG_OFF);
		display_symbol(0, LCD_SYMB_ARROW_DOWN, SEG_OFF);
		
		/* stop blinking name of current selected module */
		display_chars(0, LCD_SEG_L2_4_0, NULL, BLINK_OFF);

		/* activate item */
		if (menumode.item->activate_fn)
			menumode.item->activate_fn();

	} else if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_UP)) {
		menumode.item = menumode.item->next;
		display_chars(0, LCD_SEG_L2_4_0, menumode.item->name, SEG_SET);

	} else if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_DOWN)) {
		menumode.item = menumode.item->prev;
		display_chars(0, LCD_SEG_L2_4_0, menumode.item->name, SEG_SET);
	}
}

static void menumode_enable(void)
{
	/* deactivate current menu item */
	if (menumode.item->deactivate_fn)
		menumode.item->deactivate_fn();

	/* enable edit mode */
	menumode.enabled = 1;

	/* show MENU in the first line */
	display_chars(0, LCD_SEG_L1_3_0, "MENU", SEG_SET);

	/* turn on up/down symbols */
	display_symbol(0, LCD_SYMB_ARROW_UP, SEG_ON);
	display_symbol(0, LCD_SYMB_ARROW_DOWN, SEG_ON);

	/* show up blinking name of current selected item */
	display_chars(0, LCD_SEG_L2_4_0, NULL, BLINK_ON);
	display_chars(0, LCD_SEG_L2_4_0, menumode.item->name, SEG_SET);
}

static void check_buttons(void)
{
	if (menu_editmode.enabled) {
		editmode_handler();

	} else if (menumode.enabled) {
		menumode_handler();

	} else {
		if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_LSTAR)) {
			if (menumode.item->lstar_btn_fn)
				menumode.item->lstar_btn_fn();

		} else if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_STAR)) {
			menumode_enable();

		} else if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_LNUM)) {
			if (menumode.item->lnum_btn_fn)
				menumode.item->lnum_btn_fn();

		} else if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_NUM)) {
			if (menumode.item->num_btn_fn)
				menumode.item->num_btn_fn();
		
		} else if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_UP | PORTS_BTN_DOWN)) {
			if (menumode.item->updown_btn_fn)
				menumode.item->updown_btn_fn();

		} else if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_UP)) {
			if (menumode.item->up_btn_fn)
				menumode.item->up_btn_fn();

		} else if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_DOWN)) {
			if (menumode.item->down_btn_fn)
				menumode.item->down_btn_fn();
		}
	}

	ports_pressed_btns = 0;
}

void menu_add_entry(char const * name,
          void (*up_btn_fn)(void),
		    void (*down_btn_fn)(void),
		    void (*num_btn_fn)(void),
		    void (*lstar_btn_fn)(void),
			 void (*lnum_btn_fn)(void),
			 void (*updown_btn_fn)(void),
		    void (*activate_fn)(void),
		    void (*deactivate_fn)(void))
{
	struct menu **menu_hd = &menu_head;
	struct menu *menu_p;

	if (! *menu_hd) {
		/* Head is empty, create new menu item linked to itself */
		menu_p = (struct menu *) malloc(sizeof(struct menu));
		menu_p->next = menu_p;
		menu_p->prev = menu_p;
		*menu_hd = menu_p;
		
		/* There wasnt any menu active, so we activate this one */
		menumode.item = menu_p;
		activate_fn();
	} else {
		/* insert new item before head */
		menu_p = (struct menu *) malloc(sizeof(struct menu));
		menu_p->next = (*menu_hd);
		menu_p->prev = (*menu_hd)->prev;
		(*menu_hd)->prev = menu_p;
		menu_p->prev->next = menu_p;
	}

	menu_p->name = name;
	menu_p->up_btn_fn = up_btn_fn;
	menu_p->down_btn_fn = down_btn_fn;
	menu_p->num_btn_fn = num_btn_fn;
	menu_p->lstar_btn_fn = lstar_btn_fn;
	menu_p->lnum_btn_fn = lnum_btn_fn;
	menu_p->updown_btn_fn = updown_btn_fn;
	menu_p->activate_fn = activate_fn;
	menu_p->deactivate_fn = deactivate_fn;
}

void menu_editmode_start(void (* complete_fn)(void),
                         struct menu_editmode_item *items)
{
	menu_editmode.pos = 0;
	menu_editmode.items = items;
	menu_editmode.complete_fn = complete_fn;

	menu_editmode.enabled = 1;

	/* select the first item */
	menu_editmode.items[0].select();
}

/***************************************************************************
 ************************ INITIALIZATION ROUTINE ***************************
 **************************************************************************/

void init_application(void)
{
	volatile unsigned char *ptr;

	// ---------------------------------------------------------------------
	// Enable watchdog

	// Watchdog triggers after 16 seconds when not cleared
#ifdef USE_WATCHDOG
	WDTCTL = WDTPW + WDTIS__512K + WDTSSEL__ACLK;
#else
	WDTCTL = WDTPW + WDTHOLD;
#endif

	// ---------------------------------------------------------------------
	// Configure PMM
	SetVCore(3);

	// Set global high power request enable
	PMMCTL0_H  = 0xA5;
	PMMCTL0_L |= PMMHPMRE;
	PMMCTL0_H  = 0x00;

	// ---------------------------------------------------------------------
	// Enable 32kHz ACLK
	P5SEL |= 0x03;                            // Select XIN, XOUT on P5.0 and P5.1
	UCSCTL6 &= ~XT1OFF;        				  // XT1 On, Highest drive strength
	UCSCTL6 |= XCAP_3;                        // Internal load cap

	UCSCTL3 = SELA__XT1CLK;                   // Select XT1 as FLL reference
	UCSCTL4 = SELA__XT1CLK | SELS__DCOCLKDIV | SELM__DCOCLKDIV;

	// ---------------------------------------------------------------------
	// Configure CPU clock for 12MHz
	_BIS_SR(SCG0);                  // Disable the FLL control loop
	UCSCTL0 = 0x0000;          // Set lowest possible DCOx, MODx
	UCSCTL1 = DCORSEL_5;       // Select suitable range
	UCSCTL2 = FLLD_1 + 0x16E;  // Set DCO Multiplier
	_BIC_SR(SCG0);                  // Enable the FLL control loop

	// Worst-case settling time for the DCO when the DCO range bits have been
	// changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
	// UG for optimization.
	// 32 x 32 x 8 MHz / 32,768 Hz = 250000 = MCLK cycles for DCO to settle
#if __GNUC_MINOR__ > 5 || __GNUC_PATCHLEVEL__ > 8
	__delay_cycles(250000);
#else
	__delay_cycles(62500);
        __delay_cycles(62500);
        __delay_cycles(62500);
        __delay_cycles(62500);
#endif
  
	// Loop until XT1 & DCO stabilizes, use do-while to insure that 
	// body is executed at least once
	do {
		UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);
		SFRIFG1 &= ~OFIFG;                      // Clear fault flags
	} while ((SFRIFG1 & OFIFG));


	// ---------------------------------------------------------------------
	// Configure port mapping

	// Disable all interrupts
	__disable_interrupt();
	// Get write-access to port mapping registers:
	PMAPPWD = 0x02D52;
	// Allow reconfiguration during runtime:
	PMAPCTL = PMAPRECFG;

	// P2.7 = TA0CCR1A or TA1CCR0A output (buzzer output)
	ptr  = &P2MAP0;
	*(ptr + 7) = PM_TA1CCR0A;
	P2OUT &= ~BIT7;
	P2DIR |= BIT7;

	// P1.5 = SPI MISO input
	ptr  = &P1MAP0;
	*(ptr + 5) = PM_UCA0SOMI;
	// P1.6 = SPI MOSI output
	*(ptr + 6) = PM_UCA0SIMO;
	// P1.7 = SPI CLK output
	*(ptr + 7) = PM_UCA0CLK;

	// Disable write-access to port mapping registers:
	PMAPPWD = 0;
	// Re-enable all interrupts
	__enable_interrupt();

	// Init the hardwre real time clock (RTC_A)
	rtca_init();
#if (CONFIG_DST > 0)
	/* Initialize the DST. IMPORTANT: DST DEPENDS ON RTCA! */
	dst_init();
#endif
	// ---------------------------------------------------------------------
	// Configure ports

	// ---------------------------------------------------------------------
	// Reset radio core
	radio_reset();
	radio_powerdown();

#ifdef CONFIG_ACCELEROMETER
	// ---------------------------------------------------------------------
	// Init acceleration sensor
	as_init();
#else
	as_disconnect();
#endif

	// ---------------------------------------------------------------------
	// Init LCD
	lcd_init();

	// ---------------------------------------------------------------------
	// Init buttons
	init_buttons();

	// ---------------------------------------------------------------------
	// Configure Timer0 for use by the clock and delay functions
	timer0_init();
	
	/* Init buzzer */
	buzzer_init();

	// ---------------------------------------------------------------------
	// Init pressure sensor
	ps_init();

	/* drivers/battery */
	battery_init();

	/* drivers/temperature */
	temperature_init();

#ifdef CONFIG_INFOMEM
	if (infomem_ready() == -2) {
		infomem_init(INFOMEM_C, INFOMEM_C + 2 * INFOMEM_SEGMENT_SIZE);
	}
#endif
}


/***************************************************************************
 ************************ ENTRYPOINT AND MAIN LOOP *************************
 **************************************************************************/
int main(void)
{
	// Init MCU
	init_application();	

#ifdef CONFIG_TEST
	// Branch to welcome screen
	test_mode();
#else
	/* clear whole scren */
	display_clear(0, 0);
#endif

	/* Init modules */
	mod_init();

	/* main loop */
	while (1) {
		/* Go to LPM3, wait for interrupts */
		_BIS_SR(LPM3_bits + GIE);
		__no_operation();

		/* service watchdog on wakeup */
		#ifdef USE_WATCHDOG
			// Service watchdog (reset counter)
			WDTCTL = (WDTCTL & 0xff) | WDTPW | WDTCNTCL;
		#endif

		/* check if any driver has events pending */
		check_events();

		/* check for button presses, drive the menu */
		check_buttons();
	}
}


/***************************************************************************
 **************************** HERE BE HELPERS ******************************
 **************************************************************************/
void helpers_loop(uint8_t *value, uint8_t lower, uint8_t upper, int8_t step)
{
	/* for now only increase/decrease on steps of 1 value */
	if (step > 0) {
		/* prevent overflow */
		if (*value == 255) {
			*value = lower;
			return;
		}

		(*value)++;
		if(*value -1 == upper)
			*value = lower;
	} else {
		/* prevent overflow */
		if (*value == 0) {
			*value = upper;
			return;
		}

		(*value)--;
		if(*value +1 == lower)
			*value = upper;
	}
}



