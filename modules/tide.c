/**
    modules/tide.c: tide module for openchronos-ng

    Copyright (C) 2012 Thomas Post <openchronos-ng@post-net.ch>

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
#include "drivers/rtca.h"
#include "drivers/display.h"

struct Tide {
    uint8_t hoursLeft;  /* hours left to next low tide */
    uint8_t minutesLeft; /* minutes left to next low tide */
};

enum tide_display_state {
    TIDE_DISPLAY_STATE_GRAPH = 0,
    TIDE_DISPLAY_STATE_TOLOWCOUNTER,
    TIDE_DISPLAY_STATE_TOHIGHCOUNTER,
    /* editing mode */
    TIDE_DISPLAY_STATE_EDITING,
};

/* time to the next low tide */
static struct Tide tide;

static const uint16_t twentyFourHoursInMinutes = (uint32_t)1440;
static const uint16_t fullTideTime = (uint16_t)754;
static const uint16_t halfTideTime = (uint16_t)372; /* .5; */

/* state of the display */
static uint8_t moduleActivated;
static uint8_t editModeActivated;
static enum tide_display_state activeDisplay = TIDE_DISPLAY_STATE_GRAPH;

static const char *graphs[4] = {
    "_[^]_",
    "[^]_[",
    "^]_[^",
    "]_[^]"
};
static uint8_t graphOffset;

/* editing state */
static struct Tide enteredTimeOfNextLow;

/* MARK: Helper Functions */
uint16_t timeInMinutes(struct Tide aTide)
{
    uint16_t result = (uint16_t)aTide.minutesLeft;
    result += (uint16_t)aTide.hoursLeft * (uint16_t)60;
    return result;
}

uint16_t timeNowInMinutes(void)
{
    struct Tide timeNow;

    timeNow.hoursLeft = rtca_time.hour;
    timeNow.minutesLeft = rtca_time.min;
    if (rtca_time.sec > 30)
        timeNow.minutesLeft++;
    return timeInMinutes(timeNow);
}

struct Tide timeFromMinutes(uint16_t minutes)
{
    struct Tide newTide;

    newTide.hoursLeft = minutes/60;

    uint16_t hoursInMinutes = (uint16_t)newTide.hoursLeft*(uint32_t)60;
    minutes -= hoursInMinutes;
    newTide.minutesLeft = minutes;

    return newTide;
}

/* MARK: Drawing */

void blinkCol(uint8_t screen, uint8_t line)
{
    switch (line) {
    case 1:
        display_symbol(screen, LCD_SEG_L1_COL, SEG_ON);
        display_symbol(screen, LCD_SEG_L1_COL, BLINK_ON);
        break;
    case 2:
        display_symbol(screen, LCD_SEG_L2_COL0, SEG_ON);
        display_symbol(screen, LCD_SEG_L2_COL0, BLINK_ON);
        break;
    default:
        break;
    }
}

void drawScreen(void)
{
    /* do nothing if not visible */
    if (!moduleActivated || editModeActivated)
        return;

    display_clear(0, 0);
    display_clear(1, 0);
    display_clear(2, 0);

    uint16_t nowInMinutes = timeNowInMinutes();
    uint16_t leftUntilLow = timeInMinutes(tide);
    uint16_t leftUntilHigh = halfTideTime;

    if (leftUntilLow > leftUntilHigh)
        leftUntilHigh = leftUntilLow - leftUntilHigh;
    else
        leftUntilHigh = leftUntilLow + leftUntilHigh;

    struct Tide lowTide = tide;
    struct Tide highTide = timeFromMinutes(leftUntilHigh);

    /* screen 0 //graph + time till next peak */
    /* line1 time */
    if (leftUntilHigh < leftUntilLow) {
        /* show time till high */
        _printf(0, LCD_SEG_L1_3_2, "%02u", highTide.hoursLeft);
        _printf(0, LCD_SEG_L1_1_0, "%02u", highTide.minutesLeft);

        display_symbol(0, LCD_SYMB_MAX, SEG_ON);

    } else {
        /* show time till low */
        _printf(0, LCD_SEG_L1_3_2, "%02u", lowTide.hoursLeft);
        _printf(0, LCD_SEG_L1_1_0, "%02u", lowTide.minutesLeft);

        display_symbol(0, LCD_UNIT_L2_MI, SEG_ON);
    }
    blinkCol(0, 1);
    display_symbol(0, LCD_SEG_L2_COL1, SEG_ON);

    /* line 2 graph */
    display_chars(0, LCD_SEG_L2_4_0, graphs[graphOffset], SEG_SET);

    /** screen 1 **/
    /* line 1 time till low */
    _printf(1, LCD_SEG_L1_3_2, "%02u", lowTide.hoursLeft);
    _printf(1, LCD_SEG_L1_1_0, "%02u", lowTide.minutesLeft);

    display_symbol(1, LCD_UNIT_L2_MI, SEG_ON);

    /* line 2 calculate time of next high */
    uint16_t lowTideTimeInMinutes = (nowInMinutes + leftUntilLow)
                                                % twentyFourHoursInMinutes;
    struct Tide lowTideTime = timeFromMinutes(lowTideTimeInMinutes);

    _printf(1, LCD_SEG_L2_3_2, "%02u", lowTideTime.hoursLeft);
    _printf(1, LCD_SEG_L2_1_0, "%02u", lowTideTime.minutesLeft);

    blinkCol(1, 1);
    blinkCol(1, 2);


    /** screen 2 **/
    /* Line 1 time high */
    _printf(2, LCD_SEG_L1_3_2, "%02u", highTide.hoursLeft);
    _printf(2, LCD_SEG_L1_1_0, "%02u", highTide.minutesLeft);
    display_symbol(2, LCD_SYMB_MAX, SEG_ON);

    /* line 2 calculate time of next high */
    uint16_t highTideTimeInMinutes = (nowInMinutes + leftUntilHigh)
                                                % twentyFourHoursInMinutes;
    struct Tide highTideTime = timeFromMinutes(highTideTimeInMinutes);

    _printf(2, LCD_SEG_L2_3_2, "%02u", highTideTime.hoursLeft);
    _printf(2, LCD_SEG_L2_1_0, "%02u", highTideTime.minutesLeft);

    blinkCol(2, 1);
    blinkCol(2, 2);
}

/* MARK: System Bus Events */
void minuteTick()
{
    uint32_t tideInMinutes =  timeInMinutes(tide);
    graphOffset = (fullTideTime - tideInMinutes)/((uint16_t)186);
    graphOffset = graphOffset % 4;

    /* count down the timer */
    if (tide.minutesLeft == 0) {
        tide.minutesLeft = 60;
        if (tide.hoursLeft == 0) {
            /* TODO: change to full tide time */
            tide = timeFromMinutes(fullTideTime+1);
            return;
        }
        tide.hoursLeft--;
    }
    tide.minutesLeft--;

    /* draw screens */
    drawScreen();
}

/* MARK: - Time Setup */
void editHHSelect()
{
    display_chars(0, LCD_SEG_L1_3_2, NULL, BLINK_ON);
}

void editHHDeselect(void)
{
    display_chars(0, LCD_SEG_L1_3_2, NULL, BLINK_OFF);
}

void editHHSet(int8_t step)
{
    helpers_loop(&(enteredTimeOfNextLow.hoursLeft), 0, 23, step);
    _printf(0, LCD_SEG_L1_3_2, "%02u", enteredTimeOfNextLow.hoursLeft);
}

void editMMSelect(void)
{
    display_chars(0, LCD_SEG_L1_1_0, NULL, BLINK_ON);
}

void editMMDeselect(void)
{
    display_chars(0, LCD_SEG_L1_1_0, NULL, BLINK_OFF);
}

void editMMSet(int8_t step)
{
    helpers_loop(&(enteredTimeOfNextLow.minutesLeft), 0, 59, step);
    _printf(0, LCD_SEG_L1_1_0, "%02u", enteredTimeOfNextLow.minutesLeft);
}

static struct menu_editmode_item editModeItems[] = {
    {&editHHSelect, &editHHDeselect, &editHHSet},
    {&editMMSelect, &editMMDeselect, &editMMSet},
    { NULL }
};

void endEditing(void)
{
    /* calculate minutes left for next low */
    uint16_t nextLowMinutes = timeInMinutes(enteredTimeOfNextLow);
    uint16_t nowInMinutes = timeNowInMinutes();
    uint16_t diff = nextLowMinutes - nowInMinutes;
    if (diff < 0) {
        /* the next low is tomorrow! */
        diff = (twentyFourHoursInMinutes - nowInMinutes) + nextLowMinutes;
    }

    tide = timeFromMinutes(diff);
    editModeActivated = 0;
    display_clear(0, 0);
    drawScreen();
}

/* MARK:  - Buttons */
void longStarButton(void)
{
    /* clear screen */
    display_clear(0, 0);
    lcd_screen_activate(0);

    uint16_t nowInMinutes = timeNowInMinutes();
    uint16_t leftUntilLow = timeInMinutes(tide);
    enteredTimeOfNextLow = timeFromMinutes((nowInMinutes + leftUntilLow) % twentyFourHoursInMinutes);

    editModeActivated = 1;
    _printf(0, LCD_SEG_L1_3_2, "%02u", enteredTimeOfNextLow.hoursLeft);
    _printf(0, LCD_SEG_L1_1_0, "%02u", enteredTimeOfNextLow.minutesLeft);
    blinkCol(0, 1);
    menu_editmode_start(&endEditing, NULL, editModeItems);
}

void buttonUp(void)
{
    lcd_screen_activate(0xff);
    drawScreen();
}

void buttonDown(void)
{
    if (activeDisplay <= 0)
        activeDisplay = 2;
    else
        activeDisplay = (activeDisplay - 1) % 3;

    lcd_screen_activate(activeDisplay);
    drawScreen();
}

/* MARK: - Activate and Deactivate */
void activate(void)
{
    moduleActivated = 1;
    /* create three empty screens */
    lcd_screens_create(3);

    activeDisplay = TIDE_DISPLAY_STATE_GRAPH;
    lcd_screen_activate(activeDisplay);
    drawScreen();
}

void deactivate(void)
{
    moduleActivated = 0;
    /* destroy virtual screens */
    lcd_screens_destroy();

    display_clear(0, 0);
}

void mod_tide_init(void)
{
    sys_messagebus_register(&minuteTick, SYS_MSG_RTC_MINUTE);
    menu_add_entry("TIDE",
                   &buttonUp,
                   &buttonDown,
                   NULL,
                   &longStarButton,
                   NULL,
                   NULL,
                   &activate,
                   &deactivate);
    tide = timeFromMinutes(90); /* fullTideTime); */
    minuteTick(); /* initla display setup */
}
