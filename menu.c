/**
    menu.c: openchronos-ng menu system

    Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>
    Copyright (C) 2016 Benjamin Sølberg <benjamin.soelberg@gmail.com>

    http://github.com/BenjaminSoelberg/openchronos-ng-elf

	This file is part of openchronos-ng.

    openchronos-ng is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    openchronos-ng is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

// *************************************************************************************************
// Initialization and control of application.
// *************************************************************************************************

// *************************************************************************************************
// Include section

#include "menu.h"

#include "drivers/ports.h"
#include "drivers/display.h"

#define BIT_IS_SET(F, B)  ((F) | (B)) == (F)

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

/***************************************************************************
 ************************ USER INPUT / MAIN MENU ***************************
 **************************************************************************/

static void editmode_handler(void)
{
    /* STAR button exits edit mode */
    if (ports_button_pressed(PORTS_BTN_STAR, 0)) {
        /* deselect item */
        menu_editmode.items[menu_editmode.pos].deselect();

        menu_editmode.complete_fn();
        menu_editmode.enabled = 0;

    } else if (ports_button_pressed(PORTS_BTN_NUM, 0)) {
        /* deselect current item */
        menu_editmode.items[menu_editmode.pos].deselect();

        /* select next item */
        menu_editmode.pos++;
        if (! menu_editmode.items[menu_editmode.pos].set)
            menu_editmode.pos = 0;
        menu_editmode.items[menu_editmode.pos].select();

    } else if (ports_button_pressed(PORTS_BTN_DOWN, 0)) {
        menu_editmode.items[menu_editmode.pos].set(1);

    } else if (ports_button_pressed(PORTS_BTN_UP, 0)) {
        menu_editmode.items[menu_editmode.pos].set(-1);
    }
}

static void menumode_handler(void)
{
    if (ports_button_pressed(PORTS_BTN_STAR, 0)) {
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

    } else if (ports_button_pressed(PORTS_BTN_DOWN, 0)) {
        menumode.item = menumode.item->next;
        display_clear(0, 2);
        display_chars(0, LCD_SEG_L2_4_0, menumode.item->name, SEG_SET);

    } else if (ports_button_pressed(PORTS_BTN_UP, 0)) {
        menumode.item = menumode.item->prev;
        display_clear(0, 2);
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

void menu_check_buttons(void)
{
    if (menu_editmode.enabled) {
        editmode_handler();

    } else if (menumode.enabled) {
        menumode_handler();

    } else {
        if (ports_button_pressed(PORTS_BTN_LSTAR, 1)) {
            if (menumode.item->lstar_btn_fn)
                menumode.item->lstar_btn_fn();

        } else if (ports_button_pressed(PORTS_BTN_STAR, !!(menumode.item->lstar_btn_fn))) {
            menumode_enable();

        } else if (ports_button_pressed(PORTS_BTN_LNUM, 1)) {
            if (menumode.item->lnum_btn_fn)
                menumode.item->lnum_btn_fn();

        } else if (ports_button_pressed(PORTS_BTN_NUM, !!(menumode.item->lnum_btn_fn))) {
            if (menumode.item->num_btn_fn)
                menumode.item->num_btn_fn();

        } else if (ports_button_pressed(PORTS_BTN_UP | PORTS_BTN_DOWN, 0)) {
            if (menumode.item->updown_btn_fn)
                menumode.item->updown_btn_fn();

        } else if (ports_button_pressed(PORTS_BTN_UP, 0)) {
            if (menumode.item->up_btn_fn)
                menumode.item->up_btn_fn();

        } else if (ports_button_pressed(PORTS_BTN_DOWN, 0)) {
            if (menumode.item->down_btn_fn)
                menumode.item->down_btn_fn();
        }
    }

    ports_buttons_clear();
}

struct menu* menu_add_entry(char const *name,
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

    return menu_p;
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
