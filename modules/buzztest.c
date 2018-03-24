/**
    modules/buzztest.c: Buzzer test module for openchronos-ng

    Copyright (C) 2017 Benjamin Sølberg <benjamin.soelberg@gmail.com>

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

#include <drivers/buzzer.h>
#include "menu.h"

/* drivers */
#include "drivers/display.h"

static note n[2] = {0x3200, 0x000F};
int8_t oct = 0;
int8_t key = 1;

static void activate() {
    /* update screen */
    display_chars(0, LCD_SEG_L2_4_1, "BUZZ", SEG_ON);
}

static void deactivate() {
    /* cleanup screen */
    display_clear(0, 2);
}

static void update() {
    _printf(0, LCD_SEG_L1_3_2, "%02u", oct);
    _printf(0, LCD_SEG_L1_1_0, "%02u", key);

    n[0] = 0x3200 + (oct << 4) + key;
    buzzer_play(n);

}

static void up() {
    key++;
    if (key > 12) {
        key = 1;

        oct++;
        if (oct > 3)
            oct = 0;
    }

    update();
}

static void down() {
    key--;
    if (key <= 0) {
        key = 12;
        oct--;

        if (oct < 0)
            oct = 3;
    }

    update();
}

void mod_buzztest_init(void) {
    menu_add_entry("BUZZ",
                   &up,
                   &down,
                   NULL,
                   NULL,
                   NULL,
                   NULL,
                   &activate,
                   &deactivate);
}
