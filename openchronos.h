/**
    openchronos.h: main header file

    Copyright (C) 2011-2012 Angelo Arrifano <miknix@gmail.com>
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
#include <stdbool.h>

#include "config.h"

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
