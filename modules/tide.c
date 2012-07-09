/*
    modules/clock.c: clock module for openchronos-ng

    Copyright (C) 2012 Thomas Post <openchronos-ng@post-net.ch>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <openchronos.h>

/* driver */
#include <drivers/rtca.h>
#include <drivers/display.h>

struct Tide {
	uint8_t hoursLeft;	//hours left to next low tide
	uint8_t minutesLeft; //minutes left to next low tide
};

typedef enum {
	TideDisplayState_Graph = 0,
	TideDisplayState_ToLowCounter,
	TideDisplayState_ToHighCounter,
	//editing mode
	TideDisplayState_Editing,
} TideDisplayState;

//time to the next low tide
struct Tide tide;

const uint16_t twentyFourHoursInMinutes = (uint32_t)1440;
const uint16_t fullTideTime = (uint16_t)754;
const uint16_t halfTideTime = (uint16_t)372;//.5;

//state of the display
uint8_t moduleActivated = 0;
uint8_t editModeActivated = 0;
TideDisplayState activeDisplay = TideDisplayState_Graph;

const char* graphs[4] = {
  "_[^]_",
  "[^]_[",
  "^]_[^",
  "]_[^]"
};
uint8_t graphOffset = 0;

//editing state
struct Tide enteredTimeOfNextLow;
uint8_t fieldBeeingEdited = 0;


// MARK: Helper Functions
uint16_t timeInMinutes(struct Tide aTide) {
	uint16_t result = (uint16_t)aTide.minutesLeft;
	result += (uint16_t)aTide.hoursLeft * (uint16_t)60;
	return result;
}

uint16_t timeNowInMinutes() {
	struct Tide timeNow;
	uint8_t hour, min, sec;
	//get time from the system
	rtca_get_time(&hour, &min, &sec);
	
	timeNow.hoursLeft = hour;
	timeNow.minutesLeft = min;
	if(sec > 30) min++;
	return timeInMinutes(timeNow);
}

struct Tide timeFromMinutes(uint16_t minutes) {
	struct Tide newTide;
	
	newTide.hoursLeft = minutes/60;
	
	uint16_t hoursInMinutes = (uint16_t)newTide.hoursLeft*(uint32_t)60;
	minutes -= hoursInMinutes;
	newTide.minutesLeft = minutes;
	
	return newTide;
}

// MARK: Drawing

void blinkCol(uint8_t screen, uint8_t line) 
{
  switch(line) {
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

void drawScreen() 
{
  if(!moduleActivated || editModeActivated) return; //do nothing if not visible
  
  display_clear(0, 0);
  display_clear(1, 0);
  display_clear(2, 0);
	
  uint16_t nowInMinutes = timeNowInMinutes();
  uint16_t leftUntilLow = timeInMinutes(tide);
  uint16_t leftUntilHigh = halfTideTime;
  if (leftUntilLow > leftUntilHigh) {	
    leftUntilHigh = leftUntilLow - leftUntilHigh;
  } else {
    leftUntilHigh = leftUntilLow + leftUntilHigh;
  }
  struct Tide lowTide = tide;
  struct Tide highTide = timeFromMinutes(leftUntilHigh);
  
  //screen 0 //graph + time till next peak 
  //line1 time
  if(leftUntilHigh < leftUntilLow) {
    //show time till high
    display_chars(0, LCD_SEG_L1_3_2, _sprintf("%02u", highTide.hoursLeft), SEG_SET);
    display_chars(0, LCD_SEG_L1_1_0, _sprintf("%02u", highTide.minutesLeft), SEG_SET);
    display_symbol(0, LCD_SYMB_MAX, SEG_ON);
    
  } else {
    //show time till low
    display_chars(0, LCD_SEG_L1_3_2, _sprintf("%02u", lowTide.hoursLeft), SEG_SET);
    display_chars(0, LCD_SEG_L1_1_0, _sprintf("%02u", lowTide.minutesLeft), SEG_SET);
    display_symbol(0, LCD_UNIT_L2_MI, SEG_ON);
  }
  blinkCol(0,1);
  display_symbol(0, LCD_SEG_L2_COL1, SEG_ON);
  
  //line 2 graph
  display_chars(0, LCD_SEG_L2_4_0, graphs[graphOffset], SEG_SET);  
  
  //screen 1
  //line 1 time till low
  display_chars(1, LCD_SEG_L1_3_2, _sprintf("%02u", lowTide.hoursLeft), SEG_SET);
  display_chars(1, LCD_SEG_L1_1_0, _sprintf("%02u", lowTide.minutesLeft), SEG_SET);
  display_symbol(1, LCD_UNIT_L2_MI, SEG_ON);
  
  //line 2 calculate time of next high
  uint16_t lowTideTimeInMinutes = (nowInMinutes + leftUntilLow) % twentyFourHoursInMinutes;
  struct Tide lowTideTime = timeFromMinutes(lowTideTimeInMinutes);
  display_chars(1, LCD_SEG_L2_3_2, _sprintf("%02u", lowTideTime.hoursLeft), SEG_SET);
  display_chars(1, LCD_SEG_L2_1_0, _sprintf("%02u", lowTideTime.minutesLeft), SEG_SET);
  blinkCol(1,1);
  blinkCol(1,2);
    
    
  // screen 2  
  // Line 1 time high
  display_chars(2, LCD_SEG_L1_3_2, _sprintf("%02u", highTide.hoursLeft), SEG_SET);
  display_chars(2, LCD_SEG_L1_1_0, _sprintf("%02u", highTide.minutesLeft), SEG_SET);
  display_symbol(2, LCD_SYMB_MAX, SEG_ON);
  
  //line 2 calculate time of next high
  uint16_t highTideTimeInMinutes = (nowInMinutes + leftUntilHigh) % twentyFourHoursInMinutes;
  struct Tide highTideTime = timeFromMinutes(highTideTimeInMinutes);
  display_chars(2, LCD_SEG_L2_3_2, _sprintf("%02u", highTideTime.hoursLeft), SEG_SET);
  display_chars(2, LCD_SEG_L2_1_0, _sprintf("%02u", highTideTime.minutesLeft), SEG_SET);
  blinkCol(2,1);
  blinkCol(2,2);
}

// MARK: System Bus Events
void minuteTick()
{
  uint32_t tideInMinutes =  timeInMinutes(tide);
  graphOffset = (fullTideTime - tideInMinutes)/((uint16_t)186);
  graphOffset = graphOffset % 4;
  
  //count down the timer
  if(tide.minutesLeft == 0) {
    tide.minutesLeft = 60;
    if(tide.hoursLeft == 0) {
      //TODO: change to full tide time
      tide = timeFromMinutes(fullTideTime+1);
      return;
    }
    tide.hoursLeft--;
  }
  tide.minutesLeft--;
  
  //draw screens
  drawScreen();
}

// MARK: - Time Setup
void editHHSelect()
{
  display_chars(0, LCD_SEG_L1_3_2, NULL, BLINK_ON);
}

void editHHDeselect()
{
  display_chars(0, LCD_SEG_L1_3_2, NULL, BLINK_OFF);
}

void editHHSet(int8_t step) 
{
  helpers_loop(&(enteredTimeOfNextLow.hoursLeft), 0, 23, step);
  display_chars(0, LCD_SEG_L1_3_2, _sprintf("%02u", enteredTimeOfNextLow.hoursLeft), SEG_SET);
}

void editMMSelect()
{
  display_chars(0, LCD_SEG_L1_1_0, NULL, BLINK_ON);
}

void editMMDeselect()
{
  display_chars(0, LCD_SEG_L1_1_0, NULL, BLINK_OFF);
}

void editMMSet(int8_t step) 
{
  helpers_loop(&(enteredTimeOfNextLow.minutesLeft), 0, 59, step);
  display_chars(0, LCD_SEG_L1_1_0, _sprintf("%02u", enteredTimeOfNextLow.minutesLeft), SEG_SET);
}

static struct menu_editmode_item editModeItems[] = {
	{&editHHSelect, &editHHDeselect, &editHHSet},
	{&editMMSelect, &editMMDeselect, &editMMSet},
	{ NULL }
};

void endEditing()
{
  //calculate minutes left for next low
  uint16_t nextLowMinutes = timeInMinutes(enteredTimeOfNextLow);
  uint16_t nowInMinutes = timeNowInMinutes();
  uint16_t diff = nextLowMinutes - nowInMinutes;
  if(diff < 0) {
    //the next low is tomorrow!
    diff = (twentyFourHoursInMinutes - nowInMinutes) + nextLowMinutes;
  }
  
  tide = timeFromMinutes(diff);
  editModeActivated = 0;
	display_clear(0, 0);
	drawScreen();
}

// MARK:  - Buttons
void longStarButton() 
{
  //clear screen
  display_clear(0, 0);
	lcd_screen_activate(0);
	
  uint16_t nowInMinutes = timeNowInMinutes();
  uint16_t leftUntilLow = timeInMinutes(tide);
  enteredTimeOfNextLow = timeFromMinutes((nowInMinutes + leftUntilLow) % twentyFourHoursInMinutes);
  
	editModeActivated = 1;
  display_chars(0, LCD_SEG_L1_3_2, _sprintf("%02u", enteredTimeOfNextLow.hoursLeft), SEG_SET);
  display_chars(0, LCD_SEG_L1_1_0, _sprintf("%02u", enteredTimeOfNextLow.minutesLeft), SEG_SET);  
  blinkCol(0, 1);
  menu_editmode_start(&endEditing, editModeItems);
}

void buttonUp()
{
  lcd_screen_activate(0xff);
  drawScreen();
}

void buttonDown() 
{
  if(activeDisplay <= 0) {
    activeDisplay = 2;
  } else {
    activeDisplay = (activeDisplay - 1 ) % 3;
  }
  lcd_screen_activate(activeDisplay);
  drawScreen();
}

// MARK: - Activate and Deactivate
void activate()
{
  moduleActivated = 1;
  	/* create three empty screens */
	lcd_screens_create(3);
	
	activeDisplay = TideDisplayState_Graph;
	lcd_screen_activate(activeDisplay);
	drawScreen();
}

void deactivate()
{
  moduleActivated = 0;
  /* destroy virtual screens */
	lcd_screens_destroy();
		
	display_clear(0, 0);}

void tide_init()
{   
  sys_messagebus_register(&minuteTick, SYS_MSG_RTC_MINUTE);
  menu_add_entry("TIDE", &buttonUp, &buttonDown, NULL, &longStarButton, NULL, NULL, &activate, &deactivate);
  tide = timeFromMinutes(90);//fullTideTime);
  minuteTick(); //initla display setup
}