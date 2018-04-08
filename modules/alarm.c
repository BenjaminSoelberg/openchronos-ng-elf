/**
    modules/alarm.c: Alarm module for openchronos-ng

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

#include "messagebus.h"
#include "menu.h"

/* drivers */
#include "drivers/display.h"
#include "drivers/rtca.h"
#include "drivers/buzzer.h"
#include "drivers/ports.h"

static union {
    struct {
        /* one shot alarm */
        uint8_t alarm:1;
        /* hourly chime */
        uint8_t chime:1;
    };
    uint8_t state:2;
} alarm_state;

static uint8_t tmp_hh, tmp_mm;
static note chime_notes[2] = {0x1931, 0x000F};
static note alarm_notes[4] = {0x3234, 0x1900, 0x3234, 0x000F};

static void print_mm (void)
{
    _printf(0, LCD_SEG_L1_1_0, "%02u", tmp_mm);
}

static void print_hh(void)
{
    if (display_am_pm)
    {
        uint8_t hh = tmp_hh;
        if (hh > 12)
        { //PM
            hh -= 12;
            display_symbol (0, LCD_SYMB_PM, SEG_SET);
        }
        else
        {
            if (hh == 12)
            { // PM
                display_symbol (0, LCD_SYMB_PM, SEG_SET);
            }
            else
            { // AM
                display_symbol (0, LCD_SYMB_PM, SEG_OFF);
            }
            if (hh == 0)
                hh = 12;
        }
        _printf(0, LCD_SEG_L1_3_2, "%2u", hh);
    }
    else
    {
        _printf(0, LCD_SEG_L1_3_2, "%02u", tmp_hh);
        display_symbol (0, LCD_SYMB_PM, SEG_OFF);
    }
}

static void refresh_screen()
{
    rtca_get_alarm(&tmp_hh, &tmp_mm);
    print_hh();
    print_mm();
}

static uint8_t alarm_sec_elapsed = 0;
static void alarm_event(enum sys_message msg)
{
    if (msg & SYS_MSG_BUTTON || alarm_sec_elapsed >= 30) {
        alarm_sec_elapsed = 0;
        ports_buttons_clear();
        sys_messagebus_unregister(&alarm_event, SYS_MSG_BUTTON | SYS_MSG_RTC_SECOND);
        return;
    }

    if (msg & SYS_MSG_RTC_ALARM) {
        sys_messagebus_register(&alarm_event, SYS_MSG_BUTTON | SYS_MSG_RTC_SECOND);
    }

    alarm_sec_elapsed++;
    buzzer_play(alarm_notes);
}

static void hour_event(enum sys_message msg)
{
    if (msg & SYS_MSG_RTC_HOUR) {
        buzzer_play(chime_notes);
    }
}

/*************************** edit mode callbacks **************************/

/* Hour */
static void edit_hh_sel(void)
{
    display_chars(0, LCD_SEG_L1_3_2, NULL, BLINK_ON);
}

static void edit_hh_dsel(void)
{
    display_chars(0, LCD_SEG_L1_3_2, NULL, BLINK_OFF);
}

static void edit_hh_set(int8_t step)
{
    helpers_loop(&tmp_hh, 0, 23, step);
    print_hh();
}

/* Minute */
static void edit_mm_sel(void)
{
    display_chars(0, LCD_SEG_L1_1_0, NULL, BLINK_ON);
}

static void edit_mm_dsel(void)
{
    display_chars(0, LCD_SEG_L1_1_0, NULL, BLINK_OFF);
}

static void edit_mm_set(int8_t step)
{
    helpers_loop(&tmp_mm, 0, 59, step);
    print_mm();
}

/* Save */
static void edit_save(void)
{
    /* Here we return from the edit mode, fill in the new values! */
    rtca_set_alarm(tmp_hh, tmp_mm);
}

/* edit mode item table */
static struct menu_editmode_item edit_items[] = {
    {&edit_hh_sel, &edit_hh_dsel, &edit_hh_set},
    {&edit_mm_sel, &edit_mm_dsel, &edit_mm_set},
    { NULL },
};

/******************** menu callbacks **************************************/
static void alarm_activated()
{
    /* Force redraw of the screen */
    display_symbol(0, LCD_SEG_L1_COL, SEG_ON);
    refresh_screen();
}


static void alarm_deactivated()
{
    /* clean up screen */
    display_clear(0, 1);
}


/* NUM (#) button pressed callback */
static void num_pressed()
{
    /* this cycles between all alarm/chime combinations and overflow */
    alarm_state.state++;

    rtca_disable_alarm();
    /* Prevents double registration */
    sys_messagebus_unregister(&alarm_event, SYS_MSG_RTC_ALARM);
    /* Register RTC alarm event only if needed, saving CPU cycles.. */
    if (alarm_state.alarm) {
        display_symbol(0, LCD_ICON_ALARM, SEG_ON);
        sys_messagebus_register(&alarm_event, SYS_MSG_RTC_ALARM);
        rtca_enable_alarm();
    } else {
        display_symbol(0, LCD_ICON_ALARM, SEG_OFF);
    }

    /* Prevents double registration */
    sys_messagebus_unregister(&hour_event, SYS_MSG_RTC_HOUR);
    /* Register RTC hour event only if needed, saving CPU cycles.. */
    if (alarm_state.chime) {
        display_symbol(0, LCD_ICON_BEEPER2, SEG_ON);
        display_symbol(0, LCD_ICON_BEEPER3, SEG_ON);
        sys_messagebus_register(&hour_event, SYS_MSG_RTC_HOUR);
    } else {
        display_symbol(0, LCD_ICON_BEEPER2, SEG_OFF);
        display_symbol(0, LCD_ICON_BEEPER3, SEG_OFF);
    }
}


/* Star button long press callback. */
static void star_long_pressed()
{
    /* Save the current time in edit_buffer */
    rtca_get_alarm(&tmp_hh, &tmp_mm);

    menu_editmode_start(&edit_save, NULL, edit_items);
}


void mod_alarm_init()
{
    menu_add_entry ("ALARM",
                    NULL,
                    NULL,
                    &num_pressed,
                    &star_long_pressed,
                    NULL,
                    NULL,
                    &alarm_activated,
                    &alarm_deactivated);
}
