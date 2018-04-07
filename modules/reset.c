/**
    modules/reset.c: Reset module for openchronos-ng

    Copyright (C) 2012-2013 Angelo Arrifano <miknix@gmail.com>
    Copyright (C) 2016-2017 Benjamin Sølberg <benjamin.soelberg@gmail.com>

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

#include "messagebus.h"
#include "menu.h"

/* drivers */
#include "drivers/display.h"
#include "drivers/utils.h"
#include "drivers/ports.h"

static void num_press()
{
    /* reset microcontroller */
    REBOOT();
}

#ifdef CONFIG_MOD_RESET_EASY_RESET
static void button_event(enum sys_message event)
{
    // If up and down is pressed (at the EXACT same time) then resets the watch!
    // You might want to press them together a few times for this to work.
    if (ports_button_pressed_peek(PORTS_BTN_UP | PORTS_BTN_DOWN, 0))
    {
        num_press();
    }
}
#endif

static void reset_activate()
{
    /* update screen */
    display_chars(0, LCD_SEG_L2_4_0, "RESET", SEG_ON);
}

static void reset_deactivate()
{
    /* cleanup screen */
    display_clear(0, 2);
}

void mod_reset_init(void) {
#ifdef CONFIG_MOD_RESET_EASY_RESET
    sys_messagebus_register(&button_event, SYS_MSG_BUTTON);
#endif

    menu_add_entry("RESET",
                   NULL,
                   NULL,
                   &num_press,
                   NULL,
                   NULL,
                   NULL,
                   &reset_activate,
                   &reset_deactivate);
}
