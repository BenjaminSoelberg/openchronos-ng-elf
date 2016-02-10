/*!
	\file menu.h
	\brief Menu include file
*/

#ifndef __MENU_H__
#define __MENUH__

#include <msp430.h>

#include <stdlib.h>

#include <stdint.h>

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

void menu_check_buttons(void);

void menu_add_entry(char const * name,
          void (*up_btn_fn)(void),
		    void (*down_btn_fn)(void),
		    void (*num_btn_fn)(void),
		    void (*lstar_btn_fn)(void),
			 void (*lnum_btn_fn)(void),
			 void (*updown_btn_fn)(void),
		    void (*activate_fn)(void),
		    void (*deactivate_fn)(void));

void menu_editmode_start(void (* complete_fn)(void),
                         struct menu_editmode_item *items);

#endif /* __MESSAGEBUS_H__ */
