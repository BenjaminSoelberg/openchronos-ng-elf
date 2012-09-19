/*
 * laptimer.c
 *
 *  Created on: Sep 16, 2012
 *      Author: rhh-privat
 *
 *      This module is intended as a "hello world" application for me
 *      to get used to the openchronos-ng framework..
 *
 *      Use:
 *      You can select the LAP-mode from the function-menu
 *
 *      First line shows "LAP" (in this version) and the second line
 *      shows the lapcounter value.
 *      ^ (up-arrow) increments the lapcounter
 *      v (dn-arrow) decrements the lapcounter
 *      # (num-longpressed) resets the lapcounter
 *      # (num-shortpressed) --- noop
 *      * (star-Longpressed) --- noop
 *		- (both-arrows) --- noop
 */


#include <openchronos.h>
/* driver */
#include <drivers/display.h>


uint laps;

void drawLapcounterScreen(void)
{
	display_chars(0, LCD_SEG_L1_3_1, "LAP", SEG_SET);
	_printf(0, LCD_SEG_L2_3_0, "%4u", laps);
	display_symbol(0, LCD_SYMB_TOTAL, SEG_ON);

}

static void lapcounter_activated()
{
	drawLapcounterScreen();
}


static void lapcounter_deactivated()
{
	/* clean up screen */
	display_clear(0, 1);
	display_clear(0, 2);
	display_symbol(0, LCD_SYMB_TOTAL, SEG_OFF);
}

static void num_long_pressed()
{
	laps = 0;
	drawLapcounterScreen();
}

static void down_press()
{
	if (laps > 0)
		laps--;
	drawLapcounterScreen();
}
static void up_press()
{
	laps++;
	drawLapcounterScreen();
}


void mod_lapcounter_init(void)
{
	laps = 0;

	menu_add_entry(" LAP ",
			&up_press,			/*	void (*up_btn_fn)(void)     */
			&down_press,		/*	void (*down_btn_fn)(void)   */
			NULL, 				/*	void (*num_btn_fn)(void)    */
			NULL,				/*	void (*lstar_btn_fn)(void)  */
			&num_long_pressed,	/*	void (*lnum_btn_fn)(void)   */
			NULL,				/*	void (*updown_btn_fn)(void) */
			&lapcounter_activated,
			&lapcounter_deactivated);
}
