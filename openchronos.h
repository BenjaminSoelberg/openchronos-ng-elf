/*!
	\mainpage openchronos-ng (new generation)

	\section contact_sec Contact
	Project website: http://openchronos-ng.sourceforge.net
	            IRC: \#openchronos @ chat.freenode.net

	<i>for mailing list, bug tracker and forum please go to the project website, you will find that info there.</i>

	\section devs_sec For developers..
	If you are a module developer please take a look to the following topics, we choose them for you because they are the most important when starting a new module.

	<ol>
		<li>First have a look to <a href="http://sourceforge.net/p/openchronos-ng/wiki/Module%20build%20system/">our wiki</a> to understand how to create a module (including its sources) and make it appear in the openchronos menu config. It is really simple and should not take much of your time.</li>

		<li>Then have a look to menu_add_entry(), this is what you should call during module initialization to make it appear in the system menu. It is also in this function where you specify the module functions that are to be called when the user presses the ez430 chronos buttons.</li>

		<li>Your module is now receiving input but what about output? Have a look to drivers/display.h, you can display strings using #display_chars() and turn ON/OFF symbols using #display_symbol(). </li>

		<li>Finally, if your module needs to execute some code periodically, then have a look to sys_messagebus_register() and #sys_message, on how to make your module listen for system events, like 1Hz events from the hardware timer.</li>
	</ol>
*/

/*!
	\file openchronos.h
	\brief Main openchronos-ng include file
*/

#ifndef __EZCHRONOS_H__
#define __EZCHRONOS_H__

#include <msp430.h>

#include <stdlib.h>

#include <stdint.h>

#include "config.h"

/*!
	\brief Adds an entry to the main menu.
	\details This function is to be used by modules, so that they can be visible in the main menu. A good place to call this function is from the corresponding module's _init function.
	\note This function is NULL safe. You can set all of its parameters to NULL (except name) if you don't need their functionality.
	\note The <i>name</i> string cannot be longer than 5 characters due to the LCD screen size.
*/
void menu_add_entry(
	char const * name,          /*!< item name to be displayed in the menu */
	void (*up_btn_fn)(void),    /*!< callback for up button presses. */
	void (*down_btn_fn)(void),  /*!< callback for down button presses. */
	void (*num_btn_fn)(void),   /*!< callback for num button presses. */
	void (*lstar_btn_fn)(void), /*!< callback for long star button presses. */
	void (*lnum_btn_fn)(void),  /*!< callback for long num button presses. */
	void (*updown_btn_fn)(void),/*!< callback for up&down button presses. */
	void (*activate_fn)(void),  /*!< callback for when the user switches into this entry in the menu. */
	void (*deactivate_fn)(void) /*!< callback for when the user switches out from this entry in the menu. */
);

/*!
	\brief A item structure for menu_editmode_start.
*/
struct menu_editmode_item {
	void (* select)(void);     /*!< item selected callback */
	void (* deselect)(void);   /*!< item deselected callback */
	void (* set)(int8_t step); /*!< set value of item callback */
};

/*!
	\brief Enters edit mode.
	\details The edit mode is a mechanism that allows the user to change values being displayed in the screen. For example, if a clock alarm is being displayed, then edit mode can be used to increase/decrease the values of hours and minutes. A good place to call this function is from the module's lstar_btn_fn function (see menu_add_entry()).<br />
	See modules/alarm.c for an example how to use this.
*/
void menu_editmode_start(
	/*! callback for when the user exits from the edit mode.*/
	void (* complete_fn)(void),
	/*! A vector of #menu_editmode_item, it must be NULL terminated! */
	struct menu_editmode_item *items
);

/*!
	\brief Handy prototype typedef for helpers_loop() function.
*/
typedef void(* helpers_loop_fn_t)(uint8_t *, uint8_t, uint8_t, int8_t);

/*!
	\brief Increment/decrements value by one without exiting the [lower, upper] interval.
	\details Increment/decrements value by one without exiting the [lower, upper] interval. If the value meets the upper bound, it is restarted from lower bound. If the value meets the lower bound, it is restarded from the upper bound.<br />
	Note: For now, only steps of -1 and 1 are considered.
	\sa menu_editmode_start
*/
void helpers_loop(
	uint8_t *value, /*!< value a pointer to the variable to be incremented. */
	uint8_t lower,  /*!< lower the lower bound for the loop interval. */
	uint8_t upper,  /*!< upper the upper bound for the loop interval. */
	int8_t step     /*!< 1 for incrementing value, -1 for a decrement */
);

#endif /* __EZCHRONOS_H__ */
