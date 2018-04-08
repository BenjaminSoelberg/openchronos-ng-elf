/**
    temperature.c: temperature display module

    Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>
    Copyright (C) 2012 Matthew Excell <matt@excellclan.com>
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

#include "messagebus.h"
#include "menu.h"

/* drivers */
#include "drivers/display.h"
#include "drivers/temperature.h"

#ifdef CONFIG_TEMPERATURE_METRIC
uint8_t use_temperature_metric = 1;
#else
uint8_t use_temperature_metric = 0;
#endif

#define TEMP_UPDATE_INTERVAL_IN_SEC 4

static uint8_t sec;

static void display_temperature(void)
{
    int16_t temp;
    if (use_temperature_metric)
        temperature_get_C(&temp);
    else
        temperature_get_F(&temp);

    _printf(0, LCD_SEG_L1_3_1, "%2s", temp/10);
    display_char(0, LCD_SEG_L1_0, (temp%10)+48, SEG_SET);
}

static void event_1_sec_callback(enum sys_message msg)
{
    if (++sec >= TEMP_UPDATE_INTERVAL_IN_SEC) {
        temperature_measurement();
        display_temperature();
        sec = 0;
    }
}

/********************* edit mode callbacks ********************************/

static void display_temp_text_on_line_2()
{
    display_chars(0, LCD_SEG_L2_4_1, "TEMP", SEG_SET);
}

// Offset
void update_offset_display()
{
    _printf(0, LCD_SEG_L2_5_0, " C%03s", temperature.offset); // C for Calibration
}
static void edit_offset_sel(void)
{
    display_clear(0, 2);
    display_chars(0, LCD_SEG_L2_3_0, NULL, BLINK_ON);
    update_offset_display();
}
static void edit_offset_dsel(void)
{
    display_clear(0, 2);
    display_chars(0, LCD_SEG_L2_3_0, NULL, BLINK_OFF);
}
static void edit_offset_set(int8_t step)
{
    temperature.offset += step;
    update_offset_display();
    display_temperature();
}

// C or F
static void update_c_or_f_display()
{
    if (use_temperature_metric) {
        display_chars(0, LCD_SEG_L2_5_0, " USE\\C", SEG_SET);
    } else {
        display_chars(0, LCD_SEG_L2_5_0, " USE\\F", SEG_SET);
    }
}
static void edit_c_or_f_sel(void)
{
    display_clear(0, 2);
    display_chars(0, LCD_SEG_L2_1_0, NULL, BLINK_ON);
    update_c_or_f_display();
}
static void edit_c_or_f_dsel(void)
{
    display_clear(0, 2);
    display_chars(0, LCD_SEG_L2_1_0, NULL, BLINK_OFF);
}

static void edit_c_or_f_set(int8_t step)
{
    use_temperature_metric ^= 1;
    update_c_or_f_display();
    display_temperature();
}

/************************** menu callbacks ********************************/

static struct menu_editmode_item edit_items[] = {
    {&edit_offset_sel, &edit_offset_dsel, &edit_offset_set},
    {&edit_c_or_f_sel, &edit_c_or_f_dsel, &edit_c_or_f_set},
    { NULL },
};

static void temperature_activate(void)
{
    sec = TEMP_UPDATE_INTERVAL_IN_SEC; // Force update of temp delayed max 1 sec.
    /* display static elements */
    display_symbol(0, LCD_UNIT_L1_DEGREE, SEG_ON);
    display_symbol(0, LCD_SEG_L1_DP0, SEG_ON);

    /* display -- symbol while a measure is not performed */
    display_chars(0, LCD_SEG_L1_2_0, "---", SEG_ON);
    display_temp_text_on_line_2();
    sys_messagebus_register(&event_1_sec_callback, SYS_MSG_RTC_SECOND);
}

static void temperature_deactivate(void)
{
    sys_messagebus_unregister_all(&event_1_sec_callback);

    /* cleanup screen */
    display_symbol(0, LCD_UNIT_L1_DEGREE, SEG_OFF);
    display_symbol(0, LCD_SEG_L1_DP0, SEG_OFF);
}

static void temperature_edit(void)
{
    /* We go into edit mode  */
    menu_editmode_start(&display_temp_text_on_line_2, NULL, edit_items);
}

void mod_temperature_init(void)
{
    menu_add_entry("TEMP",
                   NULL,
                   NULL,
                   NULL,
                   &temperature_edit,
                   NULL,
                   NULL,
                   &temperature_activate,
                   &temperature_deactivate);
}
